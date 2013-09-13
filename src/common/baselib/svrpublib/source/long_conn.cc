// long_conn.cpp: implementation of the CLongConn class.
//
//////////////////////////////////////////////////////////////////////

#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

#ifdef WIN32
#pragma   warning(disable:4127)
#endif // WIN32

// /////////////////////////
// Construction/Destruction
// /////////////////////////

extern CLongConnAuxParameters*  g_long_conn_aux_parameters;

// 超时检测间隔时间暂定为10ms
struct timeval CLongConn::m_check_timeout_interval = {0, 10*1000};

CLongConn::CLongConn() {
#if (defined(AVG_ACCEPT_TCP))
    CXThreadAutoLock autoLock(&(__GetAutoLigMgrObj().m_avg_accept_mutex));
    (__GetAutoLigMgrObj()).m_num_long_conn_object_count++;
#endif // AVG_ACCEPT_TCP

    m_accept_count = 0;
    m_long_conn_valid_queue.SetQueueEmpty();

    memset(&m_timeout, 0, sizeof(m_timeout));
    memset(&m_last_check_timeout, 0, sizeof(m_last_check_timeout));
    m_debug_force_send.SetString("*force send*:");
    m_accept_speed_limit.SetMaxLimit(3000);

    m_break_epoll = false;

    m_thundering_herd_count = 0;
}

CLongConn::~CLongConn() {
#ifdef AVG_ACCEPT_TCP
    CXThreadAutoLock autoLock(&__GetAutoLigMgrObj().m_avg_accept_mutex);
    __GetAutoLigMgrObj().m_num_long_conn_object_count--;
#endif // AVG_ACCEPT_TCP
}

bool CLongConn::Init(uint32_t max_sessions,
                     SOCKET listen_sock,
                     float timeout,
                     ITasksGroupCallBack* call_back,
                     bool* break_epoll_wait) {
    bool b = false;
    // call back
    m_call_back_obj = call_back;
    // time out
    // seconds
    m_timeout.tv_sec  = int32_t(timeout);
    // milliseconds, 0.001s
    int32_t milliseconds = (int32_t((timeout)*1000.0))%1000;
    m_timeout.tv_usec = milliseconds*1000;  // micro seconds

    // start time
    lite_gettimeofday(&m_last_check_timeout, NULL);

    // Long conn sessions
    uint32_t max = MAX(1, max_sessions);
    m_long_conn_valid_queue.SetQueueEmpty();
    if (m_long_conn_nodes.Init(max)) {
        b = true;
#ifdef WIN32
        uint32_t epoll_size = 64;
#else
        uint32_t epoll_size = 256;
#endif //
        b &= CEpoll::EpollInit(epoll_size, listen_sock,
                               break_epoll_wait ?
                               break_epoll_wait :
                               (&m_break_epoll));
    }

    if (!b)
        Uninit();
    return b;
}

bool CLongConn::Init(uint32_t max_sessions,
                     const char* listen_host, uint16_t port,
                     float timeout,
                     ITasksGroupCallBack* call_back,
                     bool* break_epoll_wait) {
    bool b = false;
    // call back
    m_call_back_obj = call_back;
    // time out
    // seconds
    m_timeout.tv_sec  = int32_t(timeout);
    // milliseconds, 0.001s
    int32_t milliseconds = (int32_t((timeout)*1000.0))%1000;
    m_timeout.tv_usec = milliseconds*1000;  // micro seconds

    // start time
    lite_gettimeofday(&m_last_check_timeout, NULL);

    // Long conn sessions
    uint32_t max = MAX(1, max_sessions);
    m_long_conn_valid_queue.SetQueueEmpty();
    if (m_long_conn_nodes.Init(max)) {
        b = true;
#ifdef WIN32
        uint32_t epoll_size = 64;
#else
        uint32_t epoll_size = 128;
#endif //
        if (listen_host)
            b &= CEpoll::EpollInit(epoll_size, listen_host, port, 1024,
                                   break_epoll_wait ?
                                   break_epoll_wait :
                                   (&m_break_epoll));
        else
            b &= CEpoll::EpollInit(epoll_size, INVALID_SOCKET,
                                   break_epoll_wait ?
                                   break_epoll_wait :
                                   (&m_break_epoll));
    }

    if (!b)
        Uninit();
    return b;
}

void CLongConn::Uninit() {
    CEpoll::Uninit();

    m_long_conn_valid_queue.SetQueueEmpty();
    m_long_conn_nodes.Uninit();
}

SOCKET CLongConn::ConnectTo(uint32_t ip, uint16_t port, void* handle,
                            uint32_t timeout_secs) {
    // Try connect to server
    in_addr addr;
    addr.s_addr = ip;

#ifdef _DEBUG
    VLOG(3) << "begin ConnectTo:" << inet_ntoa(addr) << ":" << ntohs(port);
#endif //

    SOCKET sock = NewSocket(true);
    if (sock == INVALID_SOCKET) {
        LOG(ERROR) << "NewSocket fail, maybe out of handles limit.";
        return sock;
    }

    struct sockaddr_in to_addr;
    memset(&to_addr, 0, sizeof(to_addr));
    to_addr.sin_family = AF_INET;
    to_addr.sin_addr.s_addr = ip;
    to_addr.sin_port = port;
    bool writeable = false;
    int32_t err_conn = connect(sock,
                               (struct sockaddr*)&to_addr,
                               sizeof(to_addr));
    // add by lonely @ check temp local port different with peer listen port
    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    memset(&local_addr, 0, sizeof(local_addr));
    getsockname(sock, (struct sockaddr*)&local_addr, &addr_len);

    if (err_conn == 0) {
        // linux垃圾本机连接马上返回
        writeable = true;
    } else {
        writeable = ConnWriteable(sock, timeout_secs, &to_addr);
    }
    // note by lonely: 端口一样直接置为失败
    if (ntohs(local_addr.sin_port) == port)
    {
        writeable = false;
    }

    if (!writeable) {
        LOG(ERROR) << "Connect to server FAIL:" << inet_ntoa(addr) << ":" << ntohs(port) <<
                   ", error:" << errno << ":" << strerror(errno) << ", socket = " << sock;

        CloseSocketA(sock, TCP_FD_NEW);
        sock = INVALID_SOCKET;
    } else {
        VLOG(3) << "Connect to server *SUCCESS*:" << inet_ntoa(addr) << ":" << ntohs(port) <<
                  ", socket = " << sock;

        // Try add to epoll set
        epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
        ev.data.ptr = handle;
        if (!AddToEpoll(sock, &ev)) {
            VLOG(3) << "add to epoll set fail.";
            CloseSocketA(sock, TCP_FD_NEW);
            sock = INVALID_SOCKET;
        }
    }

    return sock;
}

//
// description: 创建Long connection session
//              返回hSession.handle = 0表示失败
//              可以使用这个session handle进行收发数据
//
LongConnHandle CLongConn::CreateLongConnSession(const char* to_host,
        uint16_t port) {
#ifdef _DEBUG
    __GetLongConnDebugObj().RoutineCallCreate();
#endif //

    LongConnHandle hSession;
    hSession.serial_num = 0;
    hSession.handle = 0;
    VLOG(3) << "begin CLongConn::CreateLongConnSession...";
    if (!m_long_conn_nodes.IsQueueEmpty() && to_host && port) {
        LongConnSession* long_conn_node = 0;
        m_long_conn_nodes.GetNodeOnHead(&long_conn_node);
        long_conn_node->Reset();
        //
        // 每创建一次+1
        //
        long_conn_node->long_conn_serial_num++;

        // Save ip, port
        long_conn_node->to_server_addr.num_ip = inet_addr(to_host);
        long_conn_node->to_server_addr.port = htons(port);

        // Try connect to server
        long_conn_node->sock_dir = SOCK_DIR_CONN_TO;
        long_conn_node->sock = ConnectTo(long_conn_node->to_server_addr.num_ip,
                                         long_conn_node->to_server_addr.port,
                                         long_conn_node, 2);
        if (long_conn_node->sock != INVALID_SOCKET) {
            // Save socket name
            socklen_t addr_len = sizeof(struct sockaddr_in);
            getsockname(long_conn_node->sock,
                        (struct sockaddr*)&long_conn_node->local_sock_name,
                        &addr_len);
            addr_len = sizeof(struct sockaddr_in);
            getpeername(long_conn_node->sock,
                        (struct sockaddr*)&long_conn_node->peer_sock_name,
                        &addr_len);
        } else {
            LOG(WARNING) << "CreateLongConnSession fail, "
                         "pLCNode->hSock = " << long_conn_node->sock;
            // modify by lonelyjia: net should not keep fixed ip and port;
            m_long_conn_nodes.AppendNodeAtTail(long_conn_node);
            return hSession;
        }

        // Pre. alloc temp receive data node
        long_conn_node->temp_recv_node = mempool_NEW(LongConnNode);
        if (!long_conn_node->temp_recv_node) {
            LOG(WARNING) << "CreateLongConnSession, "
                         "new temp receive data node fail. out of nodes.";
        } else {
            long_conn_node->temp_recv_node->ResetPtr();
        }

        // Make session handle
        hSession.handle = reinterpret_cast<void*>(long_conn_node);
        hSession.serial_num = long_conn_node->long_conn_serial_num;

#ifdef _DEBUG
        if (hSession.handle != 0 && long_conn_node->sock != INVALID_SOCKET) {
            __GetLongConnDebugObj().RoutineCreateOK();
        }
#endif //

        // Add to valid queue
        AddToValidLongConnHead(long_conn_node);
    } else {
        LOG(WARNING) << "***WARNING, Level 3***, CreateLongConnSession, "
                     "out of long conn. nodes.";
    }

    return hSession;
}

//
// description: 删除存在的Long connection session,
//              关闭该Session 上的所有任务，
//              回收资源，可能原因是对端退出服务
//
void CLongConn::CloseLongConnSession(LongConnHandle long_conn_session) {
    VLOG(3) << "begin CLongConn::CloseLongConnSession...";

    LongConnSession* session =
        reinterpret_cast<LongConnSession*>(long_conn_session.handle);

    if (!m_long_conn_nodes.IsValidNodePtr(session) ||
            session->long_conn_serial_num != long_conn_session.serial_num) {
        LOG(ERROR) << "RemoveLongConnSession : "
                   "invalid session "
                   << reinterpret_cast<void*>(session)
                   << ", may expried session with wrong serialnum";
        return;
    }
    // 应用主动关闭链接不需要回调
    EndSession(session, false);
}

bool CLongConn::GetPeerNameOfLongConn(LongConnHandle long_conn_handle,
                                      uint32_t* host,
                                      uint16_t* port) {
    bool b = false;
    LongConnSession* long_conn_node =
        reinterpret_cast<LongConnSession*>(long_conn_handle.handle);

    if (m_long_conn_nodes.IsValidNodePtr(long_conn_node) &&
            long_conn_node->long_conn_serial_num == long_conn_handle.serial_num) {
        if (long_conn_node->sock_dir == SOCK_DIR_CONN_TO) {
            if (host)
                *host = long_conn_node->to_server_addr.num_ip;
            if (port)
                *port = long_conn_node->to_server_addr.port;
            b = true;
        } else {
            // Accept in
            if (long_conn_node->sock != INVALID_SOCKET) {
                if (host)
                    *host = long_conn_node->peer_sock_name.sin_addr.s_addr;
                if (port)
                    *port = long_conn_node->peer_sock_name.sin_port;
                b = true;
            }
        }
    }
    return b;
}

SOCKET CLongConn::GetSockHandleOfLongConn(LongConnHandle long_conn_handle) {
    SOCKET sock = INVALID_SOCKET;
    LongConnSession* long_conn_node =
        reinterpret_cast<LongConnSession*>(long_conn_handle.handle);

    if (m_long_conn_nodes.IsValidNodePtr(long_conn_node)
            && long_conn_node->long_conn_serial_num == long_conn_handle.serial_num) {
        sock = long_conn_node->sock;
    }
    return sock;
}

LONGCONN_ERR CLongConn::SendNodeData(LongConnHandle long_conn_session,
                                     LongConnNode* data_node) {
    LONGCONN_ERR err = LERR_INVALID_HANDLE;
    if (!data_node)
        return err;

    LongConnSession* long_conn_session_node =
        reinterpret_cast<LongConnSession*>(long_conn_session.handle);

    if (ValidHandle(long_conn_session)) {
        data_node->_should_trans_len = data_node->data.valid_len;
        data_node->_transed_len = 0;
        data_node->next = 0;
        data_node->pre = 0;
        data_node->try_recv_data_now = 0;
        lite_gettimeofday(&data_node->send_start_time, NULL);

        // get seq num. in package
        GetSeqNumInPack(data_node->data.buff,
                        data_node->data.valid_len,
                        &data_node->app_serial_num);


        // //////////////////////////////////////////////////
        // force send data
        // while (pDataNode->_uCurrTransedLen<pDataNode->_uShouldDataLen)
        // {
        //    int32_t nbytes = send(pLCSessionNode->hSock,
        //                    (char*)pDataNode->data.szBuff +
        //                     pDataNode->_uCurrTransedLen,
        //                     pDataNode->_uShouldDataLen -
        //                     pDataNode->_uCurrTransedLen, 0);
        //    if (nbytes>0)
        //        pDataNode->_uCurrTransedLen += nbytes;
        //    else
        //    {
        //        if (pDataNode->_uCurrTransedLen>0)
        //          LOG(ERROR) << "send package error, must end session";
        //        return LERR_FAIL;
        //    }
        // }

        // struct sockaddr_in addr;
        // socklen_t alen = sizeof(addr);
        // getpeername(pLCSessionNode->hSock, (struct sockaddr*)&addr, &alen);
        // printf("send to:%s:%u\r\n",
        //        inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        // m_DbgForceSend.Routine();

        // bool bContinueRecv = false;
        // OnNotifyFinishedSendData((uint32_t)pLCSessionNode,
        //                         pDataNode,
        //                         &bContinueRecv,
        //                         pLCSessionNode->SockDir);
        // // ? 需要接收服务器响应
        // if (bContinueRecv)
        //    pLCSessionNode->AppendRecvNode(pDataNode);

        // err = LERR_OK;
        // //////////////////////////////////////////////////



        // Modify epoll event
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
        // Set session index as user data
        ev.data.ptr = long_conn_session.handle;
        if (ModifyFromEpoll(long_conn_session_node->sock, &ev)) {
            long_conn_session_node->AppendSendNode(data_node);
            // VLOG(3) << "after AppendSendNode, "
            //              "pLCSessionNode.oSendQue.Head = " <<
            //              reinterpret_cast<void*>(pLCSessionNode->oSendQue.GetHeadNodePtr());
            err = LERR_OK;
        } else {
            LOG(ERROR) << "ModifyFromEpoll fail.Long conn session[" <<
                       reinterpret_cast<void*>(long_conn_session_node) << "]."
                       "sock = " << long_conn_session_node->sock <<
                       ", pDataNode = " << reinterpret_cast<void*>(data_node);
            err = LERR_FAIL;
        }
    }
    return err;
}

void CLongConn::AddToValidLongConnHead(LongConnSession* node) {
    m_long_conn_valid_queue.AppendNodeAtTail(node);
}

bool CLongConn::IsValidNodePtr(const LongConnSession* long_conn_session) {
    return m_long_conn_nodes.IsValidNodePtr(long_conn_session);
}

bool CLongConn::ValidHandle(LongConnHandle long_conn_handle) {
    bool b = false;
    LongConnSession* long_conn_node =
        reinterpret_cast<LongConnSession*>(long_conn_handle.handle);

    if (long_conn_node->sock_dir == SOCK_DIR_UNKNOWN ||
            !IsValidNodePtr(long_conn_node) ||
            long_conn_handle.serial_num != long_conn_node->long_conn_serial_num) {
        LOG(ERROR) << "ValidHandle() fail, "
                   "pLCNode->SockDir = " << long_conn_node->sock_dir <<
                   "(0:SOCK_DIR_UNKNOWN), "
                   "pSession: " << reinterpret_cast<void*>(long_conn_node) <<
                   ", fd = " << long_conn_node->sock;

        VLOG(3) << "m_oLongConnNodes.IsValidNodePtr(pLCNode) = " <<
                  m_long_conn_nodes.IsValidNodePtr(long_conn_node) <<
                  ", pLCNode =  " << reinterpret_cast<void*>(long_conn_node);

        VLOG(3) << "hLCSession.SerialNum == pLCNode->LCSerialNum is:" <<
                  (long_conn_handle.serial_num == long_conn_node->long_conn_serial_num) << ", "
                  "hLCSession.SerialNum:" << long_conn_handle.serial_num <<
                  ", pLCNode->LCSerialNum:" << long_conn_node->long_conn_serial_num;
        VLOG(3) << "";
        return false;
    }

    // 只有SOCK_DIR_CONN_TO类型才进行重连
    if (long_conn_node->sock == INVALID_SOCKET &&
            long_conn_node->sock_dir == SOCK_DIR_CONN_TO) {
        // Reconnect to server
        in_addr addr;
        addr.s_addr = long_conn_node->to_server_addr.num_ip;
        LOG(WARNING) << "ValidHandle, try reconnect to server:" <<
                     inet_ntoa(addr) << ":" << ntohs(long_conn_node->to_server_addr.port);

        long_conn_node->sock = ConnectTo(long_conn_node->to_server_addr.num_ip,
                                         long_conn_node->to_server_addr.port,
                                         long_conn_handle.handle, 1);
        if (long_conn_node->sock != INVALID_SOCKET) {
            // Save socket name
            socklen_t addr_len = sizeof(struct sockaddr_in);
            getsockname(long_conn_node->sock,
                        (struct sockaddr*)&long_conn_node->local_sock_name,
                        &addr_len);
            addr_len = sizeof(struct sockaddr_in);
            getpeername(long_conn_node->sock,
                        (struct sockaddr*)&long_conn_node->peer_sock_name,
                        &addr_len);
        } else {
            addr.s_addr = long_conn_node->to_server_addr.num_ip;
            LOG(ERROR) << "reconnect to server result:" <<
                       inet_ntoa(addr) << ":" << ntohs(long_conn_node->to_server_addr.port) <<
                       " fail.";
        }
    }

    if (long_conn_node->sock != INVALID_SOCKET)
        b = true;

    return b;
}

void CLongConn::OnEvent(const epoll_event* ev) {
#ifdef _DEBUG
    VLOG(3) << "";
    VLOG(3) << "***OnEvent***:";
#endif //
    if (!ev)
        return;

    // 优先处理EPOLLIN, 如果出现EPOLLERR, 则放弃后面可能出现的EPOLLOUT
    LongConnSession* session =
        reinterpret_cast<LongConnSession*>(ev->data.ptr);

    // epoll in
    if ((ev->events & EPOLLIN) == EPOLLIN) {
        // 尝试接收该连接上的所有包, 并且尝试接收完每个数据包的完整包
        uint32_t num_ok_count = 0;
#define _MAX_RECV_CONTINUE  5
        do {
            bool is_end_connection = false;
            if (ReceiveData(session, &is_end_connection))
                num_ok_count++;
            else {
                if (is_end_connection) {
                    // Try receive data fail
                    VLOG(3) << "receive data error or "
                              "check serial number fail, try EndSession:";
                    EndSession(session);
                    return;
                }
                break;
            }
        } while (num_ok_count < _MAX_RECV_CONTINUE);
    } // modify by lonelyjia: epoll in & epoll err can happen sametime?
    else if ((ev->events & EPOLLERR) == EPOLLERR ||
             (ev->events & EPOLLHUP) == EPOLLHUP) {
        // Check remote closed
        char sz[4];
        int32_t bytes = recv(session->sock, sz, 1, MSG_PEEK);
        if (bytes  == 0 || bytes  == -1) {
            VLOG(3) << "remote closed. error info: "
                    << strerror(errno);
        } else {
            LOG(ERROR) << "error event for hock, maybe remote closed*: error info:"
                       << strerror(errno);
        }

        EndSession(session);
        return;
    }

    // epoll out
    if ((ev->events & EPOLLOUT) == EPOLLOUT) {
        if (!SendSessionData(session)) {
            VLOG(3) << "send data error, "
                      "session =  " << reinterpret_cast<void*>(session) <<
                      ", session->pSendNodeHead = " <<
                      reinterpret_cast<void*>(session ?
                                              session->send_queue.GetHeadNodePtr() :
                                              NULL);
            EndSession(session);
            return;
        }
    }
}

void CLongConn::OnEpollAsyncNotify(bool have_other_events) {
    m_epoll_async_notify_event = have_other_events;
}

bool CLongConn::IsEpollAsyncNotifyEvent() {
    return m_epoll_async_notify_event;
}

void CLongConn::OnAccept(SOCKET listen_sock) {
    VLOG(3) << "begin CLongConn::OnAccept...";
#ifdef AVG_ACCEPT_TCP
    CXThreadAutoLock autoLock(&__GetAutoLigMgrObj().m_avg_accept_mutex);
    float favg = __GetAutoLigMgrObj().m_num_total_accept /
                 static_cast<float>(
                     __GetAutoLigMgrObj().m_num_long_conn_object_count
                 );
    if (static_cast<float>(m_accept_count) > favg)
        return;
#endif // AVG_ACCEPT_TCP

    struct sockaddr_in from_addr;
    SOCKET new_sock = AcceptNewConnection(listen_sock, &from_addr);
    if (new_sock == INVALID_SOCKET) {
        ++m_thundering_herd_count;
        if(m_thundering_herd_count % 50 == 0){
            LOG(WARNING) << "accept new socket fail, maybe thundering herd happened, count:"
                         << m_thundering_herd_count << "times";
        }
        return;
    } else {
        VLOG(3) << "成功接受了一个进入的连接, socket = " << new_sock;
    }

    // if (!m_AcceptSpeedLimit.UpdateRefCount())
    // {
    //    VLOG(3) << "accept new socket too fast.";
    //    CloseSocketA(hNewSock, TCP_FD_ACCEPTED);
    //    return ;
    // }

    // Maybe no empty session nodes
    if (!InertNewAcceptedFD(new_sock)) {
        LOG(ERROR) << "OnAccept, InertNewAcceptedFD(fd = " << new_sock << ") fail.";
        CloseSocketA(new_sock, TCP_FD_ACCEPTED);
        VLOG(3) << "";
    } else {
        // Insert ok
#ifdef AVG_ACCEPT_TCP
        m_accept_count++;
        __GetAutoLigMgrObj().m_num_total_accept++;
        VLOG(3) << "this: " << reinterpret_cast<void*>(this) << ", AcceptCount = , " <<
                  m_accept_count << "total accept count = " <<
                  __GetAutoLigMgrObj().m_num_total_accept << ", "
                  "total long conn. objs = " <<
                  __GetAutoLigMgrObj().m_num_long_conn_object_count <<
                  ", favg = " << favg;
#endif // AVG_ACCEPT_TCP
    }
}

//
// 如果失败, 不会关闭socket, 需要调用者显示关闭socket
//
bool CLongConn::InertNewAcceptedFD(SOCKET new_sock) {
    VLOG(3) << "CLongConn::InertNewAcceptedFD...";
    if (new_sock == INVALID_SOCKET)
        return false;

    // 检查保留足够可用的节点, 防止临时accept fd过多
    // LongConnNode* ptr = 0;
    // int32_t nFreeNodes = _GETFREENODES(ptr);
    // if (nFreeNodes <= 48)
    //    return false;

    bool b = false;
    if (!m_long_conn_nodes.IsQueueEmpty()) {
        LongConnSession* long_conn_node = 0;
        m_long_conn_nodes.GetNodeOnHead(&long_conn_node);
        long_conn_node->Reset();

        long_conn_node->sock = new_sock;
        long_conn_node->long_conn_serial_num++;
        long_conn_node->sock_dir = SOCK_DIR_ACCEPT_IN;

        // pre. alloc temp receive data node
        long_conn_node->temp_recv_node = mempool_NEW(LongConnNode);
        if (long_conn_node->temp_recv_node) {
            long_conn_node->temp_recv_node->ResetPtr();
            bool bGetPeerName = false;
            // Save socket name
            socklen_t addr_len = sizeof(struct sockaddr_in);
            if (getsockname(long_conn_node->sock,
                            (struct sockaddr*)&long_conn_node->local_sock_name,
                            &addr_len) == 0) {
                addr_len = sizeof(struct sockaddr_in);
                if (getpeername(long_conn_node->sock,
                                (struct sockaddr*)&long_conn_node->peer_sock_name,
                                &addr_len) == 0)
                    bGetPeerName = true;
                else {
                    LOG(ERROR) << "getpeername(fd = " << long_conn_node->sock << ") fail, errno:"
                               << errno << ":" << strerror(errno);
                }
            } else {
                LOG(ERROR) << "getsockname(fd = " << long_conn_node->sock << ") fail, errno:"
                           << errno << ":" << strerror(errno);
            }

            if (bGetPeerName) {
                // Try add to epoll set
                epoll_event ev;
                memset(&ev, 0, sizeof(ev));
                ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
                // Session index
                ev.data.ptr = long_conn_node;
                if (AddToEpoll(long_conn_node->sock, &ev)) {
                    AddToValidLongConnHead(long_conn_node);
                    b = true;
                } else { //  ? fail
                    LOG(ERROR) << "add to epoll fail.errno:" << errno << ":" << strerror(errno);
                }
            }
        } else {
            VLOG(3) << "InertNewAcceptedFD, new temp receive data node fail.";
        }

        // Fail, release source
        if (!b) {
            if (long_conn_node->temp_recv_node) {
                mempool_DELETE(long_conn_node->temp_recv_node);
                long_conn_node->temp_recv_node = 0;
            }
            long_conn_node->Reset();
            m_long_conn_nodes.AppendNodeAtTail(long_conn_node);
        }
    } else {
        VLOG(3) << "***WARNING, Level 3***, InertNewAcceptedFD() fail, "
                  "no empty session nodes.";
    }

    return b;
}

void CLongConn::OnNotifyReceivedData(LongConnSession* long_conn_handle,
                                     LongConnNode* long_conn_node,
                                     bool is_req ) {
    // Received data
    VLOG(3) << "OnNotifyReceivedData, handle = " <<
              reinterpret_cast<void*>(long_conn_handle);

    if (long_conn_node) {
        mempool_DELETE(long_conn_node);
    }
}

void CLongConn::OnNotifyError(LongConnSession* long_conn_handle,
                              LongConnNode* long_conn_node,
                              bool is_timeout,
                              TASK_ERR err) {
    VLOG(3) << "OnNotifyError(), "
              "handle = " << reinterpret_cast<void*>(long_conn_handle) <<
              ", is_timeout = " << is_timeout<< ", task error:" << err;

    if (long_conn_node) {
        mempool_DELETE(long_conn_node);
    }
}

void CLongConn::OnNotifyFinishedSendData(LongConnSession* handle,
        LongConnNode* long_conn_node,
        bool* need_continue_recv) {
    VLOG(3) << "OnNotifyFinishedSendData(), handle = " << reinterpret_cast<void*>(handle);
    *need_continue_recv = (long_conn_node->need_receive_data ? true:false);

    if (!(*need_continue_recv)) {
        if (long_conn_node) {
            mempool_DELETE(long_conn_node);
        }
    }
}

//
// 关闭SOCK_DIR_ACCEPT_IN类型的Long conn session,
// 对于SOCK_DIR_CONN_TO类型的Long conn session则永不关闭,
// 因为上层一直在使用这个handle
//
void CLongConn::EndSession(LongConnSession* long_conn_session, bool need_call_back) {
    VLOG(3) << "begin end session " << reinterpret_cast<void*>(long_conn_session);
    if (!long_conn_session) {
        return;
    }

    // Remove from epoll set
    CEpoll::DeleteFromEpoll(long_conn_session->sock);

    // Close socket
    if (long_conn_session->sock != INVALID_SOCKET) {
        LOG(INFO) << "end session, remote peer:"
                  << inet_ntoa(long_conn_session->peer_sock_name.sin_addr)
                  << ":"
                  << ntohs(long_conn_session->peer_sock_name.sin_port)
                  << ", local sock name:"
                  << inet_ntoa(long_conn_session->local_sock_name.sin_addr)
                  << ":"
                  << ntohs(long_conn_session->local_sock_name.sin_port);
    } else {
        LOG(ERROR) << "end session, pSession->hSock = INVALID_SOCKET, "
                      "can't get peer name and local sock name.";
    }

    CloseSocketA(long_conn_session->sock,
                 (long_conn_session->sock_dir == SOCK_DIR_ACCEPT_IN)  ?
                 TCP_FD_ACCEPTED :
                 TCP_FD_NEW);

    long_conn_session->sock = INVALID_SOCKET;
    memset(&long_conn_session->local_sock_name, 0, sizeof(struct sockaddr_in));
    memset(&long_conn_session->peer_sock_name, 0, sizeof(struct sockaddr_in));

    // rReset miss count

    // Remove all try receive nodes queue
    VLOG(3) << "通知该长连接上所有的接收节点将要关闭了,end session";
    if (long_conn_session->recv_queue.GetHeadNodePtr()) {
        VLOG(3) << "try remove all nodes from receive queue,"
                  "long_conn_session->recv_queue.GetHeadNodePtr(): " <<
                  reinterpret_cast<void*>(long_conn_session->recv_queue.GetHeadNodePtr());

        LongConnNode* recv_node = NULL;
        while (long_conn_session->recv_queue.GetNodeOnHead(&recv_node)) {
            LOG(ERROR) << "remove recv node:" << reinterpret_cast<void*>(recv_node)
                       << ", seq num: " << recv_node->app_serial_num;

            OnNotifyError(long_conn_session,
                          recv_node,
                          false);
        }
    }

    // Remove temp receive data node
    VLOG(3) << "remove pSession->pTempRecvNode: " <<
              reinterpret_cast<void*>(long_conn_session->temp_recv_node);
    if (long_conn_session->temp_recv_node) {
        mempool_DELETE(long_conn_session->temp_recv_node);
        long_conn_session->temp_recv_node = NULL;
    }

    VLOG(3) << "通知该长连接上所有的发送节点将要关闭了 end session";
    // Remove all try send nodes queue
    if (long_conn_session->send_queue.GetHeadNodePtr()) {
        VLOG(3) << "try remove all nodes from send queue, "
                  "long_conn_session->pSendNodeHead:  " <<
                  reinterpret_cast<void*>(long_conn_session->send_queue.GetHeadNodePtr());
        LongConnNode* send_node = NULL;
        while (long_conn_session->send_queue.GetNodeOnHead(&send_node)) {
            OnNotifyError(long_conn_session,
                          send_node,
                          false);
        }
    }

    // Remove temp send node
    VLOG(3) << "remove long_conn_session->temp_send_node:  " <<
              reinterpret_cast<void*>(long_conn_session->temp_send_node);

    if (long_conn_session->temp_send_node) {
        LOG(ERROR) << "通知该长连接上的临时发送节点将要关闭了,EndSession";
        OnNotifyError(long_conn_session,
                      long_conn_session->temp_send_node,
                      false);
    }
    long_conn_session->temp_send_node = 0;

    // 减小accept进来的计数
#ifdef AVG_ACCEPT_TCP
    CXThreadAutoLock autoLock(&__GetAutoLigMgrObj().m_avg_accept_mutex);
    __GetAutoLigMgrObj().m_num_total_accept--;
    m_accept_count--;
#endif // AVG_ACCEPT_TCP

    // Remove from valid queue
    m_long_conn_valid_queue.RemoveNode(long_conn_session);
    // 回调
    if (need_call_back && long_conn_session->sock_dir == SOCK_DIR_CONN_TO)
    {
        LongConnHandle lc_handle;
        lc_handle.handle = reinterpret_cast<void*>(long_conn_session);
        lc_handle.serial_num = long_conn_session->long_conn_serial_num;
        m_call_back_obj->OnClose(lc_handle);
    }
    long_conn_session->Reset();
    // 让序列号作废
    long_conn_session->long_conn_serial_num++;

    // Add to empty queue
    m_long_conn_nodes.AppendNodeAtTail(long_conn_session);

#ifdef _DEBUG
    __GetLongConnDebugObj().RoutineClose();
#endif //
}

void CLongConn::CheckTimeout() {
    // Get now time 精确到ms
    struct timeval cur_tm = {0};
    lite_gettimeofday(&cur_tm, 0);

    // ? reach check time out point
    if (!IsReachedCheckTimePoint(&cur_tm, &m_last_check_timeout))
        return;

    LongConnSession* session_head = m_long_conn_valid_queue.GetHeadNodePtr();
    while (session_head) {
        if (session_head->sock != INVALID_SOCKET) {
            //
            // Check send data time out
            // 如果发送节点超时未发送成功, 则说明对方可能阻塞.
            // 无法接收数据, end session
            //
            if (session_head->temp_send_node) {
                LTasksGroup* tasks_group =
                    reinterpret_cast<LTasksGroup*>(session_head->temp_send_node->UserData.group);
                CHECK(NULL != tasks_group);
                if (IsReachedCheckTimePoint(&cur_tm, &tasks_group->group_timeout_point)) {
                    LOG(ERROR) << "tmp send node "
                               << reinterpret_cast<void*>(session_head->temp_send_node)
                               << ", seq num " << session_head->temp_send_node->app_serial_num
                               << " time out, end session" << reinterpret_cast<void*>(session_head)
                               << ", remote peer:["
                               << inet_ntoa(session_head->peer_sock_name.sin_addr)
                               << ":"
                               << ntohs(session_head->peer_sock_name.sin_port) << "]";
                    CLongConn::EndSession(session_head);
                    lite_gettimeofday(&m_last_check_timeout, NULL);
                    return;
                }
            }

            // Send queue
            bool have_send_node_timeout = false;
            CLinkListQueueMgr_T<LongConnNode>* send_queue =
                &session_head->send_queue;

            LongConnNode* send_node = NULL;
            while ( (send_node = send_queue->GetHeadNodePtr())) {
                LTasksGroup* tasks_group =
                    reinterpret_cast<LTasksGroup*>(send_node->UserData.group);
                CHECK(NULL != tasks_group);
                if (IsReachedCheckTimePoint(&cur_tm, &tasks_group->group_timeout_point)) {
                    // Send queue is single linked list
                    send_queue->GetNodeOnHead(&send_node);
                    LOG(ERROR) << "in send queue, "
                               "node time out:pNode =  "
                               << reinterpret_cast<void*>(send_node)
                               << ", seq num: " << send_node->app_serial_num
                               << ", remote peer:["
                               << inet_ntoa(session_head->peer_sock_name.sin_addr)
                               << ":"
                               << ntohs(session_head->peer_sock_name.sin_port) << "]";
                    send_node->pre = NULL;
                    have_send_node_timeout = true;
                    // notify
                    OnNotifyError(session_head,
                                  send_node,
                                  true);
                } else
                    break;
            }

            //
            // ? all time out, remove EPOLL_OUT from epoll, modify epoll set
            // 曾经发生send node time out才检查
            //
            if (have_send_node_timeout &&
                    !send_queue->GetHeadNodePtr() &&
                    !session_head->temp_send_node) {
                struct epoll_event ev;
                memset(&ev, 0, sizeof(ev));
                ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
                ev.data.ptr = session_head;
                CEpoll::ModifyFromEpoll(session_head->sock, &ev);
            }

            //
            // Try receive data queue
            //
            CDualLinkListQueueMgr_T<LongConnNode>* recv_queue =
                &session_head->recv_queue;

            LongConnNode* recv_node = NULL;
            while ((recv_node = recv_queue->GetHeadNodePtr())) {
                LTasksGroup* tasks_group =
                    reinterpret_cast<LTasksGroup*>(recv_node->UserData.group);

                CHECK(NULL != tasks_group);
                if (IsReachedCheckTimePoint(&cur_tm, &tasks_group->group_timeout_point)) {
                    recv_queue->GetNodeOnHead(&recv_node);
                    LOG(ERROR) << "in recv queue,node "
                               <<  reinterpret_cast<void*>(recv_node)
                               << " time out"
                               << ",seq num " << recv_node->app_serial_num
                               <<  ", remote peer:["
                               << inet_ntoa(session_head->peer_sock_name.sin_addr)
                               << ":"
                               << ntohs(session_head->peer_sock_name.sin_port) << "]";
                    // notify
                    OnNotifyError(session_head,
                                  recv_node,
                                  true);
                } else
                    break;
            }
        } else {
            // LOG(WARNING) << "***WARNING, Level 3***, "
            //               "CheckTimeout(), "
            //               "ptrSessionHead =  " << reinterpret_cast<void*>(ptrSessionHead) <<
            //               ", but ptrSessionHead->hSock =  " <<
            //               reinterpret_cast<void*>(ptrSessionHead->hSock);
        }

        session_head = session_head->next;
    }

    lite_gettimeofday(&m_last_check_timeout, NULL);
    SetAbsTimeout(m_last_check_timeout, 10);
}



void CLongConn::Routine(uint32_t epoll_wait_millisecs) {
    //
    // 如果Epoll中检测到异步通知消息且还有其他事件,
    // 则会在OnEpollAsyncNotify中设置m_bEpollAsyncNotifyEvent = true
    //
    m_epoll_async_notify_event= false;
    CEpoll::Routine(epoll_wait_millisecs);
    CheckTimeout();
}

//
// receive data
//
bool CLongConn::ReceiveData(LongConnSession* long_conn_session,
                            bool* end_connection) {
    *end_connection = false;

    //
    // Prepare temp node to receive data, check temp receive node
    //
    if (!long_conn_session->temp_recv_node) {
        VLOG(3) << "pSession->pTempRecvNode = 0x0, ReceiveData, "
                  "new temp receive node to receive data...";

        long_conn_session->temp_recv_node = mempool_NEW(LongConnNode);
        if (!long_conn_session->temp_recv_node) {
            LOG(ERROR) << "CLongConn::OnEvent, "
                       "out of LongConnNode nodes, "
                       "no buffer to try receive data.";
            return false;
        }
        long_conn_session->temp_recv_node->ResetPtr();
    }

    // try receive data
    bool is_recv_ok = true;
    LongConnNode* recv_node = long_conn_session->temp_recv_node;
    while (1) {
        // 先尝试接收sizeof(BasProtocolHead) bytes, 想要接收到的包长是多大 ?
        int32_t try_receive_len = (recv_node->_transed_len <
                                   (int32_t)sizeof(BasProtocolHead)) ?
                                  ((int32_t)sizeof(BasProtocolHead) - recv_node->_transed_len) :
                                  (recv_node->_should_trans_len - recv_node->_transed_len);

        // 如果还在接收包头sizeof(BasProtocolHead) bytes内容,
        // 则就用tmp_protocol_head当接收buffer
        char* recv_buff =
            (recv_node->_transed_len < (int32_t)sizeof(BasProtocolHead)) ?
            reinterpret_cast<char*>(&recv_node->tmp_protocol_head) :
            reinterpret_cast<char*>(recv_node->data.buff);

        int32_t bytes =  recv(long_conn_session->sock,
                              recv_buff+recv_node->_transed_len,
                              try_receive_len,
                              0);
        if (bytes > 0) {
            VLOG(3) << "received data length:" << bytes << ", from[" <<
                      inet_ntoa(long_conn_session->peer_sock_name.sin_addr) << ":" <<
                      ntohs(long_conn_session->peer_sock_name.sin_port) << "], "
                      "session: " << reinterpret_cast<void*>(long_conn_session) <<
                      ", fd = " << long_conn_session->sock;

            // Receive part data ok
            recv_node->_transed_len += bytes;
            recv_node->data.valid_len = recv_node->_transed_len;

            // Get real package head, update whole package length
            if (recv_node->_should_trans_len == 0 &&
                    recv_node->_transed_len >= (int32_t)sizeof(BasProtocolHead)) {
                // Check package
                BasProtocolHead* netorder_head =
                    reinterpret_cast<BasProtocolHead*>(recv_buff);
/*
                if (!IsValidPack(netorder_head)) {
                    struct sockaddr_in from_addr = {0};
                    socklen_t addr_len = sizeof(from_addr);
                    getpeername(long_conn_session->sock,
                                (struct sockaddr*)&from_addr,
                                &addr_len);
                    LOG(ERROR) << "received invalid package, "
                               "maybe remote not use BASE_PROTOCOL protocol, "
                               "from:" << inet_ntoa(from_addr.sin_addr) << ":" <<
                               N2HS(from_addr.sin_port);
                    *end_connection = true;
                    is_recv_ok = false;
                    break;
                }
*/
                // get package length
                recv_node->_should_trans_len = ntohl(netorder_head->length);
                // check buffer
                if (!recv_node->data.CheckBuffer(recv_node->_should_trans_len)) {
                    LOG(ERROR) << "recv_node->data.CheckBuffer(" <<
                               recv_node->_should_trans_len << ") fail, "
                               "maybe out of memory.";
                    *end_connection = true;
                    is_recv_ok = false;
                    break;
                }

                memcpy(recv_node->data.buff,
                       &recv_node->tmp_protocol_head,
                       sizeof(BasProtocolHead));
            }

            // ? Receive package finished
            if (recv_node->_should_trans_len >0 &&
                    recv_node->_transed_len >= recv_node->_should_trans_len) {
                recv_node->data.valid_len = recv_node->_should_trans_len;
                is_recv_ok = OnReceivedPackage(long_conn_session);
                *end_connection = !is_recv_ok;
                break;
            }
        } else if (bytes == 0) {
            // Remote closed or error
            LOG(ERROR) << "maybe remote closed the tcp connection:"
                       "original fd = " << long_conn_session->sock << ", nbytes = 0";
            recv_node->data.valid_len = 0;
            is_recv_ok = false;
            *end_connection = true;
            break;
        } else { // <0
            int32_t err = GetLastSocketError();
            *end_connection = (err == EAGAIN || err == POSIX_EWOULDBLOCK) ? false:true;
            if (errno != EAGAIN) {
                VLOG(3) << "***received data length:" << bytes << ", errno:" <<
                          errno << ":" << strerror(errno);
            }

            is_recv_ok = false;
            break;
        }
    }

    return is_recv_ok;
}

bool CLongConn::OnReceivedPackage(LongConnSession* long_conn_session) {
    bool b = false;
    LongConnNode* recv_node = long_conn_session->temp_recv_node;
    uint32_t seq_num = 0;
    uint32_t package_type = 0;

    if (recv_node->data.valid_len < sizeof(BasProtocolHead)) {
        // Maybe downstream out of nodes
        memcpy(&seq_num,
               recv_node->data.buff+sizeof(uint32_t),
               sizeof(uint32_t));
        seq_num = ntohl(seq_num);
        LOG(ERROR) << "接收到的数据包长度为:" << recv_node->data.valid_len <<
                   ", 太短, 包头大小" << static_cast<uint32_t>(sizeof(BasProtocolHead));
    } else {
        bool get_pkg_info = GetSeqNumInPack(recv_node->data.buff,
                                            recv_node->data.valid_len,
                                            &seq_num);
        get_pkg_info = get_pkg_info && GetPackageTypeInPack(recv_node->data.buff,
                       recv_node->data.valid_len,
                       &package_type);

        if (!get_pkg_info) {
            // 获取序列号或者option错误, 丢弃获取到的数据包
            LOG(ERROR) << "get sequence number or package type fail."
                       "maybe incorrect package.";
            long_conn_session->temp_recv_node->ResetPtr();
            return false;
        }
    }

    VLOG(3) << "get seq number: " << seq_num
              << ", package type: " << package_type
              << " in received package.";

    // Get seq num. ok
    // if (long_conn_session->sock_dir == SOCK_DIR_ACCEPT_IN)
    bool is_req_pkg;
    is_req_pkg =  (package_type & BASEPROTOCOL_OPT_IS_REQ_PACKAGE) ==
                  BASEPROTOCOL_OPT_IS_REQ_PACKAGE;
    is_req_pkg = is_req_pkg ||
                 ((package_type & BASEPROTOCOL_OPT_IS_DEFAULT_PACKAGE) &&
                  (long_conn_session->sock_dir == SOCK_DIR_ACCEPT_IN));
    if (is_req_pkg) {
        //
        // 收到的是请求包
        //
        long_conn_session->temp_recv_node = NULL;
        OnNotifyReceivedData(long_conn_session, recv_node, true);
        b = true;
    } else {
        //
        // 收到的是应答包
        // check serial number
        //
        LongConnNode* long_conn_node = GetNodeInRecvQueue(long_conn_session,
                                       seq_num);
        if (long_conn_node) {
            // Swap node to temp receive data ptr
            memcpy(&long_conn_session->temp_recv_node->UserData,
                   &long_conn_node->UserData,
                   sizeof(long_conn_node->UserData));

            long_conn_session->temp_recv_node->send_start_time =
                long_conn_node->send_start_time;
            long_conn_session->temp_recv_node->send_finish_time =
                long_conn_node->send_finish_time;

            long_conn_session->temp_recv_node->app_serial_num =
                long_conn_node->app_serial_num;

            long_conn_session->temp_recv_node->need_receive_data =
                long_conn_node->need_receive_data;
            long_conn_session->temp_recv_node->try_recv_data_now =
                long_conn_node->try_recv_data_now;

            LongConnNode* p0 = long_conn_session->temp_recv_node;
            long_conn_session->temp_recv_node = long_conn_node;
            long_conn_node = p0;
            long_conn_session->temp_recv_node->ResetPtr();

            if (recv_node->data.valid_len < sizeof(BasProtocolHead)) {
                LOG(ERROR) << "收到的数据请求包不完整";
                OnNotifyError(long_conn_session, long_conn_node,
                              false,
                              TASK_ERR_DOWNSTREAM_OUT_OF_NODES);
            } else
                OnNotifyReceivedData(long_conn_session,
                                     long_conn_node,
                                     false);
            b = true;
        } else {
            // Check serial number fail, 丢弃
            LOG(ERROR) << "check serial number fail[received seq num:" << seq_num << "]."
                       "from server:" << inet_ntoa(long_conn_session->peer_sock_name.sin_addr) <<
                       ":" << ntohs(long_conn_session->peer_sock_name.sin_port);
            long_conn_session->temp_recv_node->ResetPtr();
            b = true;
        }
    }
    return b;
}

LongConnNode* CLongConn::GetNodeInRecvQueue(LongConnSession* pSession,
        uint32_t seq_num) {
    if (!pSession || !pSession->recv_queue.GetHeadNodePtr())
        return 0;

    // Find the node, match by serial number
    bool maybe_in_queue = false;
    uint32_t head_sn = pSession->recv_queue.GetHeadNodePtr()->app_serial_num;
    uint32_t tail_sn = pSession->recv_queue.GetTailNodePtr()->app_serial_num;
    if (head_sn <= tail_sn) {
        // 队列内的节点序列号是大小有序的
        if (seq_num >= head_sn && seq_num <= tail_sn)
            maybe_in_queue = true;
        else
            // 这种情况必然已经time out
            maybe_in_queue = false;
    } else {
        // 队列内的节点大小无序, 可能序列号已经开始重新计数(0 ~ MAX)
        maybe_in_queue = true;
    }

    //
    // Add by wookin, 2009-06-30
    // 不能确定先发送的数据包一定先发送完成，后面需要修改这里逻辑
    maybe_in_queue = true;
    //

    // Search the node
    LongConnNode* node = 0;
    if (maybe_in_queue) {
        LongConnNode* p = pSession->recv_queue.GetHeadNodePtr();
        while (p) {
            if (p->app_serial_num == seq_num) {
                node = p;
                break;
            }
            p = p->next;
        }
    } else {
        LOG(ERROR) << "in try receive queue: "
                   "head_sn = " << head_sn << ", tail_sn = " << tail_sn << ", seq_num = " <<
                   seq_num;
    }

    // Remove node from receive queue
    pSession->recv_queue.RemoveNode(node);

    return node;
}

//
// Send data on session
//
bool CLongConn::SendSessionData(LongConnSession* long_conn_session) {
    bool b = false;

    if (long_conn_session && long_conn_session->sock != INVALID_SOCKET)
        b = true;

    if (!long_conn_session->temp_send_node &&
            !long_conn_session->send_queue.GetHeadNodePtr()) {
        VLOG(3) << "long_conn_session->temp_send_node = 0x0, "
                  "peer:" <<
                  inet_ntoa(long_conn_session->peer_sock_name.sin_addr) << ":" <<
                  ntohs(long_conn_session->peer_sock_name.sin_port) << ", local:" <<
                  inet_ntoa(long_conn_session->local_sock_name.sin_addr) << ":" <<
                  ntohs(long_conn_session->local_sock_name.sin_port);
    }

    // Prepare temp receive node
    if (!long_conn_session->temp_send_node) {
        long_conn_session->send_queue.GetNodeOnHead(
            &long_conn_session->temp_send_node
        );
    }

    while (long_conn_session->temp_send_node) {
        //  VLOG(3) << "before send data, debug head info:";
        int32_t send_bytes = send(long_conn_session->sock,
                                  reinterpret_cast<char*>(long_conn_session->temp_send_node->data.buff) +
                                  long_conn_session->temp_send_node->_transed_len,
                                  long_conn_session->temp_send_node->_should_trans_len -
                                  long_conn_session->temp_send_node->_transed_len,
                                  0);

        // Show debug info
        if (send_bytes <= 0) {
            LOG(ERROR) << "send data, sent len:" << send_bytes << ", errno:" <<
                       errno << ":" << strerror(errno);
        } else {
            uint32_t seq = 0;
            GetSeqNumInPack(long_conn_session->temp_send_node->data.buff,
                            long_conn_session->temp_send_node->_should_trans_len,
                            &seq);

            VLOG(3) << "send data, sent len:" << send_bytes << ", seq = " << seq <<
                      " in package, send to:[" <<
                      inet_ntoa(long_conn_session->peer_sock_name.sin_addr) << ":" <<
                      ntohs(long_conn_session->peer_sock_name.sin_port) << "], session: " <<
                      reinterpret_cast<void*>(long_conn_session);
        }

        if (send_bytes < 0) {
            b = false;
            int32_t err = GetLastSocketError();
            b = (err == EAGAIN || err == POSIX_EWOULDBLOCK) ? true:false;

            break;
        } else {
            b = true;
            // ? Send finished
            long_conn_session->temp_send_node->_transed_len += send_bytes;
            if (long_conn_session->temp_send_node->_transed_len  >=
                    long_conn_session->temp_send_node->_should_trans_len) {
                // Finished send current data
                bool bContinueRecv = false;
                OnNotifyFinishedSendData(long_conn_session,
                                         long_conn_session->temp_send_node,
                                         &bContinueRecv);

                //
                // ? 需要接收服务器响应
                //
                if (bContinueRecv) {
                    long_conn_session->AppendRecvNode(
                        long_conn_session->temp_send_node
                    );
                }

                long_conn_session->temp_send_node = 0;

                // Move to next one
                long_conn_session->send_queue.GetNodeOnHead(
                    &long_conn_session->temp_send_node
                );
            }

            // ? Network busy
            if (send_bytes == 0) {
                b = true;
                break;
            }
        }
    }

    // ? Have next one
    if (!long_conn_session->temp_send_node &&
            !long_conn_session->send_queue.GetHeadNodePtr()) {
        //
        // VLOG(3) << "SendSessionData, send data finished, "
        //              "swap epoll to EPOLLIN only:pSession = " <<
        //              reinterpret_cast<void*>(pSession) << ", hSock = " << pSession->hSock;
        // Modify epoll set
        //
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
        ev.data.ptr = long_conn_session;
        CEpoll::ModifyFromEpoll(long_conn_session->sock, &ev);
    }

    return b;
}

CLongConnTasks::CLongConnTasks() {
    m_debug_routine_count = 0;
    m_callback = 0;
    m_probe_key1 = m_probe_key2 = m_probe_tkey1 = m_probe_tkey2 = 0;
    m_probe_obj = 0;

    lite_gettimeofday(&m_last_check_time, NULL);

    m_debug_send.SetString("***DEBUG sent***:");
    m_debug_recv.SetString("***DEBUG received***:");
    m_debug_err.SetString("***DEBUG error***:");
    m_debug_call_send_data.SetString("--call send data:");
}

CLongConnTasks::~CLongConnTasks() {
}

void CLongConnTasks::SetProbeKey(uint64_t key1,
                                 uint64_t key2,
                                 uint64_t tkey1,
                                 uint64_t tkey2) {
    m_probe_key1 = key1;
    m_probe_key2 = key2;
    m_probe_tkey1 = tkey1;
    m_probe_tkey2 = tkey2;
}

void CLongConnTasks::SetProbeObjPtr(CLongConnTasks_PaddingProtec* probe_obj) {
    m_probe_obj = probe_obj;
}

void CLongConnTasks::DealRoutineLongConn(uint32_t epoll_wait_millisecs) {
    CheckProbe();

    m_debug_routine_count++;
    if (m_debug_routine_count % 1000 == 0) {
        VLOG(3) << "RoutineLongConn(), this obj = " << reinterpret_cast<void*>(this);
    }

    //
    // START:处理异步提交数据
    // ? Have some send tasks in async submit queue
    //
    while (m_async_submit_send_queue.GetHeadNodePtr()) {
        LTasksGroup* group = 0;
        m_async_submit_send_queue.GetNodeOnHead(&group);
        RealSendData(group);
    }

    // ? Have some remove tasks in async submit queue
    while (m_async_submit_remove_long_conn.GetHeadNodePtr()) {
        LongConnHandleNode* long_conn_handle = 0;
        m_async_submit_remove_long_conn.GetNodeOnHead(&long_conn_handle);
        CLongConn::CloseLongConnSession(long_conn_handle->lc);
        m_long_conn_handle_nodes.PutNode(long_conn_handle);
    }

    //  ? Have some new long conn socket
    while (m_async_submit_new_fd.GetHeadNodePtr()) {
        LongConnNewSockNode* new_fd = 0;
        m_async_submit_new_fd.GetNodeOnHead(&new_fd);
        if (!CLongConn::InertNewAcceptedFD(new_fd->fd)) {
            LOG(ERROR) << "insert new long conn socket fail, "
                       "fd = " << new_fd->fd << ", close the socket now.";
            CloseSocket(new_fd->fd);
        }
        m_long_conn_new_fd_nodes.PutNode(new_fd);
    }
    // END:结束处理异步提交数据
    //

    // Routine
    CLongConn::Routine(epoll_wait_millisecs);

    CheckConditionTasksGroupStatus();

    CheckProbe();
}

void CLongConnTasks::RoutineLongConn(uint32_t epoll_wait_millisecs) {
    DealRoutineLongConn(epoll_wait_millisecs);
    if (CLongConn::IsEpollAsyncNotifyEvent())
        DealRoutineLongConn(1);
}

void CLongConnTasks::SetTasksGroupStrategyInfo(LTasksGroup* task_group, LongConnNode* node) {
    if (task_group->m_strategy_info.strategy_type == STRATEGY_N_TASKS_M_FINISH) {
        if (task_group->__net_finished_count ==
                task_group->m_strategy_info.data.need_net_finished_count) {
            task_group->m_strategy_info.data.last_finished_task_id
            = node->UserData.task_index;
        }
    }

    if (task_group->m_strategy_info.strategy_type == STRATEGY_SOME_TASK_NOT_MUST_FINISH) {
        if (task_group->m_tasks[node->UserData.task_index].is_need_finish) {
            task_group->m_strategy_info.data.need_finished_taskid_sum -=
                node->UserData.task_index + 1;
        }
    }
}

void CLongConnTasks::OnNotifyReceivedData(LongConnSession* session,
        LongConnNode* node,
        bool is_req) {
    lite_gettimeofday(&node->recv_finish_time, NULL);

    if (!is_req) {
        //  收到server回应包
        LTasksGroup* task_group =
            reinterpret_cast<LTasksGroup*>(node->UserData.group);

        if (node->UserData.task_index < task_group->m_valid_tasks &&
                task_group->m_tasks[node->UserData.task_index]._node == 0 &&
                task_group->m_tasks[node->UserData.task_index].long_conn_session.handle
                == session) {
            if (node->UserData.task_index == 0)
                m_debug_recv.Routine();

            task_group->m_tasks[node->UserData.task_index]._node = node;
            task_group->m_tasks[node->UserData.task_index]._is_receive_ok = 1;
            task_group->m_tasks[node->UserData.task_index]._received_data =
                                              (unsigned char*)(node->data.buff);

            task_group->m_tasks[node->UserData.task_index]._received_data_len =
                node->data.valid_len;

            VLOG(3) << "OnNotifyReceivedData, "
                      "received data length:" <<
                      task_group->m_tasks[node->UserData.task_index]._received_data_len <<
                      ", on task[" << node->UserData.task_index << "]";

            // 任务完成, 后续不需要网络服务
            ++task_group->__net_finished_count;
            ++task_group->__task_finished_count;

            SetTasksGroupStrategyInfo(task_group, node);

            CheckTasksFinished(task_group);
        } else {
            LOG(FATAL) << "OnNotifyReceivedData";

            if (node) {
                mempool_DELETE(node);
            }
        }
    } else {
        // 收到来自client的请求包
        if (m_callback) {
            LongConnHandle h = { session, session->long_conn_serial_num };

            // TODO(wookin): 不往上游发送忙碌信息
            if (1) {
                VLOG(3) << "call ->OnUserRequest()";
                bool bWillResponse = true;
                m_callback->OnUserRequest(h,
                                          (const unsigned char*)(node->data.buff),
                                          node->data.valid_len,
                                          bWillResponse);
                if (node) {
                    mempool_DELETE(node);
                }
            } else {
                // Not enougth free nodes
                SendLongErrorToUpstream(h, node);
            }
        } else {
            LOG(ERROR) << "m_callback = (null), "
                       "release receive data node";
            if (node) {
                mempool_DELETE(node);
            }
        }
    }
}

void CLongConnTasks::OnNotifyError(LongConnSession* session,
                                   LongConnNode* node,
                                   bool is_timeout,
                                   TASK_ERR err) {
    LOG(ERROR) << "***OnNotifyError, is_timeout = " << is_timeout;

    if (node->UserData.task_index == 0)
        m_debug_err.Routine();

    LTasksGroup* task_group =
        reinterpret_cast<LTasksGroup*>(node->UserData.group);

    if (node->UserData.task_index < task_group->m_valid_tasks &&
            task_group->m_tasks[node->UserData.task_index]._node == 0 &&
            task_group->m_tasks[node->UserData.task_index].long_conn_session.handle ==
            session) {
        task_group->m_tasks[node->UserData.task_index]._node = node;
        task_group->m_tasks[node->UserData.task_index]._timeout_event =
            is_timeout ? (unsigned char)1 : (unsigned char)0;

        task_group->m_tasks[node->UserData.task_index]._is_downstream_busy =
            (err == TASK_ERR_DOWNSTREAM_OUT_OF_NODES) ? 1:0;

        if (!task_group->m_tasks[node->UserData.task_index]._timeout_event) {
            // Maybe net error
            // pTasksgrp->m_Tasks[pNode->UserData.uTaskIndex]._pReceivedData =
            //                                             pNode->data.szBuff;
            // pTasksgrp->m_Tasks[pNode->UserData.uTaskIndex]._uReceivedDataLen=
            //                                            pNode->data.uValidLen;
        }

        ++task_group->__net_finished_count;

        SetTasksGroupStrategyInfo(task_group, node);

        CheckTasksFinished(task_group);
    } else {
        LOG(ERROR) << "OnNotifyError";
        if (node) {
            mempool_DELETE(node);
        }
    }
}

void CLongConnTasks::OnNotifyFinishedSendData(LongConnSession* session,
        LongConnNode* node,
        bool* need_continue_recv) {
    lite_gettimeofday(&node->send_finish_time, NULL);

    // 发送给下游服务器的请求包
    LTasksGroup* task_group =
        reinterpret_cast<LTasksGroup*>(node->UserData.group);

    if (node->UserData.task_index < task_group->m_valid_tasks &&
            task_group->m_tasks[node->UserData.task_index]._node == 0 &&
            task_group->m_tasks[node->UserData.task_index].long_conn_session.handle ==
            session) {
        task_group->m_tasks[node->UserData.task_index]._is_send_ok = 1;
        if (node->need_receive_data) {
            // 发送完成, 继续准备接收数据
            *need_continue_recv = true;
        } else {
            // 发送完毕, 不需要接收数据
            // (不需要回复的任务只要发送成功就认为任务完成__uFinishedCount++)
            *need_continue_recv = false;
            task_group->m_tasks[node->UserData.task_index]._node = node;

            // 只发送的任务完成, 不再需要网络服务
            ++task_group->__task_finished_count;
            ++task_group->__net_finished_count;

            SetTasksGroupStrategyInfo(task_group, node);

            CheckTasksFinished(task_group);
        }
    } else {
        *need_continue_recv = false;
        if (node) {
            mempool_DELETE(node);
        }
        LOG(ERROR) << "OnNotifyFinishedSendData";
    }
}

void CLongConnTasks::CheckConditionTasksGroupStatus() {
    struct timeval cur_time;
    lite_gettimeofday(&cur_time, NULL);

    if (!IsReachedCheckTimePoint(&cur_time, &m_last_check_time)) {
        return;
    }

    LTasksGroup* cur_tasks_group = m_check_need_callback_queue.GetHeadNodePtr();

    while (cur_tasks_group) {
        CHECK_EQ(STRATEGY_N_TASKS_M_FINISH, cur_tasks_group->m_strategy_info.strategy_type);
        const int32_t last_fin_task_id =
            cur_tasks_group->m_strategy_info.data.last_finished_task_id;
        LongConnNode* last_fin_node = reinterpret_cast<LongConnNode*>(
                                          cur_tasks_group->m_tasks[last_fin_task_id]._node);
        CHECK(last_fin_node);

        uint64_t tc_ms =
            CalcTimeCost_MS(last_fin_node->send_finish_time, last_fin_node->recv_finish_time);
        struct timeval trigger_time = last_fin_node->recv_finish_time;
        SetAbsTimeout(trigger_time,
                      static_cast<uint64_t>(tc_ms*(m_callback_config.ratio_by_last + 1)));

        if (tc_ms > m_callback_config.time_cost_limit &&
                IsReachedCheckTimePoint(&cur_time, &trigger_time)) {
            cur_tasks_group->__group_callback_status = STATUS_PARTIAL_FINISH;
            DLOG(INFO) << "tasksgroup:" << reinterpret_cast<void*>(cur_tasks_group)
                       << ",callback stra type:" << STRATEGY_N_TASKS_M_FINISH
                       << ",time cost:" << tc_ms
                       << ",last_fin_task_id:" << last_fin_task_id
                       << ",callback status:" << STATUS_PARTIAL_FINISH
                       << ",net finished count:"
                       << cur_tasks_group->__net_finished_count
                       << ", valid tasks num:"
                       << cur_tasks_group->m_valid_tasks;
            m_callback->OnTasksFinishedCallBack(cur_tasks_group);
            LTasksGroup* temp_ptr = cur_tasks_group->next;
            m_check_need_callback_queue.RemoveNode(cur_tasks_group);
            cur_tasks_group = temp_ptr;
            continue;
        }
        // next group
        cur_tasks_group = cur_tasks_group->next;
    }

    lite_gettimeofday(&m_last_check_time, NULL);
    SetAbsTimeout(m_last_check_time, 10);
}

void CLongConnTasks::CheckTasksFinished(LTasksGroup* task_group) {
    bool  task_finished = false;
    StrategyInfo& call_back_stra = task_group->m_strategy_info;

    if (STRATEGY_ALL_TASKS_MUST_FINISH == call_back_stra.strategy_type) {
        if (task_group->__net_finished_count >= task_group->m_valid_tasks) {
            // VLOG(3) << "CheckTasksFinished, "
            //              "__uFinishedCount = " <<
            //              task_group->__uFinishedCount << ", __uValidConnections = " <<
            //              task_group->__uValidConnections;
            task_finished = true;
        }
    } else if (STRATEGY_N_TASKS_M_FINISH == call_back_stra.strategy_type) {
        // N个中的M个已经返回，移入check队列定时检测
        if (task_group->__net_finished_count == call_back_stra.data.need_net_finished_count) {
            m_check_need_callback_queue.AppendNodeAtTail(task_group);
        }
        // N个中的剩下N-M个开始返回了
        else if (task_group->__net_finished_count > call_back_stra.data.need_net_finished_count) {
            if (task_group->__net_finished_count >= task_group->m_valid_tasks) {
                task_finished = true;

                // 还没有回调过，此时需要从检测队列中移除
                if (task_group->__group_callback_status == STATUS_ALL_FINISH) {
                    m_check_need_callback_queue.RemoveNode(task_group);
                }
                // 之前已经达到回调条件回调过一次
                if (task_group->__group_callback_status == STATUS_PARTIAL_FINISH) {
                    task_group->__group_callback_status = STATUS_REMAIN_FINISH;
                }
            }
        }
    } else if (STRATEGY_SOME_TASK_NOT_MUST_FINISH == call_back_stra.strategy_type) {
        // 必须完成的子任务都完成了，可以回调了
        if (0 == call_back_stra.data.need_finished_taskid_sum) {
            task_finished = true;
            if (task_group->__net_finished_count < task_group->m_valid_tasks) {
                task_group->__group_callback_status = STATUS_PARTIAL_FINISH;
            }
            call_back_stra.data.need_finished_taskid_sum = -1;
        }
        // 不是必须要完成的子任务也回来了，此前已经回调了，这种情况也不需要做二次回调
        else if (0 > call_back_stra.data.need_finished_taskid_sum) {
            if (task_group->__net_finished_count >= task_group->m_valid_tasks) {
                task_finished = true;
                task_group->__group_callback_status = STATUS_REMAIN_FINISH;
            }
        }
        // 必须完成的子任务也没有完成
    }

    if (task_finished) {
        OnTaskFinished(task_group);
    }
}

//
// 1：发送任务完成
// 2：发送请求并获得响应结果完成
//
void CLongConnTasks::OnTaskFinished(LTasksGroup* task_group) {
    const ENUM_TASKSGROUP_CALLBACK_STRATEGY stra_type =
        task_group->m_strategy_info.strategy_type;

    const ENUM_TASKSGROUP_CALLBACK_STATUS status =
        static_cast<ENUM_TASKSGROUP_CALLBACK_STATUS>(
            task_group->__group_callback_status);

    bool need_call_back = true;

    need_call_back = need_call_back && task_group->__need_callback_count > 0;
    need_call_back = need_call_back &&
                     !(stra_type == STRATEGY_SOME_TASK_NOT_MUST_FINISH &&
                       status == STATUS_REMAIN_FINISH);

    if (need_call_back) // todo(lonely): 某些应用希望发送不需要响应的数据包
        //               之后也能回调到应用层确认发送成功了
    {
        DLOG(INFO) << "tasksgroup:" << reinterpret_cast<void*>(task_group)
                   << ",callback stra type:" << stra_type
                   << ",callback status:" << status
                   << ",net finished count:"
                   << task_group->__net_finished_count
                   << ", valid tasks num:"
                   << task_group->m_valid_tasks;
        lite_gettimeofday(&task_group->group_callback_time, NULL);
        m_callback->OnTasksFinishedCallBack(task_group);
    }

    // Release resource
    if (STATUS_ALL_FINISH == status || STATUS_REMAIN_FINISH == status) {
        FreeTasksNode(&task_group);
    }
}

bool CLongConnTasks::InitLongConn(ITasksGroupCallBack* callback,
                                  uint32_t max_sessions,
                                  const char* listen_host, uint16_t port,
                                  float timeout,
                                  bool* break_epoll_wait,
                                  CallBackStrategyConfig* callback_config) {
    // 应该不需要CHECK了？NULL也是允许的吧？
    // CHECK(callback);
    m_callback = callback;

    if (callback_config)
        memcpy(&m_callback_config, callback_config, sizeof(CallBackStrategyConfig));

    m_pack_obj.Init();

    m_long_conn_handle_nodes.InitNodes(max_sessions);

    return CLongConn::Init(max_sessions, listen_host, port, timeout, callback,
                           break_epoll_wait);
}

bool CLongConnTasks::InitLongConn(ITasksGroupCallBack* callback,
                                  uint32_t max_sessions,
                                  SOCKET listen_sock, float timeout,
                                  bool* break_epoll_wait,
                                  CallBackStrategyConfig* callback_config) {
    // CHECK(callback);
    m_callback = callback;

    if (callback_config)
        memcpy(&m_callback_config, callback_config, sizeof(CallBackStrategyConfig));

    m_pack_obj.Init();

    m_long_conn_handle_nodes.InitNodes(max_sessions);

    return CLongConn::Init(max_sessions, listen_sock,
                           timeout, callback, break_epoll_wait);
}

void CLongConnTasks::UninitLongConn() {
    // Release all temp send data in submit queue
    while (m_async_submit_send_queue.GetHeadNodePtr()) {
        LTasksGroup* ptrGroup = 0;
        m_async_submit_send_queue.GetNodeOnHead(&ptrGroup);
        FreeTasksNode(&ptrGroup);
    }

    CLongConn::Uninit();
    m_pack_obj.Uninit();
    m_callback = 0;
}

LongConnHandle CLongConnTasks::CreateLongConnSession(const char* to_host,
        uint16_t port) {
    __GetLongConnCreateCount()++;
    VLOG(3) << "begin CreateLongConnSession...";
    VLOG(3) << "try create Long conn session, "
              "connect to:" << to_host << ":" << port << ", count:" <<
              __GetLongConnCreateCount();

    LongConnHandle session_handle;
    CheckProbe();
    session_handle = CLongConn::CreateLongConnSession(to_host, port);
    CheckProbe();
    return session_handle;
}

void CLongConnTasks::RemoveLongConnSession(LongConnHandle long_conn_session) {
    VLOG(3) << "begin RemoveLongConnSession session: "
            << long_conn_session.handle
            << ", serial num: "
            << long_conn_session.serial_num;

    // 可能被多个线程调用，从此只支持异步关闭, KAO

    // New LongConnHandle node
    LongConnHandleNode* node = m_long_conn_handle_nodes.GetNode();
    if (!node) {
        LOG(WARNING) << "new LongConnHandle node fail, "
                     "out of nodes.remove long connection session fail.";
        return;
    }

    m_submit_data_mutex.Lock();

    // ///////////////////////
    // Save data
    //
    node->lc = long_conn_session;
    // Submit data to async remove queue
    bool b = m_async_submit_remove_long_conn.AppendNodeAtTail(node);
    if (b) {
        // 尝试提前结束epoll_wait
        // CEpoll::ForceBreakCurEpollWait();
    } else {
        LOG(ERROR) << "submit remove long connection session data "
                   "to async queue fail.";
        m_long_conn_handle_nodes.PutNode(node);
    }
    // ///////////////////////

    m_submit_data_mutex.UnLock();
}

bool CLongConnTasks::InsertNewFD(SOCKET new_sock) {
    // 可能被多个线程调用，从此只支持异步提交, KAO
    if (new_sock == INVALID_SOCKET)
        return false;

    // new FD node
    LongConnNewSockNode* node = m_long_conn_new_fd_nodes.GetNode();
    if (!node) {
        VLOG(3) << "new LongConnNewSockNode fail, out of nodes."
                  "insert new long connection session fail.";
        return false;
    }

    m_submit_data_mutex.Lock();

    // ///////////////////////
    // save data
    node->fd = new_sock;
    // Submit data to async remove queue
    bool b = m_async_submit_new_fd.AppendNodeAtTail(node);
    if (b) {
        // 尝试提前结束epoll_wait
        // CEpoll::ForceBreakCurEpollWait();
    } else {
        LOG(ERROR) << "submit new long connection session data to "
                   "async queue fail.";
        m_long_conn_new_fd_nodes.PutNode(node);
    }
    // ///////////////////////

    m_submit_data_mutex.UnLock();
    return b;
}

void CLongConnTasks::Release() {
    // mean delete this
    delete m_probe_obj;
    m_probe_obj = NULL;
}

bool CLongConnTasks::GetPeerName(LongConnHandle long_conn_session,
                                 uint32_t* host,
                                 uint16_t* port) {
    CheckProbe();
    bool b = CLongConn::GetPeerNameOfLongConn(long_conn_session, host, port);
    CheckProbe();
    return b;
}

SOCKET CLongConnTasks::GetSockHandle(LongConnHandle long_conn_handle) {
    return CLongConn::GetSockHandleOfLongConn(long_conn_handle);
}

void CLongConnTasks::ForceBreakCurrWait() {
    CEpoll::ForceBreakCurEpollWait();
}

void CLongConnTasks::GetWritePeerOfNotify(SOCKET** sock) {
    CEpoll::GetNotifyWriteSockOfSocketPair(sock);
}

void CLongConnTasks::CleanUpNotifyMessages() {
    CEpoll::EmptyNotifyMessage();
}

//
// 提交发送数据到异步发送队列
// 错开应用层使用其它多个线程直接提交数据的情况
//
bool CLongConnTasks::SendData(LTasksGroup* task_group) {
    CheckProbe();

    CHECK(task_group->m_valid_tasks <= _MAX_LONGCONN_TASKS)
        << "user submit tasks group valid tasks num abnormal: "
        << task_group->m_valid_tasks;

    if (!task_group || task_group->m_valid_tasks <= 0) {
        LOG(WARNING) << "SendData, fail, invalid task_group = 0x0 "
                     "or m_valid_tasks = 0";
        return false;
    }

    // New group node
    LTasksGroup* group = 0;
    group = mempool_NEW(LTasksGroup);
    if (!group) {
        LOG(WARNING) << "new pGroup fail, out of LTasksGroup nodes.";
        return false;
    }

    m_submit_data_mutex.Lock();

    // ///////////////////////
    // Save data
    task_group->node_state = group->node_state;
    memcpy(group, task_group, sizeof(LTasksGroup));
    group->__internal_SetGroupStatus();

    uint32_t u = 0;
    // step 1:准备内部节点, memcpy data
    VLOG(3) << "";
    VLOG(3) << "send data, m_uValidTasks = " << group->m_valid_tasks;
    for (u = 0; u < group->m_valid_tasks; u++) {
        group->m_tasks[u].ResetState();

        // Try get temp send node
        LongConnSession* session_node =
            reinterpret_cast<LongConnSession*>(
                group->m_tasks[u].long_conn_session.handle
            );

        if (!IsValidNodePtr(session_node)) {
            LOG(ERROR) << "send data, "
                       "invalid session handle:" << reinterpret_cast<void*>(session_node) <<
                       ", on task[" << u << "]";
            break;
        }

        LongConnNode* node = NULL;
        // 如果是回应上游用户的回应包, 则优先从预留队列中获取
        // if (pSessionNode->SockDir == SOCK_DIR_ACCEPT_IN)
        //    m_oReservedNodesUserReq.GetNodeOnHead(&ptrNode);
        // if (!ptrNode)
        // {
        node = mempool_NEW(LongConnNode);
        // }
        if (!node) {
            LOG(WARNING) << "LongConnTasks SendData fail, "
                         "try get node fail.";
            break;
        }

        // get node
        node->ResetPtr();
        // copy data
        if (group->m_tasks[u].is_pure_data) {
            // *** 替app打包
            uint32_t new_max_dat_len = sizeof(BasProtocolHead) +
                                       7 +
                                       group->m_tasks[u].valid_data_len;
            if (node->data.CheckBuffer(new_max_dat_len)) {
                m_pack_obj.ResetContent();
                m_pack_obj.SetSeq(group->m_tasks[u].seq_num);
                unsigned char* head;
                uint32_t head_len = 0;
                m_pack_obj.GetPackedHeadOfBlock(group->m_tasks[u].service_type,
                                                group->m_tasks[u].pri_key,
                                                group->m_tasks[u].valid_data_len,
                                                &head,
                                                &head_len);
                if (head_len == (sizeof(BasProtocolHead)+7) && head) {
                    // copy head
                    memcpy(node->data.buff, head, sizeof(BasProtocolHead) + 7);
                    // copy body
                    memcpy(node->data.buff+(sizeof(BasProtocolHead) + 7),
                           group->m_tasks[u].data,
                           group->m_tasks[u].valid_data_len);
                    // update valid data length
                    node->data.valid_len = sizeof(BasProtocolHead) +
                                           7 +
                                           group->m_tasks[u].valid_data_len;
                } else {
                    VLOG(3) << "RealSendData fail, head_len = " << head_len << " != "
                              "(sizeof(BasProtocolHead)+7) = "
                              << (uint32_t)(sizeof(BasProtocolHead) + 7) << ", "
                              "ptrHead = " << reinterpret_cast<void*>(head);
                    break;
                }
            } else { // maybe alloc memory fail
                VLOG(3) << "RealSendData fail, "
                          "check buffer fail(new data len:" << new_max_dat_len << ")";
                break;
            }
            // ***
        } else {
            // 直接copy data
            // real save data
            node->data.SetData(group->m_tasks[u].data,
                               group->m_tasks[u].valid_data_len);
        }

        // verify package length
        if (node->data.valid_len < sizeof(BasProtocolHead)) {
            group->m_tasks[u]._is_verify_pack_fail = 1;
            LOG(ERROR) << "invalid package length.len = " << node->data.valid_len;
            break;
        }

        uint32_t in_pack_len = 0;
        memcpy(&in_pack_len, node->data.buff, sizeof(uint32_t));
        in_pack_len = ntohl(in_pack_len);

        // 取消大小限制 || uInPackLen>_MAX_BUFF_V )
        if (in_pack_len != node->data.valid_len) {
            group->m_tasks[u]._is_verify_pack_fail = 1;
            LOG(ERROR) << "invalid length or too Long "
                       "in package:" << in_pack_len << ", data len:" << node->data.valid_len;
            break;
        }

        node->UserData.group = group;
        node->UserData.task_index = u;
        node->SetNeedRecvData(group->m_tasks[u].need_recv_response);
        if (group->m_tasks[u].need_recv_response)
            ++group->__need_callback_count;
        group->m_tasks[u]._node   = node;
    }

    bool prepare_node_ok =
        (group->m_valid_tasks>0 && u == group->m_valid_tasks) ?
        true :
        false;
    bool b = false;
    if (prepare_node_ok) {
        // submit data to async send queue
        b = m_async_submit_send_queue.AppendNodeAtTail(group);
        if (b)
        {
           // 尝试提前结束epoll_wait
           CEpoll::ForceBreakCurEpollWait();
        }
        else
        {
            LOG(ERROR) << "submit send data to async queue fail.";
        }
    }

    if (!b)
        FreeTasksNode(&group);
    // ///////////////////////

    m_submit_data_mutex.UnLock();
    CheckProbe();
    return b;
}

void CLongConnTasks::RealSendData(LTasksGroup* task_group) {
    if (!task_group || task_group->m_valid_tasks <= 0) {
        LOG(ERROR) << "RealSendData fail, "
                   "invalid task_group = 0x0 or m_uValidTasks = 0";
        FreeTasksNode(&task_group);
        return;
    }

    uint32_t u = 0;
    uint32_t valid_tasks = task_group->m_valid_tasks;
    VLOG(3) << "***RealSendData, swap node from async to "
              "send queue, task_group->m_valid_tasks = " <<
              task_group->m_valid_tasks;

    for (u = 0; u < valid_tasks; u++) {
        if (u == 0)
            m_debug_call_send_data.Routine();

        LongConnSession* session =
            reinterpret_cast<LongConnSession*>(
                task_group->m_tasks[u].long_conn_session.handle
            );

        LongConnNode* node =
            reinterpret_cast<LongConnNode*>(task_group->m_tasks[u]._node);
        task_group->m_tasks[u]._node = 0;

        // VLOG(3) << "call SendNodeData, "
        //              "ptrTasksgrp->m_Tasks[" << u << "]."
        //              "hSession =  " << reinterpret_cast<void*>(ptrSession) << ", hSock = " <<
        //              ptrSession->hSock << ", pNode = " <<
        //              reinterpret_cast<void*>(ptrTasksgrp->m_Tasks[u]._pNode);
        //
        // VLOG(3) << "before send node data,"
        //              "userdata1:" << ptrTasksgrp->m_UserData1 <<
        //              ",userdata2:" << ptrTasksgrp->m_UserData2 <<
        //              ",userdata3:" << ptrTasksgrp->m_UserData3 <<
        //              ",userdata4:" << ptrTasksgrp->m_UserData4;

        // 如果成功则插入到LongConnSession的收发队列中
        // 如果失败则需要上面层处理
        if (CLongConn::SendNodeData(task_group->m_tasks[u].long_conn_session,
                                    node) == LERR_OK) {
            task_group->__valid_connections++;
        } else {
            if (session->sock_dir == SOCK_DIR_ACCEPT_IN &&
                    session->sock == INVALID_SOCKET)
                task_group->m_tasks[u]._is_sock_already_closed = 1;

            LOG(ERROR) << "RealSendData, SendNodeData fail."
                       "Task[" << u << "]/max tasks:" << task_group->m_valid_tasks << ", "
                       "SockDir = " << session->sock_dir << ", socket fd:" << session->sock;
            // VLOG(3) << "userdata1:" << ptrTasksgrp->m_UserData1 <<
            //              ", userdata2:" << ptrTasksgrp->m_UserData2 <<
            //              ", userdata3:" << ptrTasksgrp->m_UserData3 <<
            //              ", userdata4:" << ptrTasksgrp->m_UserData4);

            // 处理失败
            OnNotifyError(session, node, false);
        }
    }
}

void CLongConnTasks::FreeTasksNode(LTasksGroup** node) {
    LTasksGroup* ptr = *node;
    for (uint32_t u = 0; u < ptr->m_valid_tasks; u++) {
        LongConnNode* long_conn_node =
            reinterpret_cast<LongConnNode*>(ptr->m_tasks[u]._node);

        if (long_conn_node) {
            mempool_DELETE(long_conn_node);
        }
    }
    mempool_DELETE(ptr);
    *node = 0;
}

//
// Send out of nodes message to upstream
//
void CLongConnTasks::SendLongErrorToUpstream(LongConnHandle long_conn_handle,
        LongConnNode* node) {
    uint32_t seq = 0;
    GetSeqNumInPack(node->data.buff, node->data.valid_len, &seq);
    node->ResetPtr();

    // make response body
    // +--------------+
    // |....length....|
    // |...sequence...|
    // +--------------+
    uint32_t len = 8;
    len = htonl(len);
    seq = htonl(seq);
    memcpy(node->data.buff, &len, sizeof(uint32_t));
    memcpy(node->data.buff+sizeof(uint32_t), &seq, sizeof(uint32_t));
    node->data.valid_len = 8;

    LTasksGroup* task_group = 0;
    task_group = mempool_NEW(LTasksGroup);

    task_group->__internal_SetGroupStatus();
    memset(&task_group->m_tasks[0], 0, sizeof(LTask));
    task_group->m_valid_tasks = 1;
    task_group->m_tasks[0].long_conn_session = long_conn_handle;

    node->UserData.group = task_group;
    node->UserData.task_index = 0;

    // 设置是否需要接收服务器回应数据
    task_group->m_tasks[0].need_recv_response = 0;
    task_group->m_tasks[0]._node = node;

    // 设置超时
    task_group->SetTasksGroupTimeoutVal(kDefaultTasksGroupTimeoutVal);
    RealSendData(task_group);
}

void CLongConnTasks::CheckProbe() {
#ifdef _DEBUG
    if (m_probe_key1 != m_probe_key2) {
        LOG(ERROR) << "memory probe error, "
                   "maybe memory ovrflow on head, "
                   "m_hKey1 = " << m_probe_key1 << ", m_hKey2 = " << m_probe_key2;
    }

    if (m_probe_tkey1 != m_probe_tkey2) {
        LOG(ERROR) << "memory probe error, "
                   "maybe memory ovrflow at tail, "
                   "m_tKey1 = " << m_probe_tkey1 << ", m_tKey2 = " << m_probe_tkey2;
    }

    if (m_probe_key2 != m_probe_tkey1) {
        LOG(ERROR) << "big error, "
                   "memory overflow to real Long conn. "
                   "object.m_hKey2 = " << m_probe_key2 << ", m_tKey1 = " << m_probe_tkey1;
    }
#endif // _DEBUG
}

ILongConn* CreateLongConnObj(void) {
    CXThreadAutoLock autolock(&__GetLongConnCreateMutex());
    __GetLongConnObjCount()++;
    VLOG(3) << "create Long conn. obj count:" << __GetLongConnObjCount();

    // CLongConnTasks* ptrObj = new CLongConnTasks;
    // ILongConn* ptrLongConn = ptrObj;
    // return ptrLongConn;

    // 使用保护探针方式启动
    CLongConnTasks_PaddingProtec* probe_obj = new CLongConnTasks_PaddingProtec;
    probe_obj->m_long_conn_task.SetProbeKey(
        probe_obj->m_key1,
        probe_obj->m_key2,
        probe_obj->m_tkey1,
        probe_obj->m_tkey2);
    probe_obj->m_long_conn_task.SetProbeObjPtr(probe_obj);
    ILongConn* long_conn_obj = &(probe_obj->m_long_conn_task);
    return long_conn_obj;
}

_END_XFS_BASE_NAMESPACE_
