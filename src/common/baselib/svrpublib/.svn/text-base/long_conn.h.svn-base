// long_conn.h: interface for the CLongConn class.
// wookin@tencent.com
//
// ////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_LONG_CONN_H_
#define COMMON_BASELIB_SVRPUBLIB_LONG_CONN_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

// ////////////////////////////
// Public data type and macro
// ////////////////////////////

//
// enum:
// description: socket handle type
//
enum SOCK_DIR {
    SOCK_DIR_UNKNOWN   = 0,
    SOCK_DIR_CONN_TO   = 100,   // connect to server
    SOCK_DIR_ACCEPT_IN = 200,   // new accept handle
};

//
// enum:
// description: 出错枚举, 错误类型
//
enum LONGCONN_ERR {
    LERR_OK = 0,
    LERR_FAIL = 2,            // unknown error
    LERR_INVALID_HANDLE,    // 句柄已经无效, 可能对方已经关闭
    LERR_OUT_OF_BUFF_NODES, // 发送buffer节点用光
    LERR_DATA_TOO_LONG,     // 发送数据太长
};

//
// enum:
// description: error number of task
//
enum TASK_ERR {
    TASK_ERR_OK = 0,
    TASK_ERR_DOWNSTREAM_OUT_OF_NODES,
};

// ////////////////////////////
// Public structs
// ////////////////////////////

#ifdef WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

//
// struct:      LongDatNode
// description: 收发数据节点
//
struct LongConnNode {
    unsigned char   node_state;
    BufferV         data;               // 使用动态内存管理,
    // 但是需要小于等于_MAX_BUFF_V,
    // 存放接收到的数据

    BasProtocolHead tmp_protocol_head;  // 临时保存数据包头部,
    // 用于检查其他非正常client扫描之用

    uint32_t        _should_trans_len;  // max. data length
    uint32_t        _transed_len;       // 已经传送的数据长度
    LongConnNode*   next;
    LongConnNode*   pre;
    uint32_t        app_serial_num;     // app. serial number
    struct timeval  send_start_time;    // send start time
    struct timeval  send_finish_time;   // send finish time
    struct timeval  recv_start_time;    // recv start time
    struct timeval  recv_finish_time;   // recv finsih time
    unsigned char   need_receive_data;  //  need receive response data
    //  from server

    unsigned char   try_recv_data_now;  //   ? 发送完毕, 等待接收数据

    //
    //  User data
    //
    struct _UserData {
        void*    group;
        uint32_t task_index;
    } UserData;

    void SetNeedRecvData(unsigned char need_recv) {
        need_receive_data = need_recv;
    }

    LongConnNode() {
        ResetPtr();
    }

    ~LongConnNode() {
    }

    void ResetPtr() {
        _should_trans_len = _transed_len = 0;
        next = pre = 0;
        app_serial_num = 0;
        need_receive_data = 0;
        try_recv_data_now = 0;
        memset(&UserData, 0, sizeof(struct _UserData));
        data.ResetParam();
    }
};



//
// struct:      LongConnSession
// description: Long connection session
//              管理该session下收发DataNode节点
//              1:节点发送完毕翻转到接收队列(如果需要接收)
//              2:定时检测发送超时, 接收超时
//
struct LongConnSession {
    SOCKET                sock;               // socket handle

    // serial num
    uint32_t              long_conn_serial_num;
    struct sockaddr_in    local_sock_name;    // hSock != INVALID_SOCKET才有效,
    // 保存长连接本地ip/port

    struct sockaddr_in    peer_sock_name;     // hSock != INVALID_SOCKET才有效,
    // 保存长连接对端ip/port

    SOCK_DIR              sock_dir;           // ? SOCK_DIR_CONN_TO or
    // SOCK_DIR_ACCEPT_IN

    XIPPortOnly           to_server_addr;     // address connect to server

    CDualLinkListQueueMgr_T<LongConnNode>
    recv_queue;         // 接收队列, 最新放到队尾

    LongConnNode*         temp_recv_node;     // *每次临时接收数据的节点

    CLinkListQueueMgr_T<LongConnNode>
    send_queue;         // 发送队列,
    // 发送节点的时候从尾部添加

    LongConnNode*         temp_send_node;     // *临时发送节点,
    // 发送新节点数据前
    // 先加入到临时发送节点,
    // 发送过程中的节点是
    // 不能被超时处理,
    // 否则tcp streaming被打断
    struct timeval         expired_time;      // 删除长连接的时间点
    LongConnSession* volatile next;
    LongConnSession* volatile pre;

    //
    // Node state data, auto manage
    //
    unsigned char             node_state;

    void Reset() {
        sock = INVALID_SOCKET;
        sock_dir = SOCK_DIR_UNKNOWN;
        memset(&to_server_addr, 0, sizeof(to_server_addr));

        // Try receive queue
        temp_recv_node = 0;

        // Try send queue
        temp_send_node = 0;

        ResetPtr();
    }

    void ResetPtr() {
        next = 0;
        pre = 0;
    }

    LongConnSession() {
        memset(&local_sock_name, 0, sizeof(local_sock_name));
        memset(&peer_sock_name, 0, sizeof(peer_sock_name));
        long_conn_serial_num = safe_rand();
        Reset();
        next = pre = 0;
    }

    //
    // 使用单向链表
    //
    void AppendSendNode(LongConnNode* node) {
        send_queue.AppendNodeAtTail(node);
    }

    //
    // Receive使用双向链表
    //
    void AppendRecvNode(LongConnNode*& recv_node) {
        // todo (lonely) lock？ 这个地方目前是单线程操作，不需要加锁
        // 如果后续修改为多线程操作，则需要加锁，队列模板类需要提供获取锁的接口
        LongConnNode* cur_node   = NULL;
        LTasksGroup*  cur_group  = NULL;
        LTasksGroup*  recv_group = reinterpret_cast<LTasksGroup*>(recv_node->UserData.group);

        cur_node = recv_queue.GetTailNodePtr();
        while (cur_node) {
            cur_group = reinterpret_cast<LTasksGroup*>(cur_node->UserData.group);
            if (IsReachedCheckTimePoint(&cur_group->group_timeout_point,
                                        &recv_group->group_timeout_point)) {
                cur_node = cur_node->pre;
                continue;
            }
            break;
        }
        recv_queue.InsertNode(cur_node, recv_node);
        recv_node = NULL;
    }
};

//
// struct:      LongConnHandleNode
// description: 异步关闭长连接句柄
//
struct LongConnHandleNode {
    LongConnHandle          lc;
    LongConnHandleNode*     next;
    unsigned char           node_state;
};

//
// struct:      LongConnNewSock
// description: 异步提交插入新连接
//
struct LongConnNewSockNode {
    SOCKET                  fd;
    LongConnNewSockNode*    next;
    unsigned char           node_state;
};

// ////////////////////////////
// Class
// ////////////////////////////

//
// class:       CLongConn
// description: 实现长连接
//              1:创建Long connection session(handle)
//              2:通过Long connection session(handle)收发数据
//              3:只负责单个节点数据的收发, 超时管理
//
class CLongConn:public CEpoll {
public:
    // //////////////////////////
    //
    // implement of Epoll method
    //
    virtual void Routine(uint32_t epoll_wait_millisecs);
    virtual void OnEvent(const epoll_event* ev);

    //
    // Listen callback
    //
    virtual void OnAccept(SOCKET listen_sock);
    virtual void OnEpollAsyncNotify(bool have_other_events);
    //
    // //////////////////////////

    LONGCONN_ERR SendNodeData(LongConnHandle long_conn_session,
                              LongConnNode* node);

    //
    // description: 创建Long connection session
    //              返回-1, or 0xFFFFFFFF表示失败
    //              可以使用这个session handle进行收发数据
    //
    LongConnHandle CreateLongConnSession(const char* host_ip, uint16_t port);
    void CloseLongConnSession(LongConnHandle long_conn_session);
    bool GetPeerNameOfLongConn(LongConnHandle long_conn_session,
                               uint32_t* host,
                               uint16_t* port);
    SOCKET GetSockHandleOfLongConn(LongConnHandle long_conn_session);

    // 后面使用单独的接收线程的时候使用
    bool Init(uint32_t max_sessions, const char* listen_host,
              uint16_t port, float timeout, ITasksGroupCallBack* callback,
              bool* break_epoll_wait = 0);

    // 后面使用竞争模型的时候使用
    bool Init(uint32_t max_sessions, SOCKET listen_sock,
              float timeout, ITasksGroupCallBack* callback,
              bool* break_epoll_wait = 0);

    // 接收新accept的FD
    bool InertNewAcceptedFD(SOCKET new_sock);

    void Uninit();

    CLongConn();
    virtual ~CLongConn();

protected:
    //
    // description: 1:通知已经完成一个数据包的接收
    //              2:完成部分任务通知
    //              3:超时通知
    //              4:函数中必须处理该节点
    //
    virtual void OnNotifyReceivedData(LongConnSession* long_conn_session,
                                      LongConnNode* node,
                                      bool is_req);
    virtual void OnNotifyError(LongConnSession* long_conn_session,
                               LongConnNode* node,
                               bool is_timeout,
                               TASK_ERR err = TASK_ERR_OK);
    virtual void OnNotifyFinishedSendData(LongConnSession* long_conn_session,
                                          LongConnNode* node,
                                          bool* is_continue_recv);

    //
    // can force send data
    //
    bool SendSessionData(LongConnSession* long_conn_session);

    //
    // 关闭SOCK_DIR_ACCEPT_IN类型的Long conn session,
    // 如果nAcceptedReqCount>0 则暂时保存节点
    void EndSession(LongConnSession* long_conn_session, bool need_call_back = true);

    //
    // 为用户请求任务保留发送回去的节点
    // 可以避免前面请求频率大于下游处理速度的时候,
    // 导致先发送请求成功并获得结果后,
    // 向上游发送结果的时候却无可用数据节点的情况
    // 保留节点可以跳出这个雪崩效应
    // CLinkListQueueMgr_T<LongConnNode>   m_oReservedNodesUserReq;

    bool IsValidNodePtr(const LongConnSession* long_conn_session);
    bool IsEpollAsyncNotifyEvent();
private:
    void   CheckTimeout();
    SOCKET ConnectTo(uint32_t ip, uint16_t port,
                     void* handle,
                     uint32_t timeout_secs);

    bool   ValidHandle(LongConnHandle long_conn_session);
    void   AddToValidLongConnHead(LongConnSession* long_conn_session);
    //
    // 尝试接收多个回应包, 直道所有回应包接收完毕, 或者对方关闭或处理接收包出错
    //
    bool ReceiveData(LongConnSession* long_conn_session, bool* end_connection);

    bool OnReceivedPackage(LongConnSession* long_conn_session);

    //
    // 通过序列号号从等待接收数据的队列里查找到对应的节点,
    // 并从队列里删除, 并返回节点
    //
    // 注意:
    //      1:通过同一个Session发给下游多个请求包,
    //        下游按FIFO顺序回应, 则不会执行查找动作.
    //        回应包和包头节点对应
    //
    //      2:回应包顺序错乱的时候才需要查找.
    //       (下游服务器使用多线程非同步处理会出现这个情况,
    //        :(, 这种情况极少, 或者下游服务器应该避免的错误框架)
    //
    //      3:连续n次(_MAX_SEQNUM_MISS)序列号查找失败
    //        则关闭该session(Long connection session)
    //
#define _MAX_SEQNUM_MISS   100
    LongConnNode* GetNodeInRecvQueue(LongConnSession* long_conn_session,
                                     uint32_t seq_num);

    //
    // Long conn sessions
    // 管理所有空闲的Long conn sessions
    //
    CLinkedListNodesMgr<LongConnSession>        m_long_conn_nodes;

    //
    // 管理所有的使用中sessions
    //
    CDualLinkListQueueMgr_T<LongConnSession>    m_long_conn_valid_queue;

    //
    // time out
    // time out, 精确到milliseconds 0.001s
    //
    struct timeval  m_timeout;
    struct timeval  m_last_check_timeout;

    CSpeedDbg   m_debug_force_send;
    CSpeedLimit m_accept_speed_limit;

    //
    // Accept new fd counter
    //
    uint32_t    m_accept_count;

    //
    // 长连接的Epoll必须支持退出机制,
    // 如果上层没有传入这个参数则强行使用这个参数
    //
    bool        m_break_epoll;

    //
    // Epoll::Routine()回来是收到异步通知消息, 且还有其他消息才置位该变量
    //
    bool        m_epoll_async_notify_event;

    //
    // 超时检测间隔时间
    //
    static struct timeval    m_check_timeout_interval;
    //
    // call back
    //
    ITasksGroupCallBack* m_call_back_obj;

    // 统计惊群次数
    uint32_t    m_thundering_herd_count;
};

// ////////////////////////////////////////////////////////////////////

//
// class:       CLongConnTasks
// description: 实现长连接, 从CLongConn派生
//              1:CLongConn负责单个数据节点的收发, 超时管理
//              2:CLongConnTasks负责管理一组任务各节点状态管理
//
class CLongConnTasks_PaddingProtec;
class CLongConnTasks:public CLongConn, public ILongConn {
public:
    CLongConnTasks();
    virtual ~CLongConnTasks();

    //
    // implement for ILongConn interface
    //
    virtual void    RoutineLongConn(uint32_t timeout_millisecs);
    virtual bool    InitLongConn(ITasksGroupCallBack* callback,
                                 uint32_t max_sessions,
                                 const char* listen_host,
                                 uint16_t port,
                                 float timeout,
                                 bool* break_epoll_wait = NULL,
                                 CallBackStrategyConfig* callback_config = NULL);

    virtual bool    InitLongConn(ITasksGroupCallBack* callback,
                                 uint32_t max_sessions,
                                 SOCKET listen_sock, float timeout,
                                 bool* break_epoll_wait = NULL,
                                 CallBackStrategyConfig* callback_config = NULL);

    virtual bool    SendData(LTasksGroup* task_group);
    virtual LongConnHandle CreateLongConnSession(const char* to_host,
            uint16_t port);

    virtual void    RemoveLongConnSession(LongConnHandle long_conn_session);
    virtual bool    InsertNewFD(SOCKET new_sock);
    virtual void    UninitLongConn();
    virtual void    Release();
    virtual bool    GetPeerName(LongConnHandle long_conn_session,
                                uint32_t* host,
                                uint16_t* port);

    virtual SOCKET  GetSockHandle(LongConnHandle long_conn_session);
    virtual void    ForceBreakCurrWait();

    virtual void GetWritePeerOfNotify(SOCKET** sock);
    virtual void CleanUpNotifyMessages();

    //
    // 设置保护探针hkey1, hkey2 on head
    // 设置保护探针tkey1, tkey2 at tail
    // 探测调用者是否内存越界威胁到本对象的运行安全 :(
    //
    void SetProbeKey(uint64_t key1, uint64_t key2,
                     uint64_t t_key1, uint64_t t_key2);
    void SetProbeObjPtr(CLongConnTasks_PaddingProtec* probe_obj);
protected:
    //
    // call back to caller
    //
    void OnTaskFinished(LTasksGroup* task_group);

    //
    // description: 1:通知已经完成一个数据包的接收或出错或超时
    //              2:具体描述查看class CLongConn
    //
    //
    virtual void OnNotifyReceivedData(LongConnSession* long_conn_session,
                                      LongConnNode* node, bool is_req);

    virtual void OnNotifyError(LongConnSession* long_conn_session,
                               LongConnNode* node,
                               bool is_timeout,
                               TASK_ERR err = TASK_ERR_OK);

    virtual void OnNotifyFinishedSendData(LongConnSession* long_conn_session,
                                          LongConnNode* node,
                                          bool* is_continue_recv);

    void CheckTasksFinished(LTasksGroup* task_group);

    //
    // 探测conitional task group是否达到回调条件
    //
    void CheckConditionTasksGroupStatus();

    void FreeTasksNode(LTasksGroup** node);

    //
    // Send out of nodes message to upstream
    //
    void SendLongErrorToUpstream(LongConnHandle long_conn_handle,
                                 LongConnNode* node);

private:
    void DealRoutineLongConn(uint32_t timeout_millisecs);
    void RealSendData(LTasksGroup* task_group);

    void SetTasksGroupStrategyInfo(LTasksGroup* task_group, LongConnNode* node);

    //
    // 探测调用者运行环境是否内存越界到这个地方
    //
    void CheckProbe();

    ITasksGroupCallBack*    m_callback;
    CBaseProtocolPack       m_pack_obj;

    //
    // Async submit queue for send data
    // 被别的线程提交异步数据,
    // 等待被本Routine线程有机会处理的时候加入到真正发送队列
    //
    CLinkListQueueMgr_T<LTasksGroup>         m_async_submit_send_queue;

    //
    // 保存需要主动探测是否达到回调条件的任务组
    //
    CDualLinkListQueueMgr_T<LTasksGroup>     m_check_need_callback_queue;
    struct timeval                           m_last_check_time;
    CallBackStrategyConfig                   m_callback_config;

    //
    // Async submit queue for remove long connection session
    //
    CLinkListQueueMgr_T<LongConnHandleNode>  m_async_submit_remove_long_conn;
    CLinkListQueueMgr_T<LongConnNewSockNode> m_async_submit_new_fd;

    //
    // 用于删除长连接时临时保存的节点
    //
    CSmartNodes<LongConnHandleNode>          m_long_conn_handle_nodes;

    //
    // 用于临时保存新提交socket fd的节点
    //
    CSmartNodes<LongConnNewSockNode>         m_long_conn_new_fd_nodes;

    //
    //  下面对象保护这个类对象被多个线程调用的保护
    //  一个对象不应该被多个线程调用
    //
    CXThreadMutex                            m_submit_data_mutex;

    //
    // probe key
    // 探测调用者环境是否内存越界到本对象区域
    //
    uint64_t        m_probe_key1;
    uint64_t        m_probe_key2;
    uint64_t        m_probe_tkey1;
    uint64_t        m_probe_tkey2;

    CLongConnTasks_PaddingProtec* m_probe_obj;

    uint32_t        m_debug_routine_count;

    //
    // 统计发送到一台机器上的发送频率
    //
    CSpeedDbg       m_debug_send;
    CSpeedDbg       m_debug_recv;
    CSpeedDbg       m_debug_err;
    CSpeedDbg       m_debug_call_send_data;
};

#define _MAX_PADDING_BLOCK  256
class CLongConnTasks_PaddingProtec {
public:
    // Head padding area
    uint64_t        m_key1;
    unsigned char   m_head_padding[_MAX_PADDING_BLOCK];
    uint64_t        m_key2;

    // Real object
    CLongConnTasks  m_long_conn_task;

    // Tail padding area
    uint64_t        m_tkey1;
    unsigned char   m_tail_padding[_MAX_PADDING_BLOCK];
    uint64_t        m_tkey2;

    CLongConnTasks_PaddingProtec() {
        m_key1 = m_key2 = m_tkey1 = m_tkey2 = (uint64_t)this;
        memset(m_tail_padding, 0, sizeof(m_tail_padding));
        memset(m_head_padding, 0, sizeof(m_head_padding));
    }

    virtual ~CLongConnTasks_PaddingProtec() {
    }
};

// 统计TCP连接个数
class CLongConnDebug {
public:
    CLongConnDebug() {
        m_call_create_count = 0;          // call create count
        m_long_conn_exist_count = 0;      // exist
        m_long_conn_create_ok_count = 0;  // create ok count
        m_long_conn_close_count = 0;      // close count
    }
    virtual ~CLongConnDebug() {}

    void RoutineCallCreate() {
        m_mutex.Lock();
        m_call_create_count++;

        //
        // just for debug
        //
        VLOG(3) << "Call, Long Conn Debug:"
                  "call create count:" << m_call_create_count << ", create ok count:" <<
                  m_long_conn_create_ok_count << ", "
                  "close count:" << m_long_conn_close_count << ", exist:" <<
                  m_long_conn_exist_count;
        m_mutex.UnLock();
    }
    void RoutineCreateOK() {
        m_mutex.Lock();
        m_long_conn_create_ok_count++;
        m_long_conn_exist_count++;

        //
        // just for debug
        //
        LOG(WARNING) << "Create, Long Conn Debug:"
                  "call create count:" << m_call_create_count << ", create ok count:" <<
                  m_long_conn_create_ok_count << ", "
                  "close count:" << m_long_conn_close_count << ", exist:" <<
                  m_long_conn_exist_count;
        m_mutex.UnLock();
    }
    void RoutineClose() {
        m_mutex.Lock();
        if (m_long_conn_exist_count >= 1)
            --m_long_conn_exist_count;
        m_long_conn_close_count++;

        //
        // just for debug
        //
        LOG(WARNING) << "close, Long Conn Debug:"
                        "call create count:" << m_call_create_count << ", "
                        "create ok count:" << m_long_conn_create_ok_count <<
                        ", close count:" << m_long_conn_close_count << ", exist:" <<
                        m_long_conn_exist_count;

        m_mutex.UnLock();
    }
private:
    int32_t volatile m_call_create_count;            // call create count
    int32_t volatile m_long_conn_exist_count;        // exist
    int32_t volatile m_long_conn_create_ok_count;    // create ok count
    int32_t volatile m_long_conn_close_count;        // close count
    CXThreadMutex    m_mutex;
};

// /////////////////////////////////////
// Public Long connection AUX parameters
// /////////////////////////////////////
class CLongConnAuxParameters {
public:
    CLongConnAuxParameters() {
        m_long_conn_obj_count = 0;
        m_create_count = 0;
    }

    virtual ~CLongConnAuxParameters() {
    }

    CXThreadMutex       m_long_conn_create_mutex;
    int32_t             m_long_conn_obj_count;
    int32_t             m_create_count;

#ifdef _DEBUG
    CLongConnDebug  m_long_conn_debug;
#endif //
};

#define __GetLongConnObjCount()     \
        (g_long_conn_aux_parameters->m_long_conn_obj_count)

#define __GetLongConnCreateMutex()  \
        (g_long_conn_aux_parameters->m_long_conn_create_mutex)

#define __GetLongConnCreateCount()  \
        (g_long_conn_aux_parameters->m_create_count)

#define __GetLongConnDebugObj()     \
        (g_long_conn_aux_parameters->m_long_conn_debug)

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_LONG_CONN_H_
