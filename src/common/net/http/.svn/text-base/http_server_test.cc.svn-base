#include <signal.h>
#include <iostream>
#include "common/net/http/http_server.h"
#include "common/base/closure.h"
#include "common/netframe/netframe.hpp"
#include "common/system/concurrency/thread.hpp"
#include "glog/logging.h"

static std::string html = "<html><header><title>demo</title></header><body><center>fuck</center></body></html>";

bool ProcessRequest(const HttpRequest& http_request, HttpResponse* http_response) {
    std::cout << http_request.HeadersToString();
    if (!http_request.http_body().empty()) {
        std::cout << http_request.http_body();
    }
    http_response->set_status(200);
    http_response->mutable_http_body()->append(html);
    return true;
}

bool g_quit = false;

static void SignalIntHandler(int)
{
    g_quit = true;
}

int main(int argc, char** argv)
{
    netframe::NetFrame* net_frame = new netframe::NetFrame();
    {
        // Two http servers share a netframe.
        poppy::HttpServer http_server_1(net_frame, false);
        poppy::HttpServer http_server_2(net_frame, false);

        // A handler can't be shared by different servers.
        poppy::HttpClosure* closure1 = NewPermanentClosure(ProcessRequest);
        poppy::SimpleHttpServerHandler* handler1 = new poppy::SimpleHttpServerHandler(
                closure1);
        http_server_1.RegisterHandler("/test11", handler1);
        http_server_1.RegisterHandler("/test12", handler1);

        // Server_2's handler
        poppy::HttpClosure* closure2 = NewPermanentClosure(ProcessRequest);
        poppy::SimpleHttpServerHandler* handler2 = new poppy::SimpleHttpServerHandler(
                closure2);
        http_server_2.RegisterHandler("/test21", handler2);
        http_server_2.RegisterHandler("/test22", handler2);


        // Start the two servers.
        http_server_1.Start("127.0.0.1:50000");
        http_server_2.Start("127.0.0.1:60000");

#if 0
        // Delete netframe. only listeners exist.
        delete net_frame;
#endif

        // Servers are running for some time....
        signal(SIGINT, SignalIntHandler);
        signal(SIGTERM, SignalIntHandler);

        while (!g_quit) {
            ThisThread::Sleep(1000);
        }

#if 0
        // Delete netframe. connections exist.
        delete net_frame;
#endif

        http_server_1.Stop();
        http_server_2.Stop();

#if 0
        // Delete netframe. servers stoped.
        delete net_frame;
#endif
    }

#if 1
    // Delete netframe. noramlly quit.
    delete net_frame;
#endif

    return EXIT_SUCCESS;
}
