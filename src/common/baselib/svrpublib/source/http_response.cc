#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/http_response.h"

_START_XFS_BASE_NAMESPACE_
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
const uint32_t kDefaultHeadLen = 4096 + 1024 + 1024;

CGetHttpResponse::CGetHttpResponse() {

    m_http_buffer = NULL;
    m_http_buffer_len=0;
    m_http_content_len=0;
}

CGetHttpResponse::~CGetHttpResponse() {
    delete m_http_buffer;
    m_http_buffer = NULL;
}

HTTP_CGI_ERROR CGetHttpResponse::GetResponse(const char* host,
        uint16_t    port,
        const char* cgi,
        const char* param,
        bool        is_post,
        uint32_t    timeout,
        const char* head)

{
#define CLOSE_SOCKET_AND_RETURN(fd, error_code) \
        CloseSocket(fd); \
        return error_code;

    if(m_http_buffer == NULL) {
        if (head) {
            int32_t head_len = strlen(head);
            m_http_buffer = new char[kDefaultHeadLen + head_len];
            m_http_buffer_len = kDefaultHeadLen + head_len;
        } else {
            m_http_buffer = new char[kDefaultHeadLen];
            m_http_buffer_len = kDefaultHeadLen;
        }

        if(m_http_buffer == NULL) {
            LOG(WARNING) << "ERROR_HTTP_NOT_ENOUGH_MEMORY";
            return ERROR_HTTP_NOT_ENOUGH_MEMORY;
        }
    }
    int32_t recv_bytes = 0;
    bool is_recv_ok = false;

    char *req = m_http_buffer;;
    uint32_t req_len=m_http_buffer_len;

    // 用户可能需要的包头合适和默认的不同，支持用户自己构造包头
    GenerateRequest(host,cgi,param,is_post,req,&req_len,head);

	VLOG(3) << "try get http response:";
    int fd= NewSocket(true);
    if(fd <=0) {
        LOG(WARNING) << " new ERROR_SOCKET";
        return ERROR_HTTP_SOCKET;
    }

    XSetSocketBlockingMode(fd, false);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    int ret = connect(fd,(struct sockaddr*)&addr,sizeof(struct sockaddr));
    if(ConnWriteable(fd, 3, &addr) == false) {
        LOG(WARNING) << "ERROR_CONNECT_TIMEOUT";
        CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_CONNECT_TIMEOUT);
    }

    // 显示拼凑后的参数
    if (head) {
        VLOG(3) << "the package head is:";
        VLOG(3) << req;
        VLOG(3) << "param:" << param;
    } else {
        VLOG(3) << "request package:\r\n" << req;
    }

    uint32_t send_len = 0;
    if( false == SendFixedBytes(fd, req, req_len, &send_len)) {
        LOG(WARNING) << "SendFixedBytes return false req_len=" << req_len;
        CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_SEND);
    }

    if(req_len != send_len) {
        LOG(WARNING) << "SendFixedBytes req_len=" << req_len << " send_len=" << send_len;
        CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_SEND);
    }

    if(!ConnReadable(fd, timeout)) {
        LOG(WARNING) << "ERROR_RECV_TIMEOUT";
        CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_RECV_TIMEOUT);
    }

    // recv head
    bool is_header_ok=false;
    for(int32_t i=0; i<m_http_buffer_len-1; i++) {
        while(1) {
            ret = RecvDat(fd, req + i, 1, 0);
            if(ret ==0) {
                LOG(WARNING) << "ERROR_HTTP_RECV_HEAD";
                CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_RECV_HEAD);
            }
            if(ret > 0)
                break;

            int32_t err = GetLastSocketError();
            if(!(err == EAGAIN || err == POSIX_EWOULDBLOCK))
                break;
        }
        if( ret !=1)
            break;

        if(i > 2 && *(uint16_t*)(req+i-1) == *(uint16_t*)"\n\n") {
            req[i+1]=0;
            is_header_ok = true;
            break;
        }

        if(i > 4 && *(uint32_t*)(req+i-3) == *(uint32_t*)"\r\n\r\n") {
            req[i+1]=0;
            is_header_ok = true;
            break;
        }
    }

    if(is_header_ok == false) {
        LOG(WARNING) << "ERROR_HTTP_CHECK_HEAD_FAIL";
        CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_CHECK_HEAD_FAIL);
    }

    int32_t http_code = 0;
    int32_t content_length = 0;
    bool b = ParseHttpHead(req, &http_code, &content_length);
    if(!b) {
        LOG(WARNING) << "ERROR_HTTP_PARSE_FAIL";
        CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_PARSE_FAIL);
    }

    if(content_length ==0) {
        LOG(WARNING) << "content_length ==0";
        CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_OK);
    }

    if(content_length >0) {
        if(m_http_buffer_len < content_length) {
            m_http_buffer_len = content_length;
            delete []m_http_buffer;
            m_http_buffer = new char[m_http_buffer_len+1];
            if(m_http_buffer == NULL) {
                LOG(WARNING) << "ERROR_HTTP_NOT_ENOUGH_MEMORY";
                CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_NOT_ENOUGH_MEMORY);
            }
        }

        is_recv_ok = ReceiveFixedPackage(fd,m_http_buffer,content_length,
                                         timeout,&recv_bytes);
        if(!is_recv_ok || recv_bytes != content_length) {
            LOG(WARNING) << "ERROR_RECV_TIMEOUT";
            CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_RECV_TIMEOUT);
        }
        SetHttpContentLen(content_length);
    } else {
        // chunked
        // 接收一行
        char lines[1024];
        int32_t current_len =0;
        while(1) {
            SetHttpContentLen(current_len);
            uint32_t lines_len = 1000;
            lines[0] = '\0';
            if(!RecvLines(fd,lines,&lines_len)) {
                LOG(WARNING) << "ERROR_HTTP_RECV_CHUNK_FAIL";
                CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_RECV_CHUNK_FAIL);
            }

            // 这个时候已经到达结尾处
            if(lines_len ==0) {
                CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_OK);
            }

            int32_t chunked_length=0;
            sscanf(lines,"%X",&chunked_length);

            if(chunked_length ==0) {
                CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_OK);
            }

            if( chunked_length + current_len >= m_http_buffer_len) {
                if( m_http_buffer_len*2 < (chunked_length + current_len)) {
                    m_http_buffer_len += chunked_length + current_len;
                } else {
                    m_http_buffer_len *= 2;
                }

                char* tmp = new char[m_http_buffer_len];
                if(tmp == NULL) {
                    LOG(WARNING) << "ERROR_HTTP_NOT_ENOUGH_MEMORY";
                    CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_NOT_ENOUGH_MEMORY);
                }

                memcpy(tmp,m_http_buffer,current_len);
                delete m_http_buffer;
                m_http_buffer = tmp;
            }

            // recv content
            is_recv_ok = ReceiveFixedPackage(fd,m_http_buffer+current_len,
                                             chunked_length,timeout,&recv_bytes);
            if(!is_recv_ok || recv_bytes != chunked_length) {
                LOG(WARNING) << "ERROR_HTTP_RECV_CONTENT_FAIL";
                CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_RECV_CONTENT_FAIL);
            }
            current_len += chunked_length;
        }

        SetHttpContentLen(current_len);
    }

    VLOG(3) << "receive http body ok";
    CLOSE_SOCKET_AND_RETURN(fd, ERROR_HTTP_OK);
}

HTTP_CGI_ERROR CGetHttpResponse::GetResponse(const char* domain,
                                             const char* cgi,
                                             const char* param,
                                             bool is_post,
                                             uint32_t timeout,
                                             const char* head) {
    // 默认就是80端口
    uint16_t port = 80;
    std::string host = "";
    std::string string_domain = domain;
    std::string::size_type pos_find = string_domain.find_first_of(":");
    
    if (pos_find == std::string::npos) {
        host = string_domain;
    } else {
        host = string_domain.substr(0, pos_find);
        port = static_cast<uint16_t>(ATOI(string_domain.substr(pos_find + 1).c_str()));
    }
    
    char ip[24];
    GetHostByName(host.c_str(), ip, sizeof(ip));
    return GetResponse(ip, port, cgi, param, is_post, timeout, head);
}

void CGetHttpResponse::GenerateRequest(const char* host,
                                       const char* cgi,
                                       const char* param,
                                       bool  is_post,
                                       char*       req,
                                       uint32_t*   req_len,
                                       const char* head) {
    uint32_t max_len = *req_len;
    uint32_t len =0;

    if (head) {
        len += safe_snprintf(req+len,max_len-len-1,head);
    } else {
        if(is_post)
            len += safe_snprintf(req+len,max_len-len-1,"POST %s",cgi);
        else {
            len += safe_snprintf(req+len,max_len-len-1,"GET %s",cgi);
            if(param) {
                len += safe_snprintf(req+len,max_len-len-1,"?%s",param);
            }
        }
        len += safe_snprintf(req+len,max_len-len-1," HTTP/1.1\r\n");
        len += safe_snprintf(req+len,max_len-len-1,"Accept: */*\r\n");
        len += safe_snprintf(req+len,max_len-len-1,"Accept-Language: zh-cn\r\n");
        len += safe_snprintf(req+len,max_len-len-1,"UA-CPU: x86\r\n");
        len += safe_snprintf(req+len,max_len-len-1,"User-Agent: Mozilla/4.0 "
                             "(compatible; XFS/1.0; +http://xfs.soso.oa.com/)\r\n");
        len += safe_snprintf(req+len,max_len-len-1,"Host: %s\r\n",host);
        if(is_post)
            len += safe_snprintf(req+len,max_len-len - 1,
                                 "Content-Length: %u\r\n", STRLEN(param));
        len += safe_snprintf(req+len,max_len-len-1,"Connection: Keep-Alive\r\n\r\n");
    }

    if(is_post)
        len += safe_snprintf(req+len,max_len-len-1,"%s\r\n\r\n",param);
    *req_len = len;
}

bool CGetHttpResponse::ParseHttpHead(char* http_head,
                                     int32_t* http_code,
                                     int32_t *content_length) {
    *content_length = -1;
    char* p = http_head;
    p =strchr(http_head,' ');
    *http_code = atoi(p+1);
    if(*http_code != 200)
        return false;

    while(p) {
        char* q = strchr(p,'\n');
        if(q)
            *q++=0;

        if(strstr(p,"CONTENT_LENGTH") || strstr(p, "Content-Length")
                || strstr(p, "content-length")) {
            while(*p && !isdigit(*p)) p++;
            *content_length = atoi(p);
            break;
        }
        p = q;
    }
    return true;
}
bool CGetHttpResponse::RecvLines(int fd,char* lines,uint32_t* lines_len) {
    uint32_t max_len = *lines_len;
    bool has_r = false;
    for(uint32_t i=0; i<max_len; i++) {
        int32_t recv_len=0;
        bool is_recv_ok = ReceiveFixedPackage(fd,lines+i,1,0,&recv_len);
        if(!is_recv_ok || recv_len !=1)
            return false;

        if (*(lines+i) =='\r') {
            has_r = true;
        }

        if(*(lines+i) =='\n') {
            *lines_len = i;
            // 可能以\r\n结尾，所以长度减1
            if (has_r && *(lines+i-1) == '\r') {
                *lines_len -= 1;
            }

            return true;
        }
    }
    return false;
}
_END_XFS_BASE_NAMESPACE_
