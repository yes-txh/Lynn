// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)

#include <stdlib.h>
#include "common/base/compatible/string.h"
#include "common/base/scoped_ptr.h"
#include "common/base/string/string_algorithm.hpp"
#include "common/rpc/proto_rpc/http_message.h"
#include "common/rpc/proto_rpc/http_service.h"
#include "common/system/concurrency/atomic/atomic.hpp"
#include "common/system/concurrency/condition_variable.hpp"
#include "common/system/concurrency/mutex.hpp"
#include "common/system/concurrency/rwlock.hpp"
// includes from thirdparty
#include "glog/logging.h"

namespace rpc {

namespace {

// class HttpServerInternalListener
class HttpServerInternalListener : public netframe::ListenSocketHandler {
public:
    HttpServerInternalListener(HttpServer* http_server)
        : netframe::ListenSocketHandler(*(http_server->mutable_net_frame())),
          m_http_server(http_server) {
    }

    virtual ~HttpServerInternalListener() {}

private:
    // Implements ListenSocketHandler interface.
    virtual void OnClose(int error_code) {
    }

    virtual netframe::StreamSocketHandler* OnAccepted(netframe::SocketId id) {
        // netframe would take the ownership of the allocated http connection.
        return new HttpConnection(m_http_server);
    }

    HttpServer* m_http_server;
};

} // namespace

// class BlockingReferenceCounter
BlockingReferenceCounter::BlockingReferenceCounter(
    Closure<void>* complete_callback)
    : m_reference_mutex(new SimpleMutex()),
      m_reference_cond(new ConditionVariable()),
      m_complete_callback(complete_callback),
      m_state(STATE_OPENED),
      m_reference_count(0) {
}

BlockingReferenceCounter::~BlockingReferenceCounter() {
    WaitForClosed();
}

void BlockingReferenceCounter::SetClosing(bool force_closed) {
    MutexLocker locker(*m_reference_mutex);
    if (m_state == STATE_CLOSED) {
        // Has been closed, nothing to do.
        return;
    }

    if (force_closed || m_reference_count == 0) {
        // Forcely closed or last reference, call the complete callback, set the
        // counter closed, wake up WaitForClosed().
        if (m_complete_callback) {
            m_complete_callback->Run();
        }
        m_state = STATE_CLOSED;  // "opened" or "closing" ==> "closed".
        m_reference_cond->Signal();
    } else if (m_state == STATE_OPENED) {
        m_state = STATE_CLOSING;  // "opened" ==> "closing"
    } else {
        // Has been set as "closing".
    }
}

void BlockingReferenceCounter::WaitForClosed() {
    SetClosing(false);  // don't forcely close the counter.

    MutexLocker locker(*m_reference_mutex);
    if (m_state == STATE_CLOSED) {
        // Has been closed, nothing to do.
        return;
    }

    if (m_state == STATE_OPENED) {
        m_state = STATE_CLOSING;  // "opened" ==> "closing"
    } else {
        // Has been set as "closing".
    }
    while (m_state != STATE_CLOSED) {
        m_reference_cond->Wait(*m_reference_mutex);
    }
}

bool BlockingReferenceCounter::AcquireReference() {
    MutexLocker locker(*m_reference_mutex);
    CHECK_GE(m_reference_count, 0);
    if (m_state != STATE_OPENED) {
        return false;
    }
    ++m_reference_count;
    return true;
}

void BlockingReferenceCounter::ReleaseReference() {
    MutexLocker locker(*m_reference_mutex);
    CHECK_GT(m_reference_count, 0);
    --m_reference_count;
    if (m_state == STATE_CLOSING && m_reference_count == 0) {
        // Last reference, call the complete callback, set the counter closed,
        // wake up WaitForClosed().
        if (m_complete_callback) {
            m_complete_callback->Run();
        }
        m_state = STATE_CLOSED;  // "closing" ==> "closed".
        m_reference_cond->Signal();
    }
}

// class CallbackReferenceCounter
CallbackReferenceCounter::CallbackReferenceCounter(
    Closure<void>* complete_callback)
    :  m_complete_callback(complete_callback),
       m_state(STATE_OPENED),
       m_reference_count(0) {
}

CallbackReferenceCounter::~CallbackReferenceCounter() {
    CHECK_EQ(STATE_CLOSED, AtomicGet(m_state))
        << "The counter hasn't be closed properly, possibly the SetClosing() "
            "hasn' been called, or some references haven't been released.";
}

void CallbackReferenceCounter::SetClosing(bool force_closed) {
    if (AtomicGet(m_state) == STATE_CLOSED) {
        // Has been closed, nothing to do.
        return;
    }

    if (force_closed) {
        AtomicSet(m_state, STATE_CLOSED);
        // As the callback might cause the reference counter is deleted, so it
        // must be the last sentence to access members.
        m_complete_callback->Run();
    } else if (AtomicGet(m_state) == STATE_OPENED) {
        // Prevent another thread who is calling ReleaseReference() from calling the
        // complete callback at the same time.
        CHECK(AcquireReference());
        AtomicSet(m_state, STATE_CLOSING);
        ReleaseReference();
    }
}

bool CallbackReferenceCounter::AcquireReference() {
    AtomicIncrement(m_reference_count);
    if (AtomicGet(m_state) != STATE_OPENED) {
        ReleaseReference();
        return false;
    }
    return true;
}

void CallbackReferenceCounter::ReleaseReference() {
    int value = AtomicDecrement(m_reference_count);
    CHECK_GE(value, 0);
    if (AtomicGet(m_state) == STATE_CLOSING && value == 0) {
        AtomicSet(m_state, STATE_CLOSED);
        // As the callback might cause the reference counter is deleted, so it
        // must be the last sentence to access members.
        m_complete_callback->Run();
    }
}

// class HttpServiceHandler
HttpServiceHandler::HttpServiceHandler(HttpServer* http_server)
    : m_http_server(http_server),
      m_reference_counter(new BlockingReferenceCounter()) {
    CHECK_NOTNULL(m_http_server);
    CHECK(IsServerSide());
}

HttpServiceHandler::HttpServiceHandler()
    : m_http_server(NULL),
      m_reference_counter(new BlockingReferenceCounter()) {
    CHECK(!IsServerSide());
}

HttpServiceHandler::~HttpServiceHandler() {
    // deleting "m_reference_counter" would wait for it to be closed.
}

// class HttpConnection
HttpConnection::HttpConnection(
    HttpServer* http_server)
    : netframe::StreamSocketHandler(*(http_server->mutable_net_frame())),
      m_http_server(http_server),
      m_http_handler(NULL),
      m_connection_id(-1),
      m_headers_received(false) {
    CHECK_NOTNULL(m_http_server);
    CHECK(IsServerSide());
    m_pending_packets.reset(
        new CallbackReferenceCounter(
            NewClosure(this, &HttpConnection::CloseEndPoint)));
}

HttpConnection::HttpConnection(
    netframe::NetFrame* net_frame, HttpServiceHandler* http_handler)
    : netframe::StreamSocketHandler(*net_frame),
      m_http_server(NULL),
      m_http_handler(http_handler),
      m_connection_id(-1),
      m_headers_received(false) {
    CHECK_NOTNULL(m_http_handler);
    CHECK(!IsServerSide());
    m_pending_packets.reset(
        new CallbackReferenceCounter(
            NewClosure(this, &HttpConnection::CloseEndPoint)));
}

HttpConnection::~HttpConnection() {
    if (m_http_handler != NULL) {
        m_http_handler->mutable_reference_counter()->ReleaseReference();
    }

    // Possibly the netframe instead of user close the connection, forcely close
    // the reference counter here to make sure the callback is invoked.
    m_pending_packets->SetClosing(true);
}

void HttpConnection::CloseConnection(bool send_pending_packets) {
    m_pending_packets->SetClosing(!send_pending_packets);
}

bool HttpConnection::SendPacket(const StringPiece& data) {
    if (data.empty()) {
        // Skip empty packet.
        return true;
    }

    if (!m_pending_packets->AcquireReference()) {
        LOG(WARNING) << "Try to send data on closed connection";
        return false;
    }

    if (GetNetFrame().SendPacket(GetEndPoint(),
                                 data.data(),
                                 data.size()) != 0) {
        m_pending_packets->ReleaseReference();
        LOG(WARNING) << "Send packet failed.";
        return false;
    }

    return true;
}

void HttpConnection::CloseEndPoint() {
    // Possibly the end point has been closed. netframe is tolerant to it.
    GetNetFrame().CloseEndPoint(GetEndPoint());
}

int HttpConnection::DetectPacketSize(const void* data,
                                        size_t size) {
    if (!m_headers_received) {
        // First time to receive packet, so it's for http start line and
        // headers.
        // TODO(hansye): handle "\n\n", without '\r'.
        static const char kHeaderEnd[] = "\r\n\r\n";
        static const size_t kHeaderEndLength = 4;
        // Here we assume the client wouldn't send a malformed http request.
        // TODO(hansye): optimize the handling of corrupted request.
        const char* p = reinterpret_cast<const char*>(
            memmem(data, size, kHeaderEnd, kHeaderEndLength));
        if (p == NULL) {
            return 0;
        }
        return p - reinterpret_cast<const char*>(data) + kHeaderEndLength;
    } else {
        // We have parsed http headers and are receiving the message body.
        if (m_http_handler == NULL) {
            // No handler registerd on this path, we should have handled it when
            // parsing headers, so pretend to it's a complete packet and discard
            // it later in OnReceived.
            return size;
        }

        return m_http_handler->DetectBodySize(
            this, StringPiece(reinterpret_cast<const char*>(data), size));
    }
}

void HttpConnection::OnReceived(const netframe::Packet& packet) {
    if (!m_headers_received) {
        // First time to receive packet, so it's for http start line and
        // headers.
        m_headers_received = true;

        HttpMessage* http_message;
        if (IsServerSide()) {
            // Server side, we need to receive http request.
            m_http_request.reset(new HttpRequest());
            http_message = m_http_request.get();
        } else {
            // Client side, we need to receive http response.
            m_http_response.reset(new HttpResponse());
            http_message = m_http_response.get();
        }

        const std::string data(reinterpret_cast<const char*>(packet.Content()),
                               packet.Length());
        if (http_message->ParseHeaders(data) <= 0) {
            // Invalid input.
            HandleInvalidMessage(data);
            return;
        }

        if (IsServerSide()) {
            // Only need to look up handler at server side. At client side, it
            // has been passed in by constructor.
            m_http_handler = m_http_server->GetHandler(m_http_request->path());
            if (m_http_handler == NULL) {
                // No handler registerd on this path.
                HandleUnregisteredPath();
                return;
            }
        } else {
            CHECK_NOTNULL(m_http_handler);
        }

        m_http_handler->HandleHeaders(this);
    } else {
        // We have parsed http headers and are receiving the message body.
        if (m_http_handler == NULL) {
            // No handler registerd on this path, we should have handled it when
            // parsing headers, so just discard it here.
            return;
        }

        StringPiece data(reinterpret_cast<const char*>(packet.Content()),
                         packet.Length());
        m_http_handler->HandleBody(this, data);
    }
}

bool HttpConnection::OnSent(netframe::Packet* packet) {
    m_pending_packets->ReleaseReference();
    return true;
}

void HttpConnection::HandleInvalidMessage(const StringPiece& data) {
    LOG(WARNING) << "Unrecognized http headers, length: " << data.size();
    if (IsServerSide()) {
        SendPacket("HTTP/1.1 400 Bad Request\r\n\r\n");
    }
    CloseConnection(true); // send pending packets.
}

void HttpConnection::HandleUnregisteredPath() {
    LOG(WARNING)
        << "No handler registerd on this path: " << m_http_request->path();
    SendPacket("HTTP/1.1 404 Not Found\r\n\r\n");
    CloseConnection(true); // send pending packets.
}

// class HttpServer
HttpServer::HttpServer(netframe::NetFrame* net_frame, bool own_net_frame)
    : m_own_net_frame(own_net_frame),
      m_net_frame(net_frame),
      m_handler_rwlock(new RWLock()) {
}

HttpServer::HttpServer(int server_threads)
    : m_own_net_frame(true),
      m_handler_rwlock(new RWLock()) {
    m_net_frame = new netframe::NetFrame(server_threads);
}

HttpServer::~HttpServer() {
    if (m_own_net_frame) {
        delete m_net_frame;
    }
}

bool HttpServer::StartServer(const std::string& server_address) {
    SocketAddressInet server_address_inet(server_address);
    HttpServerInternalListener* listener =
        new HttpServerInternalListener(this);
    if (m_net_frame->AsyncListen(
        server_address_inet, listener, kMaxPacketSize) < 0) {
        LOG(ERROR) << "Can't listen on address: " << server_address;
        delete listener;
        return false;
    }
    return true;
}

bool HttpServer::RegisterHandler(const std::string& path,
                                 HttpServiceHandler* handler) {
    RWLock::WriterLocker locker(*m_handler_rwlock);
    return m_handlers.insert(make_pair(path, handler)).second;
}

bool HttpServer::UnregisterHandler(const std::string& path) {
    RWLock::WriterLocker locker(*m_handler_rwlock);
    return m_handlers.erase(path) == 1;
}

HttpServiceHandler* HttpServer::GetHandler(const std::string& path) const {
    RWLock::ReaderLocker locker(*m_handler_rwlock);
    std::map<std::string, HttpServiceHandler*>::const_iterator it =
        m_handlers.find(path);
    if (it == m_handlers.end()) {
        return NULL;
    }

    HttpServiceHandler* handler = it->second;
    if (!handler->mutable_reference_counter()->AcquireReference()) {
        // The handler has been set for closing.
        return NULL;
    }
    return handler;
}

} // namespace rpc
