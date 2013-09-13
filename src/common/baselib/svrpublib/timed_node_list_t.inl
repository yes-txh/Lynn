// timed_node_list.inl: interface for the CTimedLoopList class.
// wookin@tencent.com
//
// ////////////////////////////////////////////////////////////////////

template<typename T_user_data>
CTimedLoopList_T<T_user_data>::CTimedLoopList_T() {
    m_nodes = 0;
    m_data_buff = 0;
    m_node_data_buff_len = 0;
    m_timeout_interval = 0;
    m_last_check_timeout = 0;

    m_node_head = 0;
    m_node_tail = 0;
    m_timeout_callback = 0;

    m_epoll_handle = INVALID_HANDLE;
    bzero(&m_listen_node, sizeof(m_listen_node));
}

template<typename T_user_data>
CTimedLoopList_T<T_user_data>::~CTimedLoopList_T() {
}

//
//  If hEpollHandle is valid, auto remove fd from epoll set when time out.
//
template<typename T_user_data>
bool    CTimedLoopList_T<T_user_data>::Init(uint16_t max_nodes,
        uint16_t node_buff_len,
        EPOLLHANDLE epoll_handle,
        uint16_t timeout_interval,
        ITimeoutCallback<T_user_data>* timeout_callback) {
    if (max_nodes <= 1)   //  must  >= 2
        return false;

    bool b = false;
    m_timeout_callback = timeout_callback;
    m_node_data_buff_len = MAX(node_buff_len, 1);
    uint16_t num_nodes = MAX(max_nodes, 1);
    m_nodes = new TimedBuffNode<T_user_data>[num_nodes];
    uint32_t total_data_len = num_nodes*m_node_data_buff_len;
    m_data_buff = new char[total_data_len];
    if (m_nodes && m_data_buff) {
        uint16_t us = 0;
        for (us = 0; us < num_nodes; us++) {
            //  Install data buffer
            m_nodes[us].data = m_data_buff + us*m_node_data_buff_len;
            if (!m_node_tail) {
                m_node_head = m_node_tail = &m_nodes[us];
            } else {
                m_node_tail->next = &m_nodes[us];
                m_nodes[us].pre = m_node_tail;
                m_node_tail = m_node_tail->next;
            }
        }

        //  Make loop linked list
        m_node_tail->next = m_node_head;
        m_node_head->pre = m_node_tail;
        m_node_tail = m_node_tail->next;

        m_timeout_interval = timeout_interval;
        m_last_check_timeout = time(0);
        m_epoll_handle = epoll_handle;
        b = true;
    }

    //   ? init fail
    if (!b)
        Uninit();
    return b;
}

template<typename T_user_data>
void    CTimedLoopList_T<T_user_data>::Uninit() {
    //  Close all socket handle
    VLOG(3) << "CTimedLoopList::Uninit, close all fd.";
    for (; m_node_tail;) {
        if (m_node_tail->fd != INVALID_SOCKET) {
            RemoveFDFromEpollSet(m_node_tail->fd);
            CloseSocket(m_node_tail->fd);
            m_node_tail->fd = INVALID_SOCKET;
        }
        if (m_node_tail == m_node_head)
            break;
        m_node_tail = m_node_tail->next;
    }

    if (m_nodes) {
        delete []m_nodes;
        m_nodes = 0;
    }
    if (m_data_buff) {
        delete []m_data_buff;
        m_data_buff = 0;
    }
    m_node_data_buff_len = 0;

    m_node_head = 0;
    m_node_tail = 0;
    m_epoll_handle = INVALID_HANDLE;
    m_timeout_interval = 0;
    m_last_check_timeout = 0;
}

template<typename T_user_data>
bool    CTimedLoopList_T<T_user_data>::IsBufferReady() {
    if (m_node_head->next == m_node_tail) {
        //  尝试移动m_ptrNodeTail, 当都不超时, 就会发生队列节点假耗尽现象
        while (m_node_tail->next != m_node_head &&
                m_node_tail->fd == INVALID_SOCKET)
            m_node_tail = m_node_tail->next;
    }
    return (m_node_head->next != m_node_tail) ? true:false;
}

template<typename T_user_data>
bool    CTimedLoopList_T<T_user_data>::GetNextEmptyNodePtr(
    TimedBuffNode<T_user_data>** node,
    uint16_t* max_buff_len) {
    bool b = false;
    if (m_node_head->next != m_node_tail) {
        if (max_buff_len)
            *max_buff_len = m_node_data_buff_len;
        if (node)
            *node = m_node_head->next;

        b = true;
    }
    return b;
}

template<typename T_user_data>
bool    CTimedLoopList_T<T_user_data>::SetDataToNextNodeFinished(
    TimedBuffNode<T_user_data>* node
) {
    bool b = false;
    if (node && m_node_head->next != m_node_tail &&
            node == m_node_head->next) {
        node->t0 = time(0);
        m_node_head = m_node_head->next;
        b = true;
    }
    return b;
}

template<typename T_user_data>
void    CTimedLoopList_T<T_user_data>::CheckTimeOut() {
    time_t now = time(0);
    if (now-m_last_check_timeout>m_timeout_interval ||
            m_node_tail->fd == INVALID_SOCKET) {
        //  Check time out
        m_last_check_timeout = now;
        for (;;) {
            if (m_node_tail->fd != INVALID_SOCKET) {
                if (now-m_node_tail->t0>m_timeout_interval) {
                    //  The node time out
                    bool processed = false;
                    if (m_timeout_callback)
                        processed =
                            m_timeout_callback->OnTimeoutCallback(m_node_tail);

                    if (!processed)
                        OnAutoNodeTimeOut(m_node_tail);
                } else
                    break;
            }

            if (m_node_tail == m_node_head)
                break;
            m_node_tail = m_node_tail->next;
        }
    }
}

template<typename T_user_data>
void    CTimedLoopList_T<T_user_data>::OnAutoNodeTimeOut(
    TimedBuffNode<T_user_data>* node
) {
    if (node && node->fd != INVALID_SOCKET) {
        RemoveFDFromEpollSet(node->fd);
        VLOG(3) << "CTimedLoopList_T::OnNodeTimeOut, "
                  "socket:" << node->fd << " time out.";
        CloseSocket(node->fd);
        node->fd = INVALID_SOCKET;
        node->should_max_valid_len = 0;
        node->current_transed_len = 0;
        node->t0 = 0;
    }
}

//
//  独立的不记time out的节点
//
template<typename T_user_data>
void    CTimedLoopList_T<T_user_data>::GetListenNode(
    TimedBuffNode<T_user_data>** node,
    uint16_t* max_buff_len) {
    if (node)
        *node = &m_listen_node;
    if (max_buff_len)
        *max_buff_len = m_node_data_buff_len;
}

//
//  Reason:EPOLLIN, EPOLLOUT, EPOLLERR
//
template<typename T_user_data>
void    CTimedLoopList_T<T_user_data>::IndicateCloseSocket(
    TimedBuffNode<T_user_data>* node,
    int32_t reason) {
    if (node) {
        if (node->fd != INVALID_SOCKET) {
            RemoveFDFromEpollSet((int32_t)node->fd);
            CloseSocket(node->fd);
            node->fd = INVALID_SOCKET;
            node->should_max_valid_len = 0;
            node->current_transed_len = 0;
            node->t0 = 0;

            if (!(node == m_node_head || node == m_node_tail)) {
                //  Remove node
                TimedBuffNode<T_user_data>* old_pre = node->pre;
                TimedBuffNode<T_user_data>* old_next = node->next;
                old_pre->next = old_next;
                old_next->pre = old_pre;

                //  Insert node
                old_pre = m_node_tail->pre;
                old_pre->next = node;
                node->pre = old_pre;
                node->next = m_node_tail;
                m_node_tail->pre = node;
            }
        } else {
            LOG(ERROR) << "IndicateCloseSocket ERROR***, "
                       "socket handle already closed. reason:" << reason;
        }
    }
}

template<typename T_user_data>
void    CTimedLoopList_T<T_user_data>::RemoveHandleFromEpollSet(
    TimedBuffNode<T_user_data>* node
) {
    if (node) {
        if (node->fd != INVALID_SOCKET) {
            RemoveFDFromEpollSet(node->fd);

            //  Don't close the socket handle
            node->fd = INVALID_SOCKET;
            node->should_max_valid_len = 0;
            node->current_transed_len = 0;
            node->t0 = 0;

            if (!(node == m_node_head || node == m_node_tail)) {
                //  Remove node
                TimedBuffNode<T_user_data>* old_pre = node->pre;
                TimedBuffNode<T_user_data>* old_next = node->next;
                old_pre->next = old_next;
                old_next->pre = old_pre;

                //  Insert node
                old_pre = m_node_tail->pre;
                old_pre->next = node;
                node->pre = old_pre;
                node->next = m_node_tail;
                m_node_tail->pre = node;
            }
        } else {
            LOG(ERROR) << "RemoveHandleFromEpollSet ERROR***, "
                       "socket handle already closed.";
        }
    }
}

template<typename T_user_data>
void    CTimedLoopList_T<T_user_data>::RemoveFDFromEpollSet(SOCKET fd) {
    //  Remove fd from epoll set
#ifdef WIN32
    if (m_epoll_handle != INVALID_HANDLE && m_epoll_handle != 0
            && fd != INVALID_SOCKET)
#else
    if (m_epoll_handle != (int32_t)INVALID_HANDLE && m_epoll_handle != 0
            && fd != INVALID_SOCKET)
#endif
    {
        struct epoll_event ev;
        bzero(&ev, sizeof(ev));
        ev.events = EPOLLIN|EPOLLOUT|EPOLLERR|EPOLLHUP;
        if (epoll_ctl(m_epoll_handle, EPOLL_CTL_DEL, fd, &ev) != 0) {
            LOG(ERROR) << "RemoveFDFromEpollSet:: remove fd from epoll fail. "
                       "fd = " << fd;
        }
    }
}

//
//  Class CNewTimedLoopList_T
//
template<typename T_user_data>
CNewTimedLoopList_T<T_user_data>::CNewTimedLoopList_T() {
    m_nodes = 0;
    m_timeout_interval = 0;
    m_last_check_timeout = 0;

    m_node_head = 0;
    m_node_tail = 0;
    m_timeout_callback = 0;
}

template<typename T_user_data>
CNewTimedLoopList_T<T_user_data>::~CNewTimedLoopList_T() {
}

//
//  If hEpollHandle is valid, auto remove fd from epoll set when time out.
//
template<typename T_user_data>
bool    CNewTimedLoopList_T<T_user_data>::Init(uint16_t max_nodes,
        uint16_t timeout,
        INewTimeoutCallback<T_user_data>* callback) {
    if (max_nodes <= 1)   //  must  >= 2
        return false;

    bool b = false;

    uint16_t num_nodes = ( uint16_t)(MAX(max_nodes, 3));

    m_timeout_callback = callback;
    m_nodes = new NewTimedNode<T_user_data>[num_nodes];
    if (m_nodes) {
        uint16_t us = 0;
        for (us = 0; us < num_nodes; us++) {
            if (!m_node_tail) {
                m_node_head = m_node_tail = &m_nodes[us];
            } else {
                m_node_tail->next = &m_nodes[us];
                m_nodes[us].pre = m_node_tail;
                m_node_tail = m_node_tail->next;
            }
        }

        //  make loop linked list
        m_node_tail->next = m_node_head;
        m_node_head->pre = m_node_tail;
        m_node_tail = m_node_tail->next;

        m_timeout_interval = timeout;
        m_last_check_timeout = time(0);
        b = true;
    }

    //  ?init fail
    if (!b)
        Uninit();
    return b;
}

template<typename T_user_data>
void    CNewTimedLoopList_T<T_user_data>::OnTimeOut(
    NewTimedNode<T_user_data>* node
) {
    VLOG(3) << "CNewTimedLoopList_T, OnTimeOut()";
    m_timeout_callback->OnTimeoutCallback(node->user_data);
    SetNodeInvalid(node);
}

template<typename T_user_data>
void    CNewTimedLoopList_T<T_user_data>::Uninit() {
    //  Close all socket handle
    VLOG(3) << "CTimedLoopList::Uninit, close all fd.";
    for (; m_node_tail;) {
        if (m_node_tail->valid) {
            m_timeout_callback->OnTimeoutCallback(m_node_tail->user_data);
            m_node_tail->valid = false;
        }
        if (m_node_tail == m_node_head)
            break;
        m_node_tail = m_node_tail->next;
    }

    if (m_nodes) {
        delete []m_nodes;
        m_nodes = 0;
    }

    m_node_head = 0;
    m_node_tail = 0;
    m_timeout_interval = 0;
    m_last_check_timeout = 0;
}

template<typename T_user_data>
bool    CNewTimedLoopList_T<T_user_data>::GetNextEmptyNodePtr(
    NewTimedNode<T_user_data>** node
) {
    bool b = false;
    if (node &&
            m_node_head->next != m_node_tail) {
        *node = m_node_head->next;
        b = true;
    }
    return b;
}

template<typename T_user_data>
bool    CNewTimedLoopList_T<T_user_data>::SetDataToNextNodeFinished(
    NewTimedNode<T_user_data>* node
) {
    bool b = false;
    if (node && m_node_head->next != m_node_tail &&
            node == m_node_head->next) {
        node->valid = true;
        node->t0 = time(0);
        m_node_head = m_node_head->next;
        b = true;
    }
    return b;
}

template<typename T_user_data>
void    CNewTimedLoopList_T<T_user_data>::CheckTimeOut() {
    time_t t_now = time(0);
    if (t_now-m_last_check_timeout>m_timeout_interval ||
            m_node_tail->valid) {
        //  Check time out
        m_last_check_timeout = t_now;
        for (;;) {
            if (m_node_tail->valid) {
                if (t_now-m_node_tail->t0 > m_timeout_interval) {
                    // The node time out
                    OnTimeOut(m_node_tail);
                } else
                    break;
            }

            if (m_node_tail == m_node_head)
                break;
            m_node_tail = m_node_tail->next;
        }
    }
}

template<typename T_user_data>
bool    CNewTimedLoopList_T<T_user_data>::IsBufferReady() {
    if (m_node_head->next == m_node_tail) {
        //  尝试移动m_ptrNodeTail, 当都不超时, 就会发生队列节点假耗尽现象
        while (m_node_tail->next != m_node_head &&
                !m_node_tail->valid)
            m_node_tail = m_node_tail->next;
    }

    return (m_node_head->next != m_node_tail) ? true:false;
}

template<typename T_user_data>
void    CNewTimedLoopList_T<T_user_data>::SetNodeInvalid(
    NewTimedNode<T_user_data>* node
) {
    if (node) {
        if (node->valid) {
            //  Don't close the socket handle
            node->valid = false;
            node->t0 = 0;

            if (!(node == m_node_head || node == m_node_tail)) {
                //  Remove node
                NewTimedNode<T_user_data>* old_pre = node->pre;
                NewTimedNode<T_user_data>* old_next = node->next;
                old_pre->next = old_next;
                old_next->pre = old_pre;

                //  Insert node
                old_pre = m_node_tail->pre;
                old_pre->next = node;
                node->pre = old_pre;
                node->next = m_node_tail;
                m_node_tail->pre = node;
            }
        } else {
            LOG(ERROR) << "SetNodeInvalid ERROR***, node flag = invalid.";
        }
    }
}
