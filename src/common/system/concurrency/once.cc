// Copyright (c) 2011, Tencent Inc. All rights reserved.

/**
 * @file once.cc
 * @brief
 * @author kypoyin, kypoyin@tencent.com
 * @date 2010-09-01
 */

#ifdef _WIN32
#include <common/base/common_windows.h>
#endif

#include <common/system/concurrency/once.hpp>
#ifdef _WIN32

struct OnceInternal
{
    OnceInternal():
        initialized(false)
    {
        InitializeCriticalSection(&critical_section);
    }

    ~OnceInternal()
    {
        DeleteCriticalSection(&critical_section);
    }

    CRITICAL_SECTION critical_section;
    volatile bool initialized;
};


Once::~Once()
{
    delete internal;
    internal = NULL;
}

Once::Once()
{
    // internal may be non-NULL if Init() was already called.
    if (internal == NULL)
        internal = new OnceInternal;
}

void Once::Init(void (*init_routine_func)())
{
    // Note:  Double-checked locking is safe on x86.
    if (!initialized) {
        DoInit(init_routine_func);
    }
}

void Once::DoInit(void (*init_routine_func)())
{
    if (internal == NULL)
        internal = new OnceInternal;

    EnterCriticalSection(&internal->critical_section);

    if (!initialized) {
        init_routine_func();
        initialized = true;
    }

    LeaveCriticalSection(&internal->critical_section);
}

#endif

