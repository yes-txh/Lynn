// Copyright (c) 2011, Tencent Inc. All rights reserved.

#include "common/system/concurrency/dynamic_shm_queue.h"

#ifndef _WIN32

DynamicShmQueue::DynamicShmQueue() {
    m_shm_key          = -1;
    m_sem_key          = -1;
    m_shm_id           = -1;
    m_sem_id           = -1;
    m_queue_size       = 0;
    m_queue_head       = NULL;
    m_queue_body       = NULL;
    m_lock             = NULL;
    m_shm              = NULL;
    m_remove_when_exit = false;
}

DynamicShmQueue::~DynamicShmQueue() {
    delete m_lock;
    m_lock = NULL;

    if (m_shm != NULL) {
        m_shm->Detach();
        delete m_shm;

        if (m_remove_when_exit)
            m_shm->Remove();

        m_shm        = NULL;
        m_queue_head = NULL;
        m_queue_body = NULL;
    }
}

strict_bool DynamicShmQueue::Init(int shm_key, int sem_key, int queue_size) {
    if (shm_key < 0 || sem_key < 0 || queue_size < 0)
        return false;

    m_shm_key    = shm_key;
    m_sem_key    = sem_key;
    m_queue_size = queue_size;

    // 分配共享内存对象
    m_shm = new ShareMemory();
    if (m_shm == NULL)
        return false;

    // 打开共享内存
    bool   shm_init = false;
    size_t shm_size = static_cast<size_t>(m_queue_size +
                        sizeof(DynamicShmQueueHeader));

    if (!m_shm->ForceOpen((key_t)m_shm_key, shm_size,
                                NULL, 0, &shm_init))
    {
        return false;
    }

    // 获取共享内存地址
    void *shm_buffer = m_shm->GetShmAddress();

    // 第一次创建,需要初始化
    if (shm_init)
        memset(shm_buffer, 0, shm_size);

    // 将共享内存分成头部和数据体两部分
    m_queue_head = reinterpret_cast<DynamicShmQueueHeader *>(shm_buffer);
    m_queue_body = reinterpret_cast<char *>(shm_buffer) +
                        sizeof(DynamicShmQueueHeader);

    // 分配信号量对象
    m_lock = new SemaphoreLock;
    if (-1 == m_lock->InitSem(m_sem_key))
        return false;

    // 获取共享内存和信号量的句柄
    m_shm_id = m_shm->GetShmId();
    m_sem_id = m_lock->GetSemId();

    return true;
}

strict_bool DynamicShmQueue::Push(char *enque_data, int enque_len) {
    // 判断参数合法性
    if (!enque_data || enque_len <= 0)
        return false;

    // 加锁队列
    if (!m_lock->Acquire())
        return false;

    // 判断数据长度是否合法
    int tail_size = 0;
    int int_size  = sizeof(enque_len);
    if ((m_queue_head->data_len + enque_len + int_size) > m_queue_size) {
        goto PUSH_POSITION;
    }

    // 拷贝数据,需要处理回绕情况
    tail_size = m_queue_size - m_queue_head->push_position;
    if (tail_size >= enque_len + int_size) {
        memcpy(m_queue_body + m_queue_head->push_position,
               &enque_len, int_size);
        memcpy(m_queue_body + m_queue_head->push_position + int_size,
               enque_data, enque_len);

        m_queue_head->push_position += int_size + enque_len;
    } else if (tail_size >= int_size) {
        memcpy(m_queue_body + m_queue_head->push_position,
               &enque_len, int_size);
        memcpy(m_queue_body + m_queue_head->push_position + int_size,
               enque_data, tail_size - int_size);
        memcpy(m_queue_body, enque_data + tail_size - int_size,
               enque_len - tail_size + int_size);

        m_queue_head->push_position = enque_len - tail_size + int_size;
    } else {
        memcpy(m_queue_body + m_queue_head->push_position,
               &enque_len, tail_size);
        memcpy(m_queue_body, &enque_len + tail_size,
               int_size - tail_size);
        memcpy(m_queue_body + int_size - tail_size,
               enque_data, enque_len);

        m_queue_head->push_position = int_size - tail_size + enque_len;
    }

    // 更新存储的数据长度
    m_queue_head->data_len += enque_len + int_size;
    assert(m_queue_head->data_len > 0 &&
           m_queue_head->data_len <= m_queue_size);

PUSH_POSITION:
    if (!m_lock->Release())
        return false;

    return false;
}

strict_bool DynamicShmQueue::Pop(char *deque_data, int max_buffer_len, int *deque_len) {
    int return_code = -1;

    // 判断参数合法性
    if (!deque_data || max_buffer_len <= 0)
        return false;

    // 检索队列
    if (!m_lock->Acquire())
        return false;

    // 判断队列的数据长度
    int tail_size = 0;
    int data_len  = 0;
    int int_size  = sizeof(max_buffer_len);
    if (m_queue_head->data_len < int_size) {
        return_code = kErrShmEmpty;
        goto POP_POSITION;
    }

    // 拷贝数据
    tail_size = m_queue_size - m_queue_head->pop_position;
    data_len  = 0;
    if (tail_size >= int_size) {
        memcpy(&data_len, m_queue_body + m_queue_head->pop_position, int_size);

        if (data_len < 0 || data_len > max_buffer_len ||
            m_queue_head->data_len < int_size + data_len) {
            return_code = kErrBlockLen;
            goto POP_POSITION;
        } else if (tail_size - int_size >= data_len) {
            memcpy(deque_data, m_queue_body + m_queue_head->pop_position +
                   int_size, data_len);

            m_queue_head->pop_position += int_size + data_len;
        } else {
            memcpy(deque_data, m_queue_body + m_queue_head->pop_position +
                   int_size, tail_size - int_size);
            memcpy(deque_data + tail_size - int_size, m_queue_body,
                   data_len - tail_size + int_size);

            m_queue_head->pop_position = data_len - tail_size + int_size;
        }
    } else {
        memcpy(&data_len, m_queue_body+m_queue_head->pop_position, tail_size);
        memcpy(&data_len + tail_size, m_queue_body, int_size - tail_size);

        if (data_len < 0 || data_len > max_buffer_len ||
            m_queue_head->data_len < int_size + data_len) {
            return_code = kErrBlockLen;
            goto POP_POSITION;
        } else {
            memcpy(deque_data, m_queue_body + int_size - tail_size, data_len);

            m_queue_head->pop_position = int_size - tail_size + data_len;
        }
    }

    m_queue_head->data_len = m_queue_head->data_len - int_size - data_len;

    *deque_len = data_len;

    if (m_queue_head->data_len < 0)
        goto POP_POSITION;

    return_code = 0;

POP_POSITION:
    if (!m_lock->Release())
        return false;

    return false;
}

#endif // _WIN32
