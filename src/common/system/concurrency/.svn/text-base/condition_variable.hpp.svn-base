// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_SYSTEM_CONCURRENCY_CONDITION_VARIABLE_HPP
#define COMMON_SYSTEM_CONCURRENCY_CONDITION_VARIABLE_HPP

#include <assert.h>

#if __unix__
#include <pthread.h>
#endif

#include "common/system/concurrency/mutex.hpp"

class ConditionVariable
{
public:
    ConditionVariable();
    ~ConditionVariable();
    void Signal();
    void Broadcast();

    // If timeout_in_ms is -1, it means infinite and equals to
    // Wait(Mutex* mutex);
    bool Wait(MutexBase* mutex, int timeout_in_ms);
    bool Wait(MutexBase& mutex, int timeout_in_ms)
    {
        return Wait(&mutex, timeout_in_ms);
    }

    void Wait(MutexBase* mutex);
    void Wait(MutexBase& mutex)
    {
        return Wait(&mutex);
    }
private:
#ifdef _WIN32
    void* m_hCondition;
    unsigned int m_nWaitCount;
#elif __unix__
    pthread_cond_t m_hCondition;
#endif
};

#endif // COMMON_SYSTEM_CONCURRENCY_CONDITION_VARIABLE_HPP

