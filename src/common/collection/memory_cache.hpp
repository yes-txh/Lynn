// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 06/22/11
// Description:

#ifndef COMMON_COLLECTION_MEMORY_CACHE_HPP
#define COMMON_COLLECTION_MEMORY_CACHE_HPP
#pragma once

#include <map>
#include "common/base/container_of.h"
#include "common/base/stdext/intrusive_list.hpp"
#include "common/system/concurrency/mutex.hpp"

template <typename Key, typename Value, typename LockType = Mutex>
class MemoryCache
{
    DECLARE_UNCOPYABLE(MemoryCache);
    struct Node
    {
        Value value;
        mutable list_node link;
    };
    // TODO(phongchen): using ClosedHashMap to instead std::map
    typedef std::map<Key, Node> MapType;
public:
    MemoryCache(size_t capacity) :
        m_capacity(capacity)
    {
    }

    size_t Capacity() const
    {
        return m_capacity;
    }

    size_t Size() const
    {
        typename LockType::Locker locker(m_lock);
        return m_map.size();
    }

    bool IsEmpty() const
    {
        typename LockType::Locker locker(m_lock);
        return m_map.empty();
    }

    bool IsFull() const
    {
        typename LockType::Locker locker(m_lock);
        return m_map.size() == m_capacity;
    }

    /// return false if key alread existed
    bool Insert(const Key& key, const Value& value)
    {
        typename LockType::Locker locker(m_lock);

        typename MapType::iterator it = m_map.find(key);
        if (it != m_map.end())
            return false;

        Node& node = m_map[key];
        node.value = value;
        node.link.init();
        MarkAsHot(&node);
        if (m_map.size() > m_capacity)
            DiscardWithLock(1);
        return true;
    }

    /// return false if key doesn't exist
    bool Replace(const Key& key, const Value& value)
    {
        typename LockType::Locker locker(m_lock);
        typename MapType::iterator it = m_map.find(key);
        if (it == m_map.end())
            return false;
        it->second.value = value;
        MarkAsHot(&it->second);
        return true;
    }

    /// insert if exist, replace if not exist
    bool Put(const Key& key, const Value& value)
    {
        typename LockType::Locker locker(m_lock);
        Node& node = m_map[key];
        node.value = value;
        MarkAsHot(&node);
        if (m_map.size() > m_capacity)
            DiscardWithLock(1);
        return true;
    }

    /// @retval false if not found
    bool Get(const Key& key, Value* value) const
    {
        typename LockType::Locker locker(m_lock);
        typename MapType::const_iterator it = m_map.find(key);
        if (it == m_map.end())
            return false;
        *value = it->second.value;
        MarkAsHot(&it->second);
        return true;
    }

    /// get value or return default value if not exist
    Value GetOrDefault(const Key& key, const Value& default_value = Value()) const
    {
        Value value(default_value);
        if (Get(key, &value))
            return value;
        return value;
    }

    /// @return whether found
    bool Contains(const Key& key) const
    {
        typename LockType::Locker locker(m_lock);
        typename MapType::const_iterator it = m_map.find(key);
        return it != m_map.end();
    }

    bool Remove(const Key& key)
    {
        typename LockType::Locker locker(m_lock);
        return m_map.erase(key) == 1;
    }

    void Clear()
    {
        typename LockType::Locker locker(m_lock);
        m_map.clear();
        assert(m_lru_list.empty());
    }

    /// iteration: return first element
    bool First(Key* key, Value* value) const
    {
        typename LockType::Locker locker(m_lock);
        if (m_map.empty())
            return false;
        typename MapType::const_iterator it = m_map.begin();
        *key = it->first;
        *value = it->second.value;
        return true;
    }

    /// iteration: return next element
    bool Next(Key* key, Value* value) const
    {
        typename LockType::Locker locker(m_lock);
        typename MapType::const_iterator it = m_map.upper_bound(*key);
        if (it == m_map.end())
            return false;
        *key = it->first;
        *value = it->second.value;
        return true;
    }

    /// discard count elements forcely
    /// @return number of elements discarded
    size_t Discard(size_t count)
    {
        return DiscardWithLock(count);
    }

private:
    void MarkAsHot(const Node* node) const
    {
        if (!m_lru_list.empty() && node == &m_lru_list.back())
            return;
        node->link.unlink();
        m_lru_list.push_back(*const_cast<Node*>(node));
    }

    void DiscardWithLock(size_t count)
    {
        while (count > 0 && !m_lru_list.empty())
        {
            typename intrusive_list<Node>::iterator it = m_lru_list.begin();
            Node* node = &*it;
            typename MapType::value_type* pair =
                container_of(node, typename MapType::value_type, second);
            m_map.erase(pair->first);
            --count;
        }
    }
private:
    mutable Mutex m_lock;
    size_t m_capacity;
    MapType m_map;
    mutable intrusive_list<Node> m_lru_list;
};

#endif // COMMON_COLLECTION_MEMORY_CACHE_HPP
