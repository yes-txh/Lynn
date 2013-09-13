#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>

#include "common/rpc/rpc.hpp"
#include "common/config/cflags.hpp"
#include "common/system/concurrency/atomic/atomic.hpp"
#include "common/system/time/timestamp.hpp"

#include "common/rpc/examples/nested_stub.h"
#include "common/rpc/examples/nested_proxy.h"
#include "common/rpc/examples/echo_proxy.h"

/// 实现对象
using namespace Demo;

CFLAGS_DEFINE_FLAG(
        std::string, server_address, cflags::DefaultIs("127.0.0.1:20000"),
        "server address, in host:port format"
        );

CFLAGS_DEFINE_FLAG(
        std::string,
        listen,
        cflags::DefaultIs("127.0.0.1:10000"),
        "listen address, in host:port format"
        );

class NestedTest : public NestedTestStub<NestedTest>
{
    public:
    NestedTest(Rpc::ObjectId_t id) : NestedTestStub<NestedTest>(id)
    {
        RequestCount = 0;
        TotalCount = 0;
    }
    /// 异步方式实现调用的处理，函数原型固定。
    Rpc::Status_t Echo(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        ++RequestCount;
        ++TotalCount;
#if 0
        int64_t t = GetTimeStampInUs();
        while (GetTimeStampInUs() - t < 70)
        {
        }
#endif
        EchoProxy echo;
        Rpc::GetRemoteObject(Flag_server_address, 2, echo);
        std::string message = "hello";
        echo.Echo0(message, NULL, 100000);

        /// 取出参数
        std::string str;
        Rpc::Status_t status = context.ExtractInputParameters(buffer, str);
        if (status == Rpc::Status_Success)
            return context.Complete(str);
        else
            return context.Abort(status);
    }
public:
    Atomic<unsigned int> RequestCount;
    Atomic<unsigned int> TotalCount;
};

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
    NestedTest test(1);

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
        printf("rate %u/s\n", test.RequestCount.Value());
        printf("total: %u\n", test.TotalCount.Value());
        test.RequestCount = 0;
    }
    return 0;
}
