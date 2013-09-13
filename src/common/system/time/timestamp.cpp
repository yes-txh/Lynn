// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/system/time/timestamp.hpp"

#include <stddef.h>

#ifdef _WIN32

#include <common/base/common_windows.h>

// Number of 100 nanosecond units from 1/1/1601 to 1/1/1970
#define EPOCH_BIAS  116444736000000000i64

int64_t GetTimeStampInUs()
{
    ULARGE_INTEGER uli;
    GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&uli));
    return (uli.QuadPart - EPOCH_BIAS) / 10;
}

int64_t GetTimeStampInMs()
{
    return GetTimeStampInUs() / 1000;
}

#endif

#ifdef __unix__

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

/// get system time, in microseconds
int64_t SystemGetTimeStampInUs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t result = tv.tv_sec;
    result *= 1000000;
    result += tv.tv_usec;
    return result;
}

/// read cpu timestamp count
static inline int64_t rdtsc()
{
    int64_t tsc;
#if defined __i386__
    __asm__ __volatile__("rdtsc" : "=A" (tsc));
#elif defined __x86_64
    unsigned int a, d;
    __asm__ __volatile__("rdtsc" : "=a" (a), "=d" (d));
    tsc = static_cast<int64_t>(a) | (static_cast<int64_t>(d) << 32);
#else
#error unsupported platform
#endif
    return tsc;
}

/// @brief get both timestamp and tsc
/// @retval true no context switch, timestamp and tsc same timeslice
/// @retval false context switched, only timestamp is trustable
static bool get_timestamp_and_tsc(int64_t* timestamp, int64_t* tsc)
{
    int64_t tsc1 = rdtsc();
    *timestamp = SystemGetTimeStampInUs();
    int64_t tsc2 = rdtsc();

    // the difference is typical several hundreds if no context switching
    if (tsc2 > tsc1 && tsc2 - tsc1 < 5000)
    {
        // no context switched between 2 rdtsc() call
        // rounded avarage value
        *tsc = tsc1 + (tsc2 - tsc1 + 1) / 2;
        return true;
    }
    else
    {
        // context switched between 2 rdtsc() callings,
        // can not decide which tsc is useable
        return false;
    }
}

// using tsc to optimize timestamp, adjust if necessary
int64_t FastGetTimeStampInUs()
{
    static __thread int64_t last_timestamp = 0; // last sample time
    static __thread int64_t last_tsc = 0; // last sample time tsc

    int64_t tsc;
    int64_t timestamp;

    if (last_timestamp != 0)
    {
        const int64_t READJUST_INTERVAL = 1000; // 1ms
        static __thread int64_t readjust_tsc_interval = 0;

        // using integer multiple and shift to eliminate float overhead
        const int64_t SHIFT = 32;
        static __thread int64_t shifted_us_per_tsc = 0;

        tsc = rdtsc();
        if (tsc - last_tsc < readjust_tsc_interval)
        {
            int64_t tsc_diff = tsc - last_tsc;
            int64_t shifted_diff = tsc_diff * shifted_us_per_tsc + (1LL << (SHIFT - 1));
            int64_t timestamp_diff = shifted_diff >> SHIFT;
            timestamp = last_timestamp + timestamp_diff;
        }
        else
        {
            // quite long after last gettimestamp() call, readjust
            if (get_timestamp_and_tsc(&timestamp, &tsc))
            {
                if (timestamp - last_timestamp > 0)
                {
                    // recalculate tsc ratio
                    double timestamp_diff = timestamp - last_timestamp;
                    double tsc_diff = tsc - last_tsc;
                    double us_per_tsc = timestamp_diff / tsc_diff;
                    shifted_us_per_tsc = us_per_tsc * (1LL << SHIFT) + 0.5;
                    if (timestamp - last_timestamp > READJUST_INTERVAL)
                        readjust_tsc_interval = tsc_diff * READJUST_INTERVAL / timestamp_diff;
                    else
                        readjust_tsc_interval = 2 * tsc_diff;

                    // update last adjusted time
                    last_timestamp = timestamp;
                    last_tsc = tsc;
                }
            }
        }
    }
    else
    {
        // first call, last timestamp is unknown
        if (get_timestamp_and_tsc(&timestamp, &tsc))
        {
            last_timestamp = timestamp;
            last_tsc = tsc;
        }
    }

    return timestamp;
}

// if kernel support vsyscall64, time call will be 10 times faster.
//  check it.
int get_vsyscall64_value()
{
    int result = -1;
    int fd = open("/proc/sys/kernel/vsyscall64", O_RDONLY, 0);
    if (fd >= 0)
    {
        char buf[2];
        int num_read = read(fd, buf, 2);
        if (num_read >= 1)
        {
            switch (buf[0])
            {
            case '0':
            case '1':
            case '2':
                result = buf[0] - '0';
            }
        }
        close(fd);
    }
    return result;
}

int64_t GetTimeStampInUs()
{
    // static const int vsyscall64 = get_vsyscall64_value();
    return FastGetTimeStampInUs();
}

int64_t GetTimeStampInMs()
{
    int64_t timestamp = GetTimeStampInUs();
    // round to nearest
    return (timestamp + 500) / 1000;
}

#endif // __unix__

