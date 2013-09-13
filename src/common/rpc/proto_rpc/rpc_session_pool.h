// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)
//
// The rpc session pool, both for server and client side.

#ifndef RPC_PROTO_RPC_RPC_SESSION_POOL_H_
#define RPC_PROTO_RPC_RPC_SESSION_POOL_H_

#include <map>
#include <string>
#include <vector>
#include "common/base/scoped_ptr.h"
#include "common/base/stdint.h"
#include "common/base/string/string_piece.hpp"
// includes from thirdparty
#include "protobuf/service.h"

class RWLock;
class SocketAddressInet;
class Thread;
class TimerManager;

namespace netframe {
class NetFrame;
} // namespace netframe

namespace rpc {

class HttpConnection;
class HttpServiceHandler;

class RpcController;
class RpcMeta;
struct RpcRequest;
class RpcRequestQueue;

// A collection of rpc session. It's a "passive" session pool and only used at
// server side, which means, once a session closes, we do NOT try to reconnect
// it and all pending requests/responses on this session are discarded.
// It holds a lock to protect internal table and all interface functions are
// reentrant.
class RpcSessionPool {
public:
    RpcSessionPool();
    virtual ~RpcSessionPool();

    // Set the session pool as closing. Providing such a function instead of
    // doing in destructor as we need to wait for all http connections are
    // really closed (OnClose is called) before delete the session pool.
    virtual void SetClosing();

    // Called when a new http connection has been verified as a valid rpc
    // session.
    virtual void OnNewSession(HttpConnection* http_connection);
    // Called when a http connection is closed.
    virtual void OnClose(int64_t connection_id, int error_code);

    // The "request" side methods.
    // Try to send request on specified session. If failed (specified session
    // doesn't exist), the closure "done" would be called immediately.
    virtual void SendRequest(const google::protobuf::MethodDescriptor* method,
                             RpcController* controller,
                             const google::protobuf::Message* request,
                             google::protobuf::Message* response,
                             google::protobuf::Closure* done);

    // Cancel an rpc request.
    virtual void CancelRequest(int64_t sequence_id);

    virtual void HandleResponse(const RpcMeta& rpc_meta,
                                const StringPiece& message_data);

    // The "response" side methods.
    virtual void SendResponse(const RpcController* controller,
                              const google::protobuf::Message* response);

protected:
    typedef std::map<int64_t, HttpConnection*> ConnectionMap;

    // The "request" side methods.
    static void SendRequest(HttpConnection* http_connection,
                            RpcRequest* rpc_request);

    // The "response" side methods.
    static void SendResponse(HttpConnection* http_connection,
                             const RpcController* controller,
                             const google::protobuf::Message* response);

    // The methods for both "response" and "request" sides.
    // This function is possbily called by netframe thread and user's thread, be
    // careful about the reentrance.
    static void SendMessage(HttpConnection* http_connection,
                            const RpcMeta& rpc_meta,
                            const StringPiece& message_data);

    RpcRequest* CreateRequest(const google::protobuf::MethodDescriptor* method,
                              RpcController* controller,
                              const google::protobuf::Message* request,
                              google::protobuf::Message* response,
                              google::protobuf::Closure* done);

    void TimerCallback(uint64_t timer_id);

    bool m_closing;

    // The rwlock to protect state of connections.
    scoped_ptr<RWLock> m_connection_rwlock;
    // Allocate for each incoming connection, only used at server side
    // currently.
    int64_t m_last_connection_id;
    // Allocate for each request. Doing this in session pool instead of session
    // is to help re-schedule request among sessions.
    int64_t m_last_sequence_id;
    // Hold all pending requests, dispatched or undispatched.
    scoped_ptr<RpcRequestQueue> m_pending_requests;
    // Now a rpc session is simply a http connection. So the session pool does
    // NOT own the rpc sessions, the netframe does.
    ConnectionMap m_rpc_sessions;

    // The timer thread, used for check timeout of rpc requests.
    scoped_ptr<TimerManager> m_timer_manager;
};

// An "Initiative" session pool and used at client side. The major difference is
// it maintains a list of server, which are identical and stateless. Once a
// session closes, all responses on this session are discarded, but all pending
// requests would be kept and sent again later. It also has a scheduling method
// for load balance among sessions.
// It holds a lock to protect internal table and all interface functions are
// reentrant.
class RpcInitiativeSessionPool : public RpcSessionPool {
public:
    explicit RpcInitiativeSessionPool(netframe::NetFrame* net_frame,
                                      HttpServiceHandler* rpc_service = NULL);
    virtual ~RpcInitiativeSessionPool();

    void set_rpc_service(HttpServiceHandler* rpc_service) {
        m_rpc_service = rpc_service;
    }

    // This is only called before client start.
    void ConnectServers(const std::vector<std::string>& server_addresses);

    virtual void SetClosing();

private:
    // Called when a new http connection has been verified as a valid rpc
    // session.
    virtual void OnNewSession(HttpConnection* http_connection);
    // Called when a http connection is closed.
    virtual void OnClose(int64_t connection_id, int error_code);

    // The "request" side methods.
    virtual void SendRequest(const google::protobuf::MethodDescriptor* method,
                             RpcController* controller,
                             const google::protobuf::Message* request,
                             google::protobuf::Message* response,
                             google::protobuf::Closure* done);

    // Try to redispatch the undispatched requests to available servers.
    void RedispatchRequests();

    // Establish a http connection by calling a async connect.
    // The real process routine is RpcService::OnConnected.
    void ConnectServer(int64_t connection_id, uint64_t timer_id);

    // It doesn't own netframe and rpc service.
    netframe::NetFrame* m_net_frame;
    HttpServiceHandler* m_rpc_service;
    std::vector<SocketAddressInet*> m_server_addresses;

    int64_t m_round_robin_connection_id;

    // The http connections which have been established but haven't been
    // verified as valid rpc sessions.
    // The session pool does NOT own the connections, the netframe does.
    ConnectionMap m_pending_connections;
};

} // namespace rpc

#endif // RPC_PROTO_RPC_RPC_SESSION_POOL_H_
