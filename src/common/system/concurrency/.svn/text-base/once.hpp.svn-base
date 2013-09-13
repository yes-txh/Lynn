// Copyright (c) 2011, Tencent Inc. All rights reserved.

/**
 * @file once.h
 * @brief
 * @author kypoyin,kypoyin@gmail.com
 * @date 2010-09-01
 */

#ifndef COMMON_SYSTEM_CONCURRENCY_ONCE_HPP
#define COMMON_SYSTEM_CONCURRENCY_ONCE_HPP

#ifndef _WIN32
#include <pthread.h>
#endif

#ifdef _WIN32

// Note: pre declare the OnceInternal which is implemented in once.cc
struct OnceInternal;

struct Once {
    Once();
    ~Once();
    void Init(void (*init_routine_func)());
private:
    void DoInit(void (*init_routine_func)());
private:
    // Once(const Once&) = delete;
    // Once operator=(const Once&) = delete;
private:
    OnceInternal* internal;
    volatile bool initialized;
};

#define DECLARE_ONCE(NAME)    Once NAME

#else

struct Once
{
    pthread_once_t once;
    void Init(void (*init_routine_func)());
private:
    // Once(const Once&) = delete;
    // Once operator=(const Once&) = delete;
};

#define DECLARE_ONCE(NAME)       Once NAME = { PTHREAD_ONCE_INIT }

void Once::Init(void (*init_routine_func)()) {
    pthread_once(&once, init_routine_func);
}

#endif

#endif // COMMON_SYSTEM_CONCURRENCY_ONCE_HPP
