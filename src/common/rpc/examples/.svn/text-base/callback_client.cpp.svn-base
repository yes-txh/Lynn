#define _CRT_NONSTDC_NO_WARNINGS 1

/// @example test.cpp
/// 演示如何使用 RPC 框架的测试程序

#include <stdio.h>
#include <common/rpc/rpc.hpp>

#include "common/rpc/examples/callback_proxy.h"
#include "common/rpc/examples/callback_stub.h"
#include "common/rpc/examples/callback_demo_proxy.h"
#include "common/rpc/examples/callback.hpp"

class Callback : public CallbackStub<Callback>
{
public:
    Callback(const Rpc::ObjectId_t& id) :
        CallbackStub<Callback>(id),
        m_callbacked(false)
    {
    }
    void OnCallback()
    {
        printf("This is callback\n");
        m_callbacked = true;
    }
    bool IsCallbacked() const
    {
        return m_callbacked;
    }
private:
    volatile bool m_callbacked;
};

int main()
{
    Rpc::Initialize();

    CallbackDemoProxy demo_proxy;
    Rpc::GetRemoteObject("127.0.0.1:20000", ID_CALLBACK_DEMO, demo_proxy);

    Callback callback(ID_CALLBACK);
    demo_proxy.RegisterCallback(callback);
    demo_proxy.RequestCallback();

    for (int i = 0; i < 3; ++i)
    {
        sleep(1);
        if (callback.IsCallbacked())
        {
            printf("Success\n");
            return 0;
        }
    }
    printf("Failure\n");

    return 1;
}
