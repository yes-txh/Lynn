// TransDataThread.h: interface for the CTransDataThread class.
// 该线程用于连接ca server端，发送用户验证请求，并解析server端的返回值
//////////////////////////////////////////////////////////////////////
#ifndef COMMON_CRYPTO_CA_CA_SERVER_CA_SERVER_PERFORMANCE_TEST_TRANSDATATHREAD_H_
#define COMMON_CRYPTO_CA_CA_SERVER_CA_SERVER_PERFORMANCE_TEST_TRANSDATATHREAD_H_
#include "common/baselib/svrpublib/server_publib_namespace.h"

typedef enum {
    TRANS_STATE_ERROR              = 0,
    TRANS_STATE_SEND               = 1,
    TRANS_STATE_RECV               = 2,
} TRANS_STATE;

struct TransItem {
    SOCKET          fd;
    TRANS_STATE     state;
    char            *buffer;
    uint32_t        valid_len;
    uint32_t        transed_len; // 用户记录传输的长度
    uint64_t        start_time;
    TransItem() {
        fd = INVALID_SOCKET;
        state = TRANS_STATE_ERROR;
        buffer = NULL;
        valid_len = 0;
        transed_len = 0;
        start_time = 0;
    }
} ;

const uint32_t kMaxTCPConnections = 1024;
const uint32_t kBuffSize = 4096;
class CTransDataThread: public CXThreadBase {
public:
    CTransDataThread() : m_epoll_events(NULL),
                         m_send_count(0),
                         m_send_fail_count(0),
                         m_ok_count(0),                         
                         m_fail_count(0),
                         m_connect_count(0),
                         m_max_tcp_conns(0),
                         m_epoll_error_count(0),
                         m_cost_time_min(100000),
                         m_cost_time_max(0),
                         m_cost_time_total(0) {
        m_send_buff[0] = 0;
        m_temp_recv_buff[0] = 0;
    }

    virtual ~CTransDataThread();

    virtual void    Routine() ; // 继承者必须实现这个函数

    // 创建epoll句柄，创建套接口，连接server，注册epoll事件
    bool Init(const char* host, uint16_t port, uint32_t conn_num);

    // 获取发送请求的数目
    uint32_t GetSendCount() { return m_send_count; }
    // 获取发送fail的数目
    uint32_t GetSendFailCount() { return m_send_fail_count; }
    // 获取server成功返回的数目
    uint32_t GetOkCount() { return m_ok_count; }
    // 获取server错误返回的数目
    uint32_t GetFailCount() { return m_fail_count; }
    // 获取总共创建的连接数
    uint32_t GetConnectCount() { return m_connect_count; }
    // 获取epoll error的数目
    uint32_t GetEpollErrorCount() { return m_epoll_error_count; }
    // 获取每个请求最大耗费时间
    uint64_t GetMaxCostTime() { return m_cost_time_max; }
    // 获取每个请求最小耗费时间
    uint64_t GetMinCostTime() { return m_cost_time_min; }
    // 获取每个请求平均耗费时间
    uint64_t GetAverCostTime() { return m_ok_count == 0 ? 0 : (m_cost_time_total / m_ok_count); }

private:
    // 连接server，注册epoll事件
    bool Connect2Serv(TransItem* item);

    // 向server发送数据，
    bool Send2Serv(TransItem* item);

    uint64_t GetCurrentTime();

    bool AddToEpoll(SOCKET sock, epoll_event* ev);
    bool DelFromEpoll(SOCKET sock);

private:
    epoll_event* m_epoll_events;
    uint32_t m_send_count;
    uint32_t m_send_fail_count;
    uint32_t m_ok_count;
    uint32_t m_fail_count;    
    uint32_t m_connect_count;
    uint32_t m_max_tcp_conns; // 最大tcp连接个数
    uint32_t m_epoll_error_count;
    uint64_t m_cost_time_min;
    uint64_t m_cost_time_max;
    uint64_t m_cost_time_total;
    char m_send_buff[kBuffSize]; // 存储待发送的数据
    char m_temp_recv_buff[kBuffSize];
    
    
    // m_max_tcp_conns数可由用户传入
    // 一个epoll中同时保持m_max_tcp_conns个tcp连接
    TransItem   m_items[kMaxTCPConnections]; 
    struct sockaddr_in m_addr;
    EPOLLHANDLE m_epoll_handle;
};

#endif // COMMON_CRYPTO_CA_CA_SERVER_CA_SERVER_PERFORMANCE_TEST_TRANSDATATHREAD_H_
