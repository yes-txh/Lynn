// epoll_accept_read.h: interface for the CEpollAcceptRead class.
// wookin 2007/01/08
// ////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_EPOLL_ACCEPT_READ_H_
#define COMMON_BASELIB_SVRPUBLIB_EPOLL_ACCEPT_READ_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

template<typename T_user_data>
class CEpollAcceptRead_T:public ITimeoutCallback<T_user_data> {
#define MAX_EPOLL_READ_DATA_LEN       256
#define MAX_TEMP_EPOLL_READ_EVENTS    256
#define MAX_READ_EPOLLWAIT_TIMEOUT    1
public:
    //  //////////////////////////////
    //  Interface of ITimeoutCallback
    //  //////////////////////////////
    virtual bool OnTimeoutCallback(TimedBuffNode<T_user_data>* timeout_node);

public:
    //
    //  线程驱动入口
    //
    void AcceptReadRoutine();

    //  继承类必须实现这个接口
    virtual bool OnReceivedRequest(SOCKET fd,
                                   const char* request_pack,
                                   uint32_t len, time_t t0) = 0;

    //  可以继承这个实现自己判断如何接收数据，本例中以'\r\n'为结束标志
    virtual ENUM_RECV_STATE TryReceiveData(TimedBuffNode<T_user_data>* node);

    //
    //  Start listen port
    //
    bool InitListen(const char* host, uint16_t port,
                    uint32_t max_recv_request_timeout,
                    uint16_t min_binary_head,
                    uint16_t listen_backlog);

    void Uninit();

    explicit CEpollAcceptRead_T(uint32_t max_fds);
    virtual ~CEpollAcceptRead_T();
private:
    bool SetRequestFD(SOCKET fd);

    SOCKET                          m_listen_sock;
    EPOLLHANDLE                     m_epoll_handle;
    unsigned char                   m_recv_buff[MAX_EPOLL_READ_DATA_LEN];
    TimedBuffNode<T_user_data>*     m_listen_node;
    CTimedLoopList_T<T_user_data>   m_timed_nodes_mgr;

    //
    //  最多可同时接收多少个tcp connections
    //
    uint32_t                m_max_fds;
    struct epoll_event      m_epoll_events[MAX_TEMP_EPOLL_READ_EVENTS];

    //
    //  Seconds
    //
    uint32_t                m_max_recv_request_timeout;

    //
    //  最小的二进制长度:适用于请求包前面为二进制头，后面为\r\n结束的文本数据
    //
    uint16_t                m_min_binary_head;
};

#include "common/baselib/svrpublib/epoll_accept_read_t.inl"

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_EPOLL_ACCEPT_READ_H_
