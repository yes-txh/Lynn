// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)

#include "common/rpc/proto_rpc/http_message.h"
#include "common/rpc/proto_rpc/rpc_meta.pb.h"
#include "common/rpc/proto_rpc/rpc_option.pb.h"
#include "common/rpc/proto_rpc/rpc_service.h"
#include "common/rpc/proto_rpc/rpc_session_pool.h"
#include "common/system/concurrency/atomic/atomic.hpp"
// The implementation of mutex and rwlock under windows would include
// <windows.h>, which defines many macros and introduces conflicts, like DELETE.
// TODO(phongchen): Avoid including <windows.h> in header files.
#include "common/system/concurrency/sync_event.hpp"
// includes from thirdparty
#include "glog/logging.h"
#include "protobuf/descriptor.h"

namespace rpc {

namespace {

// The size of the rpc packet header, which used to indicate the size of the
// whole rpc packet. Currently it's to uint32_t to indiciate the size of rpc
// meta and rpc message respectively.
static const size_t kRpcHeaderSize = 2 * sizeof(uint32_t);
// The http path registered for rpc service.
static const char kRpcHttpPath[] = "/__rpc_service__";

static inline uint32_t BufferToHostUInt32(const char* data) {
    return (static_cast<uint8_t>(data[0]) << 24) |
           (static_cast<uint8_t>(data[1]) << 16) |
           (static_cast<uint8_t>(data[2]) << 8) |
           (static_cast<uint8_t>(data[3]));
}

// Parse the full name of method into service full name and method name.
// Return false if the method name is invalid.
static inline bool ParseMethodFullName(const std::string& method_full_name,
                                       std::string* service_full_name,
                                       std::string* method_name) {
    std::string::size_type pos = method_full_name.rfind('.');
    if (pos == std::string::npos) {
        // The method full name must have one period at least to separate
        // service name and method name.
        return false;
    }

    *service_full_name = method_full_name.substr(0, pos);
    *method_name = method_full_name.substr(pos + 1);
    return true;
}

} // namespace

// class RpcService
RpcService::RpcService(HttpServer* http_server)
    : HttpServiceHandler(http_server),
      m_session_pool(new RpcSessionPool()) {
    CHECK(IsServerSide());
    CHECK(m_http_server->RegisterHandler(kRpcHttpPath, this))
        << "Failed to register handler for rpc service: " << kRpcHttpPath;
}

RpcService::RpcService(RpcSessionPool* session_pool)
    : HttpServiceHandler(),
      m_session_pool(session_pool) {
    CHECK(!IsServerSide());
}

RpcService::~RpcService() {
    if (IsServerSide()) {
        CHECK(m_http_server->UnregisterHandler(kRpcHttpPath))
            << "Failed to unregister handler for rpc service: " << kRpcHttpPath;
        // No new server side http connection after here.
    }

    m_session_pool->SetClosing();
    // No new http connection (both server and client side) after here.

    m_reference_counter->WaitForClosed();
    // All pending connections are really closed after here.
}

void RpcService::RegisterService(google::protobuf::Service* service) {
    const std::string& service_name = service->GetDescriptor()->full_name();
    CHECK(m_services.insert(make_pair(service_name, service)).second)
        << "Duplicated service on name: " << service_name;
}

void RpcService::SendRequest(
    const google::protobuf::MethodDescriptor* method,
    RpcController* controller,
    const google::protobuf::Message* request,
    google::protobuf::Message* response,
    google::protobuf::Closure* done) {
    m_session_pool->SendRequest(method, controller, request, response, done);
}

void RpcService::CancelRequest(int64_t sequence_id) {
    m_session_pool->CancelRequest(sequence_id);
}

void RpcService::OnConnected(HttpConnection* http_connection) {
    CHECK(!IsServerSide()) << "OnConnected is only called at client side.";
    // Send http request at client side.
    if (!http_connection->SendPacket(
        "POST /__rpc_service__ HTTP/1.1\r\n\r\n")) {
        // Failed to send packet, close the connection.
        http_connection->CloseConnection(false);
    }
}

void RpcService::OnClose(HttpConnection* http_connection,
                         int error_code) {
    m_session_pool->OnClose(http_connection->connection_id(), error_code);
}

void RpcService::HandleHeaders(HttpConnection* http_connection) {
    if (IsServerSide()) {
        const HttpRequest& http_request = http_connection->http_request();
        if (http_request.method() != HttpRequest::METHOD_POST) {
            // Only expect post method.
            http_connection->SendPacket("HTTP/1.1 400 Bad Request\r\n\r\n");
            http_connection->CloseConnection(true);
            return;
        }
        if (!http_connection->SendPacket("HTTP/1.1 200 OK\r\n\r\n")) {
            // Failed to send packet, close the connection.
            http_connection->CloseConnection(false);
            return;
        }
    } else {
        const HttpResponse& http_response = http_connection->http_response();
        if (http_response.status() != 200) {
            http_connection->CloseConnection(false);
            return;
        }
    }

    m_session_pool->OnNewSession(http_connection);
}

int RpcService::DetectBodySize(HttpConnection* http_connection,
                                  const StringPiece& data) {
    if (data.size() < kRpcHeaderSize) {
        return 0;
    }

    const char* buffer = data.data();
    uint32_t meta_length = BufferToHostUInt32(buffer);
    buffer += sizeof(uint32_t);
    uint32_t message_length = BufferToHostUInt32(buffer);
    buffer += sizeof(uint32_t);

    return kRpcHeaderSize + meta_length + message_length;
}

void RpcService::HandleBody(HttpConnection* http_connection,
                            const StringPiece& data) {
    CHECK_GE(data.size(), kRpcHeaderSize);

    const char* buffer = data.data();
    uint32_t meta_length = BufferToHostUInt32(buffer);
    buffer += sizeof(uint32_t);
    uint32_t message_length = BufferToHostUInt32(buffer);
    buffer += sizeof(uint32_t);

    CHECK_EQ(data.size(), kRpcHeaderSize + meta_length + message_length);

    RpcMeta rpc_meta;
    if (!rpc_meta.ParseFromArray(buffer, meta_length)) {
        // Meta is invalid, unexpected, close the connection.
        LOG(WARNING) << "Unable to parse the rpc meta.";
        http_connection->CloseConnection(false);
    }
    buffer += meta_length;

    if (rpc_meta.sequence_id() < 0) {
        // Sequence id is invalid, unexpected, close the connection.
        LOG(WARNING) << "Invalid rpc sequence id:" << rpc_meta.sequence_id();
        http_connection->CloseConnection(false);
        return;
    }

    if (rpc_meta.type() != RpcMeta::REQUEST &&
        rpc_meta.type() != RpcMeta::RESPONSE) {
        // Rpc type is invalid, unexpected, close the connection.
        LOG(WARNING) << "Invalid rpc type:" << rpc_meta.type();
        http_connection->CloseConnection(false);
        return;
    }

    StringPiece message_data(buffer, message_length);
    if (rpc_meta.type() == RpcMeta::REQUEST) {
        HandleRequest(http_connection, rpc_meta, message_data);
    } else if (rpc_meta.type() == RpcMeta::RESPONSE) {
        m_session_pool->HandleResponse(rpc_meta, message_data);
    }
}

void RpcService::HandleRequest(HttpConnection* http_connection,
                               const RpcMeta& rpc_meta,
                               const StringPiece& message_data) {
    RpcController* controller = new RpcController();
    // Defaultly set to success
    controller->set_status(RpcController::Status_Success);
    controller->set_connection_id(http_connection->connection_id());
    controller->set_sequence_id(rpc_meta.sequence_id());
    if (!DispatchRequest(controller, rpc_meta, message_data)) {
        // Failed to dispatch rpc request, send an error message to client and
        // release the controller.
        CHECK(controller->Failed());
        m_session_pool->SendResponse(controller, NULL);
        delete controller;
    } else {
        // The controller would be released in the callback.
    }
}

bool RpcService::DispatchRequest(RpcController* controller,
                                 const RpcMeta& rpc_meta,
                                 const StringPiece& message_data) {
    if (!rpc_meta.has_method()) {
        controller->SetFailed("Method name not specified.");
        controller->set_status(RpcController::Status_NoMethodName);
        return false;
    }

    const std::string& method_full_name = rpc_meta.method();
    std::string service_name;
    std::string method_name;
    if (!ParseMethodFullName(method_full_name, &service_name, &method_name)) {
        controller->SetFailed("Method name is invalid.");
        controller->set_status(RpcController::Status_InvalidMethodName);
        return false;
    }

    std::map<std::string, google::protobuf::Service*>::const_iterator it =
        m_services.find(service_name);
    if (it == m_services.end()) {
        controller->SetFailed("Service not found: " + service_name);
        controller->set_status(RpcController::Status_ServiceNotFound);
        return false;
    }

    google::protobuf::Service* service = it->second;
    const google::protobuf::MethodDescriptor* method =
        service->GetDescriptor()->FindMethodByName(method_name);
    if (method == NULL) {
        controller->SetFailed("Method not found: " + method_name);
        controller->set_status(RpcController::Status_MethodNotFound);
        return false;
    }

    google::protobuf::Message* request =
        service->GetRequestPrototype(method).New();
    if (!request->ParseFromArray(message_data.data(), message_data.size())) {
        delete request;
        controller->SetFailed("Failed to parse the request message.");
        controller->set_status(RpcController::Status_SerializationError);
        return false;
    }

    google::protobuf::Message* response =
        service->GetResponsePrototype(method).New();
    google::protobuf::Closure* callback =
        NewClosure(this,
                   &RpcService::RequestCompleteCallback,
                   controller,
                   request,
                   response);
    service->CallMethod(method, controller, request, response, callback);
    return true;
}

void RpcService::RequestCompleteCallback(RpcController* controller,
                                         google::protobuf::Message* request,
                                         google::protobuf::Message* response) {
    m_session_pool->SendResponse(controller, response);

    delete controller;
    delete request;
    delete response;
}

// class RpcServerChannel
RpcServerChannel::RpcServerChannel(RpcService* rpc_service)
    : m_rpc_service(rpc_service) {
    CHECK_NOTNULL(m_rpc_service);
}

RpcServerChannel::RpcServerChannel() : m_rpc_service(NULL) {}

RpcServerChannel::~RpcServerChannel() {}

void RpcServerChannel::CallMethod(
    const google::protobuf::MethodDescriptor* method,
    google::protobuf::RpcController* controller,
    const google::protobuf::Message* request,
    google::protobuf::Message* response,
    google::protobuf::Closure* done) {
    RpcController* rpc_controller =
        reinterpret_cast<RpcController*>(controller);

    bool synchronous = false;
    scoped_ptr<SyncEvent> completion_event;
    if (done == NULL) {
        synchronous = true;
        completion_event.reset(new SyncEvent());
        done = NewClosure(this,
                          &RpcServerChannel::SynchronousCallback,
                          completion_event.get());
    }
    rpc_controller->SetHandler(m_rpc_service);
    m_rpc_service->SendRequest(
        method, rpc_controller, request, response, done);

    if (synchronous) {
        completion_event->Wait();
    }
}

void RpcServerChannel::SynchronousCallback(SyncEvent* completion_event) {
    completion_event->Set();
}

// class RpcChannel
RpcChannel::RpcChannel(netframe::NetFrame* net_frame, bool own_net_frame)
    : RpcServerChannel(),
      m_own_net_frame(own_net_frame),
      m_net_frame(net_frame) {
    RpcInitiativeSessionPool* session_pool =
        new RpcInitiativeSessionPool(m_net_frame);
    m_rpc_service = new RpcService(session_pool);
    session_pool->set_rpc_service(m_rpc_service);
}

RpcChannel::RpcChannel(int client_threads)
    : RpcServerChannel(),
      m_own_net_frame(true) {
    m_net_frame = new netframe::NetFrame(client_threads);
}

RpcChannel::~RpcChannel() {
    if (m_own_net_frame) {
        delete m_net_frame;
    }
    delete m_rpc_service;
}

bool RpcChannel::StartClient(const std::string& server_address) {
    std::vector<std::string> server_addresses;
    server_addresses.push_back(server_address);
    return StartClient(server_addresses);
}

bool RpcChannel::StartClient(const std::vector<std::string>& server_addresses) {
    CHECK(!server_addresses.empty())
        << "Server addresses list can not be empty!";

    RpcInitiativeSessionPool* session_pool =
        reinterpret_cast<RpcInitiativeSessionPool*>(
            m_rpc_service->mutable_session_pool());
    session_pool->ConnectServers(server_addresses);
    return true;
}

} // namespace rpc
