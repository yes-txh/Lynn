// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)
//
// The rpc service implementation based on netframe http service. It supports
// dual rpc, i.e. the server could also call back the service on client side.
//
// TODO(hansye): support "dual" rpc service. Currently the underlying framework
// supports "dual" rpc service, but doesn't have a programming model now.

#ifndef RPC_PROTO_RPC_RPC_SERVICE_H_
#define RPC_PROTO_RPC_RPC_SERVICE_H_

#include <list>
#include <map>
#include <string>
#include <vector>
#include "common/base/closure.h"
#include "common/base/scoped_ptr.h"
#include "common/base/stdint.h"
#include "common/rpc/proto_rpc/http_service.h"
#include "common/rpc/proto_rpc/rpc_controller.h"
// includes from thirdparty
#include "protobuf/service.h"

class SyncEvent;

namespace rpc {

class RpcMeta;
class RpcSessionPool;

// The "dual" rpc service, i.e. both client and server could initiate a rpc
// request.
// Its main task is to deserialize rpc packets (rpc meta and request/response)
// from http stream and dispatch to corresponding service.
class RpcService : public HttpServiceHandler {
public:
    // Constructor at server side.
    // It would create a passive rpc session pool internally.
    explicit RpcService(HttpServer* http_server);
    // Constructor at client side.
    // It takes the ownership of the session pool.
    RpcService(RpcSessionPool* session_pool);
    virtual ~RpcService();

    // Register a protocol buffer rpc service here. One RpcService could
    // serve multiple rpc services at the same time.
    // This is only called before server or client start, so no lock to protect
    // the internal table.
    void RegisterService(google::protobuf::Service* service);

    RpcSessionPool* mutable_session_pool() const {
        return m_session_pool.get();
    }

    // The "request" side methods.
    // This is the start point to initiate a rpc request, and it's most likely
    // called by user's thread.
    void SendRequest(const google::protobuf::MethodDescriptor* method,
                     RpcController* controller,
                     const google::protobuf::Message* request,
                     google::protobuf::Message* response,
                     google::protobuf::Closure* done);

    void CancelRequest(int64_t sequence_id);

private:
    // Implements HttpServerHandler interface.
    // Following interface functions are only called by netframe thread and so
    // are NOT reentrant.
    // Note: OnConnected is only called at client side.
    virtual void OnConnected(HttpConnection* http_connection);
    virtual void OnClose(HttpConnection* http_connection, int error_code);
    virtual void HandleHeaders(HttpConnection* http_connection);
    virtual int DetectBodySize(HttpConnection* http_connection,
                                  const StringPiece& data);
    virtual void HandleBody(HttpConnection* http_connection,
                            const StringPiece& data);

    // Following functions don't comply with coding style strictly as controller
    // is an in/out parameter. Doing this to meet style of protobuf service
    // interface.

    // The "response" side methods.
    // Following two functions are only called by netframe thread and so are NOT
    // reentrant.
    void HandleRequest(HttpConnection* http_connection,
                       const RpcMeta& rpc_meta,
                       const StringPiece& message_data);

    bool DispatchRequest(RpcController* controller,
                         const RpcMeta& rpc_meta,
                         const StringPiece& message_data);

    // Following two functions are possbily called by netframe thread and user's
    // thread (depending on if the method implementation starts another thread
    // to process the request), be careful about the reentrance.
    void RequestCompleteCallback(RpcController* controller,
                                 google::protobuf::Message* request,
                                 google::protobuf::Message* response);

    std::map<std::string, google::protobuf::Service*> m_services;

    scoped_ptr<RpcSessionPool> m_session_pool;
};

// The server side rpc channel used for dual rpc.
// This is a "passive" rpc channel as it can't initiate a connection to client
// side.
class RpcServerChannel : public google::protobuf::RpcChannel {
public:
    // Constructor at server side. The "rpc_service" can NOT be NULL.
    explicit RpcServerChannel(RpcService* rpc_service);
    // Constructor at client side.
    RpcServerChannel();
    virtual ~RpcServerChannel();

    static RpcController* CreateDualRpcController(
        const google::protobuf::RpcController* controller) {
        RpcController* rpc_controller = new RpcController();
        rpc_controller->set_connection_id(
            reinterpret_cast<const RpcController*>(
                controller)->connection_id());
        return rpc_controller;
    }

protected:
    // Implements the RpcChannel interface.
    // If the done is NULL, it's a synchronous call, or it's an asynchronous and
    // uses the callback inform the completion (or failure).
    // It's only called by user's thread.
    virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done);

    void SynchronousCallback(SyncEvent* completion_event);

    // It doesn't own rpc service.
    RpcService* m_rpc_service;
};

// The client side rpc channel.
class RpcChannel : public RpcServerChannel {
public:
    // Pass in a net frame from external. If "own_net_frame" is true, the rpc
    // channel would own the net frame and delete it in destructor.
    explicit RpcChannel(netframe::NetFrame* net_frame,
                        bool own_net_frame = false);
    // The rpc channel create and own a net frame internally.
    // If "client_threads" is 0, it means the number of logic cpus.
    explicit RpcChannel(int client_threads = 0);
    virtual ~RpcChannel();

    // Register a protocol buffer rpc service here, called when we want to use
    // the "dual" rpc.
    void RegisterService(google::protobuf::Service* service) {
        m_rpc_service->RegisterService(service);
    }

    // The start routine at client side, it connnects to the server. We don't do
    // it at constructor as we want to give user a chance to register service
    // before connecting.
    bool StartClient(const std::string& server_address);
    // It connects to multiple server for load balance. All the servers must be
    // identical and stateless, as no way to guarantee two requests are sent to
    // the same server.
    bool StartClient(const std::vector<std::string>& server_addresses);

private:
    bool m_own_net_frame;
    netframe::NetFrame* m_net_frame;

    // It DOES own rpc service.
    // RpcService* m_rpc_service;
};

} // namespace rpc

#endif // RPC_PROTO_RPC_RPC_SERVICE_H_
