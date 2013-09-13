#include <signal.h>
#include <iostream>
#include "common/base/closure.h"
#include "common/base/string/string_number.hpp"
#include "common/net/http/http_client.h"
#include "common/net/http/http_handler.h"
#include "common/system/concurrency/thread.hpp"
#include "glog/logging.h"

using namespace poppy;

class TestHandler : public HttpHandler {
public:
    void DoRequest(HttpConnection* http_connection) {
        HttpRequest http_request;
        http_request.set_method(HttpRequest::METHOD_GET);
        http_request.set_path("/test11");

        std::string headers = http_request.HeadersToString();
        LOG(INFO) << "=========== send request ================";
        if (!http_connection->SendPacket(headers.c_str(), headers.size())) {
            LOG(INFO) << "Failed to send packet to server.";
        }
    }

    virtual void HandleHeaders(HttpConnection* http_connection) {
        LOG(INFO) << "=========== handler headers =============";
    }

    virtual void OnConnected(HttpConnection* http_connection) {
        DoRequest(http_connection);
    }

    virtual void HandleBodyPacket(HttpConnection* http_connection,
            const StringPiece& string_piece) {
        http_connection->mutable_http_response()->mutable_http_body()->append(
                string_piece.data(), string_piece.length());
    }

    virtual void HandleMessage(HttpConnection* http_connection) {
        LOG(INFO) << "=========== handler message =============";
        HttpResponse* http_response = http_connection->mutable_http_response();
        std::cout << std::endl;
        std::cout << http_response->HeadersToString();
        std::cout << http_response->http_body();
        std::cout << std::endl;
        DoRequest(http_connection);
    }

    virtual int DetectBodyPacketSize(HttpConnection* http_connection,
            const StringPiece& data) {
        std::string value;
        HttpResponse* http_response = http_connection->mutable_http_response();
        // Header field "Content-Length" exists.
        if (http_response->GetHeader("Content-Length", &value)) {
            int content_length = 0;
            if (StringToNumber(StringTrim(value), &content_length)) {
                LOG(INFO) << "content_length: " << content_length;
                return content_length;
            }
            return -1;
        }
        // (TODO) hsiaokangliu: Handle "Transfer-Encoding"
        if (http_response->GetHeader("Transfer-Encoding", &value)) {
            if (strcasecmp(value.c_str(), "chunked") == 0) {
            }
        }
        return 0;
    }
};

bool g_quit = false;

static void SignalIntHandler(int)
{
    g_quit = true;
}

int main(int argc, char** argv)
{
    HttpClient http_client_1;
    if (!http_client_1.ConnectServer("127.0.0.1:50000", new TestHandler())) {
        LOG(INFO) << "Failed to connect to server.";
        return EXIT_FAILURE;
    }

    HttpClient http_client_2;
    if (!http_client_2.ConnectServer("127.0.0.1:50000", new TestHandler())) {
        LOG(INFO) << "Failed to connect to server.";
        return EXIT_FAILURE;
    }

    signal(SIGINT, SignalIntHandler);
    signal(SIGTERM, SignalIntHandler);

    while (!g_quit) {
        ThisThread::Sleep(1000);
    }
    return EXIT_SUCCESS;
}
