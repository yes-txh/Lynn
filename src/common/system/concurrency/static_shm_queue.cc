// Copyright (c) 2011, Tencent Inc. All rights reserved.

#include "common/system/concurrency/static_shm_queue.h"

#ifndef _WIN32

StaticShmQueue::StaticShmQueue() {
    m_shm_key      = -1;
    m_sem_key      = -1;
    m_block_size   = -1;
    m_block_num    = -1;
    m_shm_id       = -1;
    m_sem_id       = -1;

    m_queue_header = NULL;
    m_queue_body   = NULL;
    m_sem_lock     = NULL;
    m_share_memory = NULL;

    m_remove_when_exit = false;
}

StaticShmQueue::~StaticShmQueue() {
    delete m_sem_lock;
    m_sem_lock = NULL;

    if (m_share_memory) {
        m_share_memory->Detach();

        if (m_remove_when_exit)
            m_share_memory->Remove();

        delete m_share_memory;

        m_share_memory = NULL;
        m_queue_header = NULL;
        m_queue_body   = NULL;
    }
}

// 初始化
int StaticShmQueue::Init(int shm_key, int sem_key,
                         size_t block_size, size_t block_num) {
    m_shm_key    = shm_key;
    m_sem_key    = sem_key;
    m_block_size = block_size;
    m_block_num  = block_num;

    // 分配共享内存对象
    m_share_memory = new ShareMemory();
    if (m_share_memory == NULL)
        return kErrCreateShm;

    // 打开共享内存
    bool   shm_init = false;
    size_t shm_size = static_cast<size_t>(m_block_num * m_block_size +
                                          sizeof(ShmQueueHeader));

    if (!m_share_memory->ForceOpen((key_t)m_shm_key, shm_size, NULL, 0, &shm_init))
        return false;

    // 获取共享内存地址
    void *shm_buffer = m_share_memory->GetShmAddress();

    // 第一次创建,需要初始化
    if (shm_init)
        memset(shm_buffer, 0, m_block_num * m_block_size + sizeof(ShmQueueHeader));

    // 将共享内存分成头部和数据体两部分
    m_queue_header = reinterpret_cast<ShmQueueHeader *>(shm_buffer);
    m_queue_body   = reinterpret_cast<char *>(shm_buffer) + sizeof(ShmQueueHeader);

    // 分配信号量对象
    m_sem_lock = new SemaphoreLock;
    if (!m_sem_lock->InitSem(m_sem_key))
        return kErrCreateSem;

    // 获取共享内存和信号量的句柄
    m_shm_id = m_share_memory->GetShmId();
    m_sem_id = m_sem_lock->GetSemId();

    return true;
}

// 往队尾插入数据
int StaticShmQueue::Push(const void *block_buffer, size_t block_size) {
    bool result = false;
    // 判断数据大小是否一致
    if (block_size != m_block_size)
        return kErrBlockSize;

    // 加锁
    if (!m_sem_lock->Acquire())
        return false;

    // 判断队列是否有空间
    if (!HasSpace()) {
        goto PUSH_POSITION;
    }

    // 拷贝数据并移动相关指针
    memcpy(m_queue_body + m_queue_header->push_position * m_block_size,
           block_buffer, block_size);
    m_queue_header->data_count++;
    m_queue_header->push_position++;

    // 确保计数范围正确
    assert(m_queue_header->data_count <= m_block_num);
    assert(m_queue_header->push_position <= m_block_num);

    // 指针回绕
    if (m_queue_header->push_position == m_block_num)
        m_queue_header->push_position = 0;

    // 修改返回码
    result = true;

PUSH_POSITION:
    if (!m_sem_lock->Release())
        return kErrShmOp;

    return result;
}

// 弹出对头数据
int StaticShmQueue::Pop(void *block_buffer, size_t max_buffer_size,
                        size_t *block_size) {
    int ret_code = -1;

    // 判断数据大小是否一致
    if (max_buffer_size < m_block_size)
        return kErrBlockSize;

    // 加锁
    if (!m_sem_lock->Acquire())
        return kErrShmOp;

    // 判断队列是否为空
    if (m_queue_header->data_count <= 0) {
        ret_code = kErrShmEmpty;
        goto POP_POSITION;
    }

    // 拷贝数据并移动指针
    memcpy(block_buffer, m_queue_body + m_queue_header->pop_position *
           m_block_size, m_block_size);
    m_queue_header->data_count--;
    m_queue_header->pop_position++;

    // 确保计数范围正确
    assert(m_queue_header->data_count < m_block_num);
    assert(m_queue_header->push_position <= m_block_num);

    // 指针回绕
    if (m_queue_header->pop_position == m_block_num)
        m_queue_header->pop_position = 0;

    *block_size = m_block_size;
    ret_code = 0;

POP_POSITION:
    if (!m_sem_lock->Release())
        return kErrShmOp;

    return ret_code;
}

int StaticShmQueue::SetValue(const void *block_buffer, size_t block_size,
                             size_t index) {
    // 判断数据大小是否一致
    if (block_size != m_block_size)
        return kErrBlockSize;

    // 判断数据位置是否越界
    if (index >= m_block_num)
        return kErrIndexNum;

    // 加锁
    if (!m_sem_lock->Acquire())
        return kErrShmOp;

    // 拷贝数据
    memcpy(m_queue_body + index * m_block_size, block_buffer, block_size);

    // 解锁
    if (!m_sem_lock->Release())
        return kErrShmOp;

    return 0;
}

int StaticShmQueue::GetValue(void *block_buffer, size_t max_buffer_size,
                             size_t index, size_t *block_size) {
    // 判断数据大小是否一致
    if (max_buffer_size < m_block_size)
        return kErrBlockSize;

    // 判断数据位置是否越界
    if (index >= m_block_num)
        return kErrIndexNum;

    // 加锁
    if (!m_sem_lock->Acquire())
        return kErrShmOp;

    // 拷贝数据
    memcpy(block_buffer, m_queue_body + index * m_block_size, m_block_size);

    *block_size = m_block_size;

    // 解锁
    if (!m_sem_lock->Release())
        return kErrShmOp;

    return 0;
}

#endif
