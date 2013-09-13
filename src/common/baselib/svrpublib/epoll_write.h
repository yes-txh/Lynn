// epoll_write.h: interface for the CEpollWrite class.
// wookin@tencent.com    2007/01/04
// ////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_EPOLL_WRITE_H_
#define COMMON_BASELIB_SVRPUBLIB_EPOLL_WRITE_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

template<uint32_t T_max_queue_len,
         uint32_t T_max_data_len,
         typename T_user_data>
class CEpollWrite_T {
#define MAX_WRITE_EPOLLWAIT_TIMEOUT        1   //  Milliseconds
#define MAX_TEMP_EPOLL_WRITE_EVENTS        256
public:
    CEpollWrite_T();
    virtual ~CEpollWrite_T();

    bool    Init(uint32_t max_fds, uint32_t    timeout);
    void    Uninit();
    void    Routine();

    //
    //  设置需要读取数据的句柄
    //
    bool    SetOutputData(SOCKET fd, time_t t0, bool close_fd_when_finish,
                          const unsigned char* data,
                          uint32_t len);

    inline bool    IsBufferReady() {
        return m_nodes_list.IsBufferReady();
    }

private:
    ENUM_SEND_STATE        TrySendData(TimedBuffNode<T_user_data>* node);
    EPOLLHANDLE            m_epoll_handle;
    struct epoll_event     m_epoll_events[MAX_TEMP_EPOLL_WRITE_EVENTS];

    //
    //  管理超时节点
    //
    CTimedLoopList_T<T_user_data>    m_nodes_list;
};

#include "common/baselib/svrpublib/epoll_write_t.inl"

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_EPOLL_WRITE_H_
