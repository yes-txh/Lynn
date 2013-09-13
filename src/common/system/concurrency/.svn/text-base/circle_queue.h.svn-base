// Copyright (c) 2011 Tenctent.com
/**********************************************************
 *环形队列类的头文件
 *可用做进程内多线程之间的数据共享与传递，消息异步处理等
 *********************************************************/

//////////////////////////////////////////////////////////////////////////
// modified by ivanhuang at 20101115
// 1.修改代码风格、增加注释、修改部分代码
// 2.增加UnitTest
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
     @brief 初始化队列(队列长度)
     @param capacity 队列长度
    */
    bool Init(int capacity) {
        if (capacity < 0)
            return false;

        m_capacity   = capacity;
        m_buffer = new UserType[m_capacity];

        return true;
    }

    /**
     @brief 将数据放入队列，原样copy指定长度data_len的数据
     @param data 待拷贝的数据头指针
     @param data_len 指定拷贝的数据长度
     @return 放入成功返回true，否则false
    */
    bool Push(const void *data, int data_len) {
        MutexLocker locker(m_mutex);

        if (m_length >= m_capacity ||
          data_len + sizeof(int) > sizeof(UserType)) { // NOLINT
            return false;
        } else {
            // 计算起始地址
            int *data_in = reinterpret_cast<int *>(m_buffer + m_tail);

            // 拷贝数据长度
            *data_in = data_len;
            ++data_in;

            // 拷贝数据
            memcpy(data_in, data, data_len);

            // 复位对列尾指针
            m_tail = (m_tail + 1) % m_capacity;
            m_length++;

            return true;
        }

        return false;
    }

    /**
     @brief 将数据放入队列，原样copyUserType的大小的数据
     @param data 待拷贝的数据头指针
     @return 放入成功返回true，否则false
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
     @brief 从队列取出数据,取出有效数据
     @param data 待拷贝的数据头指针
     @param data_len (out)取出的数据的大小
     @return 成功返回true，否则false
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

            // 复位对列头指针
            m_head = (m_head + 1) % m_capacity;
            --m_length;

            return_status = true;
        }

        return return_status;
    }

    /**
     @brief 从队列取出数据，取出整个UserType类型的数据
     @param data 待拷贝的数据头指针
     @return 成功返回true，否则false
    */
    bool Pop(UserType *data) {
        bool return_status = true;
        MutexLocker locker(m_mutex);

        if (0 == m_length) {
            return_status = false;
        } else {
            memcpy(data, m_buffer + m_head, sizeof(UserType));

            // 复位对列头指针
            m_head = (m_head + 1) % m_capacity;
            --m_length;

            return_status = true;
        }

        return return_status;
    }

    /**
     @brief 清空队列，并且将头尾指针置0
    */
    void Clear() {
        MutexLocker locker(m_mutex);
        m_tail   = 0;
        m_head   = 0;
        m_length = 0;
    }

    /**
     @brief 获取队列的大小
    */
    int Capacity() const {
        MutexLocker locker(m_mutex);
        return m_capacity;
    }

    /**
     @brief 获取队列的大小
    */
    DEPRECATED_BY(Capacity)
    int Size() const {
        return Capacity();
    }

    /**
     @brief 获取队列的实际的长度
    */
    int Length() const {
        MutexLocker locker(m_mutex);
        return m_length;
    }

    /**
     @brief 队列是否为空
    */
    bool Empty() const
    {
        MutexLocker locker(m_mutex);
        return m_length == 0;
    }

    /**
     @brief 队列是否已满
    */
    bool Full() const
    {
        return m_capacity == m_length;
    }

private:
    mutable SimpleMutex m_mutex; // 线程锁
    UserType *m_buffer;    // 存放队列的数据空间
    int      m_capacity;   // 队列的大小
    int      m_head;       // 队列头指针(出队)
    int      m_tail;       // 队列尾指针(入队)
    int      m_length;     // 队列长度
};

#endif // COMMON_SYSTEM_CONCURRENCY_CIRCLE_QUEUE_H_ // NOLINT
