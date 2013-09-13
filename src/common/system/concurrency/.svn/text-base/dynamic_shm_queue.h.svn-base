// Copyright (c) 2011, Tencent Inc. All rights reserved.

/************************************************************************
* 共享内存中的数据块动态变化的共享内存的分配
每个数据块前面加上一个int类型数据标识后面跟的数据块的大小
_________________________________________________
|  int  |  data     | int |  data  | int | data |
_________________________________________________
************************************************************************/

//////////////////////////////////////////////////////////////////////////
// modified by ivanhuang at 20101107
// 1.修改代码风格
// 2.增加UnitTest
//////////////////////////////////////////////////////////////////////////

#ifndef COMMON_SYSTEM_CONCURRENCY_DYNAMIC_SHM_QUEUE_H_ // NOLINT
#define COMMON_SYSTEM_CONCURRENCY_DYNAMIC_SHM_QUEUE_H_

#ifndef _WIN32

#include <assert.h>
#include <string.h>

#include "common/system/concurrency/semaphore.hpp"
#include "common/system/concurrency/share_memory.h"

#pragma pack(push, 1)

// 队列头部信息
struct DynamicShmQueueHeader {
    int data_len;        // 共享内存中已经存放的数据的大小
    int pop_position;    // 出队列指针
    int push_position;   // 进队列指针
};

#pragma pack(pop)

// 错误码
enum DynamicShmQueueErr {
    kErrShmFull   =  -100,
    kErrShmEmpty  =  -101,
    kErrBlockSize =  -102,
    kErrShmOp     =  -103,
    kErrCreateShm =  -104,
    kErrGetShm    =  -105,
    kErrCreateSem =  -106,
    kErrBlockLen  =  -107,
};

class DynamicShmQueue {
public:
    DynamicShmQueue();
    ~DynamicShmQueue();

    // 初始化
    strict_bool Init(int shm_key, int sem_key, int queue_size);

    // 向共享内存队列写入数据
    strict_bool Push(char *enque_data, int enque_len);

    // 向共享内存队列取出数据
    strict_bool Pop(char *deque_data, int max_buffer_len, int *deque_len);

    // getter function
    int  GetShmId() const         {return m_shm_id;}
    int  GetShmKey() const        {return m_shm_key;}
    int  GetSemId() const         {return m_sem_id;}
    int  GetSemKey() const        {return m_sem_key;}
    int  GetQueueSize() const     {return m_queue_size;}
    int  GetQueueDataLen() const  {return m_queue_head->data_len;}
    void SetRemoveFlag(bool flag) {m_remove_when_exit = flag;}

private:
    int m_shm_key;            // 共享内存Key
    int m_shm_id;             // 共享内存ID
    int m_sem_key;            // 信号量Key
    int m_sem_id;             // 信号量Id
    int m_queue_size;         // 共享内存存放数据的最大空间

    bool m_remove_when_exit;  // 退出时是否删除共享内存

    DynamicShmQueueHeader *m_queue_head;   // 共享内存队列头部信息
    char                  *m_queue_body;   // 共享内存指针
    SemaphoreLock         *m_lock;         // 信号量锁
    ShareMemory           *m_shm;          // 共享内存操作对象
};

#endif // _WIN32

#endif // COMMON_SYSTEM_CONCURRENCY_DYNAMIC_SHM_QUEUE_H_ // NOLINT
