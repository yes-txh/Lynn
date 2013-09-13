// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/31/11

#include "common/system/concurrency/this_thread.hpp"

#include <string.h>

#if __unix__
#include <pthread.h>
#include <unistd.h>

#ifdef __linux__
#include <syscall.h>
// Slackware/2.4.30 kernel, using linuxthreads, doesn't really support __thread keyword,
// will generate runtime error
static bool SupportTls()
{
    char buffer[64];
    confstr(_CS_GNU_LIBPTHREAD_VERSION, buffer, sizeof(buffer));
    return strstr(buffer, "NPTL") != NULL;
}
#else
static bool SupportTls()
{
    return true;
}
#endif

ThreadHandleType ThisThread::GetHandle()
{
    return ::pthread_self();
}

int ThisThread::GetId()
{
    static bool support_tls = SupportTls();
    if (support_tls)
    {
        static __thread pid_t tid = 0;
        if (tid == 0)
            tid = syscall(SYS_gettid);
        return tid;
    }
    return syscall(SYS_gettid);
}

void ThisThread::Exit()
{
    pthread_exit(NULL);
}

void ThisThread::Sleep(int64_t time_in_ms)
{
    if (time_in_ms >= 0)
    {
        timespec ts = { time_in_ms / 1000, (time_in_ms % 1000) * 1000000 };
        nanosleep(&ts, &ts);
    }
    else
    {
        pause();
    }
}

void ThisThread::Yield()
{
    sched_yield();
}

int ThisThread::GetLastErrorCode()
{
    return errno;
}

#endif // __unix__

#ifdef _WIN32
#include <process.h>
#include <winsock2.h>
#include "common/base/common_windows.h"
#undef Yield

ThreadHandleType ThisThread::GetHandle()
{
    return ::GetCurrentThread();
}

int ThisThread::GetId()
{
    return ::GetCurrentThreadId();
}

void ThisThread::Sleep(int64_t time_in_ms)
{
    ::Sleep(time_in_ms);
}

void ThisThread::Exit()
{
    ::_endthreadex(0);
}

void ThisThread::Yield()
{
    ::SwitchToThread();
}

int ThisThread::GetLastErrorCode()
{
    unsigned int error = ::WSAGetLastError();
    return WindowsErrorToPosixErrno(error);
}

#pragma comment(lib, "ws2_32")

#endif // _WIN32

