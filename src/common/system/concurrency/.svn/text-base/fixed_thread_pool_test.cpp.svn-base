// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/system/concurrency/fixed_thread_pool.hpp"
#include <gtest/gtest.h>

// GLOBAL_NOLINE(runtime/int)

class Foo
{
public:
    void test1()
    {
    }
    void test2(intptr_t param)
    {
    }
};

TEST(MemberFunctionTest, MemberFunction)
{
    FixedThreadPool threadpool(4);
    Foo foo;
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 20; ++j)
        {
            threadpool.AddTask(MAKE_THREAD_CALLBACK(Foo, test1), &foo);
            threadpool.AddTask(
                MAKE_PARAMETERIZED_THREAD_CALLBACK(Foo, test2, intptr_t), &foo, i * 20 + j);
            FixedThreadPool::Stats stats;
            threadpool.GetStats(&stats);
            // printf("%d: NumThreads=%zu, NumBusyThreads=%zu, NumPengdingTasks=%zu\n",
            //       i*20+j, stat.NumThreads, stat.NumBusyThreads, stat.NumPendingTasks);
        }
    }
}

TEST(MemberFunctionTest, Closure)
{
    FixedThreadPool threadpool(4);
    Foo foo;
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 20; ++j)
        {
            threadpool.AddTask(NewClosure(&foo, &Foo::test1));
            threadpool.AddTask(
                NewClosure(
                    &foo, &Foo::test2, static_cast<intptr_t>(i*20+j)));
            FixedThreadPool::Stats stats;
            threadpool.GetStats(&stats);
            // printf("%d: NumThreads=%zu, NumBusyThreads=%zu, NumPengdingTasks=%zu\n",
            //       i*20+j, stat.NumThreads, stat.NumBusyThreads, stat.NumPendingTasks);
        }
    }
}

void blocking(void*, unsigned long long)
{
    ThisThread::Sleep(1000);
}

void nonblocking(void*, unsigned long long)
{
    ThisThread::Sleep(1);
}

TEST(BlockingTest, Blocking)
{
    FixedThreadPool threadpool;
    threadpool.AddTask(blocking);
    threadpool.AddTask(blocking);
    threadpool.AddTask(blocking);
    for (int i = 0; i < 10; ++i)
    {
        threadpool.AddTask(nonblocking);
    }
}

void test(void*, unsigned long long param)
{
}

TEST(SlowCallTest, SlowCall)
{
    FixedThreadPool threadpool;
    for (int i = 0; i < 4; ++i)
    {
        threadpool.AddTask(test, NULL, i);
        ThisThread::Sleep(1000);
    }
}

TEST(GlobalFunctionTest, GlobalFunction)
{
    FixedThreadPool threadpool(4);
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 20; ++j)
        {
            threadpool.AddTask(test, NULL, i*20+j);
            FixedThreadPool::Stats stats;
            threadpool.GetStats(&stats);
        }
    }
}

