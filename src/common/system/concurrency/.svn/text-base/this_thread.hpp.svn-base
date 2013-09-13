// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/31/11
// Description: current thread scoped attributes and operations

#ifndef COMMON_SYSTEM_CONCURRENCY_THIS_THREAD_HPP
#define COMMON_SYSTEM_CONCURRENCY_THIS_THREAD_HPP
#pragma once

#include "common/base/stdint.h"
#include "common/system/concurrency/thread_types.hpp"
#ifdef _WIN32
#undef Yield
#endif

/// thread scoped attribute and operations of current thread
class ThisThread
{
    ThisThread();
    ~ThisThread();
public:
    static void Exit();
    static void Yield();
    static void Sleep(int64_t time_in_ms);
    static int GetLastErrorCode();
    static ThreadHandleType GetHandle();
    static int GetId();
};

#endif // COMMON_SYSTEM_CONCURRENCY_THIS_THREAD_HPP
