// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2010-06-18

#ifndef COMMON_SYSTEM_CONCURRENCY_FIXED_THREAD_POOL_HPP
#define COMMON_SYSTEM_CONCURRENCY_FIXED_THREAD_POOL_HPP

#include <stddef.h>
#include "common/base/closure.h"
#include "common/base/stdext/intrusive_list.hpp"
#include "common/system/concurrency/condition_variable.hpp"
#include "common/system/concurrency/mutex.hpp"
#include "common/system/concurrency/thread.hpp"

class FixedThreadPool
{
public:
    struct Stats
    {
        size_t NumThreads;
        size_t NumBusyThreads;
        size_t NumPendingTasks;
    };

    /// @param mun_threads number of threads, -1 means cpu number
    explicit FixedThreadPool(int num_threads = -1);
    ~FixedThreadPool();

    void AddTask(Thread::StartRoutine routine,
                 void* context = NULL,
                 unsigned long long param = 0,
                 int dispatch_key = -1);
    void AddTask(Closure<void>* callback, int dispatch_key = -1);

    void Terminate(bool wait = true, int timeout = 1000);
    void GetStats(Stats* stats) const;

private:
    struct Task
    {
        list_node link;

        // Description about this task by Thread::StartRoutine.
        Thread::StartRoutine Routine;
        void* Context;
        unsigned long long Param; ///< any param

        // Description about this task by Closure.
        // If it's set, ignore the above routine.
        Closure<void>* callback;

        long long EnqueTime; ///< in ms
    };

    struct PooledThread : public Thread
    {
        list_node link;
        intrusive_list<Task> PendingTasks;
    };

    void AddTaskInternal(Thread::StartRoutine routine,
                         void* context,
                         unsigned long long param,
                         Closure<void>* callback,
                         int dispatch_key);

    bool TaskPending() const;
    void WorkRoutine(PooledThread* thread);
    Task* GetPendingTask(PooledThread* thread);
    bool AnyThreadRunning() const;

    size_t m_NumThreads;

    bool m_Exit;
    mutable SimpleMutex m_Mutex;
    ConditionVariable m_Cond;
    PooledThread* m_Threads;
    volatile size_t m_NumBusyThreads;
    intrusive_list<Task> m_PendingTasks;
    intrusive_list<Task> m_FreeTasks;
    volatile size_t m_NumFreeTasks;
};

#endif // COMMON_SYSTEM_CONCURRENCY_FIXED_THREAD_POOL_HPP

