// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/16/11
// Description: test timestamp

#include "common/system/time/timestamp.hpp"
#include <time.h>
#include <sys/time.h>
#include "gtest/gtest.h"

extern int64_t SystemGetTimeStampInUs();

const int kLoopCount = 1000000;

#ifdef __unix__

#ifdef __i386
#define RDTSC_LL(llval) \
    __asm__ __volatile__("rdtsc" : "=A" (llval))

#elif defined __x86_64
#define RDTSC_LL(val) do { \
    unsigned int __a, __d; \
    __asm__ __volatile__("rdtsc" : "=a" (__a), "=d" (__d)); \
        (val) = ((int64_t)__a) | (((int64_t)__d)<<32); \
} while (0)
#endif

/// read cpu timestamp count
inline int64_t rdtsc()
{
    int64_t tsc;
    RDTSC_LL(tsc);
    return tsc;
}

TEST(Timestamp, rdtsc_order)
{
    int64_t t0 = 0;
    for (int i = 0; i < kLoopCount; ++i)
    {
        int64_t t = rdtsc();
        EXPECT_GT(t, t0);
        t0 = t;
    }
}

TEST(Timestamp, rdtsc)
{
    for (int i = 0; i < kLoopCount; ++i)
        rdtsc();
}

TEST(Timestamp, GetTimeStampInMs)
{
    for (int i = 0; i < kLoopCount; ++i)
    {
        GetTimeStampInMs();
    }
}

TEST(Timestamp, GetTimeStampInUs)
{
    for (int i = 0; i < kLoopCount; ++i)
    {
        GetTimeStampInUs();
    }
}

TEST(Timestamp, GetTimeStampInMsPrecision)
{
    const int kTestCount = kLoopCount;
    int64_t total_diff = 0;
    int64_t total_abs_diff = 0;
    for (int i = 0; i < kTestCount; ++i)
    {
        int64_t t0 = SystemGetTimeStampInUs();
        int64_t t1 = GetTimeStampInUs();
        int64_t t2 = SystemGetTimeStampInUs();
        if (t0 == t2 && t1 != t0)
        {
            int diff = t1 - t0;
            total_diff += diff;
            total_abs_diff += abs(diff);
        }
    }
    printf("average total diff=%g\n", (double)total_diff / kTestCount);
    printf("average abs diff %g\n", (double)total_abs_diff / kTestCount);
}

TEST(Timestamp, gettimeofday)
{
    struct timeval tv;
    for (int i = 0; i < kLoopCount; ++i)
    {
        gettimeofday(&tv, NULL);
    }
}

TEST(Timestamp, clock_gettime_CLOCK_REALTIME)
{
    struct timespec ts;
    for (int i = 0; i < kLoopCount; ++i)
    {
        clock_gettime(CLOCK_REALTIME, &ts);
    }
}

TEST(Timestamp, clock_gettime_CLOCK_MONOTONIC)
{
    struct timespec ts;
    for (int i = 0; i < kLoopCount; ++i)
    {
        clock_gettime(CLOCK_MONOTONIC, &ts);
    }
}

TEST(Timestamp, clock_gettime_CLOCK_PROCESS_CPUTIME_ID)
{
    struct timespec ts;
    for (int i = 0; i < kLoopCount; ++i)
    {
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    }
}

#endif

