// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/system/concurrency/thread.hpp"
#include "common/system/time/stopwatch.hpp"
#include "gtest/gtest.h"

TEST(StartStopTest, StartStop)
{
    Stopwatch sw;
    ASSERT_TRUE(sw.IsRunning());
    sw.Stop();
    ASSERT_TRUE(!sw.IsRunning());
}

TEST(TimerTest, Timer)
{
    Stopwatch sw;

    ThisThread::Sleep(100);
    int64_t t = sw.ElapsedMilliSeconds();
    ASSERT_LT(abs(static_cast<int>(t - 100)), 10);

    ThisThread::Sleep(100);
    t = sw.ElapsedMilliSeconds();
    ASSERT_LT(abs(static_cast<int>(t - 200)), 10);
}

TEST(StopTest, Stop)
{
    Stopwatch sw;

    ThisThread::Sleep(100);
    sw.Stop();
    int64_t t = sw.ElapsedMilliSeconds();
    ASSERT_LT(abs(static_cast<int>(t - 100)), 10);

    /// 停止后不计时
    ThisThread::Sleep(100);
    int64_t t1 = sw.ElapsedMilliSeconds();
    ASSERT_EQ(t, t1);
}

TEST(ResetTest, Reset)
{
    Stopwatch sw;
    ThisThread::Sleep(100);
    int64_t t = sw.ElapsedMilliSeconds();
    ASSERT_LT(abs(static_cast<int>(t - 100)), 10);

    sw.Reset();
    ASSERT_TRUE(!sw.IsRunning());
    ASSERT_EQ(sw.ElapsedMilliSeconds(), 0);
}

TEST(RestartTest, Restart)
{
    Stopwatch sw;

    ThisThread::Sleep(100);
    int64_t t = sw.ElapsedMilliSeconds();
    ASSERT_LT(abs(static_cast<int>(t - 100)), 10);

    sw.Restart();
    ASSERT_TRUE(sw.IsRunning());
    ThisThread::Sleep(100);

    t = sw.ElapsedMilliSeconds();
    ASSERT_LT(abs(static_cast<int>(t - 100)), 10);
}

TEST(SecondsTest, Seconds)
{
    Stopwatch sw;
    ThisThread::Sleep(100);
    double t = sw.ElapsedSeconds();
    ASSERT_LT(abs(t*1000 - 100), 10);
}

TEST(MilliSecondsTest, MilliSeconds)
{
    Stopwatch sw;
    ThisThread::Sleep(100);
    sw.Stop();
    int64_t t = sw.ElapsedMicroSeconds();
    ASSERT_EQ(t / 1000, sw.ElapsedMilliSeconds());
}
