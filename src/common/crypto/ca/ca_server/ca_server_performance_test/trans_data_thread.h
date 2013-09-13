// TransDataThread.h: interface for the CTransDataThread class.
// ���߳���������ca server�ˣ������û���֤���󣬲�����server�˵ķ���ֵ
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
    uint32_t        transed_len; // �û���¼����ĳ���
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

    virtual void    Routine() ; // �̳��߱���ʵ���������

    // ����epoll����������׽ӿڣ�����server��ע��epoll�¼�
    bool Init(const char* host, uint16_t port, uint32_t conn_num);

    // ��ȡ�����������Ŀ
    uint32_t GetSendCount() { return m_send_count; }
    // ��ȡ����fail����Ŀ
    uint32_t GetSendFailCount() { return m_send_fail_count; }
    // ��ȡserver�ɹ����ص���Ŀ
    uint32_t GetOkCount() { return m_ok_count; }
    // ��ȡserver���󷵻ص���Ŀ
    uint32_t GetFailCount() { return m_fail_count; }
    // ��ȡ�ܹ�������������
    uint32_t GetConnectCount() { return m_connect_count; }
    // ��ȡepoll error����Ŀ
    uint32_t GetEpollErrorCount() { return m_epoll_error_count; }
    // ��ȡÿ���������ķ�ʱ��
    uint64_t GetMaxCostTime() { return m_cost_time_max; }
    // ��ȡÿ��������С�ķ�ʱ��
    uint64_t GetMinCostTime() { return m_cost_time_min; }
    // ��ȡÿ������ƽ���ķ�ʱ��
    uint64_t GetAverCostTime() { return m_ok_count == 0 ? 0 : (m_cost_time_total / m_ok_count); }

private:
    // ����server��ע��epoll�¼�
    bool Connect2Serv(TransItem* item);

    // ��server�������ݣ�
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
    uint32_t m_max_tcp_conns; // ���tcp���Ӹ���
    uint32_t m_epoll_error_count;
    uint64_t m_cost_time_min;
    uint64_t m_cost_time_max;
    uint64_t m_cost_time_total;
    char m_send_buff[kBuffSize]; // �洢�����͵�����
    char m_temp_recv_buff[kBuffSize];
    
    
    // m_max_tcp_conns�������û�����
    // һ��epoll��ͬʱ����m_max_tcp_conns��tcp����
    TransItem   m_items[kMaxTCPConnections]; 
    struct sockaddr_in m_addr;
    EPOLLHANDLE m_epoll_handle;
};

#endif // COMMON_CRYPTO_CA_CA_SERVER_CA_SERVER_PERFORMANCE_TEST_TRANSDATATHREAD_H_
