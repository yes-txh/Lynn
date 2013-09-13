// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)
//
// An example echo client.

#include "common/base/scoped_ptr.h"
#include "common/base/string/string_algorithm.hpp"
#include "common/config/cflags.hpp"
#include "common/rpc/proto_rpc/examples/echo_service.pb.h"
#include "common/rpc/proto_rpc/rpc_service.h"
#include "common/system/concurrency/atomic/atomic.hpp"
#include "common/system/concurrency/thread.hpp"
// includes from thirdparty
#include "glog/logging.h"

CFLAGS_DEFINE_FLAG(int,
                   client_thread,
                   cflags::DefaultIs(4),
                   "the number of thread on client.");

CFLAGS_DEFINE_FLAG(int,
                   requests_number,
                   cflags::DefaultIs(10),
                   "the number of request sent to server.");

CFLAGS_DEFINE_FLAG(std::string,
                   server_address,
                   cflags::DefaultIs("127.0.0.1:10000"),
                   "the address server listen on.");

namespace rpc_examples {

Atomic<int> g_request_count;

void EchoCallback(rpc::RpcController* controller,
                  rpc_examples::EchoRequest* request,
                  rpc_examples::EchoResponse* response) {
    LOG(INFO)
        << "request: " << controller->sequence_id()
        << ", message: " << request->message();
    if (controller->Failed()) {
        LOG(INFO) << "failed: " << controller->ErrorText();
    } else {
        LOG(INFO) << "response: " << response->message();
    }
    delete controller;
    delete request;
    delete response;
    --g_request_count;
}

void EmptyCallback(rpc::RpcController* controller,
                  rpc_examples::EchoRequest* request,
                  rpc_examples::EchoResponse* response) {
    LOG(INFO) << "In empty callback!";
    if (controller->Failed()) {
        LOG(INFO) << "failed: " << controller->ErrorText();
    } else {
        LOG(INFO) << "response: " << response->message();
    }
    --g_request_count;
}

class DualEchoClientImpl : public DualEchoClient {
public:
    DualEchoClientImpl()
        : m_dual_request_count(0) {
    }
    virtual ~DualEchoClientImpl() {}

private:
    virtual void CollectDualEcho(google::protobuf::RpcController* controller,
                                 const EchoRequest* request,
                                 EchoResponse* response,
                                 google::protobuf::Closure* done) {
        ++m_dual_request_count;
        response->set_user(request->user());
        response->set_message(
            "dual echo from client, message: " + request->message());
        LOG(INFO) << "request: " << request->message();
        LOG(INFO) << "response: " << response->message();
        done->Run();
    }

    Atomic<int> m_dual_request_count;
};

} // namespace rpc_examples

int main(int argc, char** argv)
{
    if (!CFlags::ParseCommandLine(argc, argv))
        return -1;

    scoped_ptr<netframe::NetFrame> net_frame(
        new netframe::NetFrame(Flag_client_thread.Value()));
    scoped_ptr<rpc::RpcChannel> rpc_channel(
        new rpc::RpcChannel(net_frame.get()));

    rpc_examples::DualEchoClientImpl dual_echo_server;
    rpc_channel->RegisterService(&dual_echo_server);

    std::vector<std::string> server_addresses;
    SplitString(Flag_server_address.Value(), ";", &server_addresses);
    if (!rpc_channel->StartClient(server_addresses)) {
        return -1;
    }

    rpc_examples::EchoServer::Stub echo_client(rpc_channel.get());
    rpc_examples::DualEchoServer::Stub dual_echo_client(rpc_channel.get());

    // Test cancel request when delete the controller.
    {
        rpc::RpcController* rpc_controller = new rpc::RpcController();
        rpc_examples::EchoRequest* request = new rpc_examples::EchoRequest();
        rpc_examples::EchoResponse* response = new rpc_examples::EchoResponse();
        google::protobuf::Closure* done =
            NewClosure(&rpc_examples::EmptyCallback, rpc_controller, request, response);

        request->set_user("echo_test_user");
        request->set_message(StringFormat("test controller is deleted"));
        LOG(INFO) << "sending an asynchronous request: " << request->message();
        ++rpc_examples::g_request_count;
        echo_client.TimeoutEcho(rpc_controller, request, response, done);
        delete rpc_controller;
        delete request;
        delete response;
    }


    // Send dual echo request synchronously.
    for (int i = 0; i < Flag_requests_number.Value(); ++i) {
        rpc::RpcController rpc_controller;
        rpc_examples::EchoRequest request;
        rpc_examples::EchoResponse response;
        request.set_user("echo_test_user");
        request.set_message(StringFormat("the %dth request", i));
        LOG(INFO) << "sending synchronous request: " << request.message();

        dual_echo_client.DualEcho(&rpc_controller, &request, &response, NULL);

        LOG(INFO) << "request: " << request.message();
        if (rpc_controller.Failed()) {
            LOG(INFO) << "failed: " << rpc_controller.ErrorText();
        } else {
            LOG(INFO) << "response: " << response.message();
        }
    }

    // Send request synchronously.
    for (int i = 0; i < Flag_requests_number.Value(); ++i) {
        rpc::RpcController rpc_controller;
        rpc_examples::EchoRequest request;
        rpc_examples::EchoResponse response;
        request.set_user("echo_test_user");
        request.set_message(StringFormat("the %dth request", i));

        LOG(INFO) << "sending synchronous request: " << request.message();
        switch (i % 3) {
            case 0:
                echo_client.SimpleEcho(
                    &rpc_controller, &request, &response, NULL);
                break;
            case 1:
                echo_client.ComplicatedEcho(
                    &rpc_controller, &request, &response, NULL);
                break;
            case 2:
                echo_client.TimeoutEcho(
                    &rpc_controller, &request, &response, NULL);
                break;
        }

        LOG(INFO) << "request: " << request.message();
        if (rpc_controller.Failed()) {
            LOG(INFO) << "failed: " << rpc_controller.ErrorText();
        } else {
            LOG(INFO) << "response: " << response.message();
        }
    }

    // Send request asynchronously, a callback is used to receive completion
    // notification.
    for (int i = 0; i < Flag_requests_number.Value(); ++i) {
        rpc::RpcController* rpc_controller =
            new rpc::RpcController();
        rpc_examples::EchoRequest* request = new rpc_examples::EchoRequest();
        rpc_examples::EchoResponse* response = new rpc_examples::EchoResponse();
        google::protobuf::Closure* done =
            NewClosure(&rpc_examples::EchoCallback,
                       rpc_controller,
                       request,
                       response);

        request->set_user("echo_test_user");
        request->set_message(StringFormat("the %dth request", i));

        LOG(INFO) << "sending asynchronous request: " << request->message();
        ++rpc_examples::g_request_count;
        switch (i % 3) {
            case 0:
                echo_client.SimpleEcho(
                    rpc_controller, request, response, done);
                break;
            case 1:
                echo_client.ComplicatedEcho(
                    rpc_controller, request, response, done);
                break;
            case 2:
                echo_client.TimeoutEcho(
                    rpc_controller, request, response, done);
                break;
        }
    }

    for (;;) {
        ThisThread::Sleep(100);
        if (rpc_examples::g_request_count == 0) {
            break;
        }
    }

    rpc_channel.reset(NULL);
    net_frame.reset(NULL);
    return 0;
}
