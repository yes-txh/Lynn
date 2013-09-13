// epoll_lite.h: interface for the CEpoll class.
// wookin@tencent.com
// 2007-05-30
// ////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_EPOLL_LITE_H_
#define COMMON_BASELIB_SVRPUBLIB_EPOLL_LITE_H_

#include "common/baselib/svrpublib/twse_type_def.h"
#include "common/baselib/svrpublib/fake_epoll.h"
#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

//
// class:       CEpoll
// description: epoll events wrapper
//
class CEpoll {
public:
    CEpoll();
    virtual ~CEpoll() {}

    //
    // Public method
    // Set epoll events size and listen info
    //
    bool    EpollInit(uint32_t epoll_size,
                      const char* listen_host = 0,
                      uint16_t listen_port = 0,
                      uint16_t listen_baklog = 1024,
                      bool* break_epoll_wait = 0);
    bool    EpollInit(uint32_t epoll_size,
                      SOCKET listen_handle,
                      bool* break_epoll_wait = 0);

    void    Uninit();

    // Routine
    virtual void    Routine(uint32_t epoll_wait_millesecs);

    bool    AddToEpoll(SOCKET sock, epoll_event* ev);
    bool    DeleteFromEpoll(SOCKET sock);
    bool    ModifyFromEpoll(SOCKET sock, epoll_event* new_ev);
    SOCKET  GetListenHandle() const;

    // Force break current epoll_wait
    void    ForceBreakCurEpollWait();

    // Clean notify message
    void    EmptyNotifyMessage();

    // Over ridable callbacks

    //
    // Get peer_A of socket pair
    // 需要绑定回调函数, 不能声明为const
    //
    void GetNotifyWriteSockOfSocketPair(SOCKET** sock);
protected:
    virtual void OnEvent(const epoll_event* ev) {
        if (!ev) {
            VLOG(3) << "invalid pEv";
        }
    }

    // Listen callback
    virtual void OnAccept(SOCKET listen_sock) {
        if (listen_sock == INVALID_SOCKET) {
            VLOG(3) << "invalid socket";
        }
    }

    virtual void OnListenError(SOCKET listen_sock) {
        if (listen_sock == INVALID_SOCKET) {
            VLOG(3) << "invalid listen socket";
        }
    }

    //
    // 处理接收到的异步通知事件, 本函数被epoll_wait的结果调用,
    // 和CEpoll同一个线程
    //
    // 在m_fdPairRead(异步通知句柄)上收到异步通知消息;
    // bHaveOtherEvents 是否还有其他事件
    //
    virtual void OnEpollAsyncNotify(bool have_other_events) {
        VLOG(3) << "OnEpollAsyncNotify, bHaveOtherEvents = " <<
                  (have_other_events ? "true":"false");
    }

private:
    bool            Initialize(uint32_t epoll_size, SOCKET listen_handle);
    bool            RebuildNotifySocketPair();

    bool            m_is_attached_listen_handle;
    SOCKET          m_listen_sock;

    // epoll handle and events
    EPOLLHANDLE     m_epoll_handle;
    epoll_event*    m_epoll_events;
    uint32_t        m_epoll_size;

    // 跳出epoll_wait, 响应消息退出
    bool*           m_break_epoll_wait;

    // 建立异步通知socket pair对, 用于提前跳出epoll_wait
    SOCKET  m_sock_pair_read;
    SOCKET  m_sock_pair_write;
};

//
// class:
// description: 限制接入过快
//
class CSpeedLimit {
public:
    CSpeedLimit() {
        m_max = 0;
        m_last_time = 0;
        m_count = 0;
    }

    virtual ~CSpeedLimit() {
    }

    void SetMaxLimit(uint32_t max) {
        m_max = max;
    }

    bool UpdateRefCount() {
        bool b = true;
        time_t time_now = time(0);
        if (m_last_time == 0)
            m_last_time = time_now;

        time_t t;
        t = time_now-m_last_time;
        t += (t == 0) ? 1:0;

        m_count++;
        uint32_t uSpeed = m_count/(uint32_t)t;
        if (uSpeed>m_max)
            b = false;

        // Reset
        if (t>3) {
            m_last_time = 0;
            m_count = 0;
        }
        return b;
    }
private:
    uint32_t m_max;
    time_t   m_last_time;
    uint32_t m_count;
};

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_EPOLL_LITE_H_
