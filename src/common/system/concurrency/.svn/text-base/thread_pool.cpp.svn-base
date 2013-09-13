// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2010-06-18

#include "common/base/singleton.hpp"
#include "common/system/concurrency/thread_pool.hpp"
#include "common/system/system_information.h"
#include "common/system/time/timestamp.hpp"

ThreadPool::ThreadPool(int min_threads, int max_threads, int idle_timeout):
    m_Exit(false),
    m_IdleTime(idle_timeout),
    m_NumThreads(0),
    m_NumBusyThreads(0),
    m_NumPendingTasks(0)
{
    if (min_threads < 0)
        m_MinThreads = GetLogicalCpuNumber();
    else
        m_MinThreads = min_threads;

    if (max_threads < 0)
        m_MaxThreads = GetLogicalCpuNumber();
    else
        m_MaxThreads = max_threads;

    if (m_MaxThreads < m_MinThreads)
        m_MaxThreads = m_MinThreads;
}

ThreadPool::~ThreadPool()
{
    Terminate();
}

void ThreadPool::SetMinThreads(int size)
{
    if (size < 0)
        m_MinThreads = GetLogicalCpuNumber();
    else
        m_MinThreads = size;
}

void ThreadPool::SetMaxThreads(int size)
{
    if (size < 0)
        m_MaxThreads = GetLogicalCpuNumber();
    else
        m_MaxThreads = size;

    if (m_MaxThreads < m_MinThreads)
        m_MaxThreads = m_MinThreads;
}

void ThreadPool::SetIdleTime(int time)
{
    m_IdleTime = time;
}

void ThreadPool::AddTask(Thread::StartRoutine routine,
                         void* context,
                         unsigned long long param)
{
    AddTaskInternal(false, routine, context, param, NULL, NULL);
}

void ThreadPool::AddTask(Closure<void>* callback)
{
    AddTaskInternal(false, NULL, NULL, 0, callback, NULL);
}

void ThreadPool::AddTask(const Function<void ()>& function)
{
    AddTaskInternal(false, NULL, NULL, 0, NULL, function);
}

void ThreadPool::AddPriorityTask(Closure<void>* callback)
{
    AddTaskInternal(true, NULL, NULL, 0, callback, NULL);
}

void ThreadPool::AddPriorityTask(const Function<void ()>& function)
{
    AddTaskInternal(true, NULL, NULL, 0, NULL, function);
}

void ThreadPool::AddTaskInternal(
    bool is_priority,
    Thread::StartRoutine routine,
    void* context,
    unsigned long long param,
    Closure<void>* callback,
    const Function<void ()>& function)
{
    {
        MutexLocker locker(m_Mutex);

        TaskNode* task = NULL;

        // try allocate from freelist first
        if (!m_FreeTasks.empty())
        {
            task = &m_FreeTasks.front();
            m_FreeTasks.pop_front();
        }
        else
        {
            task = new TaskNode;
        }

        task->Routine = routine;
        task->Context = context;
        task->Param = param;
        task->callback = callback;
        task->function = function;

        if (is_priority)
            m_PendingTasks.push_front(*task);
        else
            m_PendingTasks.push_back(*task);

        ++m_NumPendingTasks;

        if (NeedNewThread())
        {
            AddThread();
        }
    }

    m_NewTaskCond.Signal();
}

bool ThreadPool::NeedNewThread() const
{
    if (m_NumThreads >= m_MaxThreads)
        return false;

    if (m_NumThreads < m_MinThreads)
        return true;

    // all active thread are busy
    if (m_NumBusyThreads == m_NumThreads)
        return true;

    return false;
}

void ThreadPool::AddThread()
{
    PooledThread* thread = NULL;
    if (!m_FreedThreads.empty())
    {
        // allocate from free list
        thread = &m_FreedThreads.front();
        m_FreedThreads.pop_front();
        thread->Join();

        // destroy and recreate
        thread->~PooledThread();
        new (thread) PooledThread();
    }
    else
    {
        thread = new PooledThread();
    }

    Thread::StartRoutine start_routine =
        MAKE_PARAMETERIZED_THREAD_CALLBACK(ThreadPool, WorkRoutine, PooledThread*);
    m_Threads.push_back(*thread);
    ++m_NumThreads;
    thread->Initialize(start_routine, this, (unsigned long long)thread);
    thread->Start();
}

bool ThreadPool::DequeueTask(Task* task)
{
    if (!m_PendingTasks.empty())
    {
        TaskNode* front = &m_PendingTasks.front();
        m_PendingTasks.pop_front();
        --m_NumPendingTasks;
        *task = *front;

        // recycle task node
        front->function = NULL;
        m_FreeTasks.push_back(*front);

        return true;
    }

    return false;
}

void ThreadPool::WorkRoutine(PooledThread* thread)
{
    {
        MutexLocker locker(m_Mutex);
        ++m_NumBusyThreads;
    }

    while (!m_Exit)
    {
        Task task;

        {
            MutexLocker locker(m_Mutex);
            if (!DequeueTask(&task))
            {
                // TODO(phongchen): Add some random factor
                --m_NumBusyThreads;
                bool wait_success = m_NewTaskCond.Wait(m_Mutex, m_IdleTime);
                ++m_NumBusyThreads;
                if (wait_success)
                {
                    if (!DequeueTask(&task))
                        continue;
                }
                else
                {
                    if (m_PendingTasks.empty() && m_NumThreads > m_MinThreads)
                        break;
                    continue;
                }
            }
        }

        try
        {
            if (task.function){
                task.function();
            } else if (task.callback) {
                task.callback->Run();
            } else {
                task.Routine(task.Context, task.Param);
            }
        }
        catch (...)
        {
        }
    }

    MutexLocker locker(m_Mutex);
    m_Threads.erase(*thread);
    m_FreedThreads.push_back(*thread);
    --m_NumThreads;
    --m_NumBusyThreads;
}

void ThreadPool::Terminate(bool wait, int timeout)
{
    if (!m_Exit)
    {
        if (wait)
        {
            while (m_NumPendingTasks > 0)
            {
                ThisThread::Sleep(1);
            }
        }

        m_Exit = true;
    }

    // wakeup all worker threads to exit
    while (m_NumThreads > 0)
    {
        m_NewTaskCond.Broadcast();
    }

    {
        MutexLocker locker(m_Mutex);

        while (!m_FreeTasks.empty())
        {
            TaskNode* task = &m_FreeTasks.front();
            m_FreeTasks.pop_front();
            delete task;
        }

        while (!m_PendingTasks.empty())
        {
            TaskNode* task = &m_PendingTasks.front();
            m_PendingTasks.pop_front();
            delete task->callback;
            delete task;
        }

        while (!m_FreedThreads.empty())
        {
            PooledThread* thread= &m_FreedThreads.front();
            m_FreedThreads.pop_front();
            thread->Join();
            delete thread;
        }
    }
}

void ThreadPool::GetStats(Stats* stat) const
{
    MutexLocker locker(m_Mutex);
    stat->NumThreads = m_NumThreads;
    stat->NumBusyThreads = m_NumBusyThreads;
    stat->NumPendingTasks = m_NumPendingTasks;
}

void ThreadPool::WaitForIdle()
{
    // this function is rarely used, using condition variable in WorkRoutine
    // to notify here is not economically
    for (;;)
    {
        {
            MutexLocker locker(m_Mutex);
            if (m_PendingTasks.empty())
                return;
        }

        // sleep must be out of lock
        ThisThread::Sleep(1);
    }
}

ThreadPool& ThreadPool::DefaultInstance()
{
    return Singleton<ThreadPool>::Instance();
}

