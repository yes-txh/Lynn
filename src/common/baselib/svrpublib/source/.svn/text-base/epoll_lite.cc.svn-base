// epoll_lite.cpp: implementation of the CEpoll class.
// wookin@tencent.com
// 2007-05-30
// ////////////////////////////////////////////////////////////////////

#include "common/baselib/svrpublib/general_head.h"
#include "common/baselib/svrpublib/epoll_lite.h"
#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

// ////////////////////////
// Construction/Destruction
// ////////////////////////
CEpoll::CEpoll() {
    m_is_attached_listen_handle = false;
    m_listen_sock = INVALID_SOCKET;

    //
    // epoll handle and events array
    //
    m_epoll_handle = INVALID_HANDLE;
    m_epoll_events = 0;
    m_epoll_size = 0;

    m_break_epoll_wait = 0;

    m_sock_pair_read = m_sock_pair_write = INVALID_SOCKET;
    srand((uint32_t)time(0));
}

bool    CEpoll::EpollInit(uint32_t epoll_size,
                          SOCKET listen_handle,
                          bool* break_epoll_wait) {
    m_break_epoll_wait = break_epoll_wait;
    m_is_attached_listen_handle = true;
    return Initialize(epoll_size, listen_handle);
}

bool    CEpoll::EpollInit(uint32_t epoll_size,
                          const char* listen_host, uint16_t listen_port,
                          uint16_t listen_baklog,
                          bool* break_epoll_wait) {
    m_break_epoll_wait = break_epoll_wait;
    m_is_attached_listen_handle = false;

    bool b = false;
    bool b1 = true;

    //  ? listen
    if (listen_host) {
        b1 = false;
        m_listen_sock = NewSocket(true);
        if (m_listen_sock != INVALID_SOCKET) {
            // ToDo(wookin): 这里是支持可以传入域名或者ip都可以,
            //               在R2统一所有glibc版本之前先不支持这个
            //               (静态编译警告)
            // char ip[32] = {0};
            // GetHostByName(listen_host, ip, sizeof(ip));

            const char* ip = listen_host;
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr(ip);
            addr.sin_port = H2NS(listen_port);
            XSetSocketReuseAddress(m_listen_sock, true);
            if (bind(m_listen_sock,
                     (struct sockaddr*)&addr,
                     sizeof(addr)) == 0 &&
                    listen(m_listen_sock, listen_baklog) == 0) {
                VLOG(3) << "listen on: " << listen_host << ":" << listen_port <<
                          " ok. socket = " << m_listen_sock;
                b1 = true;
            } else {
                LOG(ERROR) << "bind("
                           << listen_host
                           << ","
                           << listen_port
                           << ") fail. error info:"
                           << strerror(errno);
                CloseSocket(m_listen_sock);
            }
        }
    }

    if (b1)
        b = Initialize(epoll_size, m_listen_sock);
    return b;
}

bool  CEpoll::Initialize(uint32_t epoll_size, SOCKET listen_sock) {
    bool b = false;
    if (epoll_size) {
        m_epoll_events = new epoll_event[epoll_size+1];
        m_epoll_handle = epoll_create(epoll_size+1);
        if (m_epoll_events && m_epoll_handle != (EPOLLHANDLE)INVALID_SOCKET) {
            m_epoll_size = epoll_size+1;
            memset(m_epoll_events, 0, sizeof(epoll_event)*m_epoll_size);
            b = true;

            //  ? listen
            if (listen_sock != INVALID_SOCKET) {
                b = false;
                m_listen_sock = listen_sock;
                if (m_listen_sock != INVALID_SOCKET) {
                    //  Add listen handle to epoll set
                    struct epoll_event ev;
                    memset(&ev, 0, sizeof(ev));
                    ev.events = EPOLLIN|EPOLLHUP|EPOLLERR;
                    ev.data.ptr = &m_listen_sock;
                    b = AddToEpoll(m_listen_sock, &ev);
                }
            }
        }
    }

    if (b && m_break_epoll_wait)
        b = RebuildNotifySocketPair();

    if (!b)
        Uninit();
    return b;
}

void CEpoll::Uninit() {
    // Close listen socket
    if (m_listen_sock != INVALID_SOCKET && !m_is_attached_listen_handle)
        CloseSocket(m_listen_sock);
    m_listen_sock = INVALID_SOCKET;
    m_is_attached_listen_handle = false;

    // epoll handle and events array
    if (m_epoll_handle != (EPOLLHANDLE)INVALID_SOCKET)
        epoll_close(m_epoll_handle);
    m_epoll_handle = (EPOLLHANDLE)INVALID_SOCKET;

    // release resource
    delete []m_epoll_events;
    m_epoll_events = NULL;
    m_epoll_size = 0;

    CloseSocketA(m_sock_pair_read, TCP_FD_ACCEPTED);
    CloseSocketA(m_sock_pair_write, TCP_FD_NEW);

    m_break_epoll_wait = 0;
}

void CEpoll::Routine(uint32_t epoll_wait_millsecs) {
    if (m_epoll_handle == (EPOLLHANDLE)INVALID_HANDLE)
        return;

    int32_t nfds = epoll_wait(m_epoll_handle, m_epoll_events, m_epoll_size,
                              epoll_wait_millsecs);

    // ? break epoll_wait
    bool is_break = m_break_epoll_wait ? (*m_break_epoll_wait) : false;
    if (is_break)
        return;

    for (int32_t i = 0; i < nfds; i++) {
        if (m_epoll_events[i].data.ptr == &m_listen_sock) {
            // Maybe tcp connection new arrive
            if ((m_epoll_events[i].events & EPOLLIN) == EPOLLIN)
                OnAccept(m_listen_sock);
            else
                OnListenError(m_listen_sock);
        } else if (
            reinterpret_cast<volatile SOCKET*>(m_epoll_events[i].data.ptr) ==
                                              &m_sock_pair_read) {
            // Maybe have notify message
            char buff[4];
            int32_t received = RecvDat(m_sock_pair_read, buff, 1, 0);

            int32_t err = GetLastSocketError();
            bool rebuild = false;
            if(received == 0 ||
                (received < 0 && !(err == EAGAIN || err == POSIX_EWOULDBLOCK)) )
                rebuild = true;

            //
            // 让上层有机会优先处理其他异步提交的事务,
            // 下次继续处理Epoll消息, 只能使用Epoll水平触发
            //
            if (received > 0) {
                OnEpollAsyncNotify(((i + 1) < nfds));
                break;
            }

            if (rebuild) {
                LOG(ERROR) << "receive notify message fail, "
                           "maybe rebuild notify socket pair."
                           "errno:" << errno << ":" << strerror(errno)
                           << ", m_sock_pair_read=" << m_sock_pair_read;

                rebuild = RebuildNotifySocketPair();
                if (!rebuild) {
                    LOG(ERROR) << "rebuild notify socket pair fail.";
                }
            }
        } else {
            OnEvent(&m_epoll_events[i]);
        }
    }
}

bool    CEpoll::AddToEpoll(SOCKET sock, epoll_event* ev) {
    bool b = false;
    if (m_epoll_handle != (EPOLLHANDLE)INVALID_HANDLE &&
            sock != INVALID_SOCKET) {
        b = epoll_ctl(m_epoll_handle, EPOLL_CTL_ADD, sock, ev) == 0 ?
            true :
            false;
        if (b == false) {
            LOG(ERROR) << "epoll_ctl error(Add): sock = " << sock <<
                       ", err: " << errno << ": "  << strerror(errno);
        }
    } else {
        LOG(ERROR) << "epoll_ctl AddToEpoll error socket=" << sock;
    }
    return b;
}

SOCKET  CEpoll::GetListenHandle() const {
    return m_listen_sock;
}

bool CEpoll::DeleteFromEpoll(SOCKET sock) {
    bool b = false;
    if (m_epoll_handle != (EPOLLHANDLE)INVALID_HANDLE &&
            sock != INVALID_SOCKET) {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.events = EPOLLIN|EPOLLOUT|EPOLLERR|EPOLLHUP;
        b = epoll_ctl(m_epoll_handle, EPOLL_CTL_DEL, sock, &ev) == 0 ?
            true :
            false;
        if(!b) {
            LOG(ERROR) << "epoll_ctl error(Del): sock = " << sock <<
                       ", err: " << errno << ": "  << strerror(errno);
        }
    }
    return b;
}

bool    CEpoll::ModifyFromEpoll(SOCKET sock, epoll_event* new_event) {
    // Xprintf_DEBUG("ModifyFromEpoll, hSock = %d, m_hEpollHandle = %d\r\n", sock, m_epoll_handle);

    bool b = false;
    if (m_epoll_handle != (EPOLLHANDLE)INVALID_HANDLE &&
            sock != INVALID_SOCKET) {
        b = epoll_ctl(m_epoll_handle, EPOLL_CTL_MOD, sock, new_event) == 0  ?
            true :
            false;
        if (!b) {
            LOG(ERROR) << "epoll_ctl error(Mod): sock = " << sock <<
                       ", err: " << errno << ": "  << strerror(errno);
            LOG(ERROR) << "epoll_ctl error(Mod): new events:" << new_event->events <<
                       ", m_hEpollHandle = " << m_epoll_handle;
        }
    }
    return b;
}

void    CEpoll::ForceBreakCurEpollWait() {
    if (m_sock_pair_write == INVALID_SOCKET) {
        LOG(ERROR) << "force break current epoll_wait fail, "
                     "m_sock_pair_write = INVALID_SOCKET";
        return;
    }

    // Send notify data
    int32_t bytes = 0;
    int32_t try_count = 0;
    do {
        bytes = SendDat(m_sock_pair_write, "x", 1, 0);
        if (bytes > 0)
            break;
        if (bytes == 0) {
            // Network busy
            if (try_count >= 500)
                break;
            XSleep(1);
            try_count++;
        } else {
            int32_t err = GetLastSocketError();
            if (err == EAGAIN || err == POSIX_EWOULDBLOCK){               
                // Try again
                if (try_count >= 500)
                    break;
                XSleep(1);
                try_count++;;
            } else { // Error
                LOG(ERROR) << "***ERROR, Level 3***, send data fail, "
                           "on ForceBreakCurEpollWait(), errno:"
                           << errno << ":" << strerror(errno);
                break;
            }
        }
    } while(bytes <= 0);
}

void    CEpoll::EmptyNotifyMessage() {
    if (m_sock_pair_read != INVALID_SOCKET) {
        char buff[256];
        int32_t nbytes = 0;        
        do {
            nbytes = RecvDat(m_sock_pair_read, buff, sizeof(buff), 0);
            int32_t err = GetLastSocketError();
            if(nbytes <= 0 && !(err == EAGAIN || err == POSIX_EWOULDBLOCK)){                
                LOG(ERROR) << "try EmptyNotifyMessage fail, err:" << errno
                           << " ," << strerror(errno);
            }
        } while(nbytes >= (int32_t)(sizeof(buff)));
    }
}

//
// Get peer_A of socket pair
//
void CEpoll::GetNotifyWriteSockOfSocketPair(SOCKET** sock) {
    *sock = &m_sock_pair_write;
}

bool CEpoll::RebuildNotifySocketPair() {
    bool b = false;
    if (m_sock_pair_write != INVALID_SOCKET)
        CloseSocketA(m_sock_pair_write, TCP_FD_NEW);

    if (m_sock_pair_read != INVALID_SOCKET) {
        DeleteFromEpoll(m_sock_pair_read);
        CloseSocketA(m_sock_pair_read, TCP_FD_ACCEPTED);
    }

    uint16_t success_port = 0;
    char local_host[32] = {0};
    // 127.0.0.1
    safe_snprintf(local_host, sizeof(local_host), "127.0.%d.%d",
                  safe_rand() % 256, GetPID() % 255 + 1);
    uint32_t netorder_host = inet_addr(local_host);
    uint16_t netorder_start_port = kSocketPairStartPort + ((uint16_t)safe_rand() % 1000);
    netorder_start_port = htons(netorder_start_port);
    if (CreateSocketPairAutoPort(&m_sock_pair_read,
                                 &m_sock_pair_write,
                                 netorder_host,
                                 netorder_start_port,
                                 &success_port)) {
        // Try add notify socket
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.events = EPOLLIN|EPOLLHUP|EPOLLERR;
        ev.data.ptr = reinterpret_cast<uint32_t*>(&m_sock_pair_read);
        if (!AddToEpoll(m_sock_pair_read, &ev)) {
            LOG(ERROR) << "rebuild notify socket pair fail, "
                       "add to epoll fail.";
            CloseSocketA(m_sock_pair_write, TCP_FD_NEW);
            CloseSocketA(m_sock_pair_read, TCP_FD_ACCEPTED);
        } else
            b = true;
    } else {
        LOG(ERROR) << "RebuildNotifySocketPair fail, "
                   "create socket pair fail.";
    }

    VLOG(3) << "";
    VLOG(3) << "Create SocketPair(listen port:" << success_port << ") OK, "
              "fd_read = " << m_sock_pair_read << ", fd_write = " << m_sock_pair_write;
    VLOG(3) << "";

    return b;
}

_END_XFS_BASE_NAMESPACE_
