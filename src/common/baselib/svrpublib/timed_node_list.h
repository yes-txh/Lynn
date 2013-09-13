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
    SOCKET   volatile fd;                   // ȫ��fd information
    time_t   volatile t0;
    char*    volatile data;
    uint16_t volatile should_max_valid_len; // data����Ч����Ӧ�þ��еĳ���
    // Send data:һ��ʼ���ֵ����Ч��
    // Receive data:һ��ʼ���ֵ����Ч
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
//  ʵ������ӿ�ʵ�ֳ�ʱ��ص�����
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
//  ��β��ӵ�ѭ������ͷ�������������ݣ�
//  ÿ�ν��г�ʱ����β����ǰ���
//
//  �ǳ��ʺ�����Ҫͬʱ���һ��fd�Ķ���źŵĳ�����
//  �������ź�ӳ�䵽ͬһ���ڵ���EPOLLOUT|EPOLLIN|EPOLLERR
//
//  ������ָ��ָ��һ���ڵ������
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
    //  usMaxNodes:���С�ڵ��ڽ����ӷ�������backlog��Ŀ, listen(backlog)
    //
    //
    bool        Init(uint16_t max_nodes,
                     uint16_t node_buff_len,
                     EPOLLHANDLE epoll_handle,
                     uint16_t timeout,
                     ITimeoutCallback<T_user_data>* timeout_interface);

    void        Uninit();

    //  �����Ĳ���time out�Ľڵ�
    void        GetListenNode(TimedBuffNode<T_user_data>** node,
                              uint16_t* max_buff_len);
    inline bool IsBufferReady();

    //
    //  ��ȡ��һ����д��ڵ�ָ��
    //
    bool        GetNextEmptyNodePtr(TimedBuffNode<T_user_data>** node,
                                    uint16_t* max_buff_len);

    //
    //  ָʾ�Ѿ�����һ����д��ڵ���д�������
    //
    bool        SetDataToNextNodeFinished(TimedBuffNode<T_user_data>* node);

    //
    //  ָʾ�ر�socket handle,
    //          ���һ��socketͬʱ��EPOLLIN, EPOLLOUT, EPOLLERR������
    //  ͬʱ����������£����ܱ����ʶ��
    //
    void        IndicateCloseSocket(TimedBuffNode<T_user_data>* node,
                                    int32_t reason);

    //
    //  ��socket handle��epoll set�Ƴ��������رվ��
    //
    void        RemoveHandleFromEpollSet(TimedBuffNode<T_user_data>* node);

    uint16_t    GetNodeDataBuffLen() {
        return m_node_data_buff_len;
    }
    void        CheckTimeOut();

    //
    //  �û�����ʵ�������������ʱsocket handle
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
//  ʵ������ӿ�ʵ�ֳ�ʱ��ص�����
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
//  ֻ���нڵ㳬ʱ����
//
template<typename T_user_data>
class CNewTimedLoopList_T {
public:

    //
    //  ���С�ڵ��ڽ����ӷ�������backlog��Ŀ, listen(backlog)
    //
    bool        Init(uint16_t max_nodes,
                     uint16_t timeout,
                     INewTimeoutCallback<T_user_data>* callback);
    void        Uninit();

    inline bool IsBufferReady();

    //
    //  ��ȡ��һ����д��ڵ�ָ��
    //
    bool        GetNextEmptyNodePtr(NewTimedNode<T_user_data>** node);

    //
    //  ָʾ�Ѿ�����һ����д��ڵ���д�������
    //
    bool        SetDataToNextNodeFinished(NewTimedNode<T_user_data>* node);

    //
    //  ����Ч�ڵ�����Ƴ�, �������Ч
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
