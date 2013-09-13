/// @example test.cpp
/// 演示如何使用 RPC 框架的测试程序

#include <common/rpc/rpc.hpp>

RPC_BEGIN_PROXY_CLASS(Test)
RPC_PROXY_METHOD_0(void, Nop, 1)
RPC_PROXY_METHOD_0(int, Return0, 1)
RPC_PROXY_METHOD_1(int, Next, int, 1)
RPC_PROXY_METHOD_1(void, Inc, int&, 1)
RPC_PROXY_METHOD_2(int, Add2, int, int, 1)
RPC_PROXY_METHOD_3(int, Add3, int, int, int, 1)
RPC_PROXY_METHOD_4(int, Add4, int, int, int, int, 1)
RPC_PROXY_METHOD_5(int, Add5, int, int, int, int, int, 1)
RPC_PROXY_METHOD_6(int, Add6, const int&, const int&, const int&, const int&, const int&, const int&, 1)

RPC_PROXY_METHOD_0(void, A_Nop, 1)
RPC_PROXY_METHOD_0(int, A_Return0, 1)
RPC_PROXY_METHOD_1(int, A_Next, int, 1)
RPC_PROXY_METHOD_1(void, A_Inc, int&, 1)
RPC_PROXY_METHOD_2(int, A_Add2, int, int, 1)
RPC_PROXY_METHOD_3(int, A_Add3, int, int, int, 1)
RPC_PROXY_METHOD_4(int, A_Add4, int, int, int, int, 1)
RPC_PROXY_METHOD_5(int, A_Add5, int, int, int, int, int, 1)
RPC_PROXY_METHOD_6(int, A_Add6, const int&, const int&, const int&, const int&, const int&, const int&, 1)
RPC_END_PROXY_CLASS()

/// 定义桩类
RPC_BEGIN_STUB_CLASS(Test)
RPC_STUB_METHOD_0(void, Nop)
RPC_STUB_METHOD_0(int, Return0)
RPC_STUB_METHOD_1(int, Next, int)
RPC_STUB_METHOD_1(void, Inc, int&)
RPC_STUB_METHOD_2(int, Add2, int, int)
RPC_STUB_METHOD_3(int, Add3, int, int, int)
RPC_STUB_METHOD_4(int, Add4, int, int, int, int)
RPC_STUB_METHOD_5(int, Add5, int, int, int, int, int)
RPC_STUB_METHOD_6(int, Add6, const int&, const int&, const int&, const int&, const int&, const int&)

RPC_STUB_ASYNC_METHOD_0(void, A_Nop)
RPC_STUB_ASYNC_METHOD_0(int, A_Return0)
RPC_STUB_ASYNC_METHOD_1(int, A_Next, int)
RPC_STUB_ASYNC_METHOD_1(void, A_Inc, int&)
RPC_STUB_ASYNC_METHOD_2(int, A_Add2, int, int)
RPC_STUB_ASYNC_METHOD_3(int, A_Add3, int, int, int)
RPC_STUB_ASYNC_METHOD_4(int, A_Add4, int, int, int, int)
RPC_STUB_ASYNC_METHOD_5(int, A_Add5, int, int, int, int, int)
RPC_STUB_ASYNC_METHOD_6(int, A_Add6, const int&, const int&, const int&, const int&, const int&, const int&)
RPC_END_STUB_CLASS()

/// 定义方法调度表
RPC_BEGIN_STUB_DISPATCH(Test)
RPC_STUB_DISPATCH(Nop, 0)
RPC_STUB_DISPATCH(Return0, 0)
RPC_STUB_DISPATCH(Next, 1)
RPC_STUB_DISPATCH(Inc, 1)
RPC_STUB_DISPATCH(Add2, 2)
RPC_STUB_DISPATCH(Add3, 3)
RPC_STUB_DISPATCH(Add4, 4)
RPC_STUB_DISPATCH(Add5, 5)
RPC_STUB_DISPATCH(Add6, 6)

RPC_STUB_DISPATCH(A_Nop, 0)
RPC_STUB_DISPATCH(A_Return0, 0)
RPC_STUB_DISPATCH(A_Next, 1)
RPC_STUB_DISPATCH(A_Inc, 1)
RPC_STUB_DISPATCH(A_Add2, 2)
RPC_STUB_DISPATCH(A_Add3, 3)
RPC_STUB_DISPATCH(A_Add4, 4)
RPC_STUB_DISPATCH(A_Add5, 5)
RPC_STUB_DISPATCH(A_Add6, 6)
RPC_END_STUB_DISPATCH()

class Test : public TestStub<Test>
{
public:
    void Nop() {}

    int Return0()
    {
        return 100;
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
};

#include "common/rpc/examples/LoopbackRpcChannel.hpp"

void TestSync(TestProxy& test_proxy)
{
    int n = 0;
    test_proxy.Nop();
    n = test_proxy.Return0();
    test_proxy.Inc(n);
    n = test_proxy.Next(n);
    n = test_proxy.Add2(1, 2);
    n = test_proxy.Add3(1, 2, 3);
    n = test_proxy.Add4(1, 2, 3, 4);
    n = test_proxy.Add5(1, 2, 3, 4, 5);
    n = test_proxy.Add6(1, 2, 3, 4, 5, 6);
}

void TestAsync(TestProxy& test_proxy)
{
    int n = 0;
    test_proxy.A_Nop();
    n = test_proxy.A_Return0();
    test_proxy.A_Inc(n);
    n = test_proxy.A_Next(n);
    n = test_proxy.A_Add2(1, 2);
    n = test_proxy.A_Add3(1, 2, 3);
    n = test_proxy.A_Add4(1, 2, 3, 4);
    n = test_proxy.A_Add5(1, 2, 3, 4, 5);
    n = test_proxy.A_Add6(1, 2, 3, 4, 5, 6);
}

RPC_BEGIN_PROXY_CLASS(Callback)
RPC_PROXY_METHOD_0(void, OnCallback, 1)
RPC_END_PROXY_CLASS()

RPC_BEGIN_STUB_CLASS(Callback)
RPC_STUB_METHOD_0(void, OnCallback)
RPC_END_STUB_CLASS()

RPC_BEGIN_STUB_DISPATCH(Callback)
RPC_STUB_DISPATCH(OnCallback, 0)
RPC_END_STUB_DISPATCH()


class Callback : public CallbackStub<Callback>
{
public:
    void OnCallback()
    {

    }
};


RPC_BEGIN_PROXY_CLASS(CallbackDemo)
RPC_PROXY_METHOD_1(void, RegisterCallback, const Callback&, 1)
RPC_PROXY_METHOD_0(void, RequestCallback, 1)
RPC_END_PROXY_CLASS()

RPC_BEGIN_STUB_CLASS(CallbackDemo)
RPC_STUB_METHOD_1(void, RegisterCallback, const CallbackProxy&)
RPC_STUB_METHOD_0(void, RequestCallback)
RPC_END_STUB_CLASS()

RPC_BEGIN_STUB_DISPATCH(CallbackDemo)
RPC_STUB_DISPATCH(RegisterCallback, 1)
RPC_STUB_DISPATCH(RequestCallback, 0)
RPC_END_STUB_DISPATCH()

class CallbackDemo : public CallbackDemoStub<CallbackDemo>
{
public:
    void RegisterCallback(const CallbackProxy& callback)
    {
        m_Callback = callback;
    }
    void RequestCallback()
    {
        m_Callback.OnCallback();
    }
private:
    CallbackProxy m_Callback;
};

void TestCallback()
{
    CallbackDemo demo;
    Callback callback;

    CallbackDemoProxy demo_proxy;
    Rpc::GetRemoteObject("loopback", demo.RpcObjectId(), demo_proxy);

    demo_proxy.RegisterCallback(callback);
    demo_proxy.RequestCallback();
}

int main()
{
    LoopbackRpcChannel channel;
    Rpc::Initialize(&channel);

    Test test;

    TestProxy test_proxy;
    Rpc::GetRemoteObject("loopback", test.RpcObjectId(), test_proxy);

    TestSync(test_proxy);
    TestAsync(test_proxy);
    TestCallback();
}

