#define _CRT_NONSTDC_NO_WARNINGS 1

/// @example test.cpp
/// 演示如何使用 RPC 框架的测试程序

#include <stdio.h>
#include <common/rpc/rpc.hpp>

#include "common/rpc/examples/test_stub.h"

class Test : public TestStub<Test>
{
public:

    Test(Rpc::ObjectId_t id):
        TestStub<Test>(id)
    {
    }

    void Nop() {}

    int Return0()
    {
        return 0;
    }

    int Next(int n)
    {
        return n + 1;
    }
    void Inc(int& n)
    {
        ++n;
    }
    int Add2(int a, int b)
    {
        return a + b;
    }
    int Add3(int a, int b, int c)
    {
        return a + b + c;
    }

    int Add4(int a, int b, int c, int d)
    {
        return a + b + c + d;
    }

    int Add5(int a, int b, int c, int d, int e)
    {
        return a + b + c + d + e;
    }

    int Add6(int a, int b, int c, int d, int e, int f)
    {
        return a + b + c + d + e + f;
    }

    int Add7(int a, int b, int c, int d, int e, int f, int g)
    {
        return a + b + c + d + e + f + g;
    }

    int Add8(int a, int b, int c, int d, int e, int f, int g, int h)
    {
        return a + b + c + d + e + f + g + h;
    }

    // 以下为异步实现
    Rpc::Status_t A_Nop(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        context.Complete();
        return Rpc::Status_Success;
    }

    Rpc::Status_t A_Return0(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        context.Complete(0);
        return Rpc::Status_Success;
    }
    Rpc::Status_t A_Next(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        int n;
        Rpc::Status_t status = context.ExtractInputParameters(buffer, n);
        if (status != Rpc::Status_Success)
            return status;
        return context.Complete(n + 1);
    }
    Rpc::Status_t A_Inc(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        int n;
        Rpc::Status_t status = context.ExtractInputParameters(buffer, n);
        if (status != Rpc::Status_Success)
            return status;
        return context.Complete(n + 1);
    }
    Rpc::Status_t A_Add2(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        int a, b;
        Rpc::Status_t status = context.ExtractInputParameters(buffer, a, b);
        if (status != Rpc::Status_Success)
            return status;
        return context.Complete(a + b);
    }
    Rpc::Status_t A_Add3(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        int a, b, c;
        Rpc::Status_t status = context.ExtractInputParameters(buffer, a, b, c);
        if (status != Rpc::Status_Success)
            return status;
        return context.Complete(a + b + c);
    }
    Rpc::Status_t A_Add4(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        int a, b, c, d;
        Rpc::Status_t status = context.ExtractInputParameters(buffer, a, b, c, d);
        if (status != Rpc::Status_Success)
            return status;
        return context.Complete(a + b + c + d);
    }
    Rpc::Status_t A_Add5(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        int a, b, c, d, e;
        Rpc::Status_t status = context.ExtractInputParameters(buffer, a, b, c, d, e);
        if (status != Rpc::Status_Success)
            return status;
        return context.Complete(a + b + c + d + e);
    }

    Rpc::Status_t A_Add6(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        int a, b, c, d, e, f;
        Rpc::Status_t status = context.ExtractInputParameters(buffer, a, b, c, d, e, f);
        if (status != Rpc::Status_Success)
            return status;
        return context.Complete(a + b + c + d + e + f);
    }

    Rpc::Status_t A_Add7(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        int a, b, c, d, e, f, g;
        Rpc::Status_t status = context.ExtractInputParameters(buffer, a, b, c, d, e, f, g);
        if (status != Rpc::Status_Success)
            return status;
        return context.Complete(a + b + c + d + e + f + g);
    }

    Rpc::Status_t A_Add8(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        int a, b, c, d, e, f, g, h;
        Rpc::Status_t status = context.ExtractInputParameters(buffer, a, b, c, d, e, f, g, h);
        if (status != Rpc::Status_Success)
            return status;
        return context.Complete(a + b + c + d + e + f + g + h);
    }

    Rpc::Status_t Div(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer)
    {
        int numer, denom;
        Rpc::Status_t status = context.ExtractInputParameters(buffer, numer, denom);
        if (status != Rpc::Status_Success)
            return status;
        if (denom != 0)
        {
            div_t d = div(numer, denom);
            return context.Complete(d.quot, d.rem, true);
        }
        else
        {
            return context.CompleteError(false);
        }
    }
};

#ifdef _WIN32
#include <process.h>
#elif defined unix
#include <unistd.h>
#endif

#include "common/rpc/examples/Test.hpp"

int main()
{
    Rpc::Initialize();

    Test test(ID_TEST);

    Rpc::Listen(std::string("127.0.0.1:30000"));

    for (;;)
    {
        sleep(1);
    }

    return 0;
}
