// TransDataThread.cpp: implementation of the CTransDataThread class.
// ���߳���������ca server�ˣ������û���֤���󣬲�����server�˵ķ���ֵ
//////////////////////////////////////////////////////////////////////
#include "common/base/compatible/errno.h"
#include "common/crypto/ca/ca_server/ca_server_performance_test/trans_data_thread.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTransDataThread::~CTransDataThread() {
    // delete epoll handle and events array
    if (m_epoll_handle != (EPOLLHANDLE)INVALID_SOCKET)
        epoll_close(m_epoll_handle);
    m_epoll_handle = (EPOLLHANDLE)INVALID_SOCKET;

    // release resource
    delete []m_epoll_events;
    m_epoll_events = NULL;

}

uint64_t CTransDataThread::GetCurrentTime() {
    struct timeval tv;
    fast_getrelativetimeofday(&tv);
    uint64_t current_time = tv.tv_sec;
    current_time = current_time*1000000 + tv.tv_usec;
    return current_time;
}

// ����epoll����������׽ӿڣ�����server��ע��epoll�¼�
bool CTransDataThread::Init(const char* host, uint16_t port, uint32_t conn_num) {
    uint32_t i = 0;

    m_max_tcp_conns = conn_num < kMaxTCPConnections ? conn_num: kMaxTCPConnections;

    // set address
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = inet_addr(host);
    m_addr.sin_port = H2NS(port);

    // create epoll handle
    m_epoll_handle = epoll_create(1024);

    if (m_epoll_handle == (EPOLLHANDLE)INVALID_SOCKET) {
        return false;
    }

    // prepare events buffer
    m_epoll_events = new epoll_event[m_max_tcp_conns];
    memset(m_epoll_events, 0, sizeof(epoll_event)*m_max_tcp_conns);

    int32_t package_len = safe_snprintf(m_send_buff, kBuffSize,
        "GET /ca_check_user.html?"
        "CA_USERNAME=pecywang&SIGN=68C304E4EE1FC4ED5F533BF071624685B9AE9897"
        "246C45E1626649A642D419DEC2CEE75C72EDBFEE29B8513FF7851D2C2115252EBA"
        "9C8018FE8DD205B895FDF94074CB0909D556D88401A49E5EFB26F1B6B3BA9D94E3"
        "1C16659DC0&SIGN_LEN=91 "
        "HTTP/1.1\n"
        "Accept: */*\n"
        "Accept-Language: zh-cn\n"
        "UA-CPU: x86\n"
        "User-Agent: Mozilla/4.0 (compatible; XFS/1.0; +http://xfs.soso.oa.com/)\n"
        "Host: 172.26.1.184\n"
        "Connection: Keep-Alive\n\n");

    for (i = 0; i < m_max_tcp_conns; i++) {
        m_items[i].buffer = m_send_buff;
        m_items[i].valid_len = package_len;
    }


    // connect to server �����߳����������������m_max_tcp_conns����main������������x���߳�
    // ������ӷ������ɹ�����socket���ע�ᵽepoll��
    for (i = 0; i < m_max_tcp_conns; i++) {
        Connect2Serv(&m_items[i]);
    }
    return true;
}

bool CTransDataThread::Connect2Serv(TransItem* item) {
    // �첽�����ж�socket
    if (item->fd != INVALID_SOCKET) {
        CloseSocket(item->fd);
    }
    
    item->fd = NewSocket(true);
    if (INVALID_SOCKET == item->fd) {
        LOG(FATAL) << "NewSocket error";
        return false;
    }

    // connect to server
    XSetSocketReuseAddress(item->fd, true);
    // �첽������Ҫ�жϣ�ret != 0 && errno != EINPROGRESS��
    connect(item->fd, (struct sockaddr*)&m_addr, sizeof(m_addr));

    item->state = TRANS_STATE_SEND;
    item->transed_len = 0;
    item->start_time = GetCurrentTime();
    // add events to epoll
    epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLHUP | EPOLLERR | EPOLLOUT;
    ev.data.ptr = item;

    // add to epoll set
    AddToEpoll(item->fd, &ev);
    m_connect_count++;
    return true;
}



void CTransDataThread::Routine() {
//    char buf[2048] = {0};
    // �ȴ��¼��Ĳ���,���ں˵õ��¼��ļ���
    int32_t nfds = epoll_wait(m_epoll_handle, m_epoll_events, m_max_tcp_conns, 150);
    for (int32_t i = 0; i < nfds; ++i) {
        epoll_event* ptr_ev = &m_epoll_events[i];
        TransItem* ptr_item = reinterpret_cast<TransItem*>(ptr_ev->data.ptr);
        if (NULL == ptr_item) {
            LOG(ERROR) << "ptr_item is NULL";
            continue;
        }

        // epoll error
        if ((ptr_ev->events & EPOLLERR) == EPOLLERR || (ptr_ev->events & EPOLLHUP) == EPOLLHUP) {
            m_epoll_error_count++;
            // ����
            LOG(ERROR) << "epoll error , fd = " << ptr_item->fd
                       << " " << strerror(errno);
            DelFromEpoll(ptr_item->fd);
            CloseSocket(ptr_item->fd);
            Connect2Serv(ptr_item);
            m_epoll_error_count++;
            continue;
        }

        if ((ptr_ev->events & EPOLLOUT) == EPOLLOUT) {
            if (TRANS_STATE_SEND == ptr_item->state) {
                Send2Serv(ptr_item);
                m_send_count++;
                continue;
            } else {
                DelFromEpoll(ptr_item->fd);
                CloseSocket(ptr_item->fd);
                Connect2Serv(ptr_item);
                continue;
            }
            

        }

        // epoll in
        if ((ptr_ev->events & EPOLLIN) == EPOLLIN) {
            if (TRANS_STATE_RECV != ptr_item->state) {
                DelFromEpoll(ptr_item->fd);
                CloseSocket(ptr_item->fd);
                Connect2Serv(ptr_item);
                continue;
            }  

           //  char buf[2048] = {0};
            // try receive data
            // �����ĵķ�������ΪCheckUserResult=OK������ȡ��Ƭ���գ�һ�ν�socket������ȫ������
            int32_t bytes = recv(ptr_item->fd, m_temp_recv_buff, kBuffSize, 0);

            if (bytes < 0) {
                LOG(ERROR) << "recv error, bytes received < 0";
            } else if (bytes == 0) {
                LOG(ERROR) << "recv error, bytes received = 0, lose connection";
            } else {
                // �ж��Ƿ���֤ͨ��
                if (strstr(m_temp_recv_buff, "CheckUserResult=OK")) {
                    m_ok_count++;
                } else if (strstr(m_temp_recv_buff, "CheckUserResult=FAIL")) {
                   m_fail_count++;
                }
                
                // ����յ�</html>����Ͽ�����
                // TODO(joeytian):
                // ���ַ�ʽ��ÿ�ζ����յ�һ���������������ʱ��Ч�ʻ����ԣ�
                // ���ÿ�β����յ�һ��������������ÿ�ζ�ȥ�����Ƿ��յ�</html>,Ч�ʺܵͣ�
                // �Ժ��Ż�����
                 if ( (NULL == strstr(m_temp_recv_buff, "</html>")) ) {
                     continue;
                 } else {
                     uint64_t cost_time = GetCurrentTime() - ptr_item->start_time;
                     m_cost_time_total += cost_time;
                     if(cost_time > m_cost_time_max) m_cost_time_max = cost_time;
                     if(cost_time < m_cost_time_min) m_cost_time_min = cost_time;
                 }
            }

            // �Ͽ�����
            DelFromEpoll(ptr_item->fd);
            CloseSocket(ptr_item->fd);
            Connect2Serv(ptr_item);
        }
    }
}


bool CTransDataThread::Send2Serv(TransItem* item) {
    // send request
    int32_t bytes = send(item->fd, item->buffer + item->transed_len,
                          item->valid_len - item->transed_len, 0);
    if (bytes < 0) {
        DelFromEpoll(item->fd);
        CloseSocket(item->fd);
        Connect2Serv(item);
        return false;
    } else if (bytes == 0) {
        // mybe busy
        return true;
    } else {
        item->transed_len += bytes;
        // δ������ϣ�������
        if (item->transed_len < item->valid_len)
            return false;

        // �������Ѿ������ˣ��л���EPOLLIN�¼�������EPOLL_CTL_MOD��Ч�ʲ���ֱ��ɾ�������
        item->state = TRANS_STATE_RECV;
        DelFromEpoll(item->fd);
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
        ev.data.ptr = item;
        AddToEpoll(item->fd, &ev);
        return true;
    }
}

bool    CTransDataThread::AddToEpoll(SOCKET sock, epoll_event* ev) {
    bool b = false;
    if (m_epoll_handle != (EPOLLHANDLE)INVALID_HANDLE &&
        sock != INVALID_SOCKET) {
            b = epoll_ctl(m_epoll_handle, EPOLL_CTL_ADD, sock, ev) == 0 ? true : false;
            if (b == false) {
                LOG(ERROR) << "epoll_ctl error(Add): sock = " << sock <<
                              ", err: " << errno << ": "  << strerror(errno);
            }
    } else {
        LOG(ERROR) << "epoll_ctl AddToEpoll error socket=" << sock;
    }
    return b;
}

bool CTransDataThread::DelFromEpoll(SOCKET sock) {
    bool b = false;
    if (m_epoll_handle != (EPOLLHANDLE)INVALID_HANDLE &&
        sock != INVALID_SOCKET) {
            struct epoll_event ev;
            memset(&ev, 0, sizeof(ev));
            ev.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
            b = epoll_ctl(m_epoll_handle, EPOLL_CTL_DEL, sock, &ev) == 0 ? true : false;
            if(!b) {
                LOG(ERROR) << "epoll_ctl error(Del): sock = " << sock <<
                    ", err: " << errno << ": "  << strerror(errno);
            }
    }
    return b;
}
