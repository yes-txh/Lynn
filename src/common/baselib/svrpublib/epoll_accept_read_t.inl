// epoll_accept_read_t.inl

template<typename T_user_data>
CEpollAcceptRead_T<T_user_data>::CEpollAcceptRead_T(uint32_t max_fds) {
    m_max_fds = max_fds;
    m_max_recv_request_timeout = 0;

    m_listen_sock = INVALID_SOCKET;
    m_epoll_handle = INVALID_HANDLE;
    m_listen_node = 0;
    m_min_binary_head = 0;
}

template<typename T_user_data>
CEpollAcceptRead_T<T_user_data>::~CEpollAcceptRead_T() {
}

template<typename T_user_data>
bool    CEpollAcceptRead_T<T_user_data>::InitListen(const char* host,
        uint16_t port,
        uint32_t max_recv_request_timeout,
        uint16_t min_binary_head,
        uint16_t max_listen_backlog) {
    bool b = false;
    if (host && port && m_listen_sock == INVALID_SOCKET) {
        //  Create listen socket handle
        m_min_binary_head = min_binary_head;
        m_max_recv_request_timeout = max_recv_request_timeout;
        m_listen_sock = NewSocket(true);
        if (m_listen_sock != INVALID_SOCKET) {
            VLOG(3) << "create listen socket handle ok "
                      "fd = " << m_listen_sock << ". host:" <<
                      host << ", port:" << port;
            
            XSetSocketBlockingMode(m_listen_sock, false);

            //  Bind host and start listen
            //  Create epoll handle
            //  Init timed nodes queue

            // ToDo(wookin): 这里是支持可以传入域名或者ip都可以,
            //               在R2统一所有glibc版本之前先不支持这个
            //               (静态编译警告)
            // char ip[32] = {0};
            // GetHostByName(host, ip, sizeof(ip));
            const char* ip = host;

            struct sockaddr_in addr;
            bzero(&addr, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr(ip);
            addr.sin_port = H2NS(port);
            m_epoll_handle = epoll_create(max_listen_backlog*2);

            XSetSocketReuseAddress(m_listen_sock, true);

#ifdef WIN32
            if (m_epoll_handle != INVALID_HANDLE &&
#else
            if ((uint32_t)m_epoll_handle != INVALID_HANDLE &&
#endif
                    bind(m_listen_sock,
                         (struct sockaddr*)&addr, sizeof(addr)) == 0 &&
                    listen(m_listen_sock, max_listen_backlog) == 0 &&
                    m_timed_nodes_mgr.Init(max_listen_backlog*2,
                                           MAX_EPOLL_READ_DATA_LEN+1,
                                           m_epoll_handle,
                                           m_max_recv_request_timeout,
                                           this)) {
                m_timed_nodes_mgr.GetListenNode(&m_listen_node, 0);
                m_listen_node->fd = m_listen_sock;
                m_listen_node->t0 = time(0);

                //  add to epoll read queue
                struct epoll_event ev;
                bzero(&ev, sizeof(ev));
                ev.events = EPOLLIN|EPOLLERR|EPOLLHUP;
                ev.data.ptr = m_listen_node;  // user data
                b = epoll_ctl(m_epoll_handle,
                              EPOLL_CTL_ADD,
                              m_listen_sock, &ev) == 0 ? true:false;
                if (!b) {
                    LOG(ERROR) << "CEpollAcceptRead::InitListen(), "
                               "epoll_ctl() fail.";
                }
            } else {
                LOG(ERROR) << "bind(fd = " << m_listen_sock << ") fail, "
                           "maybe port = " << port << " in use or host not exist.";
            }
        }

        if (!b) {
            LOG(ERROR) << "CEpollAcceptRead::InitListen Fail.";
            Uninit();
        }
    }
    return b;
}

template<typename T_user_data>
void    CEpollAcceptRead_T<T_user_data>::Uninit() {
    m_timed_nodes_mgr.Uninit();
    m_min_binary_head = 0;
    m_listen_node = 0;

    epoll_close(m_epoll_handle);
    m_epoll_handle = INVALID_HANDLE;
    CloseSocket(m_listen_sock);
    m_listen_sock = INVALID_SOCKET;
}

//  线程驱动入口
template<typename T_user_data>
void    CEpollAcceptRead_T<T_user_data>::AcceptReadRoutine() {
    if (m_epoll_handle == (EPOLLHANDLE)INVALID_HANDLE) {
        XSleep(1);
        return;
    }

    struct sockaddr_in from_addr;    
    //  epoll wait
    int32_t fds = epoll_wait(m_epoll_handle,
                             m_epoll_events,
                             MAX_TEMP_EPOLL_READ_EVENTS,
                             MAX_READ_EPOLLWAIT_TIMEOUT);

    for (int32_t i = 0; i < fds; i++) {
        if ((m_epoll_events[i].events & EPOLLIN) == EPOLLIN) {
            //  New tcp connection in
            if (m_epoll_events[i].data.ptr == m_listen_node) {
                //  Get new TCP connect request
                SOCKET new_sock = AcceptNewConnection(m_listen_sock, &from_addr);

                //  VLOG(3) << "accept new connection fd = " << new_sock <<
                //               " from:" <<
                //               inet_ntoa(*(struct in_addr*)&from_addr.sin_addr.s_addr) << ":" <<
                //               N2HS(from_addr.sin_port);
                CHECK(new_sock != INVALID_SOCKET);                
                
                XSetSocketBlockingMode(new_sock, false);
                SetRequestFD(new_sock);
            } else {
                // Data in
                TimedBuffNode<T_user_data>* recv_node =
                    (TimedBuffNode<T_user_data>*)m_epoll_events[i].data.ptr;

                if (recv_node) {
                    //  Try receive data
                    ENUM_RECV_STATE state = TryReceiveData(recv_node);
                    switch (state) {
                    case ERROR_RECV_STATE_ERROR:
                    case ERROR_RECV_STATE_CLIENT_CLOSED:
                        //  Client closed or socket error,
                        //  remove node and request
                        m_timed_nodes_mgr.IndicateCloseSocket(recv_node,
                                                              ERROR_RECV_STATE_CLIENT_CLOSED);
                        break;
                    case ERROR_RECV_STATE_END_FLAG: {
                        //  Received end flag of request,
                        //  remove node from queue
                        if (!OnReceivedRequest(recv_node->fd,
                                               recv_node->data,
                                               recv_node->current_transed_len,
                                               recv_node->t0) ) {
                            VLOG(3) << "refuse request, close socket:" << recv_node->fd << ". "
                                      "maybe downstream busy";
                            m_timed_nodes_mgr.IndicateCloseSocket(recv_node,
                                                                  ERROR_PROCESS_DOWNSTREAM_BUSY);
                        } else
                            m_timed_nodes_mgr.RemoveHandleFromEpollSet(recv_node);
                    }
                    break;
                    case ERROR_RECV_STATE_TIMEOUT:
                        break;
                    default:
                        break;
                    }
                }
            }
        } else {
            // error
            if (m_epoll_events[i].data.ptr == m_listen_node) {
                VLOG(3) << "*** listen handle error, "
                          "you need restart this service***";
                CloseSocket(m_listen_sock);
                m_listen_sock = INVALID_SOCKET;
            } else {
                TimedBuffNode<T_user_data>* node =
                    (TimedBuffNode<T_user_data>*)m_epoll_events[i].data.ptr;
                m_timed_nodes_mgr.IndicateCloseSocket(node,
                                                      ERROR_PROCESS_EPOLL_FD_ERROR);
            }
        }
    }

    // Check time out
    m_timed_nodes_mgr.CheckTimeOut();
}

//  设置需要读取数据的句柄
template<typename T_user_data>
bool    CEpollAcceptRead_T<T_user_data>::SetRequestFD(SOCKET fd) {
    bool b = false;
#ifdef WIN32
    if (m_epoll_handle != INVALID_HANDLE && m_timed_nodes_mgr.IsBufferReady())
#else
    if ((uint32_t)m_epoll_handle != INVALID_HANDLE &&
            m_timed_nodes_mgr.IsBufferReady())
#endif
    {
        //  Try get an empty read node, and add request fd to epoll read queue
        TimedBuffNode<T_user_data>*    read_node = 0;
        m_timed_nodes_mgr.GetNextEmptyNodePtr(&read_node, 0);
        read_node->fd = fd;
        read_node->should_max_valid_len = 0;
        read_node->current_transed_len = 0;

        //  Add to epoll read queue
        struct epoll_event ev;
        bzero(&ev, sizeof(ev));
        ev.events = EPOLLIN|EPOLLERR|EPOLLHUP;

        //
        //  User data
        //
        ev.data.ptr = reinterpret_cast<void*>(read_node);
        if (epoll_ctl(m_epoll_handle, EPOLL_CTL_ADD, read_node->fd, &ev) == 0) {
            m_timed_nodes_mgr.SetDataToNextNodeFinished(read_node);
            b = true;
        } else {
            //  Add to epoll queue fail, close the request socket
            VLOG(3) << "CEpollAcceptRead::SetRequestFD, epoll_ctl() fail:";
            CloseSocket(read_node->fd);
            read_node->fd = INVALID_SOCKET;
        }
    } else {
        CloseSocket(fd);
    }
    return b;
}

template<typename T_user_data>
ENUM_RECV_STATE CEpollAcceptRead_T<T_user_data>::TryReceiveData(
    TimedBuffNode<T_user_data>* node
)
{
    ENUM_RECV_STATE state = ERROR_RECV_STATE_ERROR;
    if (node)
	{
        int32_t received = RecvDat(node->fd,
                                reinterpret_cast<char*>(m_recv_buff),
                                MAX_EPOLL_READ_DATA_LEN,
                                0);
        if (received == 0) 
		{
            //  Client(remote) closed
            state = ERROR_RECV_STATE_CLIENT_CLOSED;
        }
		else if (received > 0)
		{
            //  Try receive data next time
            state = ERROR_RECV_STATE_TIMEOUT;
            for (int32_t i = 0; i < received; i++) 
			{
                node->data[node->current_transed_len] = m_recv_buff[i];
                node->current_transed_len++;
                if ((node->current_transed_len>m_min_binary_head &&
                        (m_recv_buff[i] == '\r' || m_recv_buff[i] == '\n'))
                        ||
                        node->current_transed_len == MAX_EPOLL_READ_DATA_LEN) 
				{
                    //  Received the end flag of request or
                    //  request string's length is too Long
                    node->data[node->current_transed_len] = 0;
                    state = ERROR_RECV_STATE_END_FLAG;

                    //  Break 'for' loop
                    break;
                }
            }
        } 
		else 
		{
            int32_t err = GetLastSocketError();
            //  Means,  no fatal error occurs, but just no data to read
            //  no data arrived,means time out
            if (EAGAIN == err || POSIX_EWOULDBLOCK == err){
                state = ERROR_RECV_STATE_TIMEOUT;
        	}
			else  //  Something wrong
           	    state = ERROR_RECV_STATE_ERROR;
    	}
	}
	return state;
}

template<typename T_user_data>
bool CEpollAcceptRead_T<T_user_data>::OnTimeoutCallback(
    TimedBuffNode<T_user_data>* timeout_node
) {
    //  Auto remove from epoll set and close socket handle
    return false;
}
