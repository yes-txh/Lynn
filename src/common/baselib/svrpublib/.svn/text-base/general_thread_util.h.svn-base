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

    virtual bool          IsStopped() = 0;             //  是否已经停止
    virtual bool          IsJoinable() = 0;
    virtual void          EndThread() = 0;

    virtual void          StopRoutine() = 0;           // 提供给Routine()
    // 内部调用,
    //
    // 调用完这个之后
    // 线程将终止

    virtual XTHREAD_STATE GetThreadState() = 0;
    virtual const char*   GetThreadStateDesc() = 0;
    virtual uint32_t      GetThreadID() = 0;

    virtual void          IndicateThreadStopped() = 0; // 只有线程入口函数可以
    // 调用

    virtual void          IndicateThreadRunning() = 0; // 只有线程入口函数可以
    // 调用

    virtual void          InsideIdle() = 0;

    virtual void          PreRoutine() = 0;
    virtual void          Routine() = 0;

    //
    // 供用户逻辑控制回调
    //
    virtual void          OnThreadStartRunning() = 0;  //  新线程空间函数:
    //  线程创建之后，
    //  进入主循环之前
    //  回调一次

    virtual void          OnThreadStopped() = 0;       //  新线程空间函数:
    //  线程最后停止
    //  的时候回调一次,
    //  回调完毕线程就退出

    //  Set thread name
    virtual void          SetThreadName(const char* thread_name) = 0;
    virtual char*         GetThreadName() = 0;

    // --- add by jackyzhao ---
    virtual void          ForceEndThread() = 0;
    // ------------------------

    // 1:线程创建期间有效
    // 2:仅仅内部使用:用于主线程和新建线程在创建时候状态同步使用
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

    bool            IsSuspended();           //  是否挂起
    bool            IsStopped();             //  是否已经停止
    virtual bool    IsJoinable();
    virtual void    EndThread();
    virtual void    StopRoutine();           //  提供给Routine()内部调用,
    //  调用完这个之后线程将终止
    XTHREAD_STATE   GetThreadState();
    uint32_t        GetThreadID();

    void            IndicateThreadStopped(); //  只有线程入口函数可以调用
    void            IndicateThreadRunning(); //  只有线程入口函数可以调用
    virtual void    InsideIdle();

    virtual void    PreRoutine();            //  重载这个函数,
    //  可以控制Routine调用频率

    virtual void    Routine() = 0;           //  继承者必须实现这个函数

    //
    // 供用户逻辑控制回调, 用户可重载进行业务逻辑控制
    // 用户集成这两个方法之后需要显示调用
    // CXXXThread::OnThreadStartRunning()
    // {
    //      CXThreadBase::OnThreadStartRunning(); ...用户私有逻辑;
    // }
    //
    // CXXXThread::OnThreadStopped()
    //  {
    //      ...用户私有逻辑;
    //      CXThreadBase::OnThreadStopped();
    //  }
    //
    //

    virtual void        OnThreadStartRunning(); //  新线程空间函数:
    //  线程创建之后,
    //  进入主循环之前回调一次

    virtual void        OnThreadStopped();      //  新线程空间函数:
    //  线程最后停止的时候
    //  回调一次, 回调完毕
    //  线程就退出

    virtual void        SetThreadName(const char* thread_name);
    virtual char*       GetThreadName();
    virtual const char* GetThreadStateDesc();

    //
    //  implement for IXThreadBase interface
    //  --- add by jackyzhao ---
    virtual void        ForceEndThread();
    //  ------------------------

    // 1:线程创建期间有效
    // 2:仅仅内部使用:用于主线程和新建线程在创建时候状态同步使用
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
    // 挂起进程
    //
    bool                      m_thread_suspend;

    // 内部同步状态使用
    // 只在创建新线程的时候使用
    THANDLE                   m_sync_event;
    bool                      m_is_joinable;
};

//
// 挂起线程
// linux, sig = SIGUSR2
//
void sigThreadSuspend(int32_t sig);



_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_GENERAL_THREAD_UTIL_H_
