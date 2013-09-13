// Copyright (c) 2011, Tencent Inc. All rights reserved.

//////////////////////////////////////////////////////////////////////////
// modified by ivanhuang at 20101107
// 1.修改代码风格
// 2.增加UnitTest
//////////////////////////////////////////////////////////////////////////

#ifndef COMMON_SYSTEM_CONCURRENCY_SHARE_MEMORY_H_ // NOLINT
#define COMMON_SYSTEM_CONCURRENCY_SHARE_MEMORY_H_

#include <errno.h>
#include <stdio.h>

#ifdef _WIN32

#else

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "common/base/strict_bool.hpp"

class ShareMemory
{
public:
    enum ShmFlag {
        kShmCreateExcl  = IPC_CREAT | IPC_EXCL,
        kShmCreate      = IPC_CREAT,
        kShmOpen        = 0,
        kShmDefaultPerm = 0666,
        kShmErrNoMem    = -200,
        kShmErrNoPerm   = -201,
    };

    ShareMemory() : m_shm_id(-1), m_shm_size(0), m_attach_address(NULL) {}

    // 打开并映射：不存在或者存在但是大小不一致则创建
    strict_bool ForceOpen(key_t key, size_t size, const void *virtual_addr,
                          int flags, bool *new_open);

    // 打开并映射：不创建
    strict_bool Open(key_t key, size_t size, const void *virtual_addr, int flags);

    // 删除
    strict_bool Remove();

    // 映射
    strict_bool Attach(const void *virtual_addr, int flags);

    // 反映射
    strict_bool Detach();

    // 控制
    strict_bool Control(int cmd, void *buf);

    // getter function
    void   *GetShmAddress() const {
        return m_attach_address;
    }

    size_t GetShmSize() const {
        return m_shm_size;
    }

    int    GetShmId() const {
        return m_shm_id;
    }

private:
    strict_bool Open(key_t key, size_t size, int create, int perms);

private:
    int    m_shm_id;          // 共享内存ID
    size_t m_shm_size;        // 共享内存大小
    void   *m_attach_address; // 共享内存映射地址
};

#endif // _WIN32

#endif // COMMON_SYSTEM_CONCURRENCY_SHARE_MEMORY_H_ // NOLINT
