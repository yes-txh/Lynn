//  general_thread_util.h
//  wookin
// //////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_GENERAL_THREAD_UTIL_H_
#define COMMON_BASELIB_SVRPUBLIB_GENERAL_THREAD_UTIL_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_



// ///////////////////////
// XThread Interface
// ///////////////////////
enum XTHREAD_STATE {
    XTHREAD_STATE_NOT_RUNNING = 0,
    XTHREAD_STATE_RUNNING,
    XTHREAD_STATE_PAUS,
    XTHREAD_STATE_STOPPING,
    XTHREAD_STATE_STOPPED
};

interface IXThreadBase {
public:
    // private
    virtual bool*         auxGetSuspendParam() = 0;

    virtual bool          StartThread() = 0;
    virtual void          PauseThread() = 0;
    virtual void          ResumeThread() = 0;

    virtual bool          IsSuspended() = 0;

    virtual bool          IsStopped() = 0;             //  �Ƿ��Ѿ�ֹͣ
    virtual bool          IsJoinable() = 0;
    virtual void          EndThread() = 0;

    virtual void          StopRoutine() = 0;           // �ṩ��Routine()
    // �ڲ�����,
    //
    // ���������֮��
    // �߳̽���ֹ

    virtual XTHREAD_STATE GetThreadState() = 0;
    virtual const char*   GetThreadStateDesc() = 0;
    virtual uint32_t      GetThreadID() = 0;

    virtual void          IndicateThreadStopped() = 0; // ֻ���߳���ں�������
    // ����

    virtual void          IndicateThreadRunning() = 0; // ֻ���߳���ں�������
    // ����

    virtual void          InsideIdle() = 0;

    virtual void          PreRoutine() = 0;
    virtual void          Routine() = 0;

    //
    // ���û��߼����ƻص�
    //
    virtual void          OnThreadStartRunning() = 0;  //  ���߳̿ռ亯��:
    //  �̴߳���֮��
    //  ������ѭ��֮ǰ
    //  �ص�һ��

    virtual void          OnThreadStopped() = 0;       //  ���߳̿ռ亯��:
    //  �߳����ֹͣ
    //  ��ʱ��ص�һ��,
    //  �ص�����߳̾��˳�

    //  Set thread name
    virtual void          SetThreadName(const char* thread_name) = 0;
    virtual char*         GetThreadName() = 0;

    // --- add by jackyzhao ---
    virtual void          ForceEndThread() = 0;
    // ------------------------

    // 1:�̴߳����ڼ���Ч
    // 2:�����ڲ�ʹ��:�������̺߳��½��߳��ڴ���ʱ��״̬ͬ��ʹ��
    virtual THANDLE       _InternalGetSyncEvent() = 0;

    virtual ~IXThreadBase() {}
    IXThreadBase():m_thread_id(0) {
        memset(&m_thread_handle, 0, sizeof(m_thread_handle));
    }

    uint32_t     m_thread_id;
    THREAD_T     m_thread_handle;
};

class CXThreadBase: public IXThreadBase {
public:
    virtual bool*   auxGetSuspendParam();
    virtual bool    StartThread();
    void            PauseThread();
    void            ResumeThread();

    bool            IsSuspended();           //  �Ƿ����
    bool            IsStopped();             //  �Ƿ��Ѿ�ֹͣ
    virtual bool    IsJoinable();
    virtual void    EndThread();
    virtual void    StopRoutine();           //  �ṩ��Routine()�ڲ�����,
    //  ���������֮���߳̽���ֹ
    XTHREAD_STATE   GetThreadState();
    uint32_t        GetThreadID();

    void            IndicateThreadStopped(); //  ֻ���߳���ں������Ե���
    void            IndicateThreadRunning(); //  ֻ���߳���ں������Ե���
    virtual void    InsideIdle();

    virtual void    PreRoutine();            //  �����������,
    //  ���Կ���Routine����Ƶ��

    virtual void    Routine() = 0;           //  �̳��߱���ʵ���������

    //
    // ���û��߼����ƻص�, �û������ؽ���ҵ���߼�����
    // �û���������������֮����Ҫ��ʾ����
    // CXXXThread::OnThreadStartRunning()
    // {
    //      CXThreadBase::OnThreadStartRunning(); ...�û�˽���߼�;
    // }
    //
    // CXXXThread::OnThreadStopped()
    //  {
    //      ...�û�˽���߼�;
    //      CXThreadBase::OnThreadStopped();
    //  }
    //
    //

    virtual void        OnThreadStartRunning(); //  ���߳̿ռ亯��:
    //  �̴߳���֮��,
    //  ������ѭ��֮ǰ�ص�һ��

    virtual void        OnThreadStopped();      //  ���߳̿ռ亯��:
    //  �߳����ֹͣ��ʱ��
    //  �ص�һ��, �ص����
    //  �߳̾��˳�

    virtual void        SetThreadName(const char* thread_name);
    virtual char*       GetThreadName();
    virtual const char* GetThreadStateDesc();

    //
    //  implement for IXThreadBase interface
    //  --- add by jackyzhao ---
    virtual void        ForceEndThread();
    //  ------------------------

    // 1:�̴߳����ڼ���Ч
    // 2:�����ڲ�ʹ��:�������̺߳��½��߳��ڴ���ʱ��״̬ͬ��ʹ��
    virtual THANDLE     _InternalGetSyncEvent();

public:
    CXThreadBase(bool is_joinable = true);
    virtual ~CXThreadBase();

private:

    XTHREAD_STATE volatile    m_thread_state;
    char                      m_thread_name[128];

    //
    // Set thread name
    //
    uint32_t                  m_set_thread_name_once;

    //
    // �������
    //
    bool                      m_thread_suspend;

    // �ڲ�ͬ��״̬ʹ��
    // ֻ�ڴ������̵߳�ʱ��ʹ��
    THANDLE                   m_sync_event;
    bool                      m_is_joinable;
};

//
// �����߳�
// linux, sig = SIGUSR2
//
void sigThreadSuspend(int32_t sig);



_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_GENERAL_THREAD_UTIL_H_
