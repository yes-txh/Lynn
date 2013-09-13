// Copyright 2011, Tencent Inc.
// Author: Xiaokang Liu (hsiaokangliu@tencent.com)

#include "common/net/http/http_server.h"
#include "common/net/http/http_handler.h"
#include "common/net/http/http_connection.h"
#include "common/netframe/socket_handler.hpp"
#include "glog/logging.h"

namespace poppy {

HttpBase::HttpBase(netframe::NetFrame* net_frame, bool own_net_frame)
    : m_own_net_frame(own_net_frame),
      m_net_frame(net_frame),
      m_connection_manager(m_net_frame) {
}

HttpBase::HttpBase(int threads_number)
    : m_own_net_frame(true),
    m_net_frame(new netframe::NetFrame(threads_number)),
    m_connection_manager(m_net_frame) {
}

HttpBase::~HttpBase() {
    if (m_own_net_frame) {
        delete m_net_frame;
    }
}

class HttpServerListener : public netframe::ListenSocketHandler {
public:
    HttpServerListener(HttpServer* http_server)
        : netframe::ListenSocketHandler(*(http_server->mutable_net_frame())),
          m_http_server(http_server) {
    }
    virtual ~HttpServerListener() {}

private:
    // Implements ListenSocketHandler interface.
    virtual void OnClose(int error_code) {
        m_http_server->mutable_listener_manager()->RemoveConnection(
                GetEndPoint().GetId());
    }

    virtual netframe::StreamSocketHandler* OnAccepted(netframe::SocketId id) {
        // netframe would take the ownership of the allocated http connection.
        HttpServerConnection* connection =
            new HttpServerConnection(m_http_server);
        m_http_server->mutable_connection_manager()->AddConnection(id.GetId());
        return connection;
    }

    HttpServer* m_http_server;
};

void HttpServer::RegisterHandler(const std::string& path, HttpHandler* handler) {
    m_handler_manager.RegisterHandler(path, handler);
}

HttpHandler* HttpServer::FindHandler(const std::string& path) {
    return m_handler_manager.FindHandler(path);
}

bool HttpServer::Start(const std::string& server_address) {
    SocketAddressInet server_address_inet(server_address);
    HttpServerListener* listener =
        new HttpServerListener(this);
    int64_t listen_socket_id = m_net_frame->AsyncListen(
            server_address_inet, listener, kMaxMessageSize);
    if (listen_socket_id < 0) {
        LOG(ERROR) << "Can't listen on address: " << server_address;
        delete listener;
        return false;
    }
    m_listener_manager.AddConnection(listen_socket_id);
    return true;
}

void HttpServer::Stop() {
    m_listener_manager.CloseAllConnections();
    m_connection_manager.CloseAllConnections();
}

HttpServer::~HttpServer() {
    Stop();
}

} // namespace poppy
