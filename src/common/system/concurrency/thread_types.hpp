// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/31/11

#ifndef COMMON_SYSTEM_CONCURRENCY_THREAD_TYPES_HPP
#define COMMON_SYSTEM_CONCURRENCY_THREAD_TYPES_HPP
#pragma once

#if defined __unix__
# include <pthread.h>
#endif // __unix__

#include <string>

#include "common/base/compatible/errno.h"
#include "common/base/stdint.h"

#if defined _WIN32
    typedef void*       ThreadHandleType;
#elif __unix__
    typedef pthread_t   ThreadHandleType;
#endif

/// ThreadAttribute represent thread attribute.
/// Usage:
/// ThreadAttribute()
///     .SetName("ThreadPoolThread")
///     .SetStackSize(64 * 1024)
class ThreadAttribute
{
public:
    ThreadAttribute();
    ~ThreadAttribute();
    ThreadAttribute& SetName(const std::string& name);
    ThreadAttribute& SetStackSize(size_t size);
    ThreadAttribute& SetDetached(bool detached);
    ThreadAttribute& SetPriority(int priority);
private:
    std::string m_name;
#if defined __unix__
    pthread_attr_t m_attr;
#elif defined _WIN32
    size_t m_stack_size;
    int m_priority;
    int m_detached;
#endif
};

#endif // COMMON_SYSTEM_CONCURRENCY_THREAD_TYPES_HPP
