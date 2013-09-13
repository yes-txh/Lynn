// Copyright (c) 2011, Tencent Inc. All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/20/11
// Description: thread group definition

#ifndef COMMON_SYSTEM_CONCURRENCY_THREAD_GROUP_HPP
#define COMMON_SYSTEM_CONCURRENCY_THREAD_GROUP_HPP
#pragma once

#include <vector>
#include "common/system/concurrency/thread.hpp"

class ThreadGroup
{
    DECLARE_UNCOPYABLE(ThreadGroup);
public:
    ThreadGroup();
    ThreadGroup(const Function<void ()>& callback, size_t count);
    ~ThreadGroup();
    void Add(const Function<void ()>& callback, size_t count = 1);
    void Start();
    void Join();
    size_t Size() const;
private:
    std::vector<Thread*> m_threads;
};

#endif // COMMON_SYSTEM_CONCURRENCY_THREAD_GROUP_HPP
