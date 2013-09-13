// Copyright 2011, Tencent Inc.
// Author: Xiaokang Liu (hsiaokangliu@tencent.com)

#ifndef COMMON_NET_HTTP_HTTP_SERVER_H
#define COMMON_NET_HTTP_HTTP_SERVER_H

#include <string>
#include "common/base/uncopyable.hpp"
#include "common/netframe/netframe.hpp"
#include "common/net/http/http_handler_manager.h"
#include "common/net/http/http_connection_manager.h"

namespace poppy {

class HttpBase {
public:
    static const int kMaxMessageSize = 32 * 1024 * 1024; // 32M.

    explicit HttpBase(netframe::NetFrame* net_frame,
                        bool own_net_frame = false);
    explicit HttpBase(int threads = 0);
    virtual ~HttpBase();

    netframe::NetFrame* mutable_net_frame() const {
        return m_net_frame;
    }

    ConnectionManager* mutable_connection_manager() {
        return &m_connection_manager;
    }

protected:
    bool m_own_net_frame;
    netframe::NetFrame* m_net_frame;
    ConnectionManager m_connection_manager;

    DECLARE_UNCOPYABLE(HttpBase);
};

class HttpServer : public HttpBase {
public:
    // Pass in a net frame from external. If "own_net_frame" is true, the http
    // server would own the net frame and delete it in destructor.
    explicit HttpServer(netframe::NetFrame* net_frame,
                        bool own_net_frame = false) :
        HttpBase(net_frame, own_net_frame),
        m_listener_manager(net_frame) {
    }
    // The http server create and own a net frame internally.
    // If "server_threads" is 0, it means the number of logic cpus.
    explicit HttpServer(int server_threads = 0) :
        HttpBase(server_threads),
        m_listener_manager(m_net_frame) {
    }
    virtual ~HttpServer();

    // Register a handler on a specified path.
    // Return false if another handler already has been registered.
    // If a handler has been registered successfully, it will be taken over
    // by the handler manager and CANNOT be unregistered.
    void RegisterHandler(const std::string& path, HttpHandler* handler);

    // Find a handler registered on a specified path. If no handler is
    // registered, NULL will be returned.
    virtual HttpHandler* FindHandler(const std::string& path);

    // Start server to listen on this address.
    bool Start(const std::string& server_address);
    void Stop();

    ConnectionManager* mutable_listener_manager() {
        return &m_listener_manager;
    }

protected:
    ConnectionManager  m_listener_manager;
    HttpHandlerManager m_handler_manager;

    DECLARE_UNCOPYABLE(HttpServer);
};

} // namespace poppy

#endif // COMMON_NET_HTTP_HTTP_SERVER_H
