// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)
//
// Simple http service built on netframe, both for server side and client side.

#ifndef RPC_PROTO_RPC_HTTP_SERVICE_H_
#define RPC_PROTO_RPC_HTTP_SERVICE_H_

#include <map>
#include <string>
#include "common/base/closure.h"
#include "common/base/scoped_ptr.h"
#include "common/base/string/string_piece.hpp"
#include "common/netframe/netframe.hpp"
#include "common/netframe/socket_handler.hpp"

class ConditionVariable;
class SimpleMutex;
class RWLock;

namespace rpc {

class HttpConnection;
class HttpMessage;
class HttpRequest;
class HttpResponse;
class HttpServer;
class HttpServiceHandler;

// A reference counter which provides following functionalities:
// 1. A thread-safe reference counter.
// 2. The "half-closed" state to stop accepting more reference acquisition.
// 3. A block operation to wait all references are released.
// 4. A registered closure to be called back when all references are released.
//    The user can delete the reference counter in the callback.
//
// It has 3 internal states: "opened", "closing", "closed".
// "opened" means free to accept reference acquisition.
// "closing" means stop accepting more reference acquisition but possibly there
// all acquired references.
// "closed" means all references are released.
//
// TODO(hansye): move it to a common library.
class BlockingReferenceCounter {
public:
    explicit BlockingReferenceCounter(Closure<void>* complete_callback = NULL);
    ~BlockingReferenceCounter();

    // Set the reference counter for closing. It just set an internal flag to
    // prevent user from acquiring more reference.
    // If "force_closed" is true, it would forcely release all references.
    void SetClosing(bool force_closed);

    // Set the reference counter for closing and also wait for it to be closed,
    // i.e. all reference are released.
    // It's a blocking operation.
    //
    // SetClosing() and WaitForClosed() could be called for multiple times, and
    // the destructor would call WaitForClosed() so the user doesn't have to
    // call it if not necessary.
    void WaitForClosed();

    // If the reference counter is NOT set for closing or closed, increase the
    // reference count and return true, or return false.
    bool AcquireReference();

    // Decrease the reference count.
    void ReleaseReference();

private:
    enum State {
        STATE_OPENED,
        STATE_CLOSING,
        STATE_CLOSED,
    };

    scoped_ptr<SimpleMutex> m_reference_mutex;
    scoped_ptr<ConditionVariable> m_reference_cond;
    Closure<void>* m_complete_callback;
    State m_state;
    int m_reference_count;
};

// Similar to the BlockingReferenceCounter, except:
// 1. It's lock-free.
// 2. No blocking wait operation.
// 3. It's a weak model, it could guarantee the registered closure would only be
//    called once, but can't guarantee AcquireReference() must fail after
//    calling SetClosing().
//
// TODO(hansye): switch to use this for "m_pending_packets" of HttpConnection.
// TODO(hansye): move it to a common library.
class CallbackReferenceCounter {
public:
    explicit CallbackReferenceCounter(Closure<void>* complete_callback);
    // The destructor would check the counter has been closed.
    ~CallbackReferenceCounter();

    // Set the reference counter for closing. It just set an internal flag to
    // prevent user from acquiring more reference.
    // If "force_closed" is true, it would forcely release all references.
    //
    // SetClosing() must be called at least once before the counter is
    // destructed.
    void SetClosing(bool force_closed);

    // If the reference counter is NOT set for closing or closed, increase the
    // reference count and return true, or return false.
    bool AcquireReference();

    // Decrease the reference count.
    void ReleaseReference();

private:
    // It has 3 internal states: "opened", "closing", "closed".
    // "opened" means free to accept reference acquisition.
    // "closing" means stop accepting more reference acquisition but possibly
    // there all acquired references.
    // "closed" means all references are released.
    enum State {
        STATE_OPENED,
        STATE_CLOSING,
        STATE_CLOSED,
    };

    Closure<void>* m_complete_callback;
    State m_state;
    int m_reference_count;
};

// Describes how to complete a http service. It's for both server side and
// client side, and all http connections of the same http service share the same
// service handler.
// TODO(hansye): provide a defaul http service handler for high level
// application.
class HttpServiceHandler {
public:
    // Constructor at server side. The "http server" can NOT be NULL.
    explicit HttpServiceHandler(HttpServer* http_server);
    // Constructor at client side.
    HttpServiceHandler();
    virtual ~HttpServiceHandler();

    // Return true if the service is for server side, false if for client side.
    bool IsServerSide() const { return m_http_server != NULL; }

    BlockingReferenceCounter* mutable_reference_counter() {
        return m_reference_counter.get();
    }

    // The interface the derived class needs to implement.
    // Note: OnConnected is only called at client side.
    virtual void OnConnected(HttpConnection* http_connection) = 0;
    virtual void OnClose(HttpConnection* http_connection,
                         int error_code) = 0;
    virtual void HandleHeaders(HttpConnection* http_connection) = 0;
    virtual int DetectBodySize(HttpConnection* http_connection,
                                  const StringPiece& data) = 0;
    virtual void HandleBody(HttpConnection* http_connection,
                            const StringPiece& data) = 0;

protected:
    // It doesn't own the http server.
    HttpServer* m_http_server;

    // The states about reference count and if the handler is being closed, used
    // for safely quitting.
    scoped_ptr<BlockingReferenceCounter> m_reference_counter;
};

// Represents a http connection.
class HttpConnection : public netframe::StreamSocketHandler {
public:
    // Constructor at server side.
    explicit HttpConnection(HttpServer* http_server);
    // Constructor at client side.
    // The user must have acquired a reference for the http handler.
    HttpConnection(netframe::NetFrame* net_frame,
                   HttpServiceHandler* http_handler);
    virtual ~HttpConnection();

    const HttpRequest& http_request() const { return *m_http_request; }
    HttpRequest* mutable_http_request() { return m_http_request.get(); }

    const HttpResponse& http_response() const { return *m_http_response; }
    HttpResponse* mutable_http_response() { return m_http_response.get(); }

    int64_t connection_id() const { return m_connection_id; }
    void set_connection_id(const int64_t connection_id_value) {
        m_connection_id = connection_id_value;
    }

    bool headers_received() const { return m_headers_received; }

    // Return true if the connnect is for server side, false if for client side.
    bool IsServerSide() const { return m_http_server != NULL; }

    // Following functions are possbily called by netframe thread and user's
    // thread, be careful about the reentrance.

    // Close the connection. If send_pending_packets is set true, we would close
    // the connection after sending all pending packets, or we would immediately
    // close the connection.
    void CloseConnection(bool send_pending_packets);

    bool SendPacket(const StringPiece& data);

private:
    void CloseEndPoint();

    // Following functions are only called by netframe thread and so are NOT
    // reentrant.

    // Implements StreamSocketHandler interface.
    // Note: OnConnected is only called at client side.
    virtual void OnConnected() {
        if (m_http_handler != NULL) {
            m_http_handler->OnConnected(this);
        }
    }
    virtual void OnClose(int error_code) {
        if (m_http_handler != NULL) {
            m_http_handler->OnClose(this, error_code);
        }
    }
    // Actually it detects the size of http headers, including the request line.
    virtual int DetectPacketSize(const void* data, size_t size);
    virtual void OnReceived(const netframe::Packet& packet);
    // It decides if to close this connection.
    virtual bool OnSent(netframe::Packet* packet);

    void HandleInvalidMessage(const StringPiece& data);
    void HandleUnregisteredPath();

    HttpServer* m_http_server;
    HttpServiceHandler* m_http_handler;
    scoped_ptr<HttpRequest> m_http_request;
    scoped_ptr<HttpResponse> m_http_response;

    // Set and used by user, to identify different http connection.
    int64_t m_connection_id;

    // If the connection has received the headers, including the start line.
    bool m_headers_received;

    // If the connection is being closed. We would really close the connection
    // after sending all pending packets.
    scoped_ptr<CallbackReferenceCounter> m_pending_packets;
};

// A simple http server built on netframe, which dispatches http request to
// registered http handler according to the path.
class HttpServer {
public:
    static const int kMaxPacketSize = 32 * 1024 * 1024; // 32M.

    // Pass in a net frame from external. If "own_net_frame" is true, the http
    // server would own the net frame and delete it in destructor.
    explicit HttpServer(netframe::NetFrame* net_frame,
                        bool own_net_frame = false);
    // The http server create and own a net frame internally.
    // If "server_threads" is 0, it means the number of logic cpus.
    explicit HttpServer(int server_threads = 0);
    virtual ~HttpServer();

    // The start routine at server side, it starts to listen on this address.
    bool StartServer(const std::string& server_address);

    netframe::NetFrame* mutable_net_frame() const { return m_net_frame; }

    // Register/unregister a handler on a given path. The http server does NOT
    // take the ownership of handler.
    // Return true if successful.
    bool RegisterHandler(const std::string& path,
                         HttpServiceHandler* handler);
    bool UnregisterHandler(const std::string& path);

    // Find the http handler for the given path and acquire a reference.
    // Return NULL if no handler for the given path, or the handler has been set
    // for closing.
    // Note: as this function has acquire a reference of handler, it's user's
    // responsibility to release the reference.
    HttpServiceHandler* GetHandler(const std::string& path) const;

private:
    bool m_own_net_frame;
    netframe::NetFrame* m_net_frame;

    scoped_ptr<RWLock> m_handler_rwlock;
    std::map<std::string, HttpServiceHandler*> m_handlers;
};

} // namespace rpc

#endif // RPC_PROTO_RPC_HTTP_SERVICE_H_
