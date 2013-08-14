#ifndef SRC_COMMON_SINGLETON_H_
#define SRC_COMMON_SINGLETON_H_

#include "common/mutex.h"

using lynn::MutexLocker;

template <typename T>
class Singleton {
public:
    static T* Instance() {
        MutexLocker locker(m_lock);
        if (m_instance == NULL)
            m_instance = new T;
        return m_instance;
    }

private:
    static lynn::Mutex m_lock;
    static T* m_instance;
};

template <typename T>
T* Singleton<T>::m_instance = NULL;
template <typename T>
lynn::Mutex Singleton<T>::m_lock;

#endif
