/********************************
 FileName: common/singleton.h
 Author:   
 Date:     2013-08-21
 Version:  0.1
 Description: singleton, single case 
*********************************/

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
