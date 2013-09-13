// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 04/29/11
// Description: singleton class template

#ifndef COMMON_BASE_SINGLETON_HPP_INCLUDED
#define COMMON_BASE_SINGLETON_HPP_INCLUDED

#include "common/base/platform_features.hpp"
#include "common/base/uncopyable.hpp"

/*
// example: define a singleton class
class TestClass : public SingletonBase<TestClass>
{
    friend class SingletonBase<TestClass>;
private:
    TestClass() {}
    ~TestClass() {}
public:
    int Test() const
    {
        return 1;
    }
};

// example2: define a singleton class with alt access method
// private inherit make 'Instance' method unaccessable
class TestClass2 : private SingletonBase<TestClass2>
{
    friend class SingletonBase<TestClass2>;
private:
    TestClass() {}
    ~TestClass() {}
public:
    // using DefaultInstance to instead Instance
    static TestClass2& DefaultInstance()
    {
        return Instance();
    }
};

// example3: make a singleton for class
class TestClass3
{
};

typedef Singleton<TestClass3> SingletonTestClass3;
TestClass3& instance = SingletonTestClass3::Instance();

*/

#if HAS_THREAD_SAFE_STATICS

template <typename T>
class SingletonBase
{
    DECLARE_UNCOPYABLE(SingletonBase);
protected:
    SingletonBase(){}
    ~SingletonBase(){}
public:
    static T& Instance()
    {
        static T instance;
        return instance;
    }
};

#else

#include <stdlib.h>
#include "common/system/concurrency/mutex.hpp"

template <typename T>
class SingletonBase
{
    DECLARE_UNCOPYABLE(SingletonBase);
protected:
    SingletonBase(){}
    ~SingletonBase(){}
public:
    static T& Instance()
    {
        // double check locking optimize
        if (!m_instance)
        {
            ScopedLocker<Mutex> lock(m_lock);
            if (!m_instance)
            {
                m_instance = new T();
                atexit(Destroy);
            }
        }
        return *m_instance;
    }
private:
    static void Destroy()
    {
        // need not locking
        if (m_instance)
        {
            delete m_instance;
            m_instance = NULL;
        }
    }
private:
    static Mutex m_lock;
    static T* volatile m_instance;
};

template <typename T>
Mutex SingletonBase<T>::m_lock;

template <typename T>
T* volatile SingletonBase<T>::m_instance;

#endif

template <typename T>
class Singleton : public SingletonBase<T>
{
};

#endif // COMMON_BASE_SINGLETON_HPP_INCLUDED
