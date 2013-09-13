// simple_http.cpp: implementation of the CReceiveDataThread class.
//
// ////////////////////////////////////////////////////////////////////

#include <stdio.h>
#ifdef WIN32
#include <stdlib.h>
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#else
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/resource.h>
#endif

#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/simple_http.h"
#include "common/baselib/svrpublib/base_config.h"
#include "common/baselib/svrpublib/default_xsl.h"
#include "common/baselib/svrpublib/xfs_auto_build_version.h"

_START_XFS_BASE_NAMESPACE_

#ifdef WIN32
#pragma   warning(disable:4127)
#endif // WIN32

// ////////////////////////////////////////////////////////////////////
// Construction/Destruction
// ////////////////////////////////////////////////////////////////////
CSimpleHttpReceiveThread::CSimpleHttpReceiveThread() {
    m_fd_socket = INVALID_SOCKET;
    m_timeout_seconds = 0;
    memset(m_listen_host, 0, sizeof(m_listen_host));
    m_listen_port  = 0;
    m_continue_fail_count = 0;

    m_break_routine_and_end_thread = false;

    m_temp_buff[0] = 0;
    srand(static_cast<uint32_t>(time(NULL)));
}

bool CSimpleHttpReceiveThread::Init(const char* listen_host,
                                    uint16_t listen_port,
                                    uint32_t timeout_seconds) {
    bool b = false;
    m_fd_socket = NewSocket(true);

    if (m_fd_socket != INVALID_SOCKET) {
        b = ListenOnPort(m_fd_socket, listen_host, listen_port, 15);

        if(!b) {
            CloseSocketA(m_fd_socket, TCP_FD_NEW);
            LOG(ERROR) << "SimpleHttp::Init fail, try listen on: "
                       << listen_host << " : " << listen_port << " FAIL";

        } else {
            m_listen_port = listen_port;
            safe_snprintf(m_listen_host, sizeof(m_listen_host), "%s", listen_host);
        }
    }

    m_timeout_seconds = timeout_seconds;
    return b ? true : false;
}

void CSimpleHttpReceiveThread::Uninit() {
    CloseSocket(m_fd_socket);
}

CSimpleHttpReceiveThread::~CSimpleHttpReceiveThread() {
    Uninit();
}

void CSimpleHttpReceiveThread::Routine() {
    // deal invalid socket
    if (m_fd_socket == INVALID_SOCKET) {
        XSleep(200);
        return;
    }

    // listen 超时设置
    // ? readable
    if (ConnReadableWithTimeout(m_fd_socket, 100, true)) {
        m_continue_fail_count = 0;

        // accept
        sockaddr_in from;
        memset(&from, 0, sizeof(from));
        SOCKET fd_socket = AcceptNewConnection(m_fd_socket, &from);

        if (fd_socket != INVALID_SOCKET) {
            VLOG(3) << "Accept new http connect request, sock:" << fd_socket;

            // client可能是IE等非规范client,
            // 不能等到它们发送close消息或者主动关闭信号
            // 每次数据发送完毕就主动关闭连接
            XSetSocketLinger(fd_socket, true, 2);

            XSetSocketNoDelay(fd_socket);

        } else { // accept fail
            return;
        }

        // prepare node
        SimpleHttpNode* node = mempool_NEW(SimpleHttpNode);

        if (!node) {
            LOG(ERROR) << "maybe out of memory.";
            CloseSocketA(fd_socket, TCP_FD_ACCEPTED);
            return;
        }

        // ? receive data ok
        if (ReceiveData(fd_socket, &node->buffer, m_timeout_seconds)) {
            VLOG(3) << "received user request:" << reinterpret_cast<char*>(node->buffer.buff);
            node->fd_socket = fd_socket;
            node->next = NULL;
            OutputNode(node);

        } else { // receive data fail
            LOG(ERROR) << "Receive data from:" << inet_ntoa(from.sin_addr) << ":"
                       << N2HS(from.sin_port)  <<  " fail, or time out.";
            CloseSocketA(fd_socket, TCP_FD_ACCEPTED);

            mempool_DELETE(node);
        }

    } else {
        // 每半小时才输出一次, 30分钟才输出超时日志
        m_continue_fail_count++;

        if(m_continue_fail_count % (60 * 30) == 0) {
            m_continue_fail_count = 0;
            LOG(WARNING) << "detect new http request timeout, listen sock:" << m_fd_socket;
        }
    }

    // ? end thread
    if(m_break_routine_and_end_thread) {
        CXThreadBase::StopRoutine();
        return;
    }
}

// fast end thread
void CSimpleHttpReceiveThread::EndThread() {
    // break ConnReadable ...
    m_break_routine_and_end_thread = true;

    CXThreadBase::EndThread();
}

void CSimpleHttpReceiveThread::SetOutQueue(IQueuePut<SimpleHttpNode>* put_queue) {
    if(put_queue) {
        m_out_queues.push_back(put_queue);
    }
}

void CSimpleHttpReceiveThread::OutputNode(SimpleHttpNode* &node) {
    int32_t len = m_out_queues.size();

    if(len) {
        IQueuePut<SimpleHttpNode>* out = m_out_queues[rand() % len];
        out->AddSingleNode(node);

    } else {
        CloseSocketA(node->fd_socket, TCP_FD_ACCEPTED);
        LOG(ERROR) << "no output queue.";
        mempool_DELETE(node);
    }

    node = NULL;
}

bool CSimpleHttpReceiveThread::ReceiveData(SOCKET fd_socket,
        BufferV* bufferv_recv,
        uint32_t timeout_seconds) {
    if (fd_socket == INVALID_SOCKET || !bufferv_recv) {
        return false;
    }

    int32_t i = 0;

    // try receive data in timeout_seconds seconds
    time_t time_out = timeout_seconds;
    time_t t_start = time(NULL);
    time_t t_now = t_start;
    int32_t num_received = 0;
    int32_t free_buff = 0;

    int32_t first_newline_pos = 0; // 记录第一次出现换行符的地方 \r or \n
    int32_t body_start_pos = 0; // 收头部的时候会到body start的地方才结束

    //
    // 每次必须接收完所有数据,使用m_temp_buff,最大长度为kHttpRecvBuffLen + 1,
    // 保留最后一位设置为0
    //

    // 收头部
    bool recv_head_ok = false;

    while (num_received < kHttpRecvBuffLen &&
            !recv_head_ok && (t_now - t_start) <= time_out) {
        int32_t t_remain = static_cast<int32_t>(time_out) - static_cast<int32_t>(t_now - t_start);
        int32_t ret = FD_Readable(fd_socket, t_remain * 1000, false);

        if (ret == 0) {
            if(t_remain == 0) {
                break;
            }

            // time out

        } else if (ret < 0 ) {
            VLOG(3) << "FD_Readable fail, ret = -1, err:" << errno << ":" << strerror(errno);
            break;

        } else if (ret > 0) {
            // 每次尽量读取
            // 计算buff里面的剩余空间
            free_buff = kHttpRecvBuffLen - num_received;
            int32_t bytes = RecvDat(fd_socket, m_temp_buff + num_received, free_buff, 0);

            if (bytes == 0) {
                VLOG(3) << "recv fail, received bytes = 0, err:" <<
                        errno << ":" << strerror(errno) << ". Remote closed.";
                break; // remote closed

            } else if (bytes > 0) {

                if(num_received == 0) {
                    // 第一次收到数据,判断是不是正规的GET,POST,HEAD 开头的HTTP请求
                    if(!(m_temp_buff[0] == 'G' || m_temp_buff[0] == 'P' || m_temp_buff[0] == 'H')) {
                        LOG(ERROR) << "invalid request method, only accept GET, POST";
                        break;
                    }
                }

                // 查找空行结束标志的开始地方
                int32_t check_start = 0;
                int32_t ckeck_len = 0;

                if(num_received > 0) {
                    check_start = num_received - 1;
                    ckeck_len = bytes + 1;

                } else {
                    check_start = num_received;
                    ckeck_len = bytes;
                }

                num_received += bytes;

                // 至少需要两个字符才能检查
                if(ckeck_len >= 2) {
                    // 判断是否遇到第一个空行,表示收头部结束
                    int32_t check_end = check_start + ckeck_len - 1;

                    for(i = check_start; i < check_end; ++i) {
                        if(first_newline_pos == 0 &&
                                (m_temp_buff[i] == '\r' || m_temp_buff[i] == '\n')) {
                            first_newline_pos = i;
                        }

                        // GET 请求只收第一行
                        if(m_temp_buff[0] == 'G') {
                            if(m_temp_buff[i] == '\r' || m_temp_buff[i] == '\n') {
                                recv_head_ok = true;
                                break;
                            }

                        } else {
                            // 其余情况按POST, 先收头部
                            if(m_temp_buff[i] == '\n'  // 上一个字符\n,下一个字符为\r\n之一
                                    && (m_temp_buff[i + 1] == '\r' || m_temp_buff[i + 1] == '\n')) {
                                recv_head_ok = true;
                                body_start_pos = i + 1;
                                break; // break while
                            }
                        }
                    }
                }

            } else if (bytes < 0) { // error
                VLOG(3) << "receive data fail, received = " << bytes << " bytes,err:" <<
                        errno << ":" << strerror(errno) << "maybe remote closed.";
                break;
            }
        }

        // 优化下,少取一次当前时间,最后一次的当前时间无效
        if(recv_head_ok) {
            break;
        }

        t_now = time(NULL);
    }

    bool b = false;

    // 如果收头部成功
    if(!recv_head_ok) {
        return b;
    }

    // GET请求只要第一行
    if(memcmp(m_temp_buff, "GET", 3) == 0 ) {
        // 截断后面的内容
        m_temp_buff[first_newline_pos] = 0;
        char* page_name = strstr(m_temp_buff, "GET ");

        if (page_name) {
            page_name += 4; // "GET "
            // 寻找正式名字中第一个结束空格
            char* end = strchr(page_name, ' ');

            if(end) {
                *end = 0;
                // 不计算STRLEN(),优化速度 len = STRLEN(page_name);
                //
                int32_t len = static_cast<int32_t>(end - page_name);

                if (bufferv_recv->CheckBuffer(len + 1)) {
                    bufferv_recv->SetData(reinterpret_cast<unsigned char*>(page_name), len + 1);
                    b = true;
                }
            }
        }

        return b;
    }

    // 另一种情况必然为POST
    if(memcmp(m_temp_buff, "POST", 4) != 0) {
        return b;
    }

    // 获取post 数据长度
    char* post_len_tag = strstr(m_temp_buff, "Content-Length");

    if(!post_len_tag) {
        post_len_tag = strstr(m_temp_buff, "content-length");
    }

    if(!post_len_tag) {
        LOG(ERROR) << "try get content-length fail: sz:  " << m_temp_buff;
        return b;
    }

    char* digs = strstr(post_len_tag, ":");

    if(digs) {
        ++digs;
    }

    while(*digs == ' ') {
        ++digs;    // 跳过空格
    }

    int32_t post_len ATOI(digs);

    // 计算可能已经接收到的body
    int32_t received_body = 0;

    if(num_received > body_start_pos) { // 有可能已经接收到部分body
        for(i = body_start_pos; i < num_received; ++i) {
            // 跳过到body之前的 \r or \n,位置在最前面
            if(m_temp_buff[i] == '\r' || m_temp_buff[i] == '\n') {
                ++body_start_pos;

            } else {
                break;
            }
        }

        received_body = num_received - (body_start_pos);
    }

    // 接收剩余数据    b = false;

    bool recv_body_ok = ((post_len == 0) || (received_body >= post_len)) ? true : false;

    while (num_received < kHttpRecvBuffLen &&
            received_body < post_len &&
            !recv_body_ok &&
            (t_now - t_start) <= time_out) {
        int32_t t_remain = (int32_t)time_out - (int32_t)(t_now - t_start);
        int32_t ret = FD_Readable(fd_socket, t_remain * 1000, false);

        if (ret == 0) {
            if(t_remain == 0) {
                break;    // time out
            }

        } else if (ret < 0 ) {
            VLOG(3) << "FD_Readable fail, num_fds = -1, err" <<
                    errno << ":" << strerror(errno);
            break;

        } else if (ret > 0) {
            // 尽量全部读取
            free_buff = kHttpRecvBuffLen - num_received;
            int32_t bytes = RecvDat(fd_socket, m_temp_buff + num_received, free_buff, 0);

            if (bytes == 0) {
                VLOG(3) << "receive data fail, received bytes = 0, err:" << errno << ":"
                        << strerror(errno) << ". Remote closed.";
                break; // remote closed

            } else if (bytes > 0) {
                // 统计是否满足post长度
                int32_t check_start = num_received;
                int32_t check_end = check_start + bytes;
                num_received += bytes;

                for(i = check_start; i < check_end; ++i) {
                    if(received_body == 0) { // 跳过上次可能少收的一个结尾符号
                        if(!(m_temp_buff[i] == '\r' || m_temp_buff[i] == '\n')) {
                            ++received_body;
                            body_start_pos = i;
                        }

                    } else {
                        ++received_body;
                    }

                    if(received_body == post_len) {
                        recv_body_ok = true;
                        break;
                    }
                }

            } else if (bytes < 0) { // error
                VLOG(3) << "receive data fail, received = " << bytes << " bytes, err: "
                        << errno << ":" << strerror(errno) << " maybe remote closed.";
                break;
            }
        }

        if(recv_body_ok) {
            break;    // 少一次while判断
        }

        t_now = time(NULL);
    }

    if(!recv_body_ok) {
        return b;
    }

    // 合并body 到 第一行,形成如 POST /filename.html?key1=val1&key2=val2...
    char* post_page = strstr(m_temp_buff, "POST ");

    if (post_page) {
        post_page += 5; // "POST "
        // 寻找正式名字中第一个结束空格
        char* end = strchr(post_page, ' ');

        if(!end) {
            return b;
        }

        if(post_len == 0) {
            *end = 0;

        } else {
            *end = '?';
            end++;

            for(int32_t u = 0; u < post_len; u++) {
                *end = m_temp_buff[body_start_pos + u];
                ++end;
            }

            *end = 0;
        }

        int32_t page_param_len = (int32_t)STRLEN(post_page);

        if (bufferv_recv->CheckBuffer(page_param_len + 1)) {
            bufferv_recv->SetData(reinterpret_cast<unsigned char*>(post_page), page_param_len + 1);
            b = true;
        }
    }

    return b;
}

// ----------------------------------------------------------------------------

CBaseHttpProcThread::CBaseHttpProcThread() {
    memset(m_listen_host, 0, sizeof(m_listen_host));
    memset(m_default_home_url, 0, sizeof(m_default_home_url));
    memset(m_default_proxy_url, 0, sizeof(m_default_proxy_url));

    memset(m_home_dir, 0, sizeof(m_home_dir));
    GetModuleFileName(NULL, m_home_dir, sizeof(m_home_dir));

    // linux and windows
    char* p = strrchr(m_home_dir, '/');

    if ( !p ) {
        p = strrchr(m_home_dir, '\\');
    }

    if(p) {
        p++;
        *p = 0;

        // 追加目录
        sprintf(p, "htdocs");
        struct stat stat_buff;

        if (stat(m_home_dir, &stat_buff) != 0) {
#ifdef _WIN32
            _mkdir(m_home_dir);
#else
            mkdir(m_home_dir, 0777);
#endif
        }

        char xsl_filename[256] = {0};
        safe_snprintf(xsl_filename, sizeof(xsl_filename),
                      "%s/xfs_builtin_default.xsl", m_home_dir);

        if (stat(xsl_filename, &stat_buff) != 0) {
            FILE* fp = fopen(xsl_filename, "w");

            if (fp) {
                fwrite(kDefaultXSL, sizeof(kDefaultXSL) - 1 , 1, fp);
                fclose(fp);
            }
        }

    } else {
        LOG(ERROR) << "GetModuleFileName fail.";
    }
}

CBaseHttpProcThread::~CBaseHttpProcThread() {
    // 适当延迟队列中的节点
    SimpleHttpNode* node = NULL;

    while(m_delay_queue.GetNodeOnHead(&node)) {
        CloseSocketA(node->fd_socket, TCP_FD_ACCEPTED);
        mempool_DELETE(node);
    }
}

void CBaseHttpProcThread::SetInfo(const char* listen_host,
                                  uint16_t listen_port,
                                  const char* xsl_filename) {
    srand((uint32_t)(100));

    safe_snprintf(m_listen_host,
                  sizeof(m_listen_host),
                  "%s:%d",
                  listen_host,
                  listen_port);
    m_httpbuff.SetHostPort(m_listen_host);
    m_httpbuff.SetXslFile(xsl_filename);
}

void CBaseHttpProcThread::Routine() {
    // try get data queue
    SimpleHttpNode* head = 0;
    m_http_queue.GetQueue(&head);

    // deal each node
    for (; head;) {
        SimpleHttpNode* ptmp = head;
        head = head->next;
        ptmp->next = NULL;

        m_httpbuff.ResetHttpContent();
        OnSetProxyRequest(&ptmp->buffer, &m_httpbuff);

        // 判断是否是cpu, mem, disk等通用请求
        bool b = OnDefaultHttpRequest(&ptmp->buffer, &m_httpbuff);

        if (!b) {
            b = OnUserHTTPRequest(&ptmp->buffer, &m_httpbuff);
            VLOG(3) << "OnUserHTTPRequest result: " << (b ? "true" : "false");
        }

        // 用户过程没有实现这个页面
        bool load_local_file_ok = false;
        // get filename
        char file_name[MAX_PATH] = {0};
        // get type
        char req_type[32] = {0};
        BufferV raw_dat;

        // 如果默认过程和用户处理都失败则认为可能是本地文件
        if (!b) {
            // 如果是local data就保存在这个对象里面
            char* req_page = (char*)ptmp->buffer.buff;
            char* p = strchr(req_page, '?');

            if (p) {
                *p = 0;
            }

            p = strrchr(req_page, '/');

            if (!p) {
                p = strrchr(req_page, '\\');
            }

            // 本地文件也必须以/ or \开头
            if(p) {
                // get filename
                safe_snprintf(file_name, sizeof(file_name), "%s",
                              (p[0] == '/' || p[0] == '\\') ? &p[1] : p);

                char full_file_name[MAX_PATH] = {0};
                safe_snprintf(full_file_name, sizeof(full_file_name), "%s%s", m_home_dir,
                              (req_page[0] == '/' || req_page[0] == '\\') ? req_page : &req_page[1]);

                // get type
                safe_snprintf(req_type, sizeof(req_type) - 1, "%s", strchr(p, '.'));

                OnDefaultErrorRequest(req_type,
                                      full_file_name,
                                      &raw_dat,
                                      &m_httpbuff,
                                      &load_local_file_ok);

            } else {
                VLOG(3) << "invalid request filename: " << req_page << ", must as : /xxxx.xx";
            }
        }

        // is page not found
        if(!load_local_file_ok && !b) {
            m_httpbuff.BeginGroup("page_not_found", 404);
            m_httpbuff.AddKey("PAGE_NOT_FOUND", file_name, " , please check the file name.");
            m_httpbuff.EndGroup("page_not_found");
            b = true;
        }

        // 发送Raw data还是拼装html
        bool send_ok = false;

        if(load_local_file_ok) {
            send_ok = SendRawHttpResponse(ptmp->fd_socket, req_type, file_name, &raw_dat);

        } else {
            // 非local file
            if(b) {
                send_ok = SendHttpResponse(ptmp->fd_socket, &m_httpbuff);
            }
        }

        if (send_ok) { // send ok
#ifdef WIN32
            //
            // TODO(wookin):添加socket到延迟队列
            //
            // 如果发送成功则加入到延迟队列, 因为某些垃圾linux机器上set linger不起作用
            // 以前发现linux下set linger不起作用,现在linux下linger启作用了。
            // 如果linux下linger又不启作用了则继续把这个socket放到延迟队列里面
            //
            ptmp->t = time(NULL);
            m_delay_queue.AppendNodeAtTail(ptmp);
            ptmp = NULL;
#endif

        } else {
            LOG(ERROR) << (load_local_file_ok ? "SendRawHttpResponse" : "SendHttpResponse") <<
                       " return false";
        }

        // 如果ptmp还存在, 说明不是失败或者就是没有放到异步延迟队列
        // 可以关闭了
        if(ptmp) {
            CloseSocketA(ptmp->fd_socket, TCP_FD_ACCEPTED);
            mempool_DELETE(ptmp);
        }
    }

    // 检查延迟节点是否可以关闭了
    CheckTimeout();
}

bool CBaseHttpProcThread::SendRawHttpResponse(SOCKET fd_socket,
        const char* req_type,
        const char* file_name,
        BufferV* raw_data) {
    VLOG(3) << "try send local file: " << file_name;

    if(!raw_data || raw_data->valid_len <= 0) {
        return false;
    }

    char disposition[MAX_PATH] = {0};
    const char* content_type = "text/html";

    if(req_type) {
        if(strcmp(req_type, ".xml") == 0) {
            content_type = "text/xml";
        }

        if(strcmp(req_type, ".xsl") == 0) {
            content_type = "text/xsl";
        }

        if(strcmp(req_type, ".jpg") == 0 || strcmp(req_type, ".jpeg") == 0 ||
                strcmp(req_type, ".JPG") == 0) {
            content_type = "image/jpeg";
        }

        if(strcmp(req_type, ".dat") == 0) {
            // content_type = "text/plain";
            content_type = "application/force-download";
            safe_snprintf(disposition, sizeof(disposition) - 1,
                          "Content-Disposition:attachment; filename=%s\r\n",
                          file_name[0] ? file_name : "unknow.dat");
        }
    }

    char head[1024] = {0};
    time_t t;
    time(&t);
    int32_t head_len =
        safe_snprintf(head, sizeof(head),
                      "HTTP/1.1 200 OK\n"
                      "Server: XFS robot/1.0 beta (Win32/linux)\n"
                      "Accept-Ranges: bytes\n"
                      "Connection: close\n"
                      "Date: %s"
                      "%s"
                      "Content-Length: %u\n"
                      "Content-Type: %s\n\n",
                      ctime(&t),
                      disposition[0] ? disposition : "",
                      raw_data->valid_len,
                      content_type);

    // send head
    bool b = SendFixedBytes(fd_socket, head, head_len, NULL, true);

    // send body
    if(b) {
        b = SendFixedBytes(fd_socket, raw_data->buff, raw_data->valid_len, NULL, true);
    }

    // send end flag
    if(b) {
        const char* end = "\r\n\r\n";
        b = SendFixedBytes(fd_socket, end, sizeof(end) - 1, NULL, true);
    }

    if(!b) {
        LOG(ERROR) << "SendRawHttpResponse fail, len:" << raw_data->valid_len;
    }

    return b;
}

bool CBaseHttpProcThread::SendHttpResponse(SOCKET fd_socket,
        CHttpBuff* httpbuff_response) {
    // 获取返回信息
    bool b = false;
    const char* head = NULL;
    const char* tail = NULL;
    const char* body = NULL;
    uint32_t head_len = 0, tail_len = 0, body_len = 0;

    if(httpbuff_response->IsRawData()) {

        // raw data
        const char* raw_data = NULL;
        uint32_t raw_data_len = 0;
        b = httpbuff_response->GetRawData(&raw_data, &raw_data_len);

        // 发送返回信息
        if (b) {
            b = SendFixedBytes(fd_socket, raw_data, raw_data_len, NULL, true);
        }

    } else {

        // normal data or XML
        b  = httpbuff_response->GetHead(&head, &head_len);

        if (b) {
            b = httpbuff_response->GetTail(&tail, &tail_len);
        }

        if (b) {
            b = httpbuff_response->GetBody(&body, &body_len);
        }

        // 生成HttpHead信息

        char buf_head[1024];
        uint32_t buf_head_len = 0;

        const char* content_type = "text/html";
        if (httpbuff_response->IsXML())
            content_type = "application/xml";
        if (httpbuff_response->HaveJasonCallback())
            content_type = "application/javascript";

        if (b) buf_head_len = PrePareHttpHead(reinterpret_cast<char*>(buf_head),
                                                  sizeof(buf_head),
                                                  head_len + tail_len + body_len,
                                                  content_type);

        // 发送返回信息
        if (b) {
            b = SendFixedBytes(fd_socket, buf_head, buf_head_len, NULL, true);
        }

        if (b) {
            b = SendFixedBytes(fd_socket, head, head_len, NULL, true);
        }

        if (b) {
            b = SendFixedBytes(fd_socket, body, body_len, NULL, true);
        }

        if (b) {
            b = SendFixedBytes(fd_socket, tail, tail_len, NULL, true);
        }
    }

    return b ? true : false;
}

void CBaseHttpProcThread::CheckTimeout() {
    time_t t_now = time(NULL);

    while(1) {
        SimpleHttpNode* head_ptr = m_delay_queue.GetHeadNodePtr();

        if(!head_ptr) {
            break;
        }

        // 暂定延迟大于2秒开始关闭
        if(t_now - head_ptr->t > 2) {
            SimpleHttpNode* node = NULL;
            m_delay_queue.GetNodeOnHead(&node);
            CloseSocketA(node->fd_socket, TCP_FD_ACCEPTED);
            mempool_DELETE(node);

        } else { // 开始没有超时的节点了
            break;
        }
    }
}

const char* CBaseHttpProcThread::GetListenHostPort() {
    return m_listen_host;
}

void CBaseHttpProcThread::OnSetProxyRequest(const BufferV* bufferv_recevied,
        CHttpBuff* httpbuff_response) {
    char* p = strchr(reinterpret_cast<char*>(bufferv_recevied->buff), '?');

    if (!p) {
        return;
    }

    CParserCGIParameter cgi;
    int32_t ok = cgi.AttachEnvironmentString(p + 1);

    if (ok) {
        const char* proxy = NULL;
        ok = cgi.GetParameter("proxy", &proxy);

        if (ok) {
            httpbuff_response->SetProxy(proxy);
            safe_snprintf(m_default_proxy_url,
                          sizeof(m_default_proxy_url),
                          "%s",
                          proxy);

        } else {
            // use default proxy
            if (STRLEN(m_default_proxy_url)) {
                httpbuff_response->SetProxy(m_default_proxy_url);
            }
        }

        const char* home = NULL;

        ok = cgi.GetParameter("home", &home);

        if (ok) {
            httpbuff_response->SetHome(home);
            safe_snprintf(m_default_home_url,
                          sizeof(m_default_home_url),
                          "%s",
                          home);

        } else {
            // use default home
            if (STRLEN(m_default_home_url)) {
                httpbuff_response->SetHome(m_default_home_url);
            }
        }

        // &xml=true ?
        const char* xml = NULL;
        ok = cgi.GetParameter("xml", &xml);
        if (ok && STRNCASECMP(xml, "true", 4) == 0) {
            httpbuff_response->SetContentType(ENUM_HTTP_XML);

            // &jasoncallback=xxx
            const char* jasoncallback = NULL;
            ok = cgi.GetParameter("jasoncallback", &jasoncallback);
            if(ok && jasoncallback){
                httpbuff_response->SetJasonCallback(jasoncallback);
            }
        }        
    }
}

const char* CBaseHttpProcThread::GetProxyRequest() {
    return m_default_proxy_url;
}

//
// 尝试从本地读取文件,如果不能读取则设置错误
//
void CBaseHttpProcThread::OnDefaultErrorRequest(const char* req_type,
        const char* file_name,
        BufferV* raw_data,
        CHttpBuff* httpbuff_response,
        bool* load_local_file_ok) {
    // 如果请求页面存在且文件名不为空
    bool get_local_ok = false;

    if(file_name && file_name[0] != 0) {
        FILE* fp = fopen(file_name, "rb");

        if(fp) {
            fseek(fp, 0, SEEK_END);
            int32_t size = ftell(fp);

            if(size > 0) {
                fseek(fp, 0, SEEK_SET);

                if(raw_data->CheckBuffer(size)) {
                    int32_t ret = fread(raw_data->buff, size, 1, fp);

                    if(ret == 1) {
                        raw_data->valid_len = size;
                    }

                    get_local_ok = true;

                } else {
                    LOG(ERROR) << "try load local file:" << file_name << " fail, "
                               "check buffer fail,the file size is too large.";
                }

            }

            fclose(fp);

        } else {
            VLOG(3) << "try open local file: " << file_name << ", fail";

            // 如果是打开.xsl文件,如果硬盘上找不到则使用默认.xsl
            if(req_type &&
                    (strncmp(req_type, ".xsl", strlen(".xsl")) == 0 || strncmp(req_type, ".XSL", strlen(".XSL")) == 0)) {
                uint32_t xsl_len = sizeof(kDefaultXSL) - 1;

                if(raw_data->CheckBuffer(xsl_len)) {
                    memcpy(raw_data->buff, kDefaultXSL, xsl_len);
                    raw_data->valid_len = xsl_len;

                    get_local_ok = true;

                } else {
                    LOG(ERROR) << "try load local file:" << file_name << " fail, "
                               "check buffer fail,the file size is too large.";
                }
            }
        }
    }

    // 如果读取本地文件失败
    if(get_local_ok) {
        if(load_local_file_ok) {
            *load_local_file_ok = true;
        }
    }

    if(!get_local_ok) {
        httpbuff_response->AddKey(
            "",
            "The page that you requested has moved or not support",
            "");
    }
}

bool  CBaseHttpProcThread::OnDefaultHttpRequest(const BufferV* buff_recevied,
        CHttpBuff* httpbuff_response) {

    const char page_server_state[] = "/system_info.html";

    if (strncmp(reinterpret_cast<char*>(buff_recevied->buff),
                page_server_state, sizeof(page_server_state) - 1) == 0) {
        // 获取CPU统计信息
        httpbuff_response->SetAttr("Cpu Info");

        httpbuff_response->BeginGroup("CPU");
        this->GetCPUInfo(httpbuff_response);
        httpbuff_response->EndGroup("CPU");

        // 获取MEM统计信息
        httpbuff_response->BeginGroup("MEM");
        this->GetMEMInfo(httpbuff_response);
        httpbuff_response->EndGroup("MEM");

        // 获取DISK统计信息
        httpbuff_response->BeginGroup("DISK");
        this->GetDISKInfo(httpbuff_response);
        httpbuff_response->EndGroup("DISK");

        // 获取NET统计信息
        httpbuff_response->BeginGroup("NET");
        this->GetNETInfo(httpbuff_response);
        httpbuff_response->EndGroup("NET");

        // Help
        httpbuff_response->BeginGroup("Help");
        httpbuff_response->AddKey("help-xml",
                                  "&xml=true: output as xml file");
        httpbuff_response->AddKey("home",
                                  "&home=http://some-xfs-cluster/ :"
                                  "set home page of xfs-cluster");
        httpbuff_response->EndGroup("Help");

        return true;
    }

    const char page_proxy[] = "/cgi-bin/simple_proxy.cgi";

    if (strncmp(reinterpret_cast<char*>(buff_recevied->buff),
                page_proxy, sizeof(page_proxy) - 1) == 0) {

        char* params = strchr((char*)buff_recevied->buff, '?');

        if(params) {
            ++params;
        }

        if(params) {
            httpbuff_response->SetUseRawData();
            CStrBuff* raw_buff = httpbuff_response->GetRawDataObj();
            raw_buff->Reset();

            // RealProxy中会多次调用callback
            Closure<void, const char* , int32_t>* cb =
                NewPermanentClosure(CallBackProxyOutput, raw_buff);

            CParserCGIParameter parser;
            parser.AttachEnvironmentString(params);

            // 获取本次调用proxy cgi的地址
            char refer_proxf[512] = {0};
            safe_snprintf(refer_proxf, sizeof(refer_proxf), "http://%s/cgi-bin/simple_proxy.cgi",
                          GetListenHostPort());

            RealProxy(&parser, false, cb, refer_proxf);
            delete cb;
            cb = NULL;

        } else {
            httpbuff_response->BeginGroup("invalid_params_of_simple_proxy");
            httpbuff_response->AddKey("proxy_invalid_param", "miss parameters ...");
            httpbuff_response->EndGroup("invalid_params_of_simple_proxy");
        }

        return true;
    }

    const char page_about[] = "/about.html";

    if (strncmp(reinterpret_cast<char*>(buff_recevied->buff),
                page_about, sizeof(page_about) - 1) == 0) {

        // 获取xfs build revison num
        httpbuff_response->BeginGroup("xfs_build_revision_num");
        httpbuff_response->AddKey("source code info", GetXFSRevisionNum());
        httpbuff_response->EndGroup("xfs_build_revision_num");

        // xfs build info
        httpbuff_response->BeginGroup("xfs_build_info");
        httpbuff_response->AddKey("build info", GetXFSBuildInfo());
        httpbuff_response->EndGroup("xfs_build_info");

        // help and example
        httpbuff_response->BeginGroup("cgi_example");

        httpbuff_response->AddKey("xml=true", "http://domain:port/filename.html?xml=true",
                                              " output xml format");
        httpbuff_response->AddKey("xml=true&jasoncallback=xxx",
                                  "http://domain:port/filename.html?xml=true&jasoncallback=fun_xx",
                                  "jasoncallback, fun_xx(\"...\");");

        // 手动构建
        char link[1024] = {0};
        safe_snprintf(link, sizeof(link), "http://%s/system_info.html", GetListenHostPort());
        char url[1024] = {0};
        httpbuff_response->MakeRealURL(link, STRLEN(link), url, sizeof(url));
        safe_snprintf(link, sizeof(link), "<a href='%s'> system info</a>", url);
        httpbuff_response->AddKey(link, "", "", true,
                                  "system_info");

        // 使用函数自动构建key,val,desc
        safe_snprintf(link, sizeof(link), "http://%s/cgi-bin/simple_proxy.cgi",
                      GetListenHostPort());
        httpbuff_response->AddInternalHrefKey("simple proxy", link,
                                              "",
                                              "cgi-bin/simple_proxy.cgi, default simple proxy");
        httpbuff_response->AddInternalHrefKey("simple proxy", link, "", "");
        httpbuff_response->EndGroup("cgi_example");

        return true;
    }


    return false;
}

bool  CBaseHttpProcThread::GetCPUInfo(CHttpBuff* httpbuff_response) {
    CpuRate    cpu_rate;
    std::vector<CpuStatus> cpu1;
    std::vector<CpuStatus> cpu2;

    if (false == m_proc_parse.ParseCpu(&cpu1)) {
        httpbuff_response->AddKey("Desc", "ParseCpu Permission denied");
        return false;
    }

    XSleep(200);

    if (false == m_proc_parse.ParseCpu(&cpu2)) {
        httpbuff_response->AddKey("Desc", "ParseCpu Permission denied");
        return false;
    }

    if ( cpu1.size() == 0 || cpu1.size() != cpu2.size()) {
        httpbuff_response->AddKey("Desc", "cpu counter error");
        return false;
    }

    httpbuff_response->AddKey("desc", "user system nice &nbsp;"
                              "idle iowait irq &nbsp;softirq");

    for (uint32_t i = 0; i < cpu1.size(); ++i) {
        m_proc_parse.CalcCpu(&cpu_rate, &cpu1[i], &cpu2[i]);

        char buf_tmp[12];
        char key[12];
        char value[1024];
        int32_t len  = 0;
        safe_snprintf(key, sizeof(key), "cpu%d", cpu1[i].id);

        len = safe_snprintf(buf_tmp, sizeof(buf_tmp), "%.1f", cpu_rate.user);
        int32_t val_buf_len = sizeof(value);
        int32_t n = 0;
        n = safe_snprintf(value + n, val_buf_len - n, "%s", buf_tmp);

        for (int32_t j = 0; j < 6 - len; j++) {
            n += safe_snprintf(value + n, val_buf_len - n, "%s", "&nbsp;");
        }

        len = safe_snprintf(buf_tmp, sizeof(buf_tmp), "%.1f", cpu_rate.system);
        n += safe_snprintf(value + n, val_buf_len - n, "%s", buf_tmp);

        for (int32_t j = 0; j < 6 - len; j++) {
            n += safe_snprintf(value + n, val_buf_len - n, "%s", "&nbsp;");
        }

        len = safe_snprintf(buf_tmp, sizeof(buf_tmp), "%.1f", cpu_rate.nice);
        n += safe_snprintf(value + n, val_buf_len - n, "%s", buf_tmp);

        for (int32_t j = 0; j < 6 - len; j++) {
            n += safe_snprintf(value + n, val_buf_len - n, "%s", "&nbsp;");
        }

        len = safe_snprintf(buf_tmp, sizeof(buf_tmp), "%.1f", cpu_rate.idle);
        n += safe_snprintf(value + n, val_buf_len - n, "%s", buf_tmp);

        for (int32_t j = 0; j < 6 - len; j++) {
            n += safe_snprintf(value + n, val_buf_len - n, "%s", "&nbsp;");
        }

        len = safe_snprintf(buf_tmp, sizeof(buf_tmp), "%.1f", cpu_rate.iowait);
        n += safe_snprintf(value + n, val_buf_len - n, "%s", buf_tmp);

        for (int32_t j = 0; j < 6 - len; j++) {
            n += safe_snprintf(value + n, val_buf_len - n, "%s", "&nbsp;");
        }

        len = safe_snprintf(buf_tmp, sizeof(buf_tmp), "%.1f", cpu_rate.irq);
        n += safe_snprintf(value + n, val_buf_len - n, "%s", buf_tmp);

        for (int32_t j = 0; j < 6 - len; j++) {
            n += safe_snprintf(value + n, val_buf_len - n, "%s", "&nbsp;");
        }

        len = safe_snprintf(buf_tmp, sizeof(buf_tmp), "%.1f", cpu_rate.softirq);
        n += safe_snprintf(value + n, val_buf_len - n, "%s", buf_tmp);

        httpbuff_response->AddKey(key, value);
    }

    return true;
}
// 格式化内存描述
// 输入以KB为单位的内存 返回GB TB KB MB等描述
std::string FormatMemoryDesc(uint32_t memory_kb) {
    char buf[100];
    float float_value = static_cast<float>(memory_kb);
    const float kb = 1024.00;

    if ( memory_kb >> 30 ) {
        safe_snprintf(buf, sizeof(buf), "%.2f T Bytes",
                      float_value / (kb * kb * kb));

    } else if ( memory_kb >> 20 ) {
        safe_snprintf(buf, sizeof(buf), "%.2f G Bytes",
                      float_value / (kb * kb));

    } else if ( memory_kb >> 10 ) {
        safe_snprintf(buf, sizeof(buf), "%.2f M Bytes", (static_cast<float>(memory_kb)) / 1024.00);

    } else {
        safe_snprintf(buf, sizeof(buf), "%u K Bytes", memory_kb);
    }

    return std::string(buf);
}

bool  CBaseHttpProcThread::GetMEMInfo(CHttpBuff* httpbuff_response) {
    // 系统内存信息
    SystemMemoryStatus mem;
    bool b = m_proc_parse.ParseSystemMemoryStatus(&mem);

    if (!b) {
        httpbuff_response->AddKey("Desc",
                                  "ParseSystemMemoryStatus Permission denied");
        return true;
    }

    httpbuff_response->BeginGroup("SystemMemory");

    int32_t out_len = 0;
    char desc[512];
    char key[100];
    char value[100];
    safe_snprintf(key, sizeof(key), "%s", MEM_TOTAL_LABLE);
    safe_snprintf(value, sizeof(value), "%s", FormatMemoryDesc(mem.mem_total).c_str());
    ConvertAnsiToUTF8("系统可用内存",
                      strlen("系统可用内存"),
                      desc,
                      sizeof(desc),
                      &out_len);
    desc[out_len] = 0;
    httpbuff_response->AddKey(key, value, desc);

    safe_snprintf(key, sizeof(key), "%s", MEM_FREE_LABLE);
    safe_snprintf(value, sizeof(value), "%s", FormatMemoryDesc(mem.mem_free).c_str());
    ConvertAnsiToUTF8("系统空闲内存",
                      strlen("系统空闲内存"),
                      desc,
                      sizeof(desc),
                      &out_len);
    desc[out_len] = 0;
    httpbuff_response->AddKey(key, value, desc);

    safe_snprintf(key, sizeof(key), "%s", MEM_BUFF_LABLE);
    safe_snprintf(value, sizeof(value), "%s", FormatMemoryDesc(mem.mem_buffers).c_str());
    ConvertAnsiToUTF8("文件缓冲区",
                      strlen("文件缓冲区"),
                      desc,
                      sizeof(desc),
                      &out_len);
    desc[out_len] = 0;
    httpbuff_response->AddKey(key, value, desc);

    safe_snprintf(key, sizeof(key), "%s", SWAP_TOTAL_LABLE);
    safe_snprintf(value, sizeof(value), "%s", FormatMemoryDesc(mem.swap_total).c_str());
    ConvertAnsiToUTF8("交换空间总大小",
                      strlen("交换空间总大小"),
                      desc,
                      sizeof(desc),
                      &out_len);
    desc[out_len] = 0;
    httpbuff_response->AddKey(key, value, desc);

    safe_snprintf(key, sizeof(key), "%s", SWAP_FREE_LABLE);
    safe_snprintf(value, sizeof(value), "%s", FormatMemoryDesc(mem.swap_free).c_str());
    ConvertAnsiToUTF8("空闲交换空间的大小",
                      strlen("空闲交换空间的大小"),
                      desc,
                      sizeof(desc),
                      &out_len);
    desc[out_len] = 0;
    httpbuff_response->AddKey(key, value, desc);

    safe_snprintf(key, sizeof(key), "%s", SWAP_CACHE_LABLE);
    safe_snprintf(value, sizeof(value), "%s", FormatMemoryDesc(mem.swap_cached).c_str());
    ConvertAnsiToUTF8("高速缓冲存储器",
                      strlen("高速缓冲存储器"),
                      desc,
                      sizeof(desc),
                      &out_len);
    desc[out_len] = 0;
    httpbuff_response->AddKey(key, value, desc);

    httpbuff_response->EndGroup("SystemMemory");

    // 进程及线程内存信息
    std::vector<ProcessMemStatus> process_mem_status;
    b = m_proc_parse.ParseProcessMemoryStatus(&process_mem_status);

    if (!b) {
        return true;
    }

    for (uint32_t i = 0; i < process_mem_status.size(); ++i) {
        if (i == 0) {
            httpbuff_response->BeginGroup("ProcessMemory");
            safe_snprintf(key, sizeof(key), "Pid");

        } else {
            httpbuff_response->BeginGroup("ThreadMemory");
            safe_snprintf(key, sizeof(key), "ThreadId");
        }

        safe_snprintf(value, sizeof(value), "%d", process_mem_status[i].pid);
        httpbuff_response->AddKey(key, value);

        safe_snprintf(key, sizeof(key), "%s", FD_SIZE_LABLE);
        safe_snprintf(value, sizeof(value), "%d", process_mem_status[i].fd_size);
        httpbuff_response->AddKey(key, value, "");

        safe_snprintf(key, sizeof(key), "%s", VM_SIZE_LABLE);
        safe_snprintf(value, sizeof(value), "%s",
                      FormatMemoryDesc(process_mem_status[i].vm_size).c_str());

        ConvertAnsiToUTF8("进程虚拟空间的大小",
                          strlen("进程虚拟空间的大小"),
                          desc, sizeof(desc), &out_len);
        desc[out_len] = 0;
        httpbuff_response->AddKey(key, value, desc);

        safe_snprintf(key, sizeof(key), "%s", VM_RSS_LABLE);
        safe_snprintf(value, sizeof(value), "%s",
                      FormatMemoryDesc(process_mem_status[i].vm_rss).c_str());

        ConvertAnsiToUTF8("进程物理内存的大小",
                          strlen("进程物理内存的大小"),
                          desc, sizeof(desc), &out_len);
        desc[out_len] = 0;
        httpbuff_response->AddKey(key, value, desc);

        safe_snprintf(key, sizeof(key), "%s", VM_DATA_LABLE);
        safe_snprintf(value, sizeof(value), "%s",
                      FormatMemoryDesc(process_mem_status[i].vm_data).c_str());

        ConvertAnsiToUTF8("程序数据段的大小", strlen("程序数据段的大小"),
                          desc, sizeof(desc), &out_len);
        desc[out_len] = 0;
        httpbuff_response->AddKey(key, value, desc);

        safe_snprintf(key, sizeof(key), "%s", VM_LIB_LABLE);
        safe_snprintf(value, sizeof(value), "%s",
                      FormatMemoryDesc(process_mem_status[i].vm_lib).c_str());

        ConvertAnsiToUTF8("被映像到任务的虚拟内存空间的库的大小",
                          strlen("被映像到任务的虚拟内存空间的库的大小"),
                          desc, sizeof(desc), &out_len);
        desc[out_len] = 0;
        httpbuff_response->AddKey(key, value, desc);

        if (i == 0) {
            httpbuff_response->EndGroup("ProcessMemory");

        } else {
            httpbuff_response->EndGroup("ThreadMemory");
        }
    }

    return true;
}

bool  CBaseHttpProcThread::GetDISKInfo(CHttpBuff* httpbuff_response) {
    const char* disk = m_proc_parse.ParseDiskStatus();

    if (disk && *disk  != 0) {
        int32_t max_len = 1024 + strlen(disk) * 6;
        char* buf = new char[max_len];
        char* ptr = buf;

        while (*disk && max_len > 0) {
            if ( *disk == ' ') {
                int32_t len = safe_snprintf(ptr, max_len, "&nbsp;");
                ptr += len;
                max_len -= len;

            } else if ( *disk == '\n' ) {
                int32_t len = safe_snprintf(ptr, max_len, "<br>");
                ptr += len;
                max_len -= len;

            } else {
                *ptr++ = *disk;
                max_len--;
            }

            disk++;
        }

        *ptr = 0;


        httpbuff_response->AddKey("", buf);
        delete [] buf;

    } else {
        httpbuff_response->AddKey("Desc", "ParseDiskStatus Permission denied");
    }

    return true;
}

bool  CBaseHttpProcThread::GetNETInfo(CHttpBuff* httpbuff_response) {

    std::vector<NetSocketPairInfo> socket_pair;
    std::vector<ProcessInfo> process_info;

    if(!m_proc_parse.ParseSocketPair(ENUM_PROTOCOL_TCP | ENUM_PROTOCOL_UDP, &socket_pair)) {
        httpbuff_response->AddKey("Desc",
                                  "ParseNETStatus Permission denied");
        return false;
    }

    m_proc_parse.ParseProcessInfo(&process_info);

    httpbuff_response->AddKey("",
                              "Proto&nbsp;Recv-Q&nbsp;Send-Q&nbsp;Local&nbsp;Address \
                              &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
                              "Foreign&nbsp;Address &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
                              "&nbsp;&nbsp;&nbsp;State&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
                              "&nbsp;PID/Program&nbsp;name\n");

    char buf_tmp[256];
    CStrBuff key;

    for (size_t i = 0; i < socket_pair.size(); ++i) {
        
        uint32_t len = 0;
        key.AppendStr(m_proc_parse.GetProtocol(socket_pair[i].protocol - 1));
        key.AppendStr("");

        len = safe_snprintf(buf_tmp, sizeof(buf_tmp), "%u", socket_pair[i].rx_queue);

        for (uint32_t k  = 0; len < 9 && k < 9 - len; k++) {
            key.AppendStr("&nbsp;");
        }

        key.AppendStr(buf_tmp);

        len = safe_snprintf(buf_tmp, sizeof(buf_tmp), "%u", socket_pair[i].tx_queue);

        for (uint32_t k  = 0; len < 9 && k < 9 - len; k++) {
            key.AppendStr("&nbsp;");
        }

        key.AppendStr(buf_tmp);

        len = safe_snprintf(buf_tmp, sizeof(buf_tmp), "%s:%d",
                            inet_ntoa(*(struct in_addr*)&socket_pair[i].local_addr),
                            socket_pair[i].local_port);

        key.AppendStr("&nbsp;&nbsp;");
        key.AppendStr(buf_tmp);

        for (uint32_t k  = 0; len < 25 && k < 25 - len; k++) {
            key.AppendStr("&nbsp;");
        }

        len = safe_snprintf(buf_tmp, sizeof(buf_tmp), "%s:%d",
                            inet_ntoa(*(struct in_addr*)&socket_pair[i].remote_addr),
                            socket_pair[i].remote_port);

        key.AppendStr(buf_tmp);

        for (uint32_t k  = 0; len < 25 && k < 25 - len; k++) {
            key.AppendStr("&nbsp;");
        }

        len = safe_snprintf(buf_tmp, sizeof(buf_tmp), "%s",
                            m_proc_parse.GetNetDesc(socket_pair[i].status - 1));

        key.AppendStr(buf_tmp);

        for (uint32_t k  = 0; len < 13 && k < 13 - len; k++) {
            key.AppendStr("&nbsp;");
        }

        int32_t process_index = -1;

        // 定位该socket_pair属于哪个进程
        for (size_t j = 0; j < process_info.size(); ++j) {
            if ( process_info[j].inode == socket_pair[i].inode ) {
                process_index = static_cast<int32_t>(j);
                break;
            }
        }


        if (process_index > 0)
            safe_snprintf(buf_tmp, sizeof(buf_tmp), "%d/%s",
                          process_info[process_index].pid, process_info[process_index].name);

        else {
            safe_snprintf(buf_tmp , sizeof(buf_tmp), "-");
        }

        key.AppendStr(buf_tmp);
        key.AppendStr("<br>");
    }

    httpbuff_response->AddKey("", key.GetString());

    return true;
}

bool CBaseHttpProcThread::OnUserHTTPRequest(const BufferV* buff_recevied,
        CHttpBuff* httpbuff_response) {
    VLOG(3) << "Received user http request:GET " << buff_recevied->buff << ", "
            "httpbuff_response=0x" << reinterpret_cast<void*>(httpbuff_response);
    return true;
}

int32_t PrePareHttpHead(char* buff, int32_t buff_len,
                        uint32_t html_body_len, const char* content_type) {
    if (!buff || buff_len <= 0 || !content_type) {
        return 0;
    }

    int32_t bytes_count = 0;
    bytes_count += safe_snprintf(buff + bytes_count, buff_len - bytes_count,
                                 "%s", "HTTP/1.1 200 OK\r\n");

    bytes_count += safe_snprintf(buff + bytes_count, buff_len - bytes_count,
                                 "%s", "Server: XFS robot/1.0 beta (Win32/linux)\r\n");

    bytes_count += safe_snprintf(buff + bytes_count, buff_len - bytes_count,
                                 "%s", "Accept-Ranges: bytes\r\n");

    bytes_count += safe_snprintf(buff + bytes_count, buff_len - bytes_count,
                                 "%s", "Connection: close\r\n");

    char buf_time[128] = {0};
    time_t t = time(0);
    safe_ctime(&t, sizeof(buf_time), buf_time);

    if (buf_time[strlen(buf_time)-1] == '\r' ||
            buf_time[strlen(buf_time)-1] == '\n') {
        buf_time[strlen(buf_time)-1] = 0;
    }

    if (buf_time[strlen(buf_time)-1] == '\r' ||
            buf_time[strlen(buf_time)-1] == '\n') {
        buf_time[strlen(buf_time)-1] = 0;
    }

    bytes_count += safe_snprintf(buff + bytes_count, buff_len - bytes_count,
                                 "Date: %s\r\n", buf_time);
    bytes_count += safe_snprintf(buff + bytes_count, buff_len - bytes_count,
                                 "Last-Modified: %s\r\n", buf_time);
    bytes_count += safe_snprintf(buff + bytes_count, buff_len - bytes_count,
                                 "Content-Length: %d\r\n", html_body_len);

    //
    // 后面是body, 修改这里代码的时候这一行代码放到最后
    //
    bytes_count += safe_snprintf(buff + bytes_count, buff_len - bytes_count,
                                 "Content-Type: %s\r\n\r\n", content_type);

    return bytes_count;
}

_END_XFS_BASE_NAMESPACE_
