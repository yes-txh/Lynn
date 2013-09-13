//
//  general_thread_util.cpp
//  wookin
// //////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib.h"

#include "common/baselib/svrpublib/base_config.h"



_START_XFS_BASE_NAMESPACE_

#ifdef WIN32
#define THREAD_EXIT_X(x) _endthreadex(x)
#define THREAD_RETURN_VAL 0
#else // linux
#define THREAD_EXIT_X(x) pthread_exit(x)
#define THREAD_RETURN_VAL NULL
#endif //



// New thread entry
#ifdef WIN32
unsigned int WINAPI
#else // linux
void*
#endif
XRunEntry(void *arg) {
    //
    // û���κβ������̲߳������ǿ��Ƶ��߳�
    //
    assert(arg != NULL);

    // Get thread ID
    // Linux:  getpid returns thread ID when gettid is absent
    IXThreadBase* thread_base_obj = reinterpret_cast<IXThreadBase*>(arg);
    thread_base_obj->m_thread_id = GetTID();

    // ���Ĭ���� unjoinable
#ifndef WIN32
    if(!thread_base_obj->IsJoinable()) {
        //  Switch to unjoinable
        pthread_detach(pthread_self());

        //  �����˳��߳�
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

        //  ��������ȡ��
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    }
#endif //

    //
    //  Resume thread
    //  ���ε�������ͣ�̵߳Ĺ���
#ifndef WIN32
    TLS_SET_VAL(g_lib_vars.m_tls_thread_continue,
                reinterpret_cast<void*>(thread_base_obj->auxGetSuspendParam()));
    signal(SIGUSR2, sigThreadSuspend);
#endif //
    //

    // ��ʾ���߳���Ϣ
    VLOG(3) << "[*new thread <id:" << thread_base_obj->GetThreadID() << ",addr:" <<
              reinterpret_cast<void*>(thread_base_obj) << ",name:" <<
              thread_base_obj->GetThreadName() << ">] XTHREAD_STATE_RUNNING";

    // ��־���߳��Ѿ�����������
    thread_base_obj->IndicateThreadRunning();

    // ֪ͨ���߳̽���ͬ���ȴ�
    // THANDLE h_event = thread_base_obj->_InternalGetSyncEvent();
    // Sem_SetEvent(h_event);

    XTHREAD_STATE volatile    state = thread_base_obj->GetThreadState();
    thread_base_obj->OnThreadStartRunning();
    while (state != XTHREAD_STATE_STOPPING) {
        switch (state) {
        case XTHREAD_STATE_RUNNING:
            thread_base_obj->InsideIdle();
            thread_base_obj->PreRoutine();
            thread_base_obj->Routine();
            break;
        case XTHREAD_STATE_PAUS:
            thread_base_obj->PreRoutine();
            break;
        default:
            break;
        }
        state = thread_base_obj->GetThreadState();
    }

    VLOG(3) << "Thread[name: " << thread_base_obj->GetThreadName() <<
              "] end. thread id: " << thread_base_obj->m_thread_id;
    thread_base_obj->OnThreadStopped();

    thread_base_obj->IndicateThreadStopped();

    // ֪ͨ��ĵȴ��߳�(�п������߳��ڵ���EndThread()):�߳̽���
    // Sem_SetEvent(h_event);

    if(!thread_base_obj->IsJoinable()) {
        THREAD_EXIT_X(0);
    }
    return THREAD_RETURN_VAL;
}

//
//  class CXThreadBase
//
CXThreadBase::CXThreadBase(bool is_joinable)
    :m_thread_suspend(false),
     m_sync_event(NULL) {
    m_is_joinable = is_joinable;
    m_set_thread_name_once = 0;
    m_thread_state = XTHREAD_STATE_NOT_RUNNING;
    m_thread_name[0] = 0;

    // �����߳�ͬ������
    // m_sync_event = Sem_CreateEvent(NULL, 0, 0, NULL);
    // CHECK(m_sync_event != NULL);
}

CXThreadBase::~CXThreadBase() {
    EndThread();

    Sem_CloseHandle(m_sync_event);
    m_sync_event = NULL;
}

bool CXThreadBase::StartThread() {
    bool b = false;

    IXThreadBase* thread_base_obj = this;
#ifdef WIN32
    m_thread_handle = (HANDLE)_beginthreadex(NULL, 0, XRunEntry, thread_base_obj, 0, &m_thread_id);
    if (m_thread_handle != NULL)
        b = true;
#else // linux    
    if (pthread_create(&m_thread_handle, NULL, XRunEntry, thread_base_obj) == 0) {
        b = true;
    } else {
        b = false;
    }
#endif // linux

    // start thread �ɹ�
    if (b) {
        // �ȴ��߳�������������
        // Sem_WaitForSingleObject(m_sync_event, (uint32_t)-1);
        while(m_thread_state == XTHREAD_STATE_NOT_RUNNING) {
            XUSleep(1);
        }

        if(m_thread_state == XTHREAD_STATE_NOT_RUNNING) {
            LOG(ERROR) << "create new thread fail.";
        }

        // ���߳��Ѿ���������
        VLOG(3) << "create new thread ok, thread id:" << thread_base_obj->GetThreadID();
    } else {
        LOG(ERROR) << "create thread fail:" << strerror(errno);
    }

    return b;
}

void sigThreadSuspend(int32_t sig) {
#ifndef WIN32
    if (sig == SIGUSR2) {
        int32_t* is_thread_suspend = NULL;
        is_thread_suspend =
            reinterpret_cast<int32_t*>(
                TLS_GET_VAL(g_lib_vars.m_tls_thread_continue)
            );

        if (is_thread_suspend) {
#ifdef _DEBUG
            VLOG(3) << "., thread paused.";
#endif // _DEBUG
            while (*is_thread_suspend)
                XSleep(10);
        }
    }
#else
    sig = sig;
#endif //
}

THANDLE CXThreadBase::_InternalGetSyncEvent() {
    return m_sync_event;
}

void CXThreadBase::PauseThread() {
    m_thread_suspend = true;

#ifdef WIN32
    SuspendThread((HANDLE)m_thread_handle);
#ifdef _DEBUG
    VLOG(3) << "thread paused.";
#endif //
#else
    pthread_kill(m_thread_handle, SIGUSR2);
#endif //

    m_thread_state = XTHREAD_STATE_PAUS;
}

void CXThreadBase::ResumeThread() {
#ifdef _DEBUG
    VLOG(3) << "thread resume.";
#endif //
#ifdef WIN32
    ::ResumeThread((HANDLE)m_thread_handle);
#endif //
    m_thread_suspend = false;

    m_thread_state = XTHREAD_STATE_RUNNING;
}

bool CXThreadBase::IsSuspended() {
    return m_thread_suspend;
}

bool CXThreadBase::IsStopped() {
    bool b = (m_thread_state == XTHREAD_STATE_STOPPED);
    if (b) {
        XUSleep(1000);
    }
    return b;
}

bool CXThreadBase::IsJoinable() {
    return m_is_joinable;
}

bool* CXThreadBase::auxGetSuspendParam() {
#ifdef WIN32
    return 0;
#else
    return &m_thread_suspend;
#endif //
}

//
//  ֻ���߳���ں������Ե���
//
void CXThreadBase::IndicateThreadStopped() {
    m_thread_state = XTHREAD_STATE_STOPPED;
}

//
//  �ṩ��Routine()�ڲ�����, ���������֮���߳̽���ֹ
//
void CXThreadBase::StopRoutine() {
    m_thread_state = XTHREAD_STATE_STOPPING;
}

void CXThreadBase::IndicateThreadRunning() {
    m_thread_state = XTHREAD_STATE_RUNNING;
}

//  ��Ҫ֧��EndThread����ε���
void    CXThreadBase::EndThread() {
    switch (m_thread_state) {
    case XTHREAD_STATE_NOT_RUNNING:
        break;
    case XTHREAD_STATE_RUNNING:
    case XTHREAD_STATE_STOPPING: {
        if(m_thread_state == XTHREAD_STATE_RUNNING)
            m_thread_state = XTHREAD_STATE_STOPPING;

        if(IsJoinable()) {
#ifdef WIN32
            while (m_thread_state != XTHREAD_STATE_STOPPED)
                XUSleep(1);
#else
            pthread_join(m_thread_handle, NULL);
#endif //
        } else {
            while (m_thread_state != XTHREAD_STATE_STOPPED)
                XUSleep(1);
            // Sem_WaitForSingleObject(m_sync_event, 100);
        }

        memset(&m_thread_handle, 0, sizeof(m_thread_handle));
    }
    break;
    case XTHREAD_STATE_PAUS:
        ResumeThread();
        EndThread();
        break;
    case XTHREAD_STATE_STOPPED:
        break;
    default:
        break;
    }
}

//
//  ---- add by jackyzhao -------
//  ǿ����ֹ�߳�
//
void CXThreadBase::ForceEndThread() {
    // Cancel thread
#ifdef WIN32

#else // linux
    if (m_thread_state != XTHREAD_STATE_NOT_RUNNING &&
        m_thread_state != XTHREAD_STATE_STOPPED) {
        pthread_cancel(m_thread_handle);
        memset(&m_thread_handle, 0, sizeof(m_thread_handle));
    }
#endif // linux
    m_thread_state = XTHREAD_STATE_STOPPED;
}

inline XTHREAD_STATE CXThreadBase::GetThreadState() {
    return m_thread_state;
}

uint32_t CXThreadBase::GetThreadID() {
    return m_thread_id;
}

void CXThreadBase::PreRoutine() {
    if (m_thread_state == XTHREAD_STATE_PAUS) {
        XUSleep(1);
    }
}

void CXThreadBase::SetThreadName(const char* thread_name) {
    if (STRLEN(thread_name) < sizeof(m_thread_name)) {
        memset(m_thread_name, 0, sizeof(m_thread_name));
        safe_snprintf(m_thread_name, sizeof(m_thread_name), "%s", thread_name);
        m_set_thread_name_once = 1;
    }
}

void CXThreadBase::InsideIdle() {
}

char* CXThreadBase::GetThreadName() {
    return m_thread_name;
}

const char* CXThreadBase::GetThreadStateDesc() {
    switch (m_thread_state) {
    case XTHREAD_STATE_NOT_RUNNING:
        return "XTHREAD_STATE_NOT_RUNNING";
    case XTHREAD_STATE_RUNNING:
        return "XTHREAD_STATE_RUNNING";
    case XTHREAD_STATE_PAUS:
        return "XTHREAD_STATE_PAUS";
    case XTHREAD_STATE_STOPPING:
        return "XTHREAD_STATE_STOPPING";
    case XTHREAD_STATE_STOPPED:
        return "XTHREAD_STATE_STOPPED";
    default:
        return "XTHREAD_STATE_UNKNOWN";
    }
}

//
// ���߳̿ռ亯��:�̴߳���֮��, ������ѭ��֮ǰ�ص�һ��
//
void CXThreadBase::OnThreadStartRunning() {
}

//
// ���߳̿ռ亯��:�߳����ֹͣ��ʱ��ص�һ��,
// �ص�����߳̾��˳�
//
void CXThreadBase::OnThreadStopped() {
}

_END_XFS_BASE_NAMESPACE_
