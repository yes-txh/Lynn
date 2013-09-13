// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)
// Xiaokang Liu (hsiaokangliu@tencent.com)

#include <stdlib.h>
#include "common/net/http/http_connection.h"
#include "common/net/http/http_server.h"
#include "common/net/http/http_client.h"
#include "common/base/string/string_number.hpp"
#include "common/base/string/string_algorithm.hpp"
// includes from thirdparty
#include "glog/logging.h"

namespace poppy {

HttpConnection::HttpConnection(netframe::NetFrame* net_frame) :
    netframe::StreamSocketHandler(*net_frame),
    m_http_handler(NULL),
    m_header_received(false) {
}

void HttpConnection::Close(bool immidiate) {
    // Reset the connection status.
    m_header_received = false;
    GetNetFrame().CloseEndPoint(GetEndPoint(), immidiate);
}

bool HttpConnection::SendPacket(const void* data, size_t size) {
    if (GetNetFrame().SendPacket(GetEndPoint(), data, size) < 0) {
        LOG(WARNING) << "Send packet failed.";
        return false;
    }
    return true;
}

bool HttpConnection::SendPacket(netframe::Packet* packet) {
    if (GetNetFrame().SendPacket(GetEndPoint(), packet) < 0) {
        LOG(WARNING) << "Send packet failed.";
        return false;
    }
    return true;
}

bool HttpConnection::OnSent(netframe::Packet* packet) {
    return true;
}

int HttpConnection::DetectPacketSize(const void* data, size_t size) {
    if (!m_header_received) {
        return DetectHeaderSize(data, size);
    } else {
        StringPiece string_piece(reinterpret_cast<const char*>(data), size);
        return m_http_handler->DetectBodyPacketSize(this, string_piece);
    }
}

int HttpConnection::DetectHeaderSize(const void* data, size_t size) {
    static const char kHeaderEnd_1[] = "\r\n\r\n";
    static const size_t kHeaderEndLength_1 = 4;
    // Here we assume there is no malformed http message.
    const char* p = reinterpret_cast<const char*>(
            memmem(data, size, kHeaderEnd_1, kHeaderEndLength_1));
    if (p != NULL) {
        return p - reinterpret_cast<const char*>(data) + kHeaderEndLength_1;
    }
    static const char kHeaderEnd_2[] = "\n\n";
    static const size_t kHeaderEndLength_2 = 2;
    p = reinterpret_cast<const char*>(
            memmem(data, size, kHeaderEnd_2, kHeaderEndLength_2));
    if (p != NULL) {
        return p - reinterpret_cast<const char*>(data) + kHeaderEndLength_2;
    }
    return 0;
}

HttpServerConnection::HttpServerConnection(HttpServer* http_server)
    : HttpConnection(http_server->mutable_net_frame()),
      m_http_server(http_server) {
}

void HttpServerConnection::OnClose(int error_code) {
    if (m_http_handler) {
        m_http_handler->OnClose(this, error_code);
    }
    m_http_server->mutable_connection_manager()->RemoveConnection(
            GetConnectionId());
}

void HttpServerConnection::OnReceived(const netframe::Packet& packet) {
    bool message_completed = false;
    if (!m_header_received) {
        m_header_received = true;
        std::string header(reinterpret_cast<const char*>(packet.Content()),
                           packet.Length());
        m_http_request.Reset();
        if (m_http_request.ParseHeaders(header) != HttpMessage::ERROR_NORMAL) {
            // (TODO) handler parser error.
            LOG(INFO) << "Failed to parse the http header: " << header;
            std::string response = "HTTP/1.1 400 Bad Request\r\n\r\n";
            SendPacket(response.c_str(), response.size());
            Close();
            return;
        }
        std::string path = m_http_request.path();
        std::string::size_type question_mark_pos = path.find("?");
        if (question_mark_pos != std::string::npos) {
            path = path.substr(0, question_mark_pos);
        }

        m_http_handler = m_http_server->FindHandler(path);
        if (m_http_handler == NULL) {
            // (TODO) no handler registered.
            LOG(INFO) << "No handler registered on the path: " << path;
            std::string response = "HTTP/1.1 404 Not Found\r\n\r\n\
                    <html><head><title>404 not found</title></head>\
                    <body><font size=10><b>Page Not Found!</b></font></body></html>";
            SendPacket(response.c_str(), response.size());
            Close();
            return;
        }
        m_http_handler->HandleHeaders(this);

        // Check if the message is complete
        std::string value;
        if (m_http_request.method() == HttpRequest::METHOD_GET ||
            m_http_request.method() == HttpRequest::METHOD_HEAD) {
            message_completed = true;
        } else if (m_http_request.GetHeader("Content-Length", &value)) {
            int content_length = 0;
            if (StringToNumber(value, &content_length) && content_length == 0) {
                message_completed = true;
            }
        }

        if (message_completed) {
            m_header_received = false;
            m_http_handler->HandleMessage(this);
        }
    } else {
        StringPiece data(reinterpret_cast<const char*>(packet.Content()),
                         packet.Length());
        m_http_handler->HandleBodyPacket(this, data);

        // Check if the message is complete
        std::string value;
        if (m_http_request.GetHeader("Transfer-Encoding", &value)) {
            // (TODO) handle transfer encoding.
        } else if (m_http_request.GetHeader("Content-Length", &value)) {
            int content_length = 0;
            if (StringToNumber(value, &content_length) &&
                    content_length == static_cast<int>(packet.Length())) {
                message_completed = true;
            }
        }

        if (message_completed) {
            m_header_received = false;
            m_http_handler->HandleMessage(this);
        }
    }
}

HttpClientConnection::HttpClientConnection(HttpClient* http_client,
        HttpHandler* http_handler) :
    HttpConnection(http_client->mutable_net_frame()),
    m_http_client(http_client) {
    CHECK_NOTNULL(http_handler);
    m_http_handler = http_handler;
}

HttpClientConnection::~HttpClientConnection() {
    delete m_http_handler;
}

void HttpClientConnection::OnConnected() {
    m_http_handler->OnConnected(this);
}

void HttpClientConnection::OnClose(int error_code) {
    m_http_handler->OnClose(this, error_code);
    m_http_client->mutable_connection_manager()->RemoveConnection(
            GetConnectionId());
}

void HttpClientConnection::OnReceived(const netframe::Packet& packet) {
    bool message_completed = false;
    if (!m_header_received) {
        m_header_received = true;
        std::string header(reinterpret_cast<const char*>(packet.Content()),
                           packet.Length());
        m_http_response.Reset();
        if (m_http_response.ParseHeaders(header) != HttpMessage::ERROR_NORMAL) {
            LOG(INFO) << "Failed to parse the http header: " << header;
            Close();
            return;
        }
        m_http_handler->HandleHeaders(this);

        // Check if the message is complete
        std::string value;
        if (m_http_response.status() < 200 ||
                m_http_response.status() == 204 ||
                m_http_response.status() == 304) {
            message_completed = true;
        } else if (m_http_response.GetHeader("Content-Length", &value)) {
            int content_length = 0;
            if (StringToNumber(value, &content_length) && content_length == 0) {
                message_completed = true;
            }
        }
        if (message_completed) {
            m_header_received = false;
            m_http_handler->HandleMessage(this);
        }
    } else {
        StringPiece data(reinterpret_cast<const char*>(packet.Content()),
                         packet.Length());
        m_http_handler->HandleBodyPacket(this, data);

        // Check if the message is complete
        std::string value;
        if (m_http_response.GetHeader("Transfer-Encoding", &value)) {
            // (TODO) handle transfer encoding.
        } else if (m_http_response.GetHeader("Content-Length", &value)) {
            int content_length = 0;
            if (StringToNumber(value, &content_length) &&
                    content_length == static_cast<int>(packet.Length())) {
                message_completed = true;
            }
        }

        if (message_completed) {
            m_header_received = false;
            m_http_handler->HandleMessage(this);
        }
    }
}

} // namespace poppy
