//  port_event.h
//
//  1:linux下模拟WaitForSingleObject(win32)
//  2:封装sem_xx,Sem_Event,win32,linux都方便使用
//
//  wookin@tencent.com
//  2010-06-04
//
// ///////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_PORT_EVENT_H_
#define COMMON_BASELIB_SVRPUBLIB_PORT_EVENT_H_

#ifdef WIN32
#pragma   warning(disable:4127)
#include <windows.h>
#endif // WIN32

#ifndef WIN32
#include <stdint.h>

#include <time.h>
#include <sys/time.h>

// error info.
#include <errno.h>
#include <string.h>

#include <pthread.h>
#include <semaphore.h>
#endif // linux

#include "common/baselib/svrpublib/base_config.h"

// glog
#include "thirdparty/glog/logging.h"

// typedef THANDLE
#ifndef THANDLE
typedef void*               THANDLE;
#endif // THANDLE

_START_XFS_BASE_NAMESPACE_

//using namespace google;

#undef ERROR

// ----------------------------------------------------------------------------
// 在linux上模拟 WaitForSingleObject
// ----------------------------------------------------------------------------

#ifndef WIN32
enum ENUM_HANDLE {
    ENUM_HANDLE_UNKNOWN    = 0,
    ENUM_HANDLE_COND_EVENT = 1,
    ENUM_HANDLE_SEM_EVENT  = 2,
};

struct PthreadEventHandle {
    ENUM_HANDLE     type;
    pthread_cond_t  cond;
    pthread_mutex_t mutex;
};

struct SemEventHandle {
    ENUM_HANDLE     type;
    sem_t           sem_handle;
};

//
// WaitForSingleObject接口函数
// 在linux系统模拟WaitForSingleObject
//
#define CreateEvent             pthread_CreateEvent
#define SetEvent                pthread_SetEvent
#define CloseHandle(ev_handle)  pthread_CloseHandle(ev_handle); ev_handle = NULL
#define WaitForSingleObject     pthread_WaitForSingleObject
#define WaitEvent               pthread_WaitEvent
#else // linux
// win32
inline void WaitEvent(HANDLE handle);
#endif //



// ----------------------------------------------------------------------------
// 模拟CreateEvent, Sem_CreateEvent, Sem_SetEvent
// ----------------------------------------------------------------------------

//
// 2010-06-04 新增加, by wookin
//
// /////////////////////////////////////////////////////
// 使用信号量模拟windows下Event
#ifdef WIN32
// -----------------------------------------------------
#define Sem_CreateEvent         CreateEvent
#define Sem_SetEvent            SetEvent
#define Sem_WaitForSingleObject WaitForSingleObject
#define Sem_CloseHandle         CloseHandle
// -----------------------------------------------------
#else // linux

inline THANDLE  Sem_CreateEvent(
    void* event_attributes,
    bool is_manual_reset,
    bool initial_state,
    char* name
);
inline bool     Sem_SetEvent(THANDLE handle);
inline uint32_t Sem_WaitForSingleObject(THANDLE handle, uint32_t millisecs);
inline void     Sem_CloseHandle(THANDLE& handle);

//
//  WaitForSingleObject return value
//
//  The specified object is a mutex object that was not released by the thread
//  that owned the mutex object before the owning thread terminated.
//  Ownership of the mutex object is granted to the calling thread,
//  and the mutex is set to nonsignaled.
//
#define WAIT_ABANDONED  0x00000080L

//  The state of the specified object is signaled.
#define WAIT_OBJECT_0   0x00000000L

//  The time-out interval elapsed, and the object's state is non signaled.
#define WAIT_TIMEOUT    0x00000102L
#endif // linux

class SemEvent{
public:
    //
    // 触发
    //
    void Set(){ Sem_SetEvent(m_sem_handle); }

    // 等待,等待成功返回true, 超时或者失败返回false
    // timeout_millesecs: 超时
    // ret_code: 等待时候的返回码
    bool Wait(uint32_t timeout_millesecs, uint32_t* ret_code = NULL){

        uint32_t ret = Sem_WaitForSingleObject(m_sem_handle, timeout_millesecs);
        if(ret != WAIT_OBJECT_0){
            LOG(ERROR) << "Sem_WaitForSingleObject fail: " << 
                ((ret == WAIT_TIMEOUT) ? "WAIT_TIMEOUT" :
                ((ret == WAIT_ABANDONED) ? "WAIT_ABANDONED" : "unknown error code" ))
                << ", error code: " << ret;
        }
        if(ret_code) *ret_code = ret;

        return ret == WAIT_OBJECT_0;
    }

    SemEvent(){ m_sem_handle = Sem_CreateEvent(NULL, false, false, NULL); }
    ~SemEvent(){ Sem_CloseHandle(m_sem_handle); }

private:
    THANDLE m_sem_handle;
};


// ----------------------------------------------------------------------------
// ... 实现 xxx ...
// ----------------------------------------------------------------------------

//
//  Simu. linux WaitForSingleObject
//
#ifndef WIN32
// 声明函数,用于模拟WaitForSingleObject
inline THANDLE  pthread_CreateEvent(
    void* event_attributes,
    bool manual_reset,
    bool is_initial_state,
    char* name
);
inline bool        pthread_SetEvent(THANDLE event);
inline void        pthread_CloseHandle(THANDLE handle);
inline uint32_t    pthread_WaitForSingleObject(THANDLE handle,
        uint32_t milliseconds);

// 实现函数,用于模拟WaitForSingleObject
inline THANDLE pthread_CreateEvent(void* lpEventAttributes,
                                   bool bManualReset,
                                   bool bInitialState,
                                   char* lpName
                                  ) {
    PthreadEventHandle* handle = new PthreadEventHandle;
    if (handle) {
        pthread_condattr_t cond_attr;
        pthread_condattr_init(&cond_attr);

        pthread_mutex_init(&handle->mutex, 0);
        pthread_cond_init(&handle->cond, &cond_attr);
        pthread_condattr_destroy(&cond_attr);

        handle->type = ENUM_HANDLE_COND_EVENT;
    }
    return (THANDLE)handle;
}

inline bool pthread_SetEvent(THANDLE handle) {
    bool b = false;
    if (handle) {
        PthreadEventHandle* ev_handle =
            reinterpret_cast<PthreadEventHandle*>(handle);

        pthread_mutex_lock(&ev_handle->mutex);
        if (ev_handle->type == ENUM_HANDLE_COND_EVENT)
            b = (pthread_cond_signal(&ev_handle->cond) == 0) ? true:false;
        pthread_mutex_unlock(&ev_handle->mutex);
    }
    return b;
}

inline void pthread_CloseHandle(THANDLE handle) {
    if (handle) {
        PthreadEventHandle* ev_handle =
            reinterpret_cast<PthreadEventHandle*>(handle);

        if (ev_handle->type == ENUM_HANDLE_COND_EVENT) {
            pthread_cond_destroy(&ev_handle->cond);
            pthread_mutex_destroy(&ev_handle->mutex);
            delete ev_handle;
        } else {
            LOG(ERROR) << "pthread_CloseHandle, fail, "
                       "unknown handle type.";
        }
    }
}

inline uint32_t pthread_WaitForSingleObject(THANDLE handle, uint32_t millisecs) {
    uint32_t val = WAIT_TIMEOUT;
    if (handle) {
        PthreadEventHandle* ev_handle =
            reinterpret_cast<PthreadEventHandle*>(handle);

        if (ev_handle->type == ENUM_HANDLE_COND_EVENT) {
            pthread_mutex_lock(&ev_handle->mutex);
            struct timespec ts;

            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                LOG(ERROR) << "clock_gettime fail: " << errno << ":" << strerror(errno);
                int32_t sec = millisecs/1000;
                ts.tv_sec = time(0)+sec;
                ts.tv_nsec = (millisecs%1000)*1000000;
            } else {
                uint32_t sec = millisecs/1000;
                ts.tv_sec += sec;

                uint32_t nano_sec = (millisecs%1000) * 1000000;

                // total nano secs
                nano_sec += ts.tv_nsec;
                ts.tv_nsec = nano_sec % 1000000000;
                ts.tv_sec += nano_sec / 1000000000;
            }

            int32_t ret = pthread_cond_timedwait(&ev_handle->cond,
                                                 &ev_handle->mutex, &ts);
            if (ret == 0)
                val = WAIT_OBJECT_0;
            else {
                if (ret == ETIMEDOUT)
                    val = WAIT_TIMEOUT;
                else {
                    VLOG(3) << "pthread_WaitForSingleObject, fail, "
                              "return error val:" << ret;
                    val = WAIT_ABANDONED;
                }
            }
            pthread_mutex_unlock(&ev_handle->mutex);
        } else
            val = WAIT_ABANDONED;
    }
    return val;
}

inline void pthread_WaitEvent(THANDLE handle) {
    if (handle) {
        PthreadEventHandle* ev_handle =
            reinterpret_cast<PthreadEventHandle*>(handle);

        bool b = false;
        if (ev_handle->type == ENUM_HANDLE_COND_EVENT) {
            pthread_mutex_lock(&ev_handle->mutex);
            b = (pthread_cond_wait(&ev_handle->cond, &ev_handle->mutex) == 0) ? true:false;
            pthread_mutex_unlock(&ev_handle->mutex);
        }

        if(!b) {
            LOG(ERROR) << "pthread_cond_wait fail.";
        }
    }
}

#else // WIN32
inline void WaitEvent(HANDLE handle) {
    if(handle) {
        DWORD err = ::WaitForSingleObject(handle, INFINITE);
        if(!(err == WAIT_OBJECT_0 || err == WAIT_TIMEOUT)) {
            LOG(ERROR) << "WaitEvent fail.";
        }
    }
}
#endif // win32

// ----------------------------------------------------------------------------
// 模拟CreateEvent, Sem_CreateEvent, Sem_SetEvent
// ----------------------------------------------------------------------------

//
// 2010-06-04 新增加, by wookin
//
// /////////////////////////////////////////////////////
// 使用信号量模拟windows下Event
#ifndef WIN32 // linux
inline THANDLE  Sem_CreateEvent(
    void* lpEventAttributes,
    bool bManualReset,
    bool bInitialState,
    char* lpName
) {
    SemEventHandle* handle = new SemEventHandle;
    if (handle) {
        handle->type = ENUM_HANDLE_SEM_EVENT;
        if (sem_init(&handle->sem_handle, 0, 0) != 0) {
            LOG(ERROR) << "sem_init fail.";
            delete handle;
            handle = 0;
        }
    }
    return (THANDLE)handle;
}

inline bool Sem_SetEvent(THANDLE handle) {
    bool b = false;
    if (handle) {
        SemEventHandle* ev_handle = reinterpret_cast<SemEventHandle*>(handle);
        if (ev_handle->type == ENUM_HANDLE_SEM_EVENT)
            b = (sem_post(&ev_handle->sem_handle) == 0) ? true:false;
    }
    return b;
}

inline void Sem_CloseHandle(THANDLE& handle) {
    if (handle) {
        SemEventHandle* ev_handle = reinterpret_cast<SemEventHandle*>(handle);
        if (ev_handle->type == ENUM_HANDLE_SEM_EVENT) {
            sem_destroy(&ev_handle->sem_handle);
            delete ev_handle;
        } else {
            LOG(ERROR) << "Sem_CloseHandle, fail, unknown handle type.";
            abort();
            char* aa=0;
            aa[256]=0;
        }

        handle = NULL;
    }
}

inline uint32_t Sem_WaitForSingleObject(THANDLE handle, uint32_t millisecs) {
    uint32_t val = WAIT_TIMEOUT;
    if (handle) {
        SemEventHandle* ev_handle = reinterpret_cast<SemEventHandle*>(handle);
        if (ev_handle->type == ENUM_HANDLE_SEM_EVENT) {
            // get current time
            struct timeval now;
            gettimeofday(&now, NULL);

            struct timespec ts = {0};
            ts.tv_sec = now.tv_sec;
            ts.tv_nsec = now.tv_usec * 1000; // usec to nsec
            // add the offset to get timeout value
            uint32_t sec = millisecs/1000;
            ts.tv_sec += sec;

            uint32_t nano_sec = (millisecs%1000) * 1000000;

            // total nano secs
            nano_sec += ts.tv_nsec;
            ts.tv_sec += nano_sec / 1000000000;
            ts.tv_nsec = nano_sec % 1000000000;

            int32_t ret = sem_timedwait(&ev_handle->sem_handle, &ts);
            if (ret == 0)
                val = WAIT_OBJECT_0;
            else {
                // nRet is -1
                if (errno == ETIMEDOUT)
                    val = WAIT_TIMEOUT;
                else {
                    VLOG(3) << "Sem_WaitForSingleObject, fail, "
                              "return error val:" << ret << ", err:" <<
                              errno << ":" << strerror(errno) <<
                              " ETIMEDOUT = " << ETIMEDOUT;
                    val = WAIT_ABANDONED;
                }
            }
        } else
            val = WAIT_ABANDONED;
    }
    return val;
}
#endif // linux
// /////////////////////////////////////////////////////

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_PORT_EVENT_H_
