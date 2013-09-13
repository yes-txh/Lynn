/// @example test.cpp
/// 演示如何使用 RPC 框架的测试程序

#include <common/rpc/rpc.hpp>
#include "common/rpc/examples/Test.hpp"
#include <gtest/gtest.h>

#include "common/rpc/examples/test_proxy.h"

#ifdef _WIN32
#include <process.h>
#elif defined unix
#include <unistd.h>
#endif

#include <sys/time.h>
long long time_stamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000LL + tv.tv_usec;
}

int callback_count;
Mutex mutex;
ConditionVariable cond;

void OnReturn0Complete(void* context, Rpc::Status_t status, void* param)
{
    printf("*****************************************%s\n", __func__);
}

void OnAsyncNextComplete(void* context, Rpc::Status_t status, void* param)
{
    MutexLocker locker(mutex);

    printf("%lld %s: status=%s, param=%d\n",
           time_stamp(),
           __func__,
           Rpc::StatusString(status), (int)(intptr_t)context
          );
    TestProxy* proxy = (TestProxy*) context;
    try
    {
        Rpc::AsyncTokenOf<int> token;
        if (proxy->AsyncReturn0(&token, OnReturn0Complete) == Rpc::Status_Pending)
            token.Wait();
    }
    catch (...)
    {
        ASSERT_FALSE(true);
    }
    ++callback_count;
    cond.Signal();
}

TEST(Rpc, Timeout)
{
    MutexLocker locker(mutex);

    TestProxy test_proxy;
    Rpc::GetRemoteObject("127.0.0.1:30000", ID_TEST, test_proxy);

    Rpc::AsyncTokenOf<int> token1;
    test_proxy.AsyncNext(1, &token1, OnAsyncNextComplete, &test_proxy, &token1);
    printf("%lld 1\n", time_stamp());

    Rpc::AsyncTokenOf<int> token2;
    test_proxy.AsyncNext(2, &token2, OnAsyncNextComplete, &test_proxy, &token2);
    printf("%lld 2\n", time_stamp());

    while (callback_count != 2)
        cond.Wait(mutex);
    printf("%lld 3\n", time_stamp());
}

TEST(Rpc, SyncCall)
{
    TestProxy test_proxy;
    Rpc::GetRemoteObject("127.0.0.1:30000", ID_TEST, test_proxy);

    test_proxy.Nop();
    ASSERT_TRUE(test_proxy.Return0() == 0);

    int n = 0;
    test_proxy.Inc(n);
    ASSERT_TRUE(n == 1) << "n = " << n;
    ASSERT_TRUE(test_proxy.Next(n, NULL, 5000) == 2);
    ASSERT_TRUE(test_proxy.Add2(1, 2) == 3);
    ASSERT_TRUE(test_proxy.Add3(1, 2, 3) == 6);
    ASSERT_TRUE(test_proxy.Add4(1, 2, 3, 4) == 10);
    ASSERT_TRUE(test_proxy.Add5(1, 2, 3, 4, 5) == 15);
    ASSERT_TRUE(test_proxy.Add6(1, 2, 3, 4, 5, 6) == 21);
    ASSERT_TRUE(test_proxy.Add7(1, 2, 3, 4, 5, 6, 7) == 28);
    ASSERT_TRUE(test_proxy.Add8(1, 2, 3, 4, 5, 6, 7, 8) == 36);
}

TEST(Rpc, Performance)
{
    TestProxy test_proxy;
    Rpc::GetRemoteObject("127.0.0.1:30000", ID_TEST, test_proxy);

    long long t = time_stamp();
    for (int i = 0; i < 100000; ++i)
        test_proxy.Nop();
    t = time_stamp() - t;
    //UNIT_TEST_INFO("t=%lld", t);
}

TEST(Rpc, AsyncCall)
{
    TestProxy test_proxy;
    Rpc::GetRemoteObject("127.0.0.1:30000", ID_TEST, test_proxy);


    test_proxy.A_Nop();
    ASSERT_TRUE(test_proxy.A_Return0() == 0);

    int n = 0;
    test_proxy.A_Inc(n);
    ASSERT_TRUE(n == 1) << "n = " << n;
    ASSERT_TRUE(test_proxy.A_Next(n) == 2);
    ASSERT_TRUE(test_proxy.A_Add2(1, 2) == 3);
    ASSERT_TRUE(test_proxy.A_Add3(1, 2, 3) == 6);
    ASSERT_TRUE(test_proxy.A_Add4(1, 2, 3, 4) == 10);
    ASSERT_TRUE(test_proxy.A_Add5(1, 2, 3, 4, 5) == 15);
    ASSERT_TRUE(test_proxy.A_Add6(1, 2, 3, 4, 5, 6) == 21);
    ASSERT_TRUE(test_proxy.A_Add7(1, 2, 3, 4, 5, 6, 7) == 28);
    ASSERT_TRUE(test_proxy.A_Add8(1, 2, 3, 4, 5, 6, 7, 8) == 36);

    int quot, rem;
    bool b = test_proxy.Div(12, 5, quot, rem);
    ASSERT_TRUE(b == true);
    ASSERT_TRUE(quot == 2);
    ASSERT_TRUE(rem == 2);

    b = test_proxy.Div(1, 0, quot, rem);
    ASSERT_TRUE(b == false);
}

int main(int argc, char** argv)
{
    Rpc::Initialize();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
