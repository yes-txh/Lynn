// Copyright 2011, Tencent Inc.
// Author: Xiaokang Liu (hsiaokangliu@tencent.com)

#ifndef COMMON_NET_HTTP_HTTP_HANDLER_H
#define COMMON_NET_HTTP_HTTP_HANDLER_H

#include "common/base/closure.h"
#include "common/base/string/string_piece.hpp"
#include "common/netframe/netframe.hpp"
#include "common/net/http/http_connection.h"

namespace poppy {

typedef ::Closure<bool, const HttpRequest&, HttpResponse*> HttpClosure;

// Describes how to complete a http service.
// All http connections of the same http service share the same
// service handler.

// CAUTION: One handler can only be registered to one determined http frame!
class HttpHandler {
public:
    HttpHandler() {}
    virtual ~HttpHandler() {}

    // Note: OnConnected is only called at client side.
    virtual void OnConnected(HttpConnection* connection) {}
    virtual void OnClose(HttpConnection* connection, int error_code) {};
    // Handle a comlete message.
    virtual void HandleMessage(HttpConnection* http_connection) {}

    // The interface the derived class needs to implement.

    // Handle headers. It's called when http header received.
    virtual void HandleHeaders(HttpConnection* http_connection) = 0;
    // Handle a body packet. It's called when a body packet received.
    virtual void HandleBodyPacket(HttpConnection* http_connection,
            const StringPiece& string_piece) = 0;
    // Dectect the body packet size. If unknown, 0 is returned.
    virtual int DetectBodyPacketSize(HttpConnection* http_connection,
            const StringPiece& data) = 0;
};

class SimpleHttpServerHandler : public HttpHandler {
public:
    SimpleHttpServerHandler() {}
    SimpleHttpServerHandler(HttpClosure* closure, bool own_closure = true) :
        m_closure(closure),
        m_own_closure(own_closure) {
    }
    virtual ~SimpleHttpServerHandler() {
        if (m_own_closure) {
            delete m_closure;
        }
    }

    virtual void HandleHeaders(HttpConnection* http_connection) {}
    virtual void HandleBodyPacket(HttpConnection* http_connection,
            const StringPiece& string_piece);
    virtual int DetectBodyPacketSize(HttpConnection* http_connection,
            const StringPiece& data);
    virtual void HandleMessage(HttpConnection* http_connection);

    void set_closure(HttpClosure* closure, bool own_closure = true) {
        if (m_own_closure) {
            delete m_closure;
        }
        m_closure = closure;
        m_own_closure = own_closure;
    }
protected:
    // It should be a permanent closure.
    HttpClosure* m_closure;
    bool m_own_closure;
};

} // namespace poppy

#endif // COMMON_NET_HTTP_HTTP_HANDLER_H
