// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "Executor.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ;

class ExecutorHandler : virtual public ExecutorIf {
 public:
  ExecutorHandler() {
    // Your initialization goes here
  }

  int32_t Helloworld() {
    // Your implementation goes here
    printf("Helloworld\n");
  }

  void SendVMHeartbeat(const std::string& heartbeat_ad) {
    // Your implementation goes here
    printf("SendVMHeartbeat\n");
  }

  bool StartTask(const std::string& task_ad) {
    // Your implementation goes here
    printf("StartTask\n");
  }

  bool StopTask(const int32_t job_id, const int32_t task_id) {
    // Your implementation goes here
    printf("StopTask\n");
  }

  bool KillTask(const int32_t job_id, const int32_t task_id) {
    // Your implementation goes here
    printf("KillTask\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<ExecutorHandler> handler(new ExecutorHandler());
  shared_ptr<TProcessor> processor(new ExecutorProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

