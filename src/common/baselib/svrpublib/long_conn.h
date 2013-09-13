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
// description: ����ö��, ��������
//
enum LONGCONN_ERR {
    LERR_OK = 0,
    LERR_FAIL = 2,            // unknown error
    LERR_INVALID_HANDLE,    // ����Ѿ���Ч, ���ܶԷ��Ѿ��ر�
    LERR_OUT_OF_BUFF_NODES, // ����buffer�ڵ��ù�
    LERR_DATA_TOO_LONG,     // ��������̫��
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
// description: �շ����ݽڵ�
//
struct LongConnNode {
    unsigned char   node_state;
    BufferV         data;               // ʹ�ö�̬�ڴ����,
    // ������ҪС�ڵ���_MAX_BUFF_V,
    // ��Ž��յ�������

    BasProtocolHead tmp_protocol_head;  // ��ʱ�������ݰ�ͷ��,
    // ���ڼ������������clientɨ��֮��

    uint32_t        _should_trans_len;  // max. data length
    uint32_t        _transed_len;       // �Ѿ����͵����ݳ���
    LongConnNode*   next;
    LongConnNode*   pre;
    uint32_t        app_serial_num;     // app. serial number
    struct timeval  send_start_time;    // send start time
    struct timeval  send_finish_time;   // send finish time
    struct timeval  recv_start_time;    // recv start time
    struct timeval  recv_finish_time;   // recv finsih time
    unsigned char   need_receive_data;  //  need receive response data
    //  from server

    unsigned char   try_recv_data_now;  //   ? �������, �ȴ���������

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
//              �����session���շ�DataNode�ڵ�
//              1:�ڵ㷢����Ϸ�ת�����ն���(�����Ҫ����)
//              2:��ʱ��ⷢ�ͳ�ʱ, ���ճ�ʱ
//
struct LongConnSession {
    SOCKET                sock;               // socket handle

    // serial num
    uint32_t              long_conn_serial_num;
    struct sockaddr_in    local_sock_name;    // hSock != INVALID_SOCKET����Ч,
    // ���泤���ӱ���ip/port

    struct sockaddr_in    peer_sock_name;     // hSock != INVALID_SOCKET����Ч,
    // ���泤���ӶԶ�ip/port

    SOCK_DIR              sock_dir;           // ? SOCK_DIR_CONN_TO or
    // SOCK_DIR_ACCEPT_IN

    XIPPortOnly           to_server_addr;     // address connect to server

    CDualLinkListQueueMgr_T<LongConnNode>
    recv_queue;         // ���ն���, ���·ŵ���β

    LongConnNode*         temp_recv_node;     // *ÿ����ʱ�������ݵĽڵ�

    CLinkListQueueMgr_T<LongConnNode>
    send_queue;         // ���Ͷ���,
    // ���ͽڵ��ʱ���β�����

    LongConnNode*         temp_send_node;     // *��ʱ���ͽڵ�,
    // �����½ڵ�����ǰ
    // �ȼ��뵽��ʱ���ͽڵ�,
    // ���͹����еĽڵ���
    // ���ܱ���ʱ����,
    // ����tcp streaming�����
    struct timeval         expired_time;      // ɾ�������ӵ�ʱ���
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
    // ʹ�õ�������
    //
    void AppendSendNode(LongConnNode* node) {
        send_queue.AppendNodeAtTail(node);
    }

    //
    // Receiveʹ��˫������
    //
    void AppendRecvNode(LongConnNode*& recv_node) {
        // todo (lonely) lock�� ����ط�Ŀǰ�ǵ��̲߳���������Ҫ����
        // ��������޸�Ϊ���̲߳���������Ҫ����������ģ������Ҫ�ṩ��ȡ���Ľӿ�
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
// description: �첽�رճ����Ӿ��
//
struct LongConnHandleNode {
    LongConnHandle          lc;
    LongConnHandleNode*     next;
    unsigned char           node_state;
};

//
// struct:      LongConnNewSock
// description: �첽�ύ����������
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
// description: ʵ�ֳ�����
//              1:����Long connection session(handle)
//              2:ͨ��Long connection session(handle)�շ�����
//              3:ֻ���𵥸��ڵ����ݵ��շ�, ��ʱ����
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
    // description: ����Long connection session
    //              ����-1, or 0xFFFFFFFF��ʾʧ��
    //              ����ʹ�����session handle�����շ�����
    //
    LongConnHandle CreateLongConnSession(const char* host_ip, uint16_t port);
    void CloseLongConnSession(LongConnHandle long_conn_session);
    bool GetPeerNameOfLongConn(LongConnHandle long_conn_session,
                               uint32_t* host,
                               uint16_t* port);
    SOCKET GetSockHandleOfLongConn(LongConnHandle long_conn_session);

    // ����ʹ�õ����Ľ����̵߳�ʱ��ʹ��
    bool Init(uint32_t max_sessions, const char* listen_host,
              uint16_t port, float timeout, ITasksGroupCallBack* callback,
              bool* break_epoll_wait = 0);

    // ����ʹ�þ���ģ�͵�ʱ��ʹ��
    bool Init(uint32_t max_sessions, SOCKET listen_sock,
              float timeout, ITasksGroupCallBack* callback,
              bool* break_epoll_wait = 0);

    // ������accept��FD
    bool InertNewAcceptedFD(SOCKET new_sock);

    void Uninit();

    CLongConn();
    virtual ~CLongConn();

protected:
    //
    // description: 1:֪ͨ�Ѿ����һ�����ݰ��Ľ���
    //              2:��ɲ�������֪ͨ
    //              3:��ʱ֪ͨ
    //              4:�����б��봦��ýڵ�
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
    // �ر�SOCK_DIR_ACCEPT_IN���͵�Long conn session,
    // ���nAcceptedReqCount>0 ����ʱ����ڵ�
    void EndSession(LongConnSession* long_conn_session, bool need_call_back = true);

    //
    // Ϊ�û��������������ͻ�ȥ�Ľڵ�
    // ���Ա���ǰ������Ƶ�ʴ������δ����ٶȵ�ʱ��,
    // �����ȷ�������ɹ�����ý����,
    // �����η��ͽ����ʱ��ȴ�޿������ݽڵ�����
    // �����ڵ�����������ѩ��ЧӦ
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
    // ���Խ��ն����Ӧ��, ֱ�����л�Ӧ���������, ���߶Է��رջ�����հ�����
    //
    bool ReceiveData(LongConnSession* long_conn_session, bool* end_connection);

    bool OnReceivedPackage(LongConnSession* long_conn_session);

    //
    // ͨ�����кźŴӵȴ��������ݵĶ�������ҵ���Ӧ�Ľڵ�,
    // ���Ӷ�����ɾ��, �����ؽڵ�
    //
    // ע��:
    //      1:ͨ��ͬһ��Session�������ζ�������,
    //        ���ΰ�FIFO˳���Ӧ, �򲻻�ִ�в��Ҷ���.
    //        ��Ӧ���Ͱ�ͷ�ڵ��Ӧ
    //
    //      2:��Ӧ��˳����ҵ�ʱ�����Ҫ����.
    //       (���η�����ʹ�ö��̷߳�ͬ����������������,
    //        :(, �����������, �������η�����Ӧ�ñ���Ĵ�����)
    //
    //      3:����n��(_MAX_SEQNUM_MISS)���кŲ���ʧ��
    //        ��رո�session(Long connection session)
    //
#define _MAX_SEQNUM_MISS   100
    LongConnNode* GetNodeInRecvQueue(LongConnSession* long_conn_session,
                                     uint32_t seq_num);

    //
    // Long conn sessions
    // �������п��е�Long conn sessions
    //
    CLinkedListNodesMgr<LongConnSession>        m_long_conn_nodes;

    //
    // �������е�ʹ����sessions
    //
    CDualLinkListQueueMgr_T<LongConnSession>    m_long_conn_valid_queue;

    //
    // time out
    // time out, ��ȷ��milliseconds 0.001s
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
    // �����ӵ�Epoll����֧���˳�����,
    // ����ϲ�û�д������������ǿ��ʹ���������
    //
    bool        m_break_epoll;

    //
    // Epoll::Routine()�������յ��첽֪ͨ��Ϣ, �һ���������Ϣ����λ�ñ���
    //
    bool        m_epoll_async_notify_event;

    //
    // ��ʱ�����ʱ��
    //
    static struct timeval    m_check_timeout_interval;
    //
    // call back
    //
    ITasksGroupCallBack* m_call_back_obj;

    // ͳ�ƾ�Ⱥ����
    uint32_t    m_thundering_herd_count;
};

// ////////////////////////////////////////////////////////////////////

//
// class:       CLongConnTasks
// description: ʵ�ֳ�����, ��CLongConn����
//              1:CLongConn���𵥸����ݽڵ���շ�, ��ʱ����
//              2:CLongConnTasks�������һ��������ڵ�״̬����
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
    // ���ñ���̽��hkey1, hkey2 on head
    // ���ñ���̽��tkey1, tkey2 at tail
    // ̽��������Ƿ��ڴ�Խ����в������������а�ȫ :(
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
    // description: 1:֪ͨ�Ѿ����һ�����ݰ��Ľ��ջ�����ʱ
    //              2:���������鿴class CLongConn
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
    // ̽��conitional task group�Ƿ�ﵽ�ص�����
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
    // ̽����������л����Ƿ��ڴ�Խ�絽����ط�
    //
    void CheckProbe();

    ITasksGroupCallBack*    m_callback;
    CBaseProtocolPack       m_pack_obj;

    //
    // Async submit queue for send data
    // ������߳��ύ�첽����,
    // �ȴ�����Routine�߳��л��ᴦ���ʱ����뵽�������Ͷ���
    //
    CLinkListQueueMgr_T<LTasksGroup>         m_async_submit_send_queue;

    //
    // ������Ҫ����̽���Ƿ�ﵽ�ص�������������
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
    // ����ɾ��������ʱ��ʱ����Ľڵ�
    //
    CSmartNodes<LongConnHandleNode>          m_long_conn_handle_nodes;

    //
    // ������ʱ�������ύsocket fd�Ľڵ�
    //
    CSmartNodes<LongConnNewSockNode>         m_long_conn_new_fd_nodes;

    //
    //  ������󱣻��������󱻶���̵߳��õı���
    //  һ������Ӧ�ñ�����̵߳���
    //
    CXThreadMutex                            m_submit_data_mutex;

    //
    // probe key
    // ̽������߻����Ƿ��ڴ�Խ�絽����������
    //
    uint64_t        m_probe_key1;
    uint64_t        m_probe_key2;
    uint64_t        m_probe_tkey1;
    uint64_t        m_probe_tkey2;

    CLongConnTasks_PaddingProtec* m_probe_obj;

    uint32_t        m_debug_routine_count;

    //
    // ͳ�Ʒ��͵�һ̨�����ϵķ���Ƶ��
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

// ͳ��TCP���Ӹ���
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
