// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

/// @author phongchen@tencent.com
/// @date Nov 8 2010

#ifndef COMMON_SYSTEM_CONCURRENCY_SEMAPHORE_HPP_INCLUDED
#define COMMON_SYSTEM_CONCURRENCY_SEMAPHORE_HPP_INCLUDED

#include <stdlib.h>
#include "common/system/concurrency/system_error.hpp"

#ifdef _WIN32

class Semaphore
{
public:
    explicit Semaphore(unsigned int value);
    ~Semaphore();
    void Wait();
    bool TimedWait(int timeout); // in ms
    bool TryWait();
    void Release();
    unsigned int GetValue() const;
private:
    static void Checked(int success);
    static void HandleError();
private:
    Semaphore(const Semaphore& src);
    Semaphore& operator=(const Semaphore& src);
private:
    void* m_hSemaphore;
};

#else // _WIN32

#include <errno.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <sys/types.h>
#include "common/system/eintr_ignored.h"
#include "common/system/time/posix_time.hpp"

class Semaphore
{
public:
    explicit Semaphore(unsigned int value)
    {
        CHECK_POSIX_ERROR(sem_init(&m_sem, false, value));
    }

    ~Semaphore()
    {
        CHECK_POSIX_ERROR(sem_destroy(&m_sem));
    }

    void Wait()
    {
        CHECK_POSIX_ERROR(EINTR_IGNORED(sem_wait(&m_sem)));
    }

    bool TryWait()
    {
        if (EINTR_IGNORED(sem_trywait(&m_sem) == 0))
            return true;
        int error = errno;
        if (error == EAGAIN)
            return false;
        CHECK_ERRNO_ERROR(error);
        return true;
    }

    bool TimedWait(int timeout) // in ms
    {
        if (timeout < 0)
        {
            Wait();
            return true;
        }

        struct timespec ts;
        RelativeTimeInMillSecondsToAbsTimeInTimeSpec(timeout, &ts);
        return CHECK_POSIX_TIMED_ERROR(EINTR_IGNORED(sem_timedwait(&m_sem, &ts)));
    }

    void Release()
    {
        CHECK_POSIX_ERROR(sem_post(&m_sem));
    }

    unsigned int GetValue() const
    {
        int value;
        CHECK_POSIX_ERROR(sem_getvalue(const_cast<sem_t*>(&m_sem), &value));
        return value;
    }

private:
    Semaphore(const Semaphore& src);
    Semaphore& operator=(const Semaphore& src);

private:
    sem_t m_sem;
};

// added by ivanhuang at 20101116
class SemaphoreLock
{
public:
    // 无参构造函数,需要调用InitSem进行初始化
    SemaphoreLock()
    {
        m_initialize = false;
    }

    // 单参数构造函数,外部提供id,无需初始化
    explicit SemaphoreLock(int sem_id)
    {
        m_initialize = true;
        m_sem_id     = sem_id;
    }

    int InitSem(int key)
    {
        if (m_initialize)
            return m_sem_id;

        // 获取kye值
        m_key_id = key;

        // 创建信号量句柄
        m_sem_id = semget(m_key_id, 1, IPC_CREAT | IPC_EXCL | 0666);
        if (m_sem_id < 0)
        {
            if (errno != EEXIST)
            {
                return -1;
            }

            if ((m_sem_id = semget(m_key_id, 1, 0666)) < 0)
            {
                return -1;
            }
        }

        // 初始化信号量
        unsigned short init_array[] = {1};
        int ret = semctl(m_sem_id, 0, SETALL, init_array);

        if (ret < 0)
        {
            return -1;
        }

        m_initialize = true;

        return m_sem_id;
    }

    int GetSemId() const
    {
        if (!m_initialize)
            return -1;

        return m_sem_id;
    }

    int GetSemKey() const
    {
        if (!m_initialize)
            return -1;

        return m_key_id;
    }

    bool Acquire()
    {
        if (!m_initialize)
            return false;

        for (;;)
        {
            struct sembuf sops;
            sops.sem_num = 0;
            sops.sem_op  = -1;
            sops.sem_flg = SEM_UNDO;

            int ret = semop(m_sem_id, &sops, 1);
            if (ret < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }
    }

    bool Release()
    {
        if (!m_initialize)
            return false;

        for (;;)
        {
            struct sembuf sops;
            sops.sem_num = 0;
            sops.sem_op  = 1;
            sops.sem_flg = SEM_UNDO;

            int ret = semop(m_sem_id, &sops, 1);
            if (ret < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }
    }

    bool IsLock() const
    {
        struct sembuf op[1] = {{0, 0, IPC_NOWAIT}};

        if (semop(m_sem_id, &op[0], 1) == -1)
            return false;

        return true;
    }

private:
    bool m_initialize;
    int  m_sem_id;
    int  m_key_id;
};

#endif // _WIN32

#endif // COMMON_SYSTEM_CONCURRENCY_SEMAPHORE_HPP_INCLUDED
