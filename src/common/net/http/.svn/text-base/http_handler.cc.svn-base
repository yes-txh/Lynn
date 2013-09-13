// Copyright 2011, Tencent Inc.
// Author: Xiaokang Liu (hsiaokangliu@tencent.com)

#include "common/base/closure.h"
#include "common/base/string/string_number.hpp"
#include "common/net/http/http_handler.h"
#include "glog/logging.h"

namespace poppy {

void SimpleHttpServerHandler::HandleBodyPacket(HttpConnection* http_connection,
        const StringPiece& string_piece) {
    http_connection->mutable_http_request()->mutable_http_body()->append(
            string_piece.data(), string_piece.length());
}

int SimpleHttpServerHandler::DetectBodyPacketSize(
        HttpConnection* http_connection, const StringPiece &data) {
    std::string value;
    HttpRequest* http_request = http_connection->mutable_http_request();
    // Header field "Content-Length" exists.
    if (http_request->GetHeader("Content-Length", &value)) {
        int content_length = 0;
        if (StringToNumber(StringTrim(value), &content_length)) {
            return content_length;
        }
        return -1;
    }
    // (TODO) hsiaokangliu: Handle "Transfer-Encoding"
    if (http_request->GetHeader("Transfer-Encoding", &value)) {
        if (strcasecmp(value.c_str(), "chunked") == 0) {
        }
    }
    return 0;
}

void SimpleHttpServerHandler::HandleMessage(HttpConnection* http_connection) {
    const HttpRequest& http_request = http_connection->http_request();
    HttpResponse* http_response = http_connection->mutable_http_response();
    http_response->Reset();
    bool retval = false;
    http_response->set_status(200);
    if (m_closure) {
        retval = m_closure->Run(http_request, http_response);
    }
    if (!retval) {
        LOG(INFO) << "Failed to process request on connection: "
            << http_connection->GetConnectionId();
        std::string response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        http_connection->SendPacket(response.c_str(), response.size());
        http_connection->Close();
    }

    // Add some default values for often used fields.
    std::string header_value;
    if (!http_response->GetHeader("Content-Type", &header_value)) {
        http_response->AddHeader("Content-Type", "text/html");
    }
    if (!http_response->GetHeader("Content-Length", &header_value)) {
        http_response->AddHeader("Content-Length",
                IntegerToString(http_response->http_body().size()));
    }

    std::string headers = http_response->HeadersToString();
    http_connection->SendPacket(headers.c_str(), headers.size());
    // Netframe can't send an empty message.
    if (http_response->http_body().size() > 0) {
        http_connection->SendPacket(http_response->http_body().c_str(),
                http_response->http_body().size());
    }

    // Client request that don't keep alive.
    if (!http_request.IsKeepAlive()) {
        LOG(INFO) << "Not keep alive connection, close: "
            << http_connection->GetConnectionId();
        http_connection->Close();
    } else if (!http_response->IsKeepAlive()) {
        LOG(INFO) << "Response set connection closed: "
            << http_connection->GetConnectionId();
        // User set the connection close.
        http_connection->Close();
    }
}

} // namespace poppy
