#ifndef COMMON_BASELIB_SVRPUBLIB_HTTP_RESPONSE_H_
#define COMMON_BASELIB_SVRPUBLIB_HTTP_RESPONSE_H_

#include <stdio.h>
#include <string>
#include "common/baselib/svrpublib/base_config.h"
#include "common/baselib/svrpublib/server_publib.h"

_START_XFS_BASE_NAMESPACE_

#define TIMEOUT_INFINITE 30
//连接服务器超时时间
#define MAX_CONN_TIMEOUT 3

enum HTTP_CGI_ERROR {
    ERROR_HTTP_OK = 1,
    ERROR_HTTP_NOT_ENOUGH_MEMORY = 2,
    ERROR_HTTP_CONNECT,
    ERROR_HTTP_CONNECT_TIMEOUT,
    ERROR_HTTP_RECV_TIMEOUT,
    ERROR_HTTP_SEND,
    ERROR_HTTP_SOCKET,
    ERROR_HTTP_RECV_HEAD,
    ERROR_HTTP_CHECK_HEAD_FAIL,
    ERROR_HTTP_PARSE_FAIL,
    ERROR_HTTP_RECV_CHUNK_FAIL,
    ERROR_HTTP_RECV_CONTENT_FAIL,
    ERROR_HTTP_HTTPSTATUS
};

inline const char* GetHttpErrString(HTTP_CGI_ERROR err) {
    switch(err) {
    case ERROR_HTTP_OK:
        return "ERROR_HTTP_OK";
    case ERROR_HTTP_NOT_ENOUGH_MEMORY:
        return "ERROR_HTTP_NOT_ENOUGH_MEMORY";
    case ERROR_HTTP_CONNECT:
        return "ERROR_HTTP_CONNECT";
    case ERROR_HTTP_CONNECT_TIMEOUT:
        return "ERROR_HTTP_CONNECT_TIMEOUT";
    case ERROR_HTTP_RECV_TIMEOUT:
        return "ERROR_HTTP_RECV_TIMEOUT";
    case ERROR_HTTP_SEND:
        return "ERROR_HTTP_SEND";
    case ERROR_HTTP_SOCKET:
        return "ERROR_HTTP_SOCKET";
    case ERROR_HTTP_RECV_HEAD:
        return "ERROR_HTTP_RECV_HEAD";
    case ERROR_HTTP_CHECK_HEAD_FAIL:
        return "ERROR_HTTP_CHECK_HEAD_FAIL";
    case ERROR_HTTP_PARSE_FAIL:
        return "ERROR_HTTP_PARSE_FAIL";
    case ERROR_HTTP_RECV_CONTENT_FAIL:
        return "ERROR_HTTP_RECV_CONTENT_FAIL";
    case ERROR_HTTP_RECV_CHUNK_FAIL:
        return "ERROR_HTTP_RECV_CHUNK_FAIL";
    case ERROR_HTTP_HTTPSTATUS:
        return "ERROR_HTTP_HTTPSTATUS";
    default:
        return "unknown http error code";
    }
}

class CGetHttpResponse {
public:
    HTTP_CGI_ERROR GetResponse(const char* host,
                               uint16_t    port,
                               const char* cgi,
                               const char* param,
                               bool        is_post,
                               uint32_t    timeout=TIMEOUT_INFINITE,
                               const char* head=NULL);

    // domain可以是xfs.soso.oa.com，默认为80
    // 也可以是xfs.soso.oa.com:8080
    // 也支持127.0.0.1:8080和127.0.0.1等ip模式
    HTTP_CGI_ERROR GetResponse(const char* domain,
                               const char* cgi,
                               const char* param,
                               bool        is_post,
                               uint32_t    timeout=TIMEOUT_INFINITE,
                               const char* head=NULL);

    const char* GetHttpContent() {
        return m_http_buffer;
    }

    void SetHttpContentLen(int32_t len) {
        m_http_content_len = len;
    }

    int32_t GetHttpContentLen() {
        return m_http_content_len;
    }

    CGetHttpResponse();
    virtual ~CGetHttpResponse();
private:
    void GenerateRequest(   const char* host,
                            const char* cgi,
                            const char* param,
                            bool  is_post,
                            char*       req,
                            uint32_t*   req_len,
                            const char* head);

    bool ParseHttpHead(char* http_head,int32_t* http_code,int32_t *content_length);
    bool RecvLines(int fd,char* lines,uint32_t* lines_len);


private:
    char * m_http_buffer;
    int32_t m_http_buffer_len;
    int32_t m_http_content_len;
};
_END_XFS_BASE_NAMESPACE_
#endif // COMMON_BASELIB_SVRPUBLIB_HTTP_RESPONSE_H_
