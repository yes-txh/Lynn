// Copyright (c) 2009, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_OBJECT_POOL_HPP
#define COMMON_BASE_OBJECT_POOL_HPP

#include <limits.h>
#include <algorithm>
#include <vector>

#include "common/system/concurrency/mutex.hpp"
#include "common/system/concurrency/spinlock.hpp"

#undef min
#undef max

/// @file ObjectType.hpp
/// @brief 定义了 ObjectPool 和 FixedObjectPool 两个类模板
/// 对象池减缓对象的构造和析构，但是为了限制同时存在的对象数量，可以设置配额
/// 对象池可以指定清空方式，要求含有一个 static 的 Clear 成员函数，参数为对象
/// 的指针类型。
///
/// 以下是几个常用的清空方法

/// 清空方式：调用 Clear 成员函数
struct CallMember_Clear
{
    template <typename T>
    static void Clear(T* p)
    {
        p->Clear();
    }
};

/// 清空方式：调用 clear 成员函数，适用于 STL
struct CallMember_clear
{
    template <typename T>
    static void Clear(T* p)
    {
        p->clear();
    }
};

/// 不做清空
struct NullClear
{
    template <typename T>
    static void Clear(T* p)
    {
    }
};

/// 支持动态设置配额的对象池
template <typename T,
    typename ClearTraits = NullClear,
    typename LockType = Spinlock
>
class ObjectPool
{
    typedef ObjectPool ThisType;
public:
    typedef T ObjectType;
public:
    explicit ObjectPool(
        size_t quota = INT_MAX,  ///< 最大缓存的配额
        size_t initial_size = 0, ///< 初始大小
        bool auto_create = true  ///< 池为空时请求对象是否自动创建
    ):
        m_Quota(quota), m_AutoCreate(auto_create)
    {
        Reserve(std::min(initial_size, quota));
    }

    ~ObjectPool()
    {
        Clear();
    }

    /// 返回配额
    size_t GetQuota() const
    {
        return m_Quota;
    }

    size_t Size() const
    {
        typename LockType::Locker locker(m_Lock);
        return m_PooledObjects.size();
    }

    /// 设置配额
    void SetQuota(size_t size)
    {
        typename LockType::Locker locker(m_Lock);
        m_Quota = size;
        UnlockedShrink(size);
    }

    /// 预留对象
    /// @param size 期望预留的池中对象的个数
    void Reserve(size_t size)
    {
        if (size > m_Quota)
            size = m_Quota;
        typename LockType::Locker locker(m_Lock);
        while (m_PooledObjects.size() < size)
        {
            m_PooledObjects.push_back(new T());
        }
    }

    /// 从对象池获得对象
    T* Acquire()
    {
        {
            typename LockType::Locker locker(m_Lock);
            if (m_PooledObjects.empty())
            {
                if (!m_AutoCreate)
                    return NULL;
            }
            else
            {
                T* p = m_PooledObjects.back();
                m_PooledObjects.pop_back();
                return p;
            }
        }
        return new T();
    }

    /// 归还对象
    void Release(const T* p)
    {
        T* q = const_cast<T*>(p);
        ClearTraits::Clear(q);
        {
            typename LockType::Locker locker(m_Lock);
            if (m_PooledObjects.size() < m_Quota)
            {
                m_PooledObjects.push_back(q);
                return;
            }
        }
        delete p;
    }

    /// 收缩对象池里缓存的对象到不多于 size 个
    void Shrink(size_t size = 0)
    {
        typename LockType::Locker locker(m_Lock);
        UnlockedShrink(size);
    }

    /// 清空所有缓存的对象
    void Clear()
    {
        Shrink(0);
    }
private:
    void UnlockedShrink(size_t size)
    {
        while (m_PooledObjects.size() > size)
        {
            delete m_PooledObjects.back();
            m_PooledObjects.pop_back();
        }
    }
private:
    ObjectPool(const ObjectPool& src);
    ObjectPool& operator=(const ObjectPool& rhs);
private:
    mutable LockType m_Lock;
    std::vector<T*> m_PooledObjects;
    size_t m_Quota;
    bool m_AutoCreate;
    void (*m_Clear)(T*);
};

/// @brief 编译期间指定配额的对象池
/// @tparam T 对象类型
/// @tparam Quota 设置的配额
/// @tparam ClearTraits 指定清空对象的方法
template <
    typename T,
    size_t Quota,
    typename ClearTraits = NullClear,
    typename LockType = Spinlock
>
class FixedObjectPool
{
    typedef FixedObjectPool ThisType;
public:
    typedef T ObjectType;
public:
    explicit FixedObjectPool(
        size_t initial_size = 0,
        bool auto_create = true
    ) :
        m_Count(0), m_AutoCreate(auto_create)
    {
        Reserve(std::min(Quota, initial_size));
    }

    ~FixedObjectPool()
    {
        Clear();
    }

    /// 从对象池获得对象
    T* Acquire()
    {
        typename LockType::Locker locker(m_Lock);
        if (m_Count > 0)
        {
            return m_PooledObjects[--m_Count];
        }
        else
        {
            if (m_AutoCreate)
                return new T();
            else
                return NULL;
        }
    }

    /// 归还对象
    void Release(const T* p)
    {
        typename LockType::Locker locker(m_Lock);
        if (m_Count < Quota)
        {
            T* q = const_cast<T*>(p);
            ClearTraits::Clear(q);
            m_PooledObjects[m_Count++] = q;
        }
        else
        {
            delete p;
        }
    }

    size_t Size() const
    {
        typename LockType::Locker locker(m_Lock);
        return m_Count;
    }


    /// 预留对象
    /// @param size 期望预留的池中对象的个数
    void Reserve(size_t size)
    {
        if (size > Quota)
            size = Quota;
        typename LockType::Locker locker(m_Lock);
        while (m_Count < size)
        {
            T* p = new T();
            m_PooledObjects[m_Count++] = p;
        }
    }

    /// 收缩缓存的对象
    void Shrink(size_t size = 0)
    {
        typename LockType::Locker locker(m_Lock);
        while (m_Count > size)
        {
            delete m_PooledObjects[--m_Count];
        }
    }

    /// 清除所有缓存的对象
    void Clear()
    {
        Shrink(0);
    }

private:
    FixedObjectPool(const FixedObjectPool& src);
    FixedObjectPool& operator=(const FixedObjectPool& rhs);
private:
    mutable LockType m_Lock;
    T* m_PooledObjects[Quota];
    size_t m_Count;
    bool m_AutoCreate;
};

#endif // COMMON_BASE_OBJECT_POOL_HPP

