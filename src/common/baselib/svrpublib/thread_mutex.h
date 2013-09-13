// thread_mutex.h: interface for the CXThreadMutex class.
// wookin@tencent.com    2010/05/06
// ////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_THREAD_MUTEX_H_
#define COMMON_BASELIB_SVRPUBLIB_THREAD_MUTEX_H_

#include "common/baselib/svrpublib/base_config.h"
#include "thirdparty/glog/logging.h"

_START_XFS_BASE_NAMESPACE_

#ifdef WIN32

// if _WIN32_WINNT not defined, TryEnterCriticalSection will not be declared
// in windows.h
# if(_WIN32_WINNT < 0x0400)
extern "C" WINBASEAPI
BOOL WINAPI TryEnterCriticalSection(__inout LPCRITICAL_SECTION lpCriticalSection);
# endif

#endif


class CXThreadMutex {
public:
    CXThreadMutex() {
#ifdef WIN32
        memset((unsigned char*)&m_thread_mutex,
               0,
               sizeof(m_thread_mutex));

        InitializeCriticalSection(&m_thread_mutex);
#else // linux
        if (pthread_mutex_init(&m_thread_mutex, NULL) != 0) {
			LOG(FATAL) << "pthread_mutex_init() fail.";
        }
#endif // linux
        m_is_locked = false;
    }

    ~CXThreadMutex() {
#ifdef WIN32
        DeleteCriticalSection(&m_thread_mutex);
#else // linux
        //pthread_mutex_destroy(&m_thread_mutex);
#endif // linux
    }

    void Lock() {
#ifdef WIN32
        EnterCriticalSection(&m_thread_mutex);
#else // linux
        pthread_mutex_lock(&m_thread_mutex);
#endif // linux
        m_is_locked = true;
    }

    bool TryLock(){
#ifdef WIN32
        return TryEnterCriticalSection(&m_thread_mutex) != FALSE;
#else
        return pthread_mutex_trylock(&m_thread_mutex) == 0;
#endif
    }


    void UnLock() {
#ifdef WIN32
        LeaveCriticalSection(&m_thread_mutex);
#else // linux
        pthread_mutex_unlock(&m_thread_mutex);
#endif // linux
        m_is_locked = false;
    }

    bool IsLocked() const {
        return m_is_locked;
    }

private:

#ifdef WIN32
    CRITICAL_SECTION        m_thread_mutex;
#else // linux
    pthread_mutex_t         m_thread_mutex;
#endif //
    bool volatile           m_is_locked;
};

//
//  Thread mutex, auto lock
//
class CXThreadAutoLock {
public:
    explicit CXThreadAutoLock(CXThreadMutex* mutex) {
        m_mutex_obj = mutex;
        if (m_mutex_obj)
            m_mutex_obj->Lock();
    }

    virtual ~CXThreadAutoLock() {
        if (m_mutex_obj)
            m_mutex_obj->UnLock();
    }

private:
    CXThreadMutex* volatile m_mutex_obj;
};

#define ThreadMutex CXThreadMutex

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_THREAD_MUTEX_H_
