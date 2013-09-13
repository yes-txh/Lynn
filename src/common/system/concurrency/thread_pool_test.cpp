// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#undef __DEPRECATED
#include "common/system/concurrency/thread_pool.hpp"
#include "common/system/concurrency/atomic/atomic.h"
#include "common/system/time/timestamp.hpp"
#include "gtest/gtest.h"

void test_blocking(void*, unsigned long long)
{
    ThisThread::Sleep(1000);
}

void test_nonblocking(void*, unsigned long long)
{
}

TEST(ThreadPool, MessTasks)
{
    ThreadPool threadpool(0, 4);
    for (int i = 0; i < 10000; ++i)
    {
        threadpool.AddTask(test_nonblocking);
    }
}

TEST(ThreadPool, AfterBusy)
{
    ThreadPool threadpool(0, 4);
    for (int i = 0; i < 1000; ++i)
    {
        threadpool.AddTask(test_nonblocking);
    }
    ThisThread::Sleep(100);
    for (int i = 0; i < 1000; ++i)
    {
        threadpool.AddTask(test_nonblocking);
    }
}

TEST(ThreadPool, MixedBlockingAndNonblocking)
{
    ThreadPool threadpool(0, 4);
    threadpool.AddTask(test_blocking);
    threadpool.AddTask(test_blocking);
    threadpool.AddTask(test_blocking);
    for (int i = 0; i < 10; ++i)
    {
        threadpool.AddTask(test_nonblocking);
    }
}

TEST(ThreadPool, Blocking)
{
    ThreadPool threadpool(0, 4);
    threadpool.AddTask(test_blocking);
    threadpool.AddTask(test_blocking);
    threadpool.AddTask(test_blocking);
}

void test_sleep(void*, unsigned long long param)
{
    ThisThread::Sleep(10);
}

TEST(ThreadPool, SlowCall)
{
    ThreadPool threadpool;
    for (int i = 0; i < 10; ++i)
    {
        threadpool.AddTask(test_sleep, NULL, i);
    }
}

TEST(ThreadPool, GlobalFunction)
{
    ThreadPool threadpool(4, 4);
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 20; ++j)
        {
            threadpool.AddTask(test_sleep, NULL, i*20+j);
            ThreadPool::Stats stats;
            threadpool.GetStats(&stats);
        }
    }
}

class Foo
{
public:
    void test1()
    {
        ThisThread::Sleep(10);
    }
    void test2(intptr_t param)
    {
        ThisThread::Sleep(10);
    }
};

TEST(ThreadPool, MemberFunction)
{
    ThreadPool threadpool(4, 4);
    Foo foo;
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 20; ++j)
        {
            threadpool.AddTask(MAKE_THREAD_CALLBACK(Foo, test1), &foo);
            threadpool.AddTask(
                MAKE_PARAMETERIZED_THREAD_CALLBACK(Foo, test2, intptr_t), &foo, i*20+j);
            ThreadPool::Stats stats;
            threadpool.GetStats(&stats);
        }
    }
}

TEST(ThreadPool, Closure)
{
    ThreadPool threadpool(4, 4);
    Foo foo;
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 20; ++j)
        {
            threadpool.AddTask(NewClosure(&foo, &Foo::test1));
            threadpool.AddTask(
                NewClosure(
                    &foo, &Foo::test2, static_cast<intptr_t>(i*20+j)));
            ThreadPool::Stats stats;
            threadpool.GetStats(&stats);
        }
    }
}

TEST(ThreadPool, Function)
{
    ThreadPool threadpool;
    Foo foo;
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 20; ++j)
        {
            threadpool.AddTask(NewClosure(&foo, &Foo::test1));
            threadpool.AddTask(
                Bind(&Foo::test2, &foo, static_cast<intptr_t>(i*20+j)));
            ThreadPool::Stats stats;
            threadpool.GetStats(&stats);
        }
    }
}

const int kLoopCount = 5000000;
int n0 = 0;
int64_t t0;

static void Inc(void* p, unsigned long long)
{
    int& n = *(int*)p;
    if (AtomicIncrement(n) == kLoopCount)
    {
        int64_t t = GetTimeStampInUs();
        double tps = 1000000.0 * (n - n0) / (t - t0);
        printf("n0 = %d\n", n0);
        printf("t/s = %g\n", tps);
    }
}

TEST(ThreadPool, Performance)
{
    int n = 0;
    ThreadPool& threadpool = ThreadPool::DefaultInstance();
    for (int i = 0; i < kLoopCount; ++i)
        threadpool.AddTask(Inc, &n, 0);
    n0 = n;
    t0 = GetTimeStampInUs();

    threadpool.WaitForIdle();
}

