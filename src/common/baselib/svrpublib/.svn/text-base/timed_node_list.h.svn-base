// timed_node_list.h: interface for the CTimedLoopList class.
// wookin@tencent.com
// ////////////////////////////////////////////////////////////////////
#ifndef COMMON_BASELIB_SVRPUBLIB_TIMED_NODE_LIST_H_
#define COMMON_BASELIB_SVRPUBLIB_TIMED_NODE_LIST_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

#ifdef WIN32
#pragma pack(push, 1)
#else // linux
#pragma pack(1)
#endif // linux

template<typename T_user_data>
struct TimedBuffNode {
    SOCKET   volatile fd;                   // 全局fd information
    time_t   volatile t0;
    char*    volatile data;
    uint16_t volatile should_max_valid_len; // data中有效数据应该具有的长度
    // Send data:一开始这个值是有效的
    // Receive data:一开始这个值是无效
    uint16_t volatile current_transed_len;
    T_user_data                 user_data;
    TimedBuffNode* volatile  next;
    TimedBuffNode* volatile  pre;
    void ResetParam() {
        should_max_valid_len = 0;
        current_transed_len = 0;
        t0 = 0;
    }
    TimedBuffNode() {
        fd = INVALID_SOCKET;
        t0 = 0;
        data = 0;
        should_max_valid_len = 0;
        current_transed_len = 0;
        next = 0;
        pre = 0;
    }
};

template<typename T_user_data>
struct NewTimedNode {
    unsigned char   volatile    valid;
    time_t          volatile    t0;
    T_user_data                 user_data;
    NewTimedNode*   volatile    next;
    NewTimedNode*   volatile    pre;

    NewTimedNode() {
        valid = false;
        t0 = 0;
        next = 0;
        pre = 0;
    }
};

#ifdef WIN32
#pragma pack(pop)
#else // linux
#pragma pack()
#endif //

//
//  ITimeoutCallback
//  实现这个接口实现超时间回调处理
//
template<class T_user_data>
interface ITimeoutCallback {
public:

    //
    //  return value:
    //        true,  processed by user
    //        false, need auto remove from epoll set
    //               and close socket by CTimedLoopList object
    //
    virtual bool OnTimeoutCallback(
        TimedBuffNode<T_user_data>* timeout_node) = 0;

    ITimeoutCallback() {}
    virtual ~ITimeoutCallback() {}
};

//
//  CTimedLoopList
//  首尾相接的循环链表，头部插入最新数据，
//  每次进行超时检查从尾部朝前检测
//
//  非常适合于需要同时检测一个fd的多个信号的场景。
//  多个检测信号映射到同一个节点上EPOLLOUT|EPOLLIN|EPOLLERR
//
//  解决多个指针指向一个节点的问题
//  Q7     If more than one event comes in between epoll_wait(2) calls,
//         are they combined or reported separately ?
//
//  A7     They will be combined.
//

template<typename T_user_data>
class CTimedLoopList_T {
public:
    //
    //  if hEpollHandle is valid, auto remove fd from epoll set when time out.
    //  usMaxNodes:最好小于等于将连接服务器的backlog数目, listen(backlog)
    //
    //
    bool        Init(uint16_t max_nodes,
                     uint16_t node_buff_len,
                     EPOLLHANDLE epoll_handle,
                     uint16_t timeout,
                     ITimeoutCallback<T_user_data>* timeout_interface);

    void        Uninit();

    //  独立的不记time out的节点
    void        GetListenNode(TimedBuffNode<T_user_data>** node,
                              uint16_t* max_buff_len);
    inline bool IsBufferReady();

    //
    //  获取下一个可写入节点指针
    //
    bool        GetNextEmptyNodePtr(TimedBuffNode<T_user_data>** node,
                                    uint16_t* max_buff_len);

    //
    //  指示已经在下一个可写入节点上写数据完毕
    //
    bool        SetDataToNextNodeFinished(TimedBuffNode<T_user_data>* node);

    //
    //  指示关闭socket handle,
    //          如果一个socket同时被EPOLLIN, EPOLLOUT, EPOLLERR监听，
    //  同时触发的情况下，可能被访问多次
    //
    void        IndicateCloseSocket(TimedBuffNode<T_user_data>* node,
                                    int32_t reason);

    //
    //  将socket handle从epoll set移出，并不关闭句柄
    //
    void        RemoveHandleFromEpollSet(TimedBuffNode<T_user_data>* node);

    uint16_t    GetNodeDataBuffLen() {
        return m_node_data_buff_len;
    }
    void        CheckTimeOut();

    //
    //  用户可以实现这个函数处理超时socket handle
    //
    virtual void OnAutoNodeTimeOut(TimedBuffNode<T_user_data>* node);

    CTimedLoopList_T();
    virtual ~CTimedLoopList_T();
private:
    void                RemoveFDFromEpollSet(SOCKET fd);

    TimedBuffNode<T_user_data>*             m_nodes;
    char*                                   m_data_buff;
    uint16_t                                m_node_data_buff_len;

    TimedBuffNode<T_user_data>*    volatile m_node_head;
    TimedBuffNode<T_user_data>*    volatile m_node_tail;

    EPOLLHANDLE                             m_epoll_handle;
    uint16_t                                m_timeout_interval;
    time_t                                  m_last_check_timeout;
    TimedBuffNode<T_user_data>              m_listen_node;
    ITimeoutCallback<T_user_data>*          m_timeout_callback;
};

//
//  ITimeoutCallback
//  实现这个接口实现超时间回调处理
//
template<class T>
interface INewTimeoutCallback {
public:
    //
    //  return value:
    //                true, processed by user
    //                false, need auto remove from epoll set
    //                      and close socket by CTimedLoopList object
    //
    virtual bool    OnTimeoutCallback(T user_data) = 0;
    INewTimeoutCallback() {}
    virtual ~INewTimeoutCallback() {}
};

//
//  只进行节点超时管理
//
template<typename T_user_data>
class CNewTimedLoopList_T {
public:

    //
    //  最好小于等于将连接服务器的backlog数目, listen(backlog)
    //
    bool        Init(uint16_t max_nodes,
                     uint16_t timeout,
                     INewTimeoutCallback<T_user_data>* callback);
    void        Uninit();

    inline bool IsBufferReady();

    //
    //  获取下一个可写入节点指针
    //
    bool        GetNextEmptyNodePtr(NewTimedNode<T_user_data>** node);

    //
    //  指示已经在下一个可写入节点上写数据完毕
    //
    bool        SetDataToNextNodeFinished(NewTimedNode<T_user_data>* node);

    //
    //  将有效节点队列移出, 并标记无效
    //
    void        SetNodeInvalid(NewTimedNode<T_user_data>* node);
    void        CheckTimeOut();

    CNewTimedLoopList_T();
    virtual ~CNewTimedLoopList_T();
private:
    void        OnTimeOut(NewTimedNode<T_user_data>* node);
    NewTimedNode<T_user_data>*                m_nodes;
    NewTimedNode<T_user_data>*    volatile    m_node_head;
    NewTimedNode<T_user_data>*    volatile    m_node_tail;
    uint16_t                                m_timeout_interval;
    time_t                                    m_last_check_timeout;
    INewTimeoutCallback<T_user_data>*       m_timeout_callback;
};

#include "common/baselib/svrpublib/timed_node_list_t.inl"

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_TIMED_NODE_LIST_H_
