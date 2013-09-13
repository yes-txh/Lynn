// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2010-06-18

#include "common/system/concurrency/fixed_thread_pool.hpp"
#include "common/system/system_information.h"
#include "common/system/time/timestamp.hpp"

// GLOBAL_NOLINT(runtime/int)

FixedThreadPool::FixedThreadPool(int num_threads):
    m_NumThreads(0),
    m_Exit(false),
    m_NumBusyThreads(0),
    m_NumFreeTasks(0)
{
    if (num_threads < 0)
        m_NumThreads = GetLogicalCpuNumber();
    else if (num_threads == 0)
        m_NumThreads = 1;
    else
        m_NumThreads = num_threads;

    m_Threads = new PooledThread[m_NumThreads];
    for (size_t i = 0; i < m_NumThreads; ++i)
    {
        m_Threads[i].Initialize(
            MAKE_PARAMETERIZED_THREAD_CALLBACK(FixedThreadPool, WorkRoutine, PooledThread*),
            this, (unsigned long long) &m_Threads[i]
        );
        m_Threads[i].Start();
    }
}

FixedThreadPool::~FixedThreadPool()
{
    Terminate();
}

void FixedThreadPool::AddTask(Thread::StartRoutine routine,
                              void* context,
                              unsigned long long param,
                              int dispatch_key)
{
    AddTaskInternal(routine, context, param, NULL, dispatch_key);
}

void FixedThreadPool::AddTask(Closure<void>* callback, int dispatch_key)
{
    AddTaskInternal(NULL, NULL, 0, callback, dispatch_key);
}

void FixedThreadPool::AddTaskInternal(Thread::StartRoutine routine,
                                      void* context,
                                      unsigned long long param,
                                      Closure<void>* callback,
                                      int dispatch_key)
{
    MutexLocker locker(m_Mutex);

    Task* task = NULL;
    if (!m_FreeTasks.empty())
    {
        task = &m_FreeTasks.front();
        m_FreeTasks.pop_front();
        --m_NumFreeTasks;
    }
    else
    {
        task = new Task;
    }

    long long time_stamp = GetTimeStampInMs();

    task->Routine = routine;
    task->Context = context;
    task->Param = param;
    task->callback = callback;
    task->EnqueTime = time_stamp;

    if (dispatch_key < 0)
        m_PendingTasks.push_back(*task);
    else
        m_Threads[dispatch_key % m_NumThreads].PendingTasks.push_back(*task);

    m_Cond.Signal();
}

void FixedThreadPool::WorkRoutine(PooledThread* thread)
{
    MutexLocker locker(m_Mutex);

    while (!m_Exit)
    {
        Task* task = GetPendingTask(thread);
        if (task)
        {
            ++m_NumBusyThreads;
            m_Mutex.Unlock();

            try
            {
                if (task->callback == NULL) {
                    task->Routine(task->Context, task->Param);
                } else {
                    task->callback->Run();
                }
            }
            catch (...)
            {
            }

            m_Mutex.Lock();
            --m_NumBusyThreads;

            m_FreeTasks.push_back(*task);
            ++m_NumFreeTasks;
        }
        else
        {
            m_Cond.Wait(m_Mutex, -1);
        }
    }
}

FixedThreadPool::Task* FixedThreadPool::GetPendingTask(PooledThread* thread)
{
    Task* task = NULL;

    if (!m_PendingTasks.empty() && !thread->PendingTasks.empty())
    {
        if (m_PendingTasks.front().EnqueTime < thread->PendingTasks.front().EnqueTime)
        {
            task = &m_PendingTasks.front();
            m_PendingTasks.pop_front();
        }
        else
        {
            task = &thread->PendingTasks.front();
            thread->PendingTasks.pop_front();
        }
    }
    else if (!m_PendingTasks.empty())
    {
        task = &m_PendingTasks.front();
        m_PendingTasks.pop_front();
    }
    else if (!thread->PendingTasks.empty())
    {
        task = &thread->PendingTasks.front();
        thread->PendingTasks.pop_front();
    }
    return task;
}

bool FixedThreadPool::TaskPending() const
{
    if (!m_PendingTasks.empty())
        return true;

    for (size_t i = 0; i < m_NumThreads; ++i)
    {
        if (!m_Threads[i].PendingTasks.empty())
            return true;
    }

    return false;
}

bool FixedThreadPool::AnyThreadRunning() const
{
    for (size_t i = 0; i < m_NumThreads; ++i)
    {
        if (m_Threads[i].IsRunning())
            return true;
    }
    return false;
}

void FixedThreadPool::Terminate(bool wait, int timeout)
{
    if (!m_Exit)
    {
        if (wait)
        {
            while (TaskPending())
            {
                ThisThread::Sleep(10);
            }
        }

        m_Exit = true;
    }

    while (AnyThreadRunning())
    {
        m_Cond.Broadcast();
        ThisThread::Sleep(10);
    }

    MutexLocker locker(m_Mutex);

    while (!m_FreeTasks.empty())
    {
        Task* task = &m_FreeTasks.front();
        m_FreeTasks.pop_front();
        delete task;
    }

    while (!m_PendingTasks.empty())
    {
        Task* task = &m_PendingTasks.front();
        m_PendingTasks.pop_front();
        delete task;
    }

    for (size_t i = 0; i < m_NumThreads; ++i)
        m_Threads[i].Join();

    delete[] m_Threads;
    m_Threads = NULL;
}

void FixedThreadPool::GetStats(Stats* stats) const
{
    MutexLocker locker(m_Mutex);
    stats->NumThreads = m_NumThreads;
    stats->NumBusyThreads = m_NumBusyThreads;
    stats->NumPendingTasks = m_PendingTasks.size();
}
