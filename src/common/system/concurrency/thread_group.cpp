// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/20/11
// Description: thread group implementation
#include "common/system/concurrency/thread_group.hpp"
#include <assert.h>
#include "common/base/unique_ptr.hpp"

ThreadGroup::ThreadGroup()
{
}

ThreadGroup::ThreadGroup(const Function<void ()>& callback, size_t count)
{
    Add(callback, count);
}

ThreadGroup::~ThreadGroup()
{
    for (size_t i = 0; i < m_threads.size(); ++i)
        delete m_threads[i];
    m_threads.clear();
}

void ThreadGroup::Add(const Function<void ()>& callback, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        unique_ptr<Thread> thread(new Thread(callback));
        m_threads.push_back(thread.get());
        thread.release();
    }
}

void ThreadGroup::Start()
{
    assert(Size() > 0);
    for (size_t i = 0; i < m_threads.size(); ++i)
    {
        m_threads[i]->Start();
    }
}

void ThreadGroup::Join()
{
    // TODO(phongchen) using pthread_barrier
    for (size_t i = 0; i < m_threads.size(); ++i)
        m_threads[i]->Join();
}

size_t ThreadGroup::Size() const
{
    return m_threads.size();
}

