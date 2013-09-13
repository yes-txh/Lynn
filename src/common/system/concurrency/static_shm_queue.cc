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

// ��ʼ��
int StaticShmQueue::Init(int shm_key, int sem_key,
                         size_t block_size, size_t block_num) {
    m_shm_key    = shm_key;
    m_sem_key    = sem_key;
    m_block_size = block_size;
    m_block_num  = block_num;

    // ���乲���ڴ����
    m_share_memory = new ShareMemory();
    if (m_share_memory == NULL)
        return kErrCreateShm;

    // �򿪹����ڴ�
    bool   shm_init = false;
    size_t shm_size = static_cast<size_t>(m_block_num * m_block_size +
                                          sizeof(ShmQueueHeader));

    if (!m_share_memory->ForceOpen((key_t)m_shm_key, shm_size, NULL, 0, &shm_init))
        return false;

    // ��ȡ�����ڴ��ַ
    void *shm_buffer = m_share_memory->GetShmAddress();

    // ��һ�δ���,��Ҫ��ʼ��
    if (shm_init)
        memset(shm_buffer, 0, m_block_num * m_block_size + sizeof(ShmQueueHeader));

    // �������ڴ�ֳ�ͷ����������������
    m_queue_header = reinterpret_cast<ShmQueueHeader *>(shm_buffer);
    m_queue_body   = reinterpret_cast<char *>(shm_buffer) + sizeof(ShmQueueHeader);

    // �����ź�������
    m_sem_lock = new SemaphoreLock;
    if (!m_sem_lock->InitSem(m_sem_key))
        return kErrCreateSem;

    // ��ȡ�����ڴ���ź����ľ��
    m_shm_id = m_share_memory->GetShmId();
    m_sem_id = m_sem_lock->GetSemId();

    return true;
}

// ����β��������
int StaticShmQueue::Push(const void *block_buffer, size_t block_size) {
    bool result = false;
    // �ж����ݴ�С�Ƿ�һ��
    if (block_size != m_block_size)
        return kErrBlockSize;

    // ����
    if (!m_sem_lock->Acquire())
        return false;

    // �ж϶����Ƿ��пռ�
    if (!HasSpace()) {
        goto PUSH_POSITION;
    }

    // �������ݲ��ƶ����ָ��
    memcpy(m_queue_body + m_queue_header->push_position * m_block_size,
           block_buffer, block_size);
    m_queue_header->data_count++;
    m_queue_header->push_position++;

    // ȷ��������Χ��ȷ
    assert(m_queue_header->data_count <= m_block_num);
    assert(m_queue_header->push_position <= m_block_num);

    // ָ�����
    if (m_queue_header->push_position == m_block_num)
        m_queue_header->push_position = 0;

    // �޸ķ�����
    result = true;

PUSH_POSITION:
    if (!m_sem_lock->Release())
        return kErrShmOp;

    return result;
}

// ������ͷ����
int StaticShmQueue::Pop(void *block_buffer, size_t max_buffer_size,
                        size_t *block_size) {
    int ret_code = -1;

    // �ж����ݴ�С�Ƿ�һ��
    if (max_buffer_size < m_block_size)
        return kErrBlockSize;

    // ����
    if (!m_sem_lock->Acquire())
        return kErrShmOp;

    // �ж϶����Ƿ�Ϊ��
    if (m_queue_header->data_count <= 0) {
        ret_code = kErrShmEmpty;
        goto POP_POSITION;
    }

    // �������ݲ��ƶ�ָ��
    memcpy(block_buffer, m_queue_body + m_queue_header->pop_position *
           m_block_size, m_block_size);
    m_queue_header->data_count--;
    m_queue_header->pop_position++;

    // ȷ��������Χ��ȷ
    assert(m_queue_header->data_count < m_block_num);
    assert(m_queue_header->push_position <= m_block_num);

    // ָ�����
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
    // �ж����ݴ�С�Ƿ�һ��
    if (block_size != m_block_size)
        return kErrBlockSize;

    // �ж�����λ���Ƿ�Խ��
    if (index >= m_block_num)
        return kErrIndexNum;

    // ����
    if (!m_sem_lock->Acquire())
        return kErrShmOp;

    // ��������
    memcpy(m_queue_body + index * m_block_size, block_buffer, block_size);

    // ����
    if (!m_sem_lock->Release())
        return kErrShmOp;

    return 0;
}

int StaticShmQueue::GetValue(void *block_buffer, size_t max_buffer_size,
                             size_t index, size_t *block_size) {
    // �ж����ݴ�С�Ƿ�һ��
    if (max_buffer_size < m_block_size)
        return kErrBlockSize;

    // �ж�����λ���Ƿ�Խ��
    if (index >= m_block_num)
        return kErrIndexNum;

    // ����
    if (!m_sem_lock->Acquire())
        return kErrShmOp;

    // ��������
    memcpy(block_buffer, m_queue_body + index * m_block_size, m_block_size);

    *block_size = m_block_size;

    // ����
    if (!m_sem_lock->Release())
        return kErrShmOp;

    return 0;
}

#endif
