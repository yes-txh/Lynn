// simple_http.h: interface for the CReceiveDataThread class.
// wookin@tencent.com
// ////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_SIMPLE_HTTP_H_
#define COMMON_BASELIB_SVRPUBLIB_SIMPLE_HTTP_H_

#include "common/baselib/svrpublib/parse_proc.h"
#include "common/baselib/svrpublib/http_buff.h"
#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

#ifndef STRNCASECMP
#ifdef WIN32
#define STRNCASECMP _strnicmp
#else
#define STRNCASECMP strncasecmp
#endif
#endif

// CSimpleHttpReceiveThread�������ݵ���ʱbuffer����
const int32_t kHttpRecvBuffLen = 4096;

//
// simple http���ݴ���ڵ�
//
struct SimpleHttpNode {
    SimpleHttpNode*     next;
    SOCKET              fd_socket;
    BufferV             buffer;
    time_t              t; // ���ڼ�¼�ӳٿ�ʼʱ��
    SimpleHttpNode() {
        next = NULL;
        fd_socket = INVALID_SOCKET;
        t = 0;
    }

    ~SimpleHttpNode() {}
};

class CSimpleHttpReceiveThread:public CXThreadBase {
public:

    CSimpleHttpReceiveThread();
    virtual ~CSimpleHttpReceiveThread();

    // listen on host:port
    bool Init(const char* listen_host,
              uint16_t listen_port,
              uint32_t timeout_seconds);

    void Uninit();

    uint16_t GetListenPort() {
        return m_listen_port;
    }

    // --------------------------------------
    // implement for CXThreadBase
    virtual void    Routine(); // �̳��߱���ʵ���������

    void SetOutQueue(IQueuePut<SimpleHttpNode>* put_queue);

    // fast end thread
    virtual void EndThread();
private:
    bool ReceiveData(SOCKET fd_socket, BufferV* bufferv_recv,
                     uint32_t timeout_seconds);

    // ����̼߳��������queue
    std::vector<IQueuePut<SimpleHttpNode>* > m_out_queues;

    void OutputNode(SimpleHttpNode* &node);

    SOCKET      m_fd_socket;
    uint32_t    m_timeout_seconds;
    char        m_listen_host[32];
    uint16_t    m_listen_port;
    uint32_t    m_continue_fail_count;

    
    char m_temp_buff[kHttpRecvBuffLen+1];

    // force break routine and end thread
    bool m_break_routine_and_end_thread;
};


class CBaseHttpProcThread:public CXThreadBase {
public:
    void SetInfo(const char* listen_host,
                 uint16_t listen_port,
                 const char* xsl_filename = "");

    CBaseHttpProcThread();
    virtual ~CBaseHttpProcThread();

    const char* GetListenHostPort();
    const char* GetProxyRequest();

    // implement for IXThreadBase
    virtual void    Routine();

    // �û����Լ̳��������ʵ�־�����Ӧ
    virtual bool OnUserHTTPRequest(const BufferV* bufferv_recevied,
                                   CHttpBuff* httpbuff_response);

    IQueuePut<SimpleHttpNode>* GetInputQueueInterface() {
        IQueuePut<SimpleHttpNode>* in = &m_http_queue;
        return in;
    }

    void Init() {
        m_http_queue.Init();
    }

private:

    // ����Ĭ�ϵ�����
    virtual bool OnDefaultHttpRequest(const BufferV* bufferv_recevied,
                                      CHttpBuff* httpbuff_response);

    // ��������ʱ�ķ�������
    virtual void OnDefaultErrorRequest(const char* req_type,
                                       const char* file_name,
                                       BufferV* raw_data,
                                       CHttpBuff* httpbuff_response,
                                       bool* load_local_file_ok);

    // �������ô��������
    virtual void OnSetProxyRequest(const BufferV* bufferv_recevied,
                                   CHttpBuff* httpbuff_response);

    // ���ͽ��
    bool SendHttpResponse(SOCKET fd_socket, CHttpBuff* httpbuff_response);

    bool SendRawHttpResponse(SOCKET fd_socket, const char* req_type,
                             const char* file_name, BufferV* raw_data);

    // ��ȡϵͳ��Ϣ
    bool GetCPUInfo(CHttpBuff* httpbuff_response);
    bool GetMEMInfo(CHttpBuff* httpbuff_response);
    bool GetDISKInfo(CHttpBuff* httpbuff_response);
    bool GetNETInfo(CHttpBuff* httpbuff_response);

    void CheckTimeout();

private:
    // ÿ�������߳�����һ������,�������ݶ�������������߳��з�������,
    // �����Ƕ�������߳�ȥ�����ݶ����о���ȥ��
    CVDataQueue_T<SimpleHttpNode>   m_http_queue;

    char                        m_listen_host[64];
    CHttpBuff                   m_httpbuff;
    CParseProc                  m_proc_parse;

    char                        m_default_home_url[1024];
    char                        m_default_proxy_url[1024];

    // �����ļ��洢Ŀ¼
    char                        m_home_dir[MAX_PATH];

    CBaseQ<SimpleHttpNode>      m_delay_queue;
};

int32_t PrePareHttpHead(char* buff,
                        int32_t buff_len,
                        uint32_t html_body_len,
                        const char* content_type);

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_SIMPLE_HTTP_H_

