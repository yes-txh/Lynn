#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>

#include "common/rpc/rpc.hpp"
#include "common/config/cflags.hpp"
#include "common/system/concurrency/atomic/atomic.hpp"

// substitute with EchoStub.h and EchoProxy.h
#include "common/rpc/examples/echo_stub.h"
#include "common/rpc/examples/echo_proxy.h"

/// 实现对象
using namespace Demo;
class Echo : public EchoStub<Echo>
{
public:
    Echo(Rpc::ObjectId_t id) : EchoStub<Echo>(id)
    {
        RequestCount = 0;
    }

    /// 同步方式实现 Echo0 方法
    void Echo0(std::string& str)
    {
        ++RequestCount;
    }

    /// 同步方式处理，同普通函数。
    std::string Echo1(const std::string& str)
    {
        ++RequestCount;
        return str;
    }

    std::string Get(size_t size)
    {
        ++RequestCount;
        return std::string(size, 'A');
    }

    void Set(const std::string& str)
    {
        ++RequestCount;
    }

    /// 同步方式处理，同普通函数。
    void ToUpper(std::string& str)
    {
        ++RequestCount;
        std::transform(str.begin(), str.end(), str.begin(), &toupper);
    }

    /// 异步方式实现调用的处理，函数原型固定。
    Rpc::Status_t Echo2(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        ++RequestCount;
        /// 取出参数
        std::string str;
        Rpc::Status_t status = context.ExtractInputParameters(buffer, str);

        if (status == Rpc::Status_Success)
            return context.Complete(str);
        else
            return context.Abort(status);
    }
    Atomic<unsigned int> RequestCount;
};

CFLAGS_DEFINE_FLAG(
    std::string,
    listen,
    cflags::DefaultIs("127.0.0.1:20000"),
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
    Echo echo(2);

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

    //Rpc::Terminate();

    return 0;
}
