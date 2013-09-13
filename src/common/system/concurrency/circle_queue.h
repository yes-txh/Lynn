// Copyright (c) 2011 Tenctent.com
/**********************************************************
 *���ζ������ͷ�ļ�
 *�����������ڶ��߳�֮������ݹ����봫�ݣ���Ϣ�첽�����
 *********************************************************/

//////////////////////////////////////////////////////////////////////////
// modified by ivanhuang at 20101115
// 1.�޸Ĵ���������ע�͡��޸Ĳ��ִ���
// 2.����UnitTest
//////////////////////////////////////////////////////////////////////////

#ifndef COMMON_SYSTEM_CONCURRENCY_CIRCLE_QUEUE_H_ // NOLINT
#define COMMON_SYSTEM_CONCURRENCY_CIRCLE_QUEUE_H_

#include <stdlib.h>
#include <string.h>
#include "common/base/platform_features.hpp"
#include "common/system/concurrency/mutex.hpp"

template <class UserType>
class CircleQueue {
public:
    explicit CircleQueue(int capacity) {
        Init(capacity);
    }

    CircleQueue() {
        m_buffer = NULL;
        m_capacity = 0;
        m_tail   = 0;
        m_head   = 0;
        m_length = 0;
    }

    ~CircleQueue() {
        delete [] m_buffer;
        m_buffer = NULL;
    }

public:
    /**
     @brief ��ʼ������(���г���)
     @param capacity ���г���
    */
    bool Init(int capacity) {
        if (capacity < 0)
            return false;

        m_capacity   = capacity;
        m_buffer = new UserType[m_capacity];

        return true;
    }

    /**
     @brief �����ݷ�����У�ԭ��copyָ������data_len������
     @param data ������������ͷָ��
     @param data_len ָ�����������ݳ���
     @return ����ɹ�����true������false
    */
    bool Push(const void *data, int data_len) {
        MutexLocker locker(m_mutex);

        if (m_length >= m_capacity ||
          data_len + sizeof(int) > sizeof(UserType)) { // NOLINT
            return false;
        } else {
            // ������ʼ��ַ
            int *data_in = reinterpret_cast<int *>(m_buffer + m_tail);

            // �������ݳ���
            *data_in = data_len;
            ++data_in;

            // ��������
            memcpy(data_in, data, data_len);

            // ��λ����βָ��
            m_tail = (m_tail + 1) % m_capacity;
            m_length++;

            return true;
        }

        return false;
    }

    /**
     @brief �����ݷ�����У�ԭ��copyUserType�Ĵ�С������
     @param data ������������ͷָ��
     @return ����ɹ�����true������false
    */
    bool Push(const UserType *data) {
        bool return_status = true;
        MutexLocker locker(m_mutex);

        if (m_length >= m_capacity) {
            return_status = false;
        } else {
            memcpy(m_buffer + m_tail, data, sizeof(*data));

            m_tail = (m_tail + 1) % m_capacity;
            m_length++;

            return_status = true;
        }

        return return_status;
    }

    /**
     @brief �Ӷ���ȡ������,ȡ����Ч����
     @param data ������������ͷָ��
     @param data_len (out)ȡ�������ݵĴ�С
     @return �ɹ�����true������false
    */
    bool Pop(void *data, int &data_len) {
        bool return_status = true;
        MutexLocker locker(m_mutex);

        if (0 == m_length) {
            data_len = 0;
            return_status = false;
        } else {
            data_len = *reinterpret_cast<int *>(m_buffer + m_head);

            memcpy(data, m_buffer + m_head + sizeof(int), // NOLINT
                   data_len);

            // ��λ����ͷָ��
            m_head = (m_head + 1) % m_capacity;
            --m_length;

            return_status = true;
        }

        return return_status;
    }

    /**
     @brief �Ӷ���ȡ�����ݣ�ȡ������UserType���͵�����
     @param data ������������ͷָ��
     @return �ɹ�����true������false
    */
    bool Pop(UserType *data) {
        bool return_status = true;
        MutexLocker locker(m_mutex);

        if (0 == m_length) {
            return_status = false;
        } else {
            memcpy(data, m_buffer + m_head, sizeof(UserType));

            // ��λ����ͷָ��
            m_head = (m_head + 1) % m_capacity;
            --m_length;

            return_status = true;
        }

        return return_status;
    }

    /**
     @brief ��ն��У����ҽ�ͷβָ����0
    */
    void Clear() {
        MutexLocker locker(m_mutex);
        m_tail   = 0;
        m_head   = 0;
        m_length = 0;
    }

    /**
     @brief ��ȡ���еĴ�С
    */
    int Capacity() const {
        MutexLocker locker(m_mutex);
        return m_capacity;
    }

    /**
     @brief ��ȡ���еĴ�С
    */
    DEPRECATED_BY(Capacity)
    int Size() const {
        return Capacity();
    }

    /**
     @brief ��ȡ���е�ʵ�ʵĳ���
    */
    int Length() const {
        MutexLocker locker(m_mutex);
        return m_length;
    }

    /**
     @brief �����Ƿ�Ϊ��
    */
    bool Empty() const
    {
        MutexLocker locker(m_mutex);
        return m_length == 0;
    }

    /**
     @brief �����Ƿ�����
    */
    bool Full() const
    {
        return m_capacity == m_length;
    }

private:
    mutable SimpleMutex m_mutex; // �߳���
    UserType *m_buffer;    // ��Ŷ��е����ݿռ�
    int      m_capacity;   // ���еĴ�С
    int      m_head;       // ����ͷָ��(����)
    int      m_tail;       // ����βָ��(���)
    int      m_length;     // ���г���
};

#endif // COMMON_SYSTEM_CONCURRENCY_CIRCLE_QUEUE_H_ // NOLINT
