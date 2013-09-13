// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)
//
// An example echo server.

#include "common/base/scoped_ptr.h"
#include "common/config/cflags.hpp"
#include "common/rpc/proto_rpc/examples/echo_service.pb.h"
#include "common/rpc/proto_rpc/rpc_service.h"
#include "common/system/concurrency/atomic/atomic.hpp"
#include "common/system/concurrency/thread.hpp"
#include "common/system/concurrency/thread_pool.hpp"
// includes from thirdparty
#include "glog/logging.h"

CFLAGS_DEFINE_FLAG(int,
                   server_thread,
                   cflags::DefaultIs(4),
                   "the number of thread on server.");

CFLAGS_DEFINE_FLAG(int,
                   background_thread,
                   cflags::DefaultIs(4),
                   "the number of thread for background processing.");

CFLAGS_DEFINE_FLAG(std::string,
                   server_address,
                   cflags::DefaultIs("127.0.0.1:10000"),
                   "the address server listen on.");

namespace rpc_examples {

class EchoServerImpl : public EchoServer {
public:
    EchoServerImpl()
        : m_simple_request_count(0),
          m_complicated_request_count(0),
          m_timeout_request_count(0) {
        m_background_thread_pool.reset(
            new ThreadPool(Flag_background_thread.Value(),
                           Flag_background_thread.Value()));
    }
    virtual ~EchoServerImpl() {}

private:
    virtual void SimpleEcho(google::protobuf::RpcController* controller,
                            const EchoRequest* request,
                            EchoResponse* response,
                            google::protobuf::Closure* done) {
        ++m_simple_request_count;
        response->set_user(request->user());
        response->set_message(
            "simple echo from server: " + Flag_server_address.Value() +
            ", message: " + request->message());
        LOG(INFO) << "request: " << request->message();
        LOG(INFO) << "response: " << response->message();
        done->Run();
    }

    virtual void ComplicatedEcho(google::protobuf::RpcController* controller,
                                 const EchoRequest* request,
                                 EchoResponse* response,
                                 google::protobuf::Closure* done) {
        Closure<void>* callback =
            NewClosure(this,
                       &EchoServerImpl::DoComplicatedEcho,
                       controller,
                       request,
                       response,
                       done);
        m_background_thread_pool->AddTask(callback);
    }

    virtual void TimeoutEcho(google::protobuf::RpcController* controller,
                             const EchoRequest* request,
                             EchoResponse* response,
                             google::protobuf::Closure* done) {
        Closure<void>* callback =
            NewClosure(this,
                       &EchoServerImpl::DoTimeoutEcho,
                       controller,
                       request,
                       response,
                       done);
        m_background_thread_pool->AddTask(callback);
    }

    void DoComplicatedEcho(google::protobuf::RpcController* controller,
                           const EchoRequest* request,
                           EchoResponse* response,
                           google::protobuf::Closure* done) {
        ThisThread::Sleep(100);
        ++m_complicated_request_count;
        response->set_user(request->user());
        response->set_message(
            "complicated echo from server: " + Flag_server_address.Value() +
            ", message: " + request->message());
        LOG(INFO) << "request: " << request->message();
        LOG(INFO) << "response: " << response->message();
        done->Run();
    }

    void DoTimeoutEcho(google::protobuf::RpcController* controller,
                       const EchoRequest* request,
                       EchoResponse* response,
                       google::protobuf::Closure* done) {
        ThisThread::Sleep(3000);
        ++m_timeout_request_count;
        response->set_user(request->user());
        response->set_message(
            "timeout echo from server: " + Flag_server_address.Value() +
            ", message: " + request->message());
        LOG(INFO) << "request: " << request->message();
        LOG(INFO) << "response: " << response->message();
        done->Run();
    }

    scoped_ptr<ThreadPool> m_background_thread_pool;
    Atomic<int> m_simple_request_count;
    Atomic<int> m_complicated_request_count;
    Atomic<int> m_timeout_request_count;
};

class DualEchoServerImpl : public DualEchoServer {
public:
    DualEchoServerImpl(DualEchoClient::Stub* dual_echo_client)
        : m_dual_request_count(0),
          m_dual_echo_client(dual_echo_client) {
        m_background_thread_pool.reset(
            new ThreadPool(Flag_background_thread.Value(),
                           Flag_background_thread.Value()));
    }
    virtual ~DualEchoServerImpl() {}

private:
    virtual void DualEcho(google::protobuf::RpcController* controller,
                          const EchoRequest* request,
                          EchoResponse* response,
                          google::protobuf::Closure* done) {
        Closure<void>* callback =
            NewClosure(this,
                       &DualEchoServerImpl::DoDualEcho,
                       controller,
                       request,
                       response,
                       done);
        m_background_thread_pool->AddTask(callback);
    }

    void DoDualEcho(google::protobuf::RpcController* controller,
                    const EchoRequest* request,
                    EchoResponse* response,
                    google::protobuf::Closure* done) {
        ++m_dual_request_count;
        response->set_user(request->user());
        response->set_message(
            "dual echo from server: " + Flag_server_address.Value() +
            ", message: " + request->message());
        LOG(INFO) << "request: " << request->message();
        LOG(INFO) << "response: " << response->message();

        CallClient(controller, request, response);

        done->Run();
    }

    // Call the service on client side. The example is an asynchronous call.
    void CallClient(google::protobuf::RpcController* controller,
                    const EchoRequest* request,
                    EchoResponse* response) {
        rpc::RpcController* dual_controller =
            rpc::RpcServerChannel::CreateDualRpcController(controller);
        EchoRequest* dual_request = new EchoRequest();
        EchoResponse* dual_response = new EchoResponse();
        dual_request->set_user("echo_test_user");
        dual_request->set_message(
            "dual echo from server: " + Flag_server_address.Value() +
            ", message: " + request->message());
        Closure<void>* callback =
            NewClosure(this,
                       &DualEchoServerImpl::CallClientCallback,
                       dual_controller,
                       dual_request,
                       dual_response);
        m_dual_echo_client->CollectDualEcho(
            dual_controller, dual_request, dual_response, callback);
    }

    void CallClientCallback(rpc::RpcController* controller,
                            EchoRequest* request,
                            EchoResponse* response) {
        LOG(INFO)
            << "request: " << controller->sequence_id()
            << ", message: " << request->message();
        LOG(INFO) << "request: " << request->message();
        if (controller->Failed()) {
            LOG(INFO) << "failed: " << controller->ErrorText();
        } else {
            LOG(INFO) << "response: " << response->message();
        }
        delete controller;
        delete request;
        delete response;
    }

    Atomic<int> m_dual_request_count;
    DualEchoClient::Stub* m_dual_echo_client;
    scoped_ptr<ThreadPool> m_background_thread_pool;
};

} // namespace rpc_examples

int main(int argc, char** argv)
{
    if (!CFlags::ParseCommandLine(argc, argv))
        return -1;

    rpc::HttpServer http_server(Flag_server_thread.Value());
    rpc::RpcService rpc_service(&http_server);
    rpc::RpcServerChannel server_channel(&rpc_service);

    rpc_examples::EchoServerImpl echo_server;
    rpc_service.RegisterService(&echo_server);

    rpc_examples::DualEchoClient::Stub dual_echo_client(&server_channel);
    rpc_examples::DualEchoServerImpl dual_echo_server(&dual_echo_client);
    rpc_service.RegisterService(&dual_echo_server);

    if (!http_server.StartServer(Flag_server_address.Value())) {
        return -1;
    }

    for (;;) {
        ThisThread::Sleep(100);
    }
    return 0;
}
