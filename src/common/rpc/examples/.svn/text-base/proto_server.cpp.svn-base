#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>

#include "common/rpc/rpc.hpp"
#include "common/config/cflags.hpp"
#include "common/system/concurrency/atomic/atomic.hpp"

#include "common/rpc/examples/proto_stub.h"
#include "common/rpc/examples/proto_proxy.h"
#include "common/rpc/examples/addressbook.pb.h"

/// 实现对象
using namespace Demo;
class Echo : public ProtobufTestStub<Echo>
{
public:
    Echo(Rpc::ObjectId_t id) : ProtobufTestStub<Echo>(id)
    {
        RequestCount = 0;
    }

    /// 同步方式实现 Echo0 方法
    void Echo0(AddressBook& address)
    {
        ++RequestCount;
    }

    /// 同步方式处理，同普通函数。
    AddressBook Echo1(const AddressBook& address)
    {
        ++RequestCount;
        return address;
    }

    /// 同步方式处理，同普通函数。
    void ToUpper(AddressBook* address)
    {
        ++RequestCount;
    }

    /// 异步方式实现调用的处理，函数原型固定。
    Rpc::Status_t Echo2(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        ++RequestCount;
        /// 取出参数
        AddressBook address;
        Rpc::Status_t status = context.ExtractInputParameters(buffer, address);

        if (status == Rpc::Status_Success)
            return context.Complete(address);
        else
            return context.Abort(status);
    }
    Atomic<unsigned int> RequestCount;
};

CFLAGS_DEFINE_FLAG(
    std::string,
    listen,
    cflags::DefaultIs("127.0.0.1:40000"),
    "listen address, in host:port format"
);

volatile bool g_quit;

void sighandler(int)
{
    g_quit = true;
}

int main(int argc, char** argv)
{
    if (!CFlags::ParseCommandLine(argc, argv))
        return EXIT_FAILURE;

    Rpc::Initialize();
    Echo echo(1);

    Rpc::Status_t status = Rpc::Listen(Flag_listen);
    if (status != Rpc::Status_Success)
    {
        fprintf(stderr, "Listen error: %s, %s\n", Rpc::StatusString(status), strerror(errno));
        return EXIT_FAILURE;
    }

    signal(SIGINT, sighandler);

    while (!g_quit)
    {
        ThisThread::Sleep(1000);
        printf("rate %u/s\n", echo.RequestCount.Value());
        echo.RequestCount = 0;
    }

    return 0;
}
