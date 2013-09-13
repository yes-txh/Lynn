// epoll_write_t.inl

// class CEpollWrite_T
template<uint32_t T_max_queue_len,
         uint32_t T_max_data_len,
         typename T_user_data>
CEpollWrite_T<T_max_queue_len, T_max_data_len, T_user_data>::CEpollWrite_T() {
}

template<uint32_t T_max_queue_len,
         uint32_t T_max_data_len,
         typename T_user_data>
CEpollWrite_T<T_max_queue_len, T_max_data_len, T_user_data>::~CEpollWrite_T() {
}

template<uint32_t T_max_queue_len,
         uint32_t T_max_data_len,
         typename T_user_data>
bool    CEpollWrite_T<T_max_queue_len,
   T_max_data_len,
   T_user_data>::Init(uint32_t max_fds,
uint32_t timeout) {
    bool b = false;
    //  Create epoll handle
    m_epoll_handle = epoll_create(T_max_queue_len);
#ifdef WIN32
    if (m_epoll_handle != INVALID_HANDLE)
#else
    if ((uint32_t)m_epoll_handle != INVALID_HANDLE)
#endif
    {
        b = m_nodes_list.Init(max_fds, T_max_data_len,
                              m_epoll_handle,
                              timeout,
                              0);
    }

    if (!b)
        Uninit();
    return b;
}

template<uint32_t T_max_queue_len,
uint32_t T_max_data_len,
typename T_user_data>
void CEpollWrite_T<T_max_queue_len,
     T_max_data_len,
T_user_data>::Uninit() {
    //  Close epoll handle
#ifdef WIN32
    if (m_epoll_handle != INVALID_HANDLE)
#else
    if ((uint32_t)m_epoll_handle != INVALID_HANDLE)
#endif
    {
        epoll_close(m_epoll_handle);
        m_epoll_handle = INVALID_HANDLE;
    }

    m_nodes_list.Uninit();
}

template<uint32_t T_max_queue_len,
uint32_t T_max_data_len,
typename T_user_data>
bool    CEpollWrite_T<T_max_queue_len,
   T_max_data_len,
   T_user_data>::SetOutputData(SOCKET fd,
                               time_t t0,
                               bool close_fd_when_finish,
                               const unsigned char* data,
uint32_t len) {
    bool b = false;
    TimedBuffNode<T_user_data>* node = 0;
    if (fd != INVALID_SOCKET &&
            data && len && len <= T_max_data_len &&
    m_nodes_list.GetNextEmptyNodePtr(&node, 0)) {
        //  Copy data
        node->fd = fd;
        memcpy(node->data, data, len);

        //  data中有效数据应该具有的长度
        //  Send data:一开始这个值是有效的
        //  Receive data:一开始这个值是无效的
        node->should_max_valid_len = len;

        node->current_transed_len = 0;
        node->t0 = t0;

        //  Try add to epoll write queue
        struct epoll_event ev;
        bzero(&ev, sizeof(ev));
        ev.events = EPOLLOUT | EPOLLIN | EPOLLERR | EPOLLHUP;
        ev.data.ptr = reinterpret_cast<void*>(node); // user data
        if (epoll_ctl(m_epoll_handle, EPOLL_CTL_ADD, node->fd, &ev) == 0) {
            if (!m_nodes_list.SetDataToNextNodeFinished(node)) {
                VLOG(3) << "*** SetOutputData ***, "
                "SetDataToNextNodeFinished(0x" << reinterpret_cast<void*>(node) <<
                ") fail.";
            } else
                b = true;
        }
    }
    return b;
}

template<uint32_t T_max_queue_len,
uint32_t T_max_data_len,
typename T_user_data>
void    CEpollWrite_T<T_max_queue_len,
   T_max_data_len,
T_user_data>::Routine() {
#ifdef WIN32
    if (m_epoll_handle == INVALID_HANDLE)
#else
    if ((uint32_t)m_epoll_handle == INVALID_HANDLE)
#endif
        return;

    //  epoll wait, try send data
    int32_t fds = epoll_wait(m_epoll_handle,
                             m_epoll_events,
                             MAX_TEMP_EPOLL_WRITE_EVENTS,
                             MAX_WRITE_EPOLLWAIT_TIMEOUT);
    for (int32_t i = 0; i < fds; i++) {
        TimedBuffNode<T_user_data>* send_node =
            reinterpret_cast< TimedBuffNode<T_user_data>* >
            (m_epoll_events[i].data.ptr);
        if ((m_epoll_events[i].events & EPOLLOUT) == EPOLLOUT) {
            //  Data out
            if (send_node) {
                //  Try receive data
                ENUM_SEND_STATE state = TrySendData(send_node);
                switch (state) {
                case ERROR_SEND_STATE_ERROR:
                    m_nodes_list.IndicateCloseSocket(send_node,
                                                     ERROR_SEND_STATE_ERROR);
                    break;
                case ERROR_SEND_STATE_FINISHED: {
                    //
                    //  上层未处理, 切换到EPOLLIN状态,
                    //  检测到对方断开就close(tcp connection)
                    //  Switch to EPOLLIN
                    //
                    struct epoll_event    ev;
                    memset(&ev, 0, sizeof(ev));
                    ev.events = EPOLLIN|EPOLLERR|EPOLLHUP;
                    ev.data.ptr = send_node;
                    if (epoll_ctl(m_epoll_handle,
                                  EPOLL_CTL_MOD,
                    send_node->fd, &ev) != 0) {
                        m_nodes_list.IndicateCloseSocket(send_node,
                                                         ERROR_SEND_STATE_FINISHED);
                    }
                }
                break;
                case ERROR_SEND_STATE_NETWORK_BUSY:
                case ERROR_SEND_STATE_EWOULDBLOCK:
                case ERROR_SEND_STATE_TRY_AGAIN:
                    break;
                default:
                    break;
                }
            }
        } else if ((m_epoll_events[i].events & EPOLLIN) == EPOLLIN) {
            //  Maybe next request or remote closed the connection
            m_nodes_list.IndicateCloseSocket(send_node, EPOLLIN);
        } else {
            //  Error
            m_nodes_list.IndicateCloseSocket(send_node, EPOLLERR);
        }
    }

    //  Check time out
    m_nodes_list.CheckTimeOut();
}

template<uint32_t T_max_queue_len,
uint32_t T_max_data_len,
typename T_user_data>
ENUM_SEND_STATE CEpollWrite_T<T_max_queue_len,
                T_max_data_len,
                T_user_data>::
TrySendData(TimedBuffNode<T_user_data>* node)
{
    ENUM_SEND_STATE state = ERROR_SEND_STATE_ERROR;
    int32_t remain = node->should_max_valid_len-node->current_transed_len;
    if (node && remain > 0){
        int32_t bytes = SendDat(node->fd,
                             reinterpret_cast<char*>(node->data) +
                             node->current_transed_len,
                             remain,
                             0);
        if (bytes == 0){
            state = ERROR_SEND_STATE_NETWORK_BUSY;
        }else if (bytes>0){
            node->current_transed_len += bytes;
            if (node->current_transed_len >= node->should_max_valid_len)
                state = ERROR_SEND_STATE_FINISHED;
            else
                state = ERROR_SEND_STATE_TRY_AGAIN;
        }else {
            int32_t err = GetLastSocketError();
            if (err == EAGAIN || err == POSIX_EWOULDBLOCK){
                state = ERROR_SEND_STATE_TRY_AGAIN;
        	}else
                state = ERROR_SEND_STATE_ERROR;
	   	}
    }
	return state;
}
