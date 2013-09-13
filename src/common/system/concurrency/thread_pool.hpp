// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2010-06-18

#ifndef COMMON_SYSTEM_CONCURRENCY_THREAD_POOL_HPP_INCLUDED
#define COMMON_SYSTEM_CONCURRENCY_THREAD_POOL_HPP_INCLUDED

#include <stddef.h>
#include "common/base/closure.h"
#include "common/base/platform_features.hpp"
#include "common/base/stdext/intrusive_list.hpp"
#include "common/system/concurrency/condition_variable.hpp"
#include "common/system/concurrency/mutex.hpp"
#include "common/system/concurrency/thread.hpp"

/// thread pool class, .NET interface
class ThreadPool
{
public:
    struct Stats
    {
        size_t NumThreads;
        size_t NumBusyThreads;
        size_t NumPendingTasks;
    };

    ThreadPool(
        int min_concurrencys = 0,  ///< -1 means cpu number
        int max_concurrencys = -1, ///< -1 means cpu number
        int idle_timeout = 60000   ///< max idle time in milliseconds
    );
    ~ThreadPool();

    void SetMinThreads(int size);
    void SetMaxThreads(int size);
    void SetIdleTime(int time);
    void SetMaxLagTime(int time);

    DEPRECATED void AddTask(
        Thread::StartRoutine routine,
        void* context = NULL,
        unsigned long long param = 0
    );

    void AddTask(Closure<void>* callback);
    void AddPriorityTask(Closure<void>* callback);

    void AddTask(const Function<void ()>& function);
    void AddPriorityTask(const Function<void ()>& function);

    void Terminate(bool wait = true, int timeout = 1000);
    void GetStats(Stats* stat) const;

    void WaitForIdle();

    // a handy singleton interface
    static ThreadPool& DefaultInstance();
private:
    struct Task
    {
        // Description about this task by Thread::StartRoutine.
        Thread::StartRoutine Routine;
        void* Context;
        unsigned long long Param; ///< any param

        // Description about this task by Closure.
        // If it's set, ignore the above routine.
        Closure<void>* callback;

        // Description about this task by Function.
        // If it's set, ignore the above routine.
        Function<void ()> function;
    };

    struct TaskNode : Task
    {
        list_node link;
    };

    struct PooledThread : public Thread
    {
        list_node link;
    };

    void AddTaskInternal(
        bool is_priority,
        Thread::StartRoutine routine,
        void* context,
        unsigned long long param,
        Closure<void>* callback,
        const Function<void ()>& function
    );

    bool NeedNewThread() const;
    void AddThread();
    bool DequeueTask(Task* task);
    void WorkRoutine(PooledThread* concurrency);

private:
    bool m_Exit;
    size_t m_MinThreads;
    size_t m_MaxThreads;
    int m_IdleTime;
    int m_MaxLagTime;

    mutable SimpleMutex m_Mutex;
    ConditionVariable m_NewTaskCond; ///< to wakeup worker threads
    intrusive_list<PooledThread> m_Threads;
    intrusive_list<PooledThread> m_FreedThreads;
    volatile size_t m_NumThreads;
    volatile size_t m_NumBusyThreads;
    intrusive_list<TaskNode> m_PendingTasks;
    volatile size_t m_NumPendingTasks;
    intrusive_list<TaskNode> m_FreeTasks;
    volatile size_t m_NumIdleTasks;
};

#endif // COMMON_SYSTEM_CONCURRENCY_THREAD_POOL_HPP_INCLUDED

