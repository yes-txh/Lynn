// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)
// Xiaokang Liu (hsiaokangliu@tencent.com)

#ifndef COMMON_NET_HTTP_HTTP_CONNECTION_H
#define COMMON_NET_HTTP_HTTP_CONNECTION_H

#include "common/base/string/string_piece.hpp"
#include "common/netframe/netframe.hpp"
#include "common/netframe/socket_handler.hpp"
#include "common/net/http/http_message.h"

namespace poppy {

class HttpServer;
class HttpClient;
class HttpHandler;

// Represents a http connection.
class HttpConnection : public netframe::StreamSocketHandler {
public:
    HttpConnection(netframe::NetFrame* net_frame);
    virtual ~HttpConnection() {}

    int64_t GetConnectionId() {
        return GetEndPoint().GetId();
    }

    const HttpRequest& http_request() const {
        return m_http_request;
    }
    HttpRequest* mutable_http_request() {
        return &m_http_request;
    }
    HttpResponse* mutable_http_response() {
        return &m_http_response;
    }

    // Close the connection. 'immidiate' is defaultly set to false. It means
    // connection will be closed when all pendding packets are sent.
    void Close(bool immidiate = false);
    bool SendPacket(const void* data, size_t size);
    bool SendPacket(netframe::Packet* packet);

protected:
    virtual bool OnSent(netframe::Packet* packet);
    virtual int  DetectPacketSize(const void* data, size_t size);
    virtual int  DetectHeaderSize(const void* data, size_t size);

    virtual void OnConnected() = 0;
    virtual void OnClose(int error_code) = 0;
    virtual void OnReceived(const netframe::Packet& packet) = 0;

    HttpHandler* m_http_handler;
    HttpRequest  m_http_request;
    HttpResponse m_http_response;
    bool         m_header_received;
};

class HttpServerConnection : public HttpConnection {
public:
    explicit HttpServerConnection(HttpServer* http_server);
    virtual ~HttpServerConnection() {}

protected:
    virtual void OnConnected() {}
    virtual void OnClose(int error_code);
    virtual void OnReceived(const netframe::Packet& packet);

private:
    HttpServer* m_http_server;
};

class HttpClientConnection : public HttpConnection {
public:
    HttpClientConnection(HttpClient* http_client, HttpHandler* handler);
    virtual ~HttpClientConnection();

protected:
    friend class HttpClient;
    void set_http_client(HttpClient* http_client) {
        m_http_client = http_client;
    }
    virtual void OnConnected();
    virtual void OnClose(int error_code);
    virtual void OnReceived(const netframe::Packet& packet);

private:
    HttpClient* m_http_client;
};

} // namespace poppy

#endif // COMMON_NET_HTTP_HTTP_CONNECTION_H
