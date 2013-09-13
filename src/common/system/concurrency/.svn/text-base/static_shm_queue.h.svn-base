// Copyright (c) 2011, Tencent Inc. All rights reserved.
// modified by ivanhuang at 20101108

#ifndef COMMON_SYSTEM_CONCURRENCY_STATIC_SHM_QUEUE_H_
#define COMMON_SYSTEM_CONCURRENCY_STATIC_SHM_QUEUE_H_

#ifndef _WIN32

#include <assert.h>
#include <string.h>

#include "common/system/concurrency/semaphore.hpp"
#include "common/system/concurrency/share_memory.h"

#pragma pack(push, 1)

// 队列头部信息
struct ShmQueueHeader {
    size_t data_count;    // 队列数据数目
    size_t pop_position;  // 出队列指针
    size_t push_position; // 进队列指针
};

#pragma pack(pop)

// 操作返回码
enum StaticShmQueueErr {
    kErrShmFull     =  -100,
    kErrShmEmpty    =  -101,
    kErrBlockSize   =  -102,
    kErrShmOp       =  -103,
    kErrCreateShm   =  -104,
    kErrGetShm      =  -105,
    kErrCreateSem   =  -106,
    kErrIndexNum    =  -107,
};

class StaticShmQueue {
public:
    StaticShmQueue();
    ~StaticShmQueue();

    int GetShmId() const        {return m_shm_id;}
    int GetShmKey()const        {return m_shm_key;}

    int GetSemId() const        {return m_sem_id;}
    int GetSemKey() const       {return m_sem_key;}

    int GetBlockSize() const    {return m_block_size;}
    int Size() const            {return m_queue_header->data_count;}

    // 初始化队列
    int Init(int shm_key, int sem_key, size_t block_size, size_t block_num);

    // 析构时删除共享内存
    void SetRemoveFlag(bool flag) {m_remove_when_exit = flag;}

    // 向共享内存队列写入数据
    int Push(const void *block_buffer, size_t block_len);

    // 向共享内存队列取出数据
    int Pop(void *block_buffer, size_t max_buffer_size, size_t *block_size);

    // 向共享内存设置相应位置的数据
    int SetValue(const void *block_buffer, size_t block_size, size_t index);

    // 获取共享内存相应位置的数据
    int GetValue(void *block_buffer, size_t max_buffer_size,
                 size_t index, size_t *block_size);

    // 检查共享内存是否有空间
    bool HasSpace() const {
        return m_queue_header->data_count >= m_block_num ? false : true;
    }

private:
    int m_shm_key;                          // 共享内存Key
    int m_shm_id;                           // 共享内存ID
    int m_sem_key;                          // 信号量Key
    int m_sem_id;                           // 信号量Id

    size_t m_block_num;                     // 共享内存最大存放的节点数目
    size_t m_block_size;                    // 共享内存存放的节点的大小

    bool m_remove_when_exit;                // 退出时是否删除共享内存

    ShmQueueHeader *m_queue_header;         // 共享内存队列头部信息
    char           *m_queue_body;           // 共享内存指针

    SemaphoreLock  *m_sem_lock;             // 信号量锁
    ShareMemory    *m_share_memory;         // 共享内存操作对象
};

#endif // _WIN32

#endif // COMMON_SYSTEM_CONCURRENCY_STATIC_SHM_QUEUE_H_
