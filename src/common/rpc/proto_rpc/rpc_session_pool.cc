// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)

#include <utility>
#include "common/base/stl_container_deleter.h"
#include "common/rpc/proto_rpc/http_service.h"
#include "common/rpc/proto_rpc/rpc_controller.h"
#include "common/rpc/proto_rpc/rpc_option.pb.h"
#include "common/rpc/proto_rpc/rpc_meta.pb.h"
#include "common/rpc/proto_rpc/rpc_request_queue.h"
#include "common/rpc/proto_rpc/rpc_session_pool.h"
#include "common/system/concurrency/atomic/atomic.hpp"
#include "common/system/concurrency/rwlock.hpp"
#include "common/system/concurrency/thread.hpp"
#include "common/system/time/timestamp.hpp"
#include "common/system/timer/timer_manager.hpp"
// includes from thirdparty
#include "glog/logging.h"
#include "protobuf/descriptor.h"

namespace rpc {

namespace {

// The size of the rpc packet header, which used to indicate the size of the
// whole rpc packet. Currently it's to uint32_t to indiciate the size of rpc
// meta and rpc message respectively.
static const int kRpcHeaderSize = 2 * sizeof(uint32_t);

static inline void HostToBufferUInt32(const uint32_t value, char* data) {
    data[0] = static_cast<uint8_t>(value >> 24);
    data[1] = static_cast<uint8_t>(value >> 16);
    data[2] = static_cast<uint8_t>(value >> 8);
    data[3] = static_cast<uint8_t>(value);
}

// Return current time in milliseconds from the epoch.
// Define an inline function as we have several version of implementation.
static inline int64_t GetCurrentTimeInMilliSeconds() {
    return GetTimeStampInMs();
}

// Call this function when receiving response for a pending request, or the
// pending request is being canceled.
static void ResponseReceivedCallback(RpcRequest* rpc_request,
                                     const RpcMeta* rpc_meta,
                                     const StringPiece* message_data) {
    // Called from netframe thread when receiving response packet.
    rpc_request->controller->set_status(rpc_meta->error_code());
    if (rpc_meta->failed()) {
        rpc_request->controller->SetFailed(rpc_meta->reason());
        if (rpc_meta->canceled()) {
            rpc_request->controller->set_canceled(true);
            rpc_request->controller->set_status(RpcController::Status_Canceled);
        }
    } else {
        if (!rpc_request->response->ParseFromArray(message_data->data(),
                                                   message_data->size())) {
            rpc_request->controller->SetFailed("Failed to parse the response message.");
            rpc_request->controller->set_status(RpcController::Status_SerializationError);
        }
    }
    rpc_request->done->Run();
    delete rpc_request;
}

} // namespace

// class RpcSessionPool
RpcSessionPool::RpcSessionPool()
    : m_closing(false),
      m_connection_rwlock(new RWLock()),
      m_last_connection_id(0),
      m_last_sequence_id(0),
      m_pending_requests(new RpcRequestQueue()),
      m_timer_manager(new TimerManager()) {
    Closure<void, uint64_t>* timer_callback =
        NewPermanentClosure(this, &RpcSessionPool::TimerCallback);
    // The period is 10ms.
    m_timer_manager->AddPeriodTimer(10, timer_callback);
}

RpcSessionPool::~RpcSessionPool() {
    CHECK(m_closing);
}

void RpcSessionPool::SetClosing() {
    // The user must guarantee no newly added http connection here.
    {
        RWLock::WriterLocker locker(*m_connection_rwlock);
        m_closing = true;
        for (ConnectionMap::const_iterator i = m_rpc_sessions.begin();
             i != m_rpc_sessions.end();
             ++i) {
            i->second->CloseConnection(false);
        }
        m_timer_manager->Clear();
    }
}

void RpcSessionPool::OnNewSession(HttpConnection* http_connection) {
    CHECK_EQ(-1, http_connection->connection_id())
        << "The connection id must have NOT been set at server side before "
        << "connected.";
    {
        // The connection is valid as a rpc communication channel.
        RWLock::WriterLocker locker(*m_connection_rwlock);
        http_connection->set_connection_id(m_last_connection_id++);
        CHECK(m_rpc_sessions.insert(
            std::make_pair(http_connection->connection_id(),
                           http_connection)).second)
            << "Duplicated connection id: " << http_connection->connection_id();
    }
}

void RpcSessionPool::OnClose(int64_t connection_id, int error_code) {
    CHECK_GE(connection_id, 0)
        << "The connection id must have been set at server side before closed.";
    {
        // The connection is a valid rpc session.
        RWLock::WriterLocker locker(*m_connection_rwlock);
        ConnectionMap::iterator it = m_rpc_sessions.find(connection_id);
        CHECK(it != m_rpc_sessions.end())
            << "Connection id doesn't exist: " << connection_id;
        m_rpc_sessions.erase(it);
    }

    // Cancel all pending requests for this connection.
    m_pending_requests->ReleaseConnection(connection_id, true);
}

void RpcSessionPool::SendRequest(
    const google::protobuf::MethodDescriptor* method,
    RpcController* controller,
    const google::protobuf::Message* request,
    google::protobuf::Message* response,
    google::protobuf::Closure* done) {
    CHECK_GE(controller->connection_id(), 0)
        << "The connection id of controller must be set at server side.";
    // Send a request, set it's status to pending.
    controller->set_status(RpcController::Status_Pending);
    {
        RWLock::ReaderLocker locker(*m_connection_rwlock);
        ConnectionMap::const_iterator it =
            m_rpc_sessions.find(controller->connection_id());
        if (it != m_rpc_sessions.end()) {
            RpcRequest* rpc_request =
                CreateRequest(method, controller, request, response, done);
            // AddRequest always succeeds.
            m_pending_requests->AddRequest(rpc_request);
            SendRequest(it->second, rpc_request);
            return;
        }
    }

    controller->SetFailed(
        "No session found for the connection id, request discarded.");
    controller->set_status(RpcController::Status_NoSessionFound);
    done->Run();
}

void RpcSessionPool::CancelRequest(int64_t sequence_id) {
    m_pending_requests->CancelRequest(sequence_id);
}

void RpcSessionPool::HandleResponse(const RpcMeta& rpc_meta,
                                    const StringPiece& message_data) {
    RpcRequest* rpc_request =
        m_pending_requests->RemoveRequest(rpc_meta.sequence_id());
    if (rpc_request == NULL) {
        // Possibly the session has been closed, just discard the received
        // response.
        return;
    }

    ResponseReceivedCallback(rpc_request, &rpc_meta, &message_data);
}

void RpcSessionPool::SendResponse(const RpcController* controller,
                                  const google::protobuf::Message* response) {
    CHECK_GE(controller->connection_id(), 0)
        << "The connection id of controller must be set at server side.";
    {
        RWLock::ReaderLocker locker(*m_connection_rwlock);
        ConnectionMap::const_iterator it =
            m_rpc_sessions.find(controller->connection_id());
        if (it != m_rpc_sessions.end()) {
            SendResponse(it->second, controller, response);
        } else {
            // The session has been closed, just don't send the response.
        }
    }
}

void RpcSessionPool::SendRequest(HttpConnection* http_connection,
                                 RpcRequest* rpc_request) {
    RpcMeta rpc_meta;
    rpc_meta.set_type(RpcMeta::REQUEST);
    rpc_meta.set_sequence_id(rpc_request->controller->sequence_id());
    rpc_meta.set_method(rpc_request->method->full_name());

    std::string message_data;
    rpc_request->request->AppendToString(&message_data);

    SendMessage(http_connection, rpc_meta, message_data);
}

void RpcSessionPool::SendResponse(HttpConnection* http_connection,
                                  const RpcController* controller,
                                  const google::protobuf::Message* response) {
    RpcMeta rpc_meta;
    rpc_meta.set_type(RpcMeta::RESPONSE);
    rpc_meta.set_error_code(controller->status());
    rpc_meta.set_sequence_id(controller->sequence_id());
    if (controller->Failed()) {
        rpc_meta.set_failed(true);
        rpc_meta.set_reason(controller->ErrorText());
        if (controller->IsCanceled()) {
            rpc_meta.set_canceled(true);
        }
    }

    std::string message_data;
    if (!controller->Failed()) {
        // Don't bother sending response data if failed.
        response->AppendToString(&message_data);
    }

    SendMessage(http_connection, rpc_meta, message_data);
}

void RpcSessionPool::SendMessage(HttpConnection* http_connection,
                                 const RpcMeta& rpc_meta,
                                 const StringPiece& message_data) {
    // The buffer is initialized with 8 bytes.
    std::string buffer;
    buffer.resize(kRpcHeaderSize);
    // Append content of rpc meta to the buffer.
    rpc_meta.AppendToString(&buffer);
    // Encode the size of rpc meta.
    HostToBufferUInt32(buffer.size() - kRpcHeaderSize, &buffer[0]);
    // Encode the size of rpc message.
    HostToBufferUInt32(message_data.size(), &buffer[sizeof(uint32_t)]);
    // Append content of message to the buffer.
    // We have to send all data for one message in one packet to ensure they are
    // sent and received as a whole.
    buffer.append(message_data.data(), message_data.size());

    // Send the header and rpc meta data.
    if (!http_connection->SendPacket(buffer)) {
        // Failed to send packet, close the connection.
        http_connection->CloseConnection(false);
        return;
    }
}

RpcRequest* RpcSessionPool::CreateRequest(
    const google::protobuf::MethodDescriptor* method,
    RpcController* controller,
    const google::protobuf::Message* request,
    google::protobuf::Message* response,
    google::protobuf::Closure* done) {
    // Allocate a unique sequence id by increasing a counter monotonously.
    int64_t sequence_id =
        AtomicExchangeAdd(m_last_sequence_id, (int64_t)1LL);

    // Calculate the expected timeout.
    int64_t timeout_in_milliseconds = 0;
    if (method->options().HasExtension(rpc_method_timeout)) {
        timeout_in_milliseconds = method->options().GetExtension(rpc_method_timeout);
    } else {
        timeout_in_milliseconds =
            method->service()->options().GetExtension(rpc_service_timeout);
    }
    if (timeout_in_milliseconds <= 0) {
        // Just a protection, it shouldn't happen.
        timeout_in_milliseconds = 1;
    }

    int64_t expected_timeout = GetCurrentTimeInMilliSeconds() +
                               timeout_in_milliseconds;

    RpcRequest* rpc_request = new RpcRequest(sequence_id,
                                             expected_timeout,
                                             method,
                                             controller,
                                             request,
                                             response,
                                             done);
    return rpc_request;
}

void RpcSessionPool::TimerCallback(uint64_t timer_id) {
    m_pending_requests->CheckTimeoutRequest(GetCurrentTimeInMilliSeconds());
}

// class RpcInitiativeSessionPool
RpcInitiativeSessionPool::RpcInitiativeSessionPool(
    netframe::NetFrame* net_frame, HttpServiceHandler* rpc_service)
    : m_net_frame(net_frame),
      m_rpc_service(rpc_service),
      m_round_robin_connection_id(-1) {
}

RpcInitiativeSessionPool::~RpcInitiativeSessionPool() {
    STLDeleteElements(&m_server_addresses);
}

void RpcInitiativeSessionPool::ConnectServers(
    const std::vector<std::string>& server_addresses) {
    CHECK(m_server_addresses.empty())
        << "Can only call StartClient once!";
    CHECK(!server_addresses.empty())
        << "Server addresses list can not be empty!";

    m_server_addresses.resize(server_addresses.size());
    for (size_t i = 0; i < server_addresses.size(); ++i) {
        m_server_addresses[i] = new SocketAddressInet(server_addresses[i]);
        ConnectServer(i, 0);  // timer id is not used, just set it to 0.
    }
}

void RpcInitiativeSessionPool::SetClosing() {
    {
        RWLock::WriterLocker locker(*m_connection_rwlock);
        m_rpc_service->mutable_reference_counter()->SetClosing(
            false);  // don't forcely release all references.
        // No new client side http connection after here.
        for (ConnectionMap::const_iterator i = m_pending_connections.begin();
             i != m_pending_connections.end();
             ++i) {
            i->second->CloseConnection(false);
        }
    }
    RpcSessionPool::SetClosing();
}

void RpcInitiativeSessionPool::OnNewSession(HttpConnection* http_connection) {
    CHECK_GE(http_connection->connection_id(), 0)
        << "The connection id must have been set at client side before "
        << "connected.";
    {
        // The connection is valid as a rpc communication channel.
        RWLock::WriterLocker locker(*m_connection_rwlock);

        // Remove from pending connections firstly.
        ConnectionMap::iterator it =
            m_pending_connections.find(http_connection->connection_id());
        CHECK(it != m_pending_connections.end())
            << "Connection id doesn't exist: "
            << http_connection->connection_id();
        m_pending_connections.erase(it);

        // Add to valid sessions.
        CHECK(m_rpc_sessions.insert(
            std::make_pair(http_connection->connection_id(),
                           http_connection)).second)
            << "Duplicated connection id: " << http_connection->connection_id();
    }

    RedispatchRequests();
}

void RpcInitiativeSessionPool::OnClose(int64_t connection_id, int error_code) {
    CHECK_GE(connection_id, 0)
        << "The connection id must have been set at client side before closed.";
    {
        // The connection is NOT necessarily a valid rpc session, possibly a
        // pending connection.
        RWLock::WriterLocker locker(*m_connection_rwlock);
        ConnectionMap::iterator it = m_rpc_sessions.find(connection_id);
        if (it != m_rpc_sessions.end()) {
            // A valid rpc session.
            m_rpc_sessions.erase(it);
        } else {
            // Not a valid rpc session, must be a pending connection.
            it = m_pending_connections.find(connection_id);
            CHECK(it != m_pending_connections.end())
                << "Connection id doesn't exist: " << connection_id;
            m_pending_connections.erase(it);
        }

        if (!m_closing) {
            m_timer_manager->AddOneshotTimer(
                5 * 1000,  // wait for 5 seconds before reconnecting.
                NewClosure(this,
                           &RpcInitiativeSessionPool::ConnectServer,
                           connection_id));
        }
    }

    // Don't cancel the pending requests and just set them as "undispatched".
    m_pending_requests->ReleaseConnection(connection_id, false);

    RedispatchRequests();
}

void RpcInitiativeSessionPool::SendRequest(
    const google::protobuf::MethodDescriptor* method,
    RpcController* controller,
    const google::protobuf::Message* request,
    google::protobuf::Message* response,
    google::protobuf::Closure* done) {
    CHECK_EQ(-1, controller->connection_id())
        << "The connection id of controller must NOT be set at client side.";
    RpcRequest* rpc_request =
        CreateRequest(method, controller, request, response, done);
    // Send a request, set it's status to pending.
    controller->set_status(RpcController::Status_Pending);
    {
        RWLock::ReaderLocker locker(*m_connection_rwlock);
        if (!m_rpc_sessions.empty()) {
            int64 connection_id = AtomicGet(m_round_robin_connection_id);
            ConnectionMap::const_iterator it =
                m_rpc_sessions.upper_bound(connection_id);

            if (it == m_rpc_sessions.end()) {
                it = m_rpc_sessions.begin();
            }
            AtomicSet(m_round_robin_connection_id, it->first);
            rpc_request->controller->set_connection_id(it->first);
            // AddRequest always succeeds.
            m_pending_requests->AddRequest(rpc_request);
            RpcSessionPool::SendRequest(it->second, rpc_request);
            return;
        }
        // No available session now, add as an undispatched request.
        m_pending_requests->AddRequest(rpc_request);
    }
}

void RpcInitiativeSessionPool::RedispatchRequests() {
    {
        RWLock::ReaderLocker locker(*m_connection_rwlock);
        if (m_rpc_sessions.empty()) {
            return;
        }

        int64 connection_id = AtomicGet(m_round_robin_connection_id);
        ConnectionMap::const_iterator it =
            m_rpc_sessions.upper_bound(connection_id);

        while (true) {
            if (it == m_rpc_sessions.end()) {
                it = m_rpc_sessions.begin();
            }

            RpcRequest* rpc_request =
                m_pending_requests->RedispatchRequest(it->first);
            if (rpc_request == NULL) {
                break;
            }

            RpcSessionPool::SendRequest(it->second, rpc_request);
            ++it;
        }
        AtomicSet(m_round_robin_connection_id, it->first);
    }
}

void RpcInitiativeSessionPool::ConnectServer(
    const int64_t connection_id, uint64_t timer_id) {
    {
        RWLock::WriterLocker locker(*m_connection_rwlock);
        if (!m_rpc_service->mutable_reference_counter()->AcquireReference()) {
            // The rpc service has been set for closing.
            return;
        }

        HttpConnection* http_connection =
            new HttpConnection(m_net_frame, m_rpc_service);
        http_connection->set_connection_id(connection_id);
        CHECK(m_pending_connections.insert(
            std::make_pair(http_connection->connection_id(),
                           http_connection)).second)
            << "Duplicated connection id: " << http_connection->connection_id();

        // Netframe takes the ownership of http connection and delete it when
        // the connection is closed.
        size_t index = static_cast<size_t>(connection_id);
        // TODO(hansye): AsyncConnect might fail, handle it for retry. Possibly
        // it needs a timer as no other way to trigger the retry event.
        CHECK_GT(m_net_frame->AsyncConnect(*(m_server_addresses[index]),
                                           http_connection,
                                           HttpServer::kMaxPacketSize),
                 0)
            << "Can't connect to address: "
            << m_server_addresses[index]->ToString();
    }
}

} // namespace rpc
