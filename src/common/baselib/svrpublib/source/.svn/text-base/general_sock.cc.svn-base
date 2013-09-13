//
//  general_sock.cc
//  wookin
// ///////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/base_config.h"
#include "thirdparty/glog/logging.h"

#include <map>

// linux
#ifndef WIN32
#include <poll.h>
#endif

DECLARE_USING_LOG_LEVEL_NAMESPACE


_START_XFS_BASE_NAMESPACE_

// max package length of per. routine
static const int32_t kMaxPackagePerRoutine = 32*1024*1024;

static const uint32_t kMaxSubStringLength = 64 * 1024 * 1024;

// socket pair start port
const uint16_t kSocketPairStartPort = 36000;

#ifdef WIN32
#pragma   warning(disable:4127)
#endif // WIN32


//
// 将用户创建的socket放入到队列,释放的时候清空,_DEBUG有效
// 用于帮助用户debug socket handle泄漏
// 用户必须使用NewSocket,AcceptNewConnection,CloseSocket配对使用
// 使用ListSocketsInUse()来显示还有哪些handle还在使用中
//
std::map<SOCKET, std::string> g_debug_inuse_sockets;

void ListSocketsInUse();
void InsertSocketInfoToInUsePool(SOCKET sock, bool is_tcp, const char* file, int32_t line);
void EraseSocketFromInUsePool(SOCKET sock);


// ///////////////////////////
// Class AutoSocketStartup
// ///////////////////////////
AutoSocketStartup::AutoSocketStartup() {
#ifdef WIN32
    bool is_ok = false;
    WSADATA wsa_data = {0};
    WORD version_requested = MAKEWORD(2, 2);
    int32_t num_err = WSAStartup(version_requested, &wsa_data);
    if ( num_err != 0 ) {
        is_ok = false;
    } else {
        if (LOBYTE(wsa_data.wVersion) != 2 ||
                HIBYTE(wsa_data.wVersion) != 2 ) {
            WSACleanup();
            is_ok = false;
        } else {
            is_ok = true;
        }
    }

    if (!is_ok) {
        printf("WSAStartup fail.\r\n");
    }
#else // linux
    // ...
#endif // linux
}

AutoSocketStartup::~AutoSocketStartup() {
#ifdef WIN32
    WSACleanup();
#else // linux
    // ...
#endif // linux
}

// ///////////////////////////
// End of CXSocketLibAutoManage
// ///////////////////////////

bool XSetSocketBlockingMode(SOCKET sock, bool is_blocking) {
#ifdef WIN32
    uint32_t ul;
    if (is_blocking)  // Disable the nonblocking mode of sockets [blocking]
        ul = 0;
    else              // Enable the nonblocking mode of sockets  [nonblocking]
        ul = 1;

    if (ioctlsocket(sock, FIONBIO, reinterpret_cast<u_long*>(&ul)) == -1)
        return false;
    else
        return true;
#else
    int32_t old_flag = fcntl(sock, F_GETFL, 0);
    if (!is_blocking)
        old_flag |= O_NONBLOCK;   // add nonblock mask
    else
        old_flag &= ~O_NONBLOCK; // remove nonblock mask
    fcntl(sock, F_SETFL, old_flag);
    return true;
#endif // linux
}

bool XSetSocketReuseAddress(SOCKET sock, bool is_reuse) {
    int32_t option = is_reuse;
    return setsockopt(sock,
                      SOL_SOCKET,
                      SO_REUSEADDR,
                      (const char*)&option,
                      sizeof(option)) == 0 ? true:false;
}

bool    XSetSocketLinger(SOCKET sock, bool is_linger_on, uint16_t delay_secs) {
    struct linger lin = {0};
    lin.l_onoff = (uint16_t)is_linger_on;
    lin.l_linger = delay_secs;
    return setsockopt(sock,
                      SOL_SOCKET,
                      SO_LINGER,
                      (const char*)&lin,
                      sizeof(lin)) == 0 ? true:false;
}

bool XSetSocketReceiveBuffSize(SOCKET sock, int32_t size) {
    return setsockopt(sock,
                      SOL_SOCKET,
                      SO_RCVBUF,
                      (const char*)&size, sizeof(size)) == 0 ? true:false;
}

bool XSetSocketSendBuffSize(SOCKET sock, int32_t size) {
    return setsockopt(sock,
                      SOL_SOCKET,
                      SO_SNDBUF,
                      (const char*)&size, sizeof(size)) == 0 ? true:false;
}

bool XSetSocketNoDelay(SOCKET sock) {
    int32_t no_delay = 1;
    return setsockopt(sock,
                      IPPROTO_TCP,
                      TCP_NODELAY,
                      (const char*)&no_delay,
                      sizeof(no_delay)) == 0 ? true:false;
}

//
// linux下对于listen的socket handle, 启用这个选项可以在TCP三次握手过程中,
// server一方不用等到client发送来ACK就向App发出已经收到socket,
// 提前进入receive状态
//
bool XSetSocketAcceptFilter(SOCKET sock) {
#ifdef WIN32
    return (sock != INVALID_SOCKET) ? true:false;
#else
    int32_t defer_accept = 1;
    return setsockopt(sock,
                      IPPROTO_TCP,
                      TCP_DEFER_ACCEPT,
                      (const char*)&defer_accept,
                      sizeof(defer_accept)) == 0 ? true:false;
#endif
}

//
// linux 下对于client的socket handle, 启用这个选项可以在TCP三次握手过程中,
// client发送ACK数据给server的时候可以在后面附带上数据包体
//
bool XSetSocketQuickAck(SOCKET sock) {
#ifdef WIN32
    return (sock != INVALID_SOCKET) ? true:false;
#else
    int32_t quick_ack = 1;
    return setsockopt(sock,
                      IPPROTO_TCP,
                      TCP_QUICKACK,
                      (const char*)&quick_ack,
                      sizeof(quick_ack)) == 0 ? true:false;
#endif //
}

bool XSetSocketTCPKeepAlive(SOCKET sock, int32_t interval) {
#ifdef WIN32
    bool b = false;
    //  定义结构及宏
    struct   TCP_KEEPALIVE {
        uint32_t onoff;
        uint32_t keepalivetime;
        uint32_t keepaliveinterval;
    };
#define   SIO_KEEPALIVE_VALS       _WSAIOW(IOC_VENDOR, 4)

    //
    // KeepAlive实现
    //
    TCP_KEEPALIVE   in_keep_alive = {0};   // 输入参数
    uint32_t        in_len = sizeof(TCP_KEEPALIVE);
    TCP_KEEPALIVE   out_keep_alive = {0};   // 输出参数
    uint32_t        out_len = sizeof(TCP_KEEPALIVE);
    uint32_t        bytes_return = 0;

    //
    //  设置socket的keep alive为5秒, 并且发送次数为3次
    //
    in_keep_alive.onoff = 1;

    //  两次KeepAlive探测间的时间间隔
    in_keep_alive.keepaliveinterval = interval*1000;

    //  开始首次KeepAlive探测前的TCP空闭时间
    in_keep_alive.keepalivetime = 5000;

    if (WSAIoctl((uint32_t)sock, SIO_KEEPALIVE_VALS,
                 (LPVOID)&in_keep_alive, in_len,
                 (LPVOID)&out_keep_alive, out_len,
                 (unsigned long*)&bytes_return, NULL, NULL) == SOCKET_ERROR) {
        b = false;
        LOG(ERROR) << "XSetSocketTCPKeepAlive(), "
                   "WSAIoctl failed.error code(" << WSAGetLastError() << ")!";
    } else {
        b = true;
    }
    return b;
#else // linux
    int32_t    keep_alive   =   1;        // 设定KeepAlive
    int32_t    keep_idle    =   5;        // 开始首次KeepAlive探测前的TCP空闭时间
    int32_t    keep_interval=   interval; // 两次KeepAlive探测间的时间间隔
    int32_t    keep_count   =   3;        // 判定断开前的KeepAlive探测次数

    if (setsockopt(sock,
                   SOL_SOCKET,
                   SO_KEEPALIVE,
                   reinterpret_cast<void*>(&keep_alive),
                   sizeof(keep_alive)) == -1) {
        LOG(ERROR) << "XSetSocketTCPKeepAlive(), "
                   "setsockopt SO_KEEPALIVE error:" << errno << ":" << strerror(errno);
        return false;
    }

    if (setsockopt(sock,
                   SOL_TCP,
                   TCP_KEEPIDLE,
                   reinterpret_cast<void*>(&keep_idle),
                   sizeof(keep_idle)) == -1) {
        LOG(ERROR) << "XSetSocketTCPKeepAlive(), "
                   "setsockopt TCP_KEEPIDLE error:" << errno << ":" << strerror(errno);
        return false;
    }

    if (setsockopt(sock,
                   SOL_TCP,
                   TCP_KEEPINTVL,
                   reinterpret_cast<void*>(&keep_interval),
                   sizeof(keep_interval)) == -1) {
        LOG(ERROR) << "XSetSocketTCPKeepAlive(), "
                   "setsockopt TCP_KEEPINTVL error:" << errno << ":" << strerror(errno);
        return false;
    }

    if (setsockopt(sock,
                   SOL_TCP,
                   TCP_KEEPCNT,
                   reinterpret_cast<void*>(&keep_count),
                   sizeof(keep_count)) == -1) {
        LOG(ERROR) << "XSetSocketTCPKeepAlive(), "
                   "setsockopt TCP_KEEPCNT error:" << errno << ":" << strerror(errno);
        return false;
    }
    return true;
#endif //
}

int32_t XGetSocketError(SOCKET sock) {
    if(sock != INVALID_SOCKET)
        return -1;
    // try get error number
    int32_t so_err = -1;
    socklen_t err_len = sizeof(so_err);
    getsockopt(sock, SOL_SOCKET, SO_ERROR,
               reinterpret_cast<char *>(&so_err), &err_len);
    return so_err;
}


//
// ToDo(wookin): 这里是支持可以传入域名或者ip都可以,
//               在R2统一所有glibc版本之前先不支持这个(静态编译警告)
//
// donniechen : 现在放开，支持域名解析
bool GetHostByName(const char* name, char* ip_addr,
                   int32_t ip_addr_buff_len)
{
    bool b = false;
    if (!name || !ip_addr)
        return b;

    struct hostent* hptr = 0;
    if ((hptr = gethostbyname(name)) == NULL){
        LOG(ERROR) << "gethostbyname error for host:" << name;
        return b;
    }

    if (hptr->h_addr_list[0]){
        struct in_addr s;
        s = *(reinterpret_cast<in_addr*>(hptr->h_addr_list[0]));
        int32_t n = safe_snprintf(ip_addr, ip_addr_buff_len, "%s",
                                 inet_ntoa(s));
        ip_addr[n] = 0;
        b = true;
    }else{
        memset(ip_addr, 0, ip_addr_buff_len);
        safe_snprintf(ip_addr, sizeof(ip_addr), "%s", "127.0.0.1");
    }

    return b;
}

//
// return > 0 ok
//        = 0 timeout
//        < 0 fail
//
int32_t FD_Readable(SOCKET fd,
                    int32_t millisecs_timeout,
                    bool test_connection,     // 测试是否网络中断
                    bool silence_when_timeout // 超时的时候不输出日志
                   ) {
    if (fd == INVALID_SOCKET)
        return -1;

#ifdef WIN32
    fd_set rfds;
    fd_set efds;
    FD_ZERO(&rfds);
    FD_ZERO(&efds);
    FD_SET(fd, &rfds);
    FD_SET(fd, &efds);
    struct timeval t = {0};
    t.tv_sec = millisecs_timeout / 1000;
    t.tv_usec = (millisecs_timeout % 1000) * 1000;

    int32_t num_fds = select(fd + 1, &rfds, 0, &efds, &t);
    if (num_fds == 0) {
        // time out
        if(!silence_when_timeout) {
            LOG(WARNING) << "try detect readable of sock:" << fd <<
                         ", time out = " << millisecs_timeout << " millisecs";
        }
    } else if (num_fds < 0) {
        LOG(ERROR) << "***ERROR, Level 3***, "
                   "try detect readable of fd:" << fd <<
                   ", error:" << errno << ":" << strerror(errno)
                   << " timeout = " << millisecs_timeout << " millisecs";
    } else {
        if (FD_ISSET(fd, &efds)) {
            // Maybe error
            LOG(ERROR) << "***ERROR, Level 3***, try detect readable, "
                       "select maybe error, nfds = " << num_fds <<", errno:"
                       << errno << ":" << strerror(errno);

            num_fds = -1; // return error
        } else {
            // 有可读信号, 测试是否对方关闭
            if (test_connection) {
                char tmp_ch = 0;
                int32_t bytes = recv(fd, &tmp_ch, 1, MSG_PEEK);
                if (bytes == 0 || bytes == -1) {
                    LOG(ERROR) << "***ERROR, Level 3***, "
                               "in FD_Readable, maybe remote closed.";
                    num_fds = -1; // return error
                }
            }
        }
    }

    return num_fds;

#else // linux
    pollfd pfds = {0};
    pfds.fd = fd;
    pfds.events = POLLIN | POLLERR | POLLHUP;
    int32_t ret = poll(&pfds, 1, millisecs_timeout);
    if (ret >= 1) {
        // error
        if ((pfds.revents & POLLERR) == POLLERR || (pfds.revents & POLLHUP) == POLLHUP) {
            // 显示为警告,这个函数的上层调用者会显示失败,减少重复显示
            // 确定返回失败
            LOG(WARNING) << "Detect socket readable, result: FAIL, sock: " << fd
                         << ", error no:" << errno << ", " << strerror(errno);
            ret = -1;
        }

        if (ret == 1 && (pfds.revents & POLLIN)) {
            // 有可读信号, 测试是否对方关闭
            if (test_connection) {
                char tmp_ch = 0;
                int32_t bytes = recv(fd, &tmp_ch, 1, MSG_PEEK);
                // return error
                if (bytes == 0 || bytes == -1) {
                    LOG(ERROR) << "in FD_Readable, maybe remote closed.";
                    ret = -1;
                }
            }
        }
    }
    return ret;
#endif // linux
}

//
// return > 0 ok
//        = 0 timeout
//        < 0 fail
//
int32_t FD_Writeable(SOCKET fd,
                     int32_t millisecs_timeout,
                     bool test_connection, // 测试是否网络中断
                     bool* readable,
                     bool* maybe_fail) {
    if (fd == INVALID_SOCKET)
        return -1;

#ifdef WIN32

    fd_set rfds;
    fd_set wfds;
    fd_set efds;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);
    FD_SET(fd, &rfds);
    FD_SET(fd, &wfds);
    FD_SET(fd, &efds);
    struct timeval t = {0};
    t.tv_sec = millisecs_timeout / 1000;
    t.tv_usec = (millisecs_timeout % 1000) * 1000;

    bool maybe_is_fail = false;
    bool writeable = false;
    int32_t num_fds = select(fd + 1,
                             test_connection ? &rfds : NULL,
                             &wfds, &efds,
                             &t);
    if (num_fds == 0) {
        // time out
        LOG(WARNING) << "Detect socket writeable fail, select time out,"
                     " sock:" << fd << ", timeout = " << millisecs_timeout << " millisecs";
    } else if (num_fds < 0) {
        // error
        LOG(ERROR) << "Detect socket writeable, select fail, "
                   "sock:" << fd << ", error:" << errno << ":" << strerror(errno) <<
                   ", timeout = " << millisecs_timeout <<" millisecs";
    } else {
        if (FD_ISSET(fd, &wfds)) {
            // ? writeable
            writeable = true;

            // ToDo(wookin): 目前linux内核好像有点问题,暂时对这个判断做警告处理
            // 下面的一句一定要，主要针对防火墙,进一步确认是否是否连接成功

            // confirm writeable?
            int32_t so_err = -1;
            socklen_t err_len = sizeof(so_err);
            if (getsockopt(fd, SOL_SOCKET, SO_ERROR,
                           reinterpret_cast<char *>(&so_err), &err_len) < 0) {
                // Fake writeable message; connect to downstream fail
                // Get socket option fail
                writeable = false;
                LOG(ERROR) << "Detect socket writeable fail, getsockopt(SO_ERROR..) fail,"
                           " errno:" << errno << ":" << strerror(errno);
            } else {
                if (so_err != 0) {
                    // Fake writeable message;connect to downstream fail

                    // ToDo(wookin) :内核有点问题,
                    //               出现这种情况不判断为连接失败,继续
                    // writeable = false;
                    maybe_is_fail = true;
                    LOG(WARNING) << "Detect socket writeable fail, (getsockopt) result: "
                                 "SO_ERROR err:" << so_err << ":" << strerror(so_err) <<
                                 ", erro:" << errno << ":" << strerror(errno);
#ifndef WIN32
                    const char* err_msg = "";
                    if (so_err == ETIMEDOUT)
                        err_msg = "connected timeout";
                    else if (so_err == ECONNREFUSED)
                        err_msg = "No one listening on the remote address.";
                    LOG(WARNING) << "Detect socket writeable fail, " << err_msg;
#endif // !WIN32
                }
            }
        }

        if (FD_ISSET(fd, &efds)) {
            // maybe error
            writeable = false;
            VLOG(3) << "Detect socket writeable, select error, "
                      "nfds = " << num_fds << ", FD_ISSET(sock=" << fd << ", efds) = true, "
                      "error:" << errno << ":" << strerror(errno);
        }

        if(test_connection && FD_ISSET(fd, &rfds)) {
            if(readable) *readable = true;

            // 有可读信号, 测试是否对方关闭
            char tmp_ch = 0;
            int32_t bytes = recv(fd, &tmp_ch, 1, MSG_PEEK);
            if (bytes == 0 || bytes == -1) {
                LOG(ERROR) << "maybe remote closed.";
                writeable = false;
                if(readable) *readable = false;
            }
        }
    }

    if(maybe_fail)
        *maybe_fail = maybe_is_fail;

    if(num_fds == 0)
        return 0; // timeout
    return writeable ? 1 : -1;

#else // linux
    pollfd pfds = {0};
    pfds.fd = fd;
    if(test_connection)
        pfds.events = POLLIN | POLLOUT | POLLERR | POLLHUP;
    else
        pfds.events = POLLOUT | POLLERR | POLLHUP;

    int32_t ret = poll(&pfds, 1, millisecs_timeout);

    if(ret >= 1){
        // error
        if((pfds.revents & POLLERR) == POLLERR || (pfds.revents & POLLHUP) == POLLHUP) {
            if(maybe_fail)
                *maybe_fail = true;

            // 显示为警告,这个函数的上层调用者会显示失败,减少重复显示
            LOG(WARNING) << "Detect socket writeable, result: FAIL, sock: " << fd
                         << ", error no:" << errno << ", " << strerror(errno);

            // 确定返回失败
            ret = -1;
        }

        if(test_connection && (pfds.revents & POLLIN) == POLLIN) {
            if(readable) *readable = true;

            // 有可读信号, 测试是否对方关闭
            char tmp_ch = 0;
            int32_t bytes = recv(fd, &tmp_ch, 1, MSG_PEEK);
            if (bytes == 0 || bytes == -1) {
                LOG(ERROR) << "maybe remote closed, sock = " << fd;
                if(readable) *readable = false;
                return -1;
            }
        }
    }
    else if (0 == ret) {
        LOG(WARNING) << "poll ret 0, timeout "
                     << millisecs_timeout
                     << "ms";
    }
    else {
        LOG(ERROR) << "poll ret err, error info: " << strerror(errno);
    }
    return ret;
#endif// linux
}

// 判断connect之后连接是否可写
bool ConnWriteable(SOCKET sock,
                   uint32_t timeout_secs,
                   struct sockaddr_in* debug_to_addr) {
    bool maybe_fail = false;
    int32_t ret = FD_Writeable(sock, timeout_secs * 1000, false, NULL, &maybe_fail);

    // 连接失败或者有可能失败,显示连接信息
    char connect_to[64] = {0};
    safe_snprintf(connect_to, sizeof(connect_to),
                  "Try connect to host: %s:%d",
                  debug_to_addr ? inet_ntoa(debug_to_addr->sin_addr) : "*NULL addr*",
                  debug_to_addr ? N2HS(debug_to_addr->sin_port) : 0);
    if (ret <= 0) {
        LOG(ERROR) << connect_to << ", result: fail, sock:" << sock <<
                   ", timeout = " << timeout_secs * 1000 << "ms";
    } else {
        if (maybe_fail) {
            LOG(WARNING) << connect_to << ", result: OK (maybe fail), "
                         "sock:" << sock << ", timeout = " <<
                         timeout_secs * 1000 << " millisecs";
        } else {
            VLOG(3) << connect_to << ", result: OK, sock:" << sock <<
                      ", timeout = " << timeout_secs * 1000 << " millisecs";
        }
    }
    return ret > 0;
}

// 判断接收到的connect之后连接是否可读
bool ConnReadableWithTimeout(SOCKET sock, uint32_t timeout_millisecs, bool silence_when_timeout){
    int32_t ret = FD_Readable(sock, timeout_millisecs, false, silence_when_timeout);
    if(ret == 0 && !silence_when_timeout) {
        // Time out
        LOG(WARNING) << "try detect readable of fd:" << sock <<
                     ", time out, millisec = " << timeout_millisecs;
    }

    return ret > 0;
}

bool ConnReadable(SOCKET sock,
                  uint32_t timeout_secs,
                  bool silence_when_timeout) {
    return ConnReadableWithTimeout(sock, timeout_secs * 1000, silence_when_timeout);
}

bool ConnectToHost(SOCKET sock, uint32_t netorder_host,
                   uint16_t netorder_port, uint32_t timeout_secs) {
    bool b = false;
    if (sock == INVALID_SOCKET)
        return b;

    struct sockaddr_in toaddr;
    memset(&toaddr, 0, sizeof(toaddr));
    toaddr.sin_family = AF_INET;
    toaddr.sin_addr.s_addr = netorder_host;
    toaddr.sin_port = netorder_port;

    // Set nonblocking
    XSetSocketBlockingMode(sock, false);
    connect(sock, reinterpret_cast<struct sockaddr*>(&toaddr), sizeof(toaddr));
    b = ConnWriteable(sock, timeout_secs, &toaddr);

    return b;
}

bool SendFixedBytes(SOCKET sock,
                    const char* buff,
                    int32_t fixed_len,
                    uint32_t* sent,
                    bool break_readable // 当有数据可读时候中断发送
                    ) {
    return SendFixedBytesInTime(sock, buff, fixed_len, 0, sent, break_readable);
}

bool SendFixedBytesInTime(SOCKET sock,
                    const char* buff,
                    int32_t fixed_len,
                    int32_t timeout,
                    uint32_t* sent,
                    bool break_readable // 当有数据可读时候中断发送
                   ) {
    bool b = false;
    if (sock != INVALID_SOCKET && buff && fixed_len) {
        const char* p = buff;
        int32_t remain_len = fixed_len;
        uint32_t num_timeout = 0;
        int32_t err = 0;
        time_t t_start = time(NULL);
        time_t t_now = t_start;
        uint32_t real_timeout = (timeout == 0) ? -1 : timeout;

        while (remain_len > 0 && (t_now - t_start) <= real_timeout) {
             int32_t t_remain = 1;
            if (timeout != 0) {
                int32_t t_remain = static_cast<int32_t>(timeout)
                    - static_cast<int32_t>(t_now - t_start);
                // t_remain must > 0, fix bug of last second
                // t_remain = 0, FD_Writeable return immediately
                t_remain = (t_remain == 0) ? 1 : t_remain;
            }


            bool readable = false;
            int32_t ret = FD_Writeable(sock, t_remain * 1000, break_readable, &readable);
            if (ret < 0) {
                err = GetLastSocketError();
                VLOG(3) << "in SendFixedBytes error:" << err << ":" << strerror(err)
                        << ", time out count:" << num_timeout << " sock : " << sock;

                break;
            }

            if (ret == 0) {
                // Time out
                t_now = time(NULL);
                num_timeout++;

                err = GetLastSocketError();
                //  一般是被对方塞住了(缓冲区被塞住)
                if (num_timeout == 1 || (num_timeout % 5) == 0) {
                    VLOG(3) << "in SendFixedBytes time out, Error Code = "
                              << err << ":" << strerror(err);
                }
                if (err == EAGAIN || err == POSIX_EWOULDBLOCK)
                    continue;


                if (timeout == 0 && num_timeout % 10 == 0) {
                    LOG(ERROR) << "in SendFixedBytes fail.";
                    break;
                }
            }

            if (ret > 0) {
                if(break_readable && readable)
                    break;

                //
                //  Can send data
                //  Send 32M Bytes(Max.) per routine
                //  避免多个tcp连接发送大数据量的时候
                //  阻塞其他进程对网络的申请
                //

                //
                // TODO(wookin): xxx
                // 以后发现发送速度太慢优化这里
                //
                int32_t seg = (remain_len > kMaxPackagePerRoutine)
                               ? kMaxPackagePerRoutine : remain_len;

                int32_t now_sent = SendDat(sock, p, seg, 0);
                if (now_sent > 0) {
                    p += now_sent;
                    remain_len -= now_sent;
                } else if (now_sent == 0) {
                    //  Network busy
                    XSleep(1);
                } else if (now_sent < 0) {
                    err = GetLastSocketError();
                    VLOG(3) << "Error Code = " << err << ":" << strerror(err);
                    if (err == EAGAIN || err == POSIX_EWOULDBLOCK)
                        continue;

                    break;
                }
                t_now = time(NULL);
            }
        }

        b = remain_len == 0;
        if (sent)
            *sent = (fixed_len - remain_len);
    }
    return b;
}

bool ReceiveFixedPackage(SOCKET sock,
                         char* buff,
                         int32_t buff_len,
                         int32_t timeout,
                         int32_t* received) {

    VLOG(3) << "try ReceiveFixedPackage, timeout:" << timeout
            << " seconds, request buff_len: " << buff_len;

    if (sock != INVALID_SOCKET &&
            buff && buff_len > 0 && received) {
        time_t t_start = time(NULL);
        time_t t_now = t_start;
        *received = 0;
        while (*received < buff_len && (t_now - t_start) <= timeout) {
            int32_t t_remain = static_cast<int32_t>(timeout)
                                - static_cast<int32_t>(t_now - t_start);
            // t_remain must > 0, fix bug of last second
            // t_remain = 0, FD_Readable return immediately
            t_remain = (t_remain == 0) ? 1 : t_remain;
            int32_t ret = FD_Readable(sock, t_remain * 1000, true);

            // timeout or error, getpeername
            sockaddr_in from;
            if(ret <= 0){
                memset(&from, 0, sizeof(from));
                socklen_t addr_len = sizeof(from);
                if(getpeername(sock, reinterpret_cast<sockaddr*>(&from), &addr_len) != 0 ){
                    LOG(WARNING) << "try getpeername of sock:" << sock << " fail. err: "
                                 << errno << ":" << strerror(errno);
                }
            }

            if(ret == 0) {
                t_now = time(NULL);
                LOG(WARNING) << "try receive data timeout, from:" << inet_ntoa(from.sin_addr)
                             << ":" << N2HS(from.sin_port);
                continue; // timeout
            } else if(ret == -1) {
                LOG(WARNING) << "ReceiveFixedPackage fail, FD_Readable fail, from:"
                             << inet_ntoa(from.sin_addr) << ":" << N2HS(from.sin_port);
                break;
            }

            // readable
            //
            // max 32M Bytes per routine
            //
            int32_t seg = (buff_len - *received) > kMaxPackagePerRoutine ?
                          kMaxPackagePerRoutine : (buff_len - *received);

            int32_t n = RecvDat(sock, buff + (*received), seg, 0);
            if (n == 0) {
                LOG(WARNING) << "ReceiveFixedPackage fail, received = 0, err:"
                             << errno << "," << strerror(errno) << ",Remote closed.";
                break; // remote closed
            } else if (n > 0) {
                *received += n;
                VLOG(3) << "now received len: " << n << ", try receive len:"
                        << seg << ", total received: " << (*received);
            } else if (n < 0) { // error

                int32_t err = GetLastSocketError();
                // ? retry
                if(err == EAGAIN || err == POSIX_EWOULDBLOCK)
                    continue;

                // error
                LOG(WARNING) << "ReceiveFixedPackage fail, "
                             "received = " << n <<
                             ", err:" << errno << ":" << strerror(err) <<
                             ", maybe remote closed.";
                break;
            }
            t_now = time(NULL);
        }
    }

    VLOG(3) << "receive data finished, received len: " << *received
            << ", request buff_len: " << buff_len;

    return *received == buff_len ? true:false;
}

char* Inet_ntoa(struct in_addr in, char* buff, int32_t buff_len){
    unsigned char* ptr = reinterpret_cast<unsigned char*>(&in.s_addr);
    safe_snprintf(buff, buff_len, "%u.%u.%u.%u", ptr[0], ptr[1], ptr[2], ptr[3]);
    return buff;
}

//
// Send datagram
//
bool SendDatagram(SOCKET sock,
                  const char* pack,  int32_t pack_len,
                  const char* to_host, uint16_t to_port,
                  int32_t timeout_secs) {
    bool b = false;
    if (sock == INVALID_SOCKET || !pack || pack_len <= 0 || !to_host)
        return b;

    struct sockaddr_in to_addr;
    memset(&to_addr, 0, sizeof(to_addr));
    to_addr.sin_family = AF_INET;
    to_addr.sin_addr.s_addr = inet_addr(to_host);
    to_addr.sin_port = H2NS(to_port);

    int32_t ret = FD_Writeable(sock, timeout_secs * 1000);
    if (ret > 0) {
        // can send data
        int32_t now_sent = sendto(sock,
                                  pack,
                                  pack_len,
                                  0,
                                  reinterpret_cast<struct sockaddr*>(&to_addr),
                                  sizeof(struct sockaddr));
        if (now_sent == pack_len)
            b = true;
        else {
            LOG(ERROR) << "Send datagram fail, "
                       "sock:" << sock << ", errno:" << errno << ":" << strerror(errno);
        }
    }

    if (ret == 0) {
        // time out
        VLOG(3) << "in SendDatagram time out, Error Code = " << errno << ":" << strerror(errno);
    }

    if (ret < 0) {
#ifdef WIN32
        VLOG(3) << "in SendFixedBytes select error. "
                  "error:" << WSAGetLastError() << ", time out count:" << timeout_secs;
#else // linux
        VLOG(3) << "in SendFixedBytes select error. "
                  "error:" << errno << ":" << strerror(errno) <<
                  ", time out count:" << timeout_secs;
#endif // linux
    }

    return b;
}

// 包装send()函数,自动retry EINTR
int SendDat(SOCKET sock, const char* buff, int32_t len, int32_t flags){
    if(sock == INVALID_SOCKET)
        return -1;

    for (;;){
        int32_t sent = (int32_t) ::send(sock, buff, len, flags);
        if (sent > 0)
            return sent;

        int32_t err = GetLastSocketError();
        if(err == EAGAIN || err == POSIX_EWOULDBLOCK)
            return 0;
        else if(err != EINTR) // EINTR 可以继续
            return -1;
    }
}

// 封装recv()函数,自动retry EINTR
// return: >0 , OK
//         0  , remote closed
//         <0 , fail
int32_t RecvDat(SOCKET sock, char* buff, int32_t len, int32_t flags){
    if(sock == INVALID_SOCKET)
        return -1;

    int32_t received = -1;
    for(;;){
        received = ::recv(sock, buff, len, flags);
        if(received >= 0)
            return received;

        if(received == -1){
            int32_t err = GetLastSocketError();
            if(err != EINTR)
                return -1;
        }
    }

    return  received;
}

bool ReceiveDatagram(SOCKET sock,
                     char* buff,
                     int32_t buff_len,
                     int32_t* received_len,
                     struct sockaddr_in* from_addr,
                     int32_t timeout_secs) {
    bool b = false;
    if (sock == INVALID_SOCKET || !buff || buff_len <= 0)
        return b;

    struct sockaddr* ptr_addr = (struct sockaddr*)from_addr;
    struct sockaddr addr;
    if (!ptr_addr)
        ptr_addr = &addr;
    socklen_t l = sizeof(addr);

    int32_t ret = FD_Readable(sock, timeout_secs * 1000);
    if (ret > 0) {
        //  Can send data
        int32_t bytes = recvfrom(sock,
                                 buff, buff_len, 0,
                                 reinterpret_cast<struct sockaddr*>(ptr_addr),
                                 &l);
        if (bytes > 0) {
            b = true;
            if (received_len)
                *received_len = bytes;
        } else {
            LOG(ERROR) << "Receive datagram fail, sock:" << sock << ", errno:"
                       << errno << "," << strerror(errno);
        }
    }

    if (ret == 0) {
        //  Time out
        VLOG(3) << "in ReceiveDatagram time out, "
                  "Error Code = " << errno << ", " << strerror(errno);
    }

    if (ret < 0) {
#ifdef WIN32
        VLOG(3) << "in SendFixedBytes select error."
                  " error:" << WSAGetLastError() << ", time out count:" << timeout_secs;
#else // linux
        VLOG(3) << "in SendFixedBytes select error."
                  " error:" << errno << "," << strerror(errno) <<
                  ", time out count:" << timeout_secs;
#endif // linux
    }

    return b;
}

int32_t _CloseSocket(SOCKET fd) {
    return _CloseSocket(fd, TCP_FD_UNKNOWN);
}

int32_t _CloseSocket(SOCKET fd, TCP_FD_TYPE type) {
    if (fd == INVALID_SOCKET)
        return -1;

#if defined(_DEBUG) || defined(_DEBUG_COUNT)
    // Get socket type
    int32_t val = 0;
    socklen_t val_len = sizeof(int32_t);
    if (getsockopt(fd, SOL_SOCKET, SO_TYPE,
                   reinterpret_cast<char*>(&val), &val_len) == 0) {
        if (val == SOCK_STREAM)
            __AddRefCloseFDCount(true, type, fd);
        else if (val == SOCK_DGRAM)
            __AddRefCloseFDCount(false, type, fd);
        else {
            VLOG(3) << "unsupport socket type in this lib.";
        }
    } else {
        VLOG(3) << "get sockopt fail, SOL_SOCKET, SO_TYPE";
    }
#else
    // nothing for release version
    if (type) {}
#endif // _DEBUG || _DEBUG_COUNT

    EraseSocketFromInUsePool(fd);

#ifdef WIN32
    return closesocket(fd);
#else
    return close(fd);
#endif
}

SOCKET NewSocketImp(const char* file, int32_t line, bool is_tcp) {
    SOCKET sock = INVALID_SOCKET;
    if (is_tcp) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock != INVALID_SOCKET){
            __AddRefNewTCPFDCount();
            VLOG(3) << "tcp, new sock " << sock;
        }
    } else {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock != INVALID_SOCKET)
            __AddRefNewUDPFDCount();
    }

    if (sock != INVALID_SOCKET) {
        // 默认非阻塞, 方便类似EPOLL_ET模式接收数据
        XSetSocketBlockingMode(sock, false);

        // 默认收发大小buffer, 16 k
        XSetSocketReceiveBuffSize(sock, 512 * 1024);
        XSetSocketSendBuffSize(sock, 512 * 1024);

        // no delay
        XSetSocketNoDelay(sock);

        // quick ACK
        // XSetSocketQuickAck(hSock);

        // linger
        // 如果调用者不是用我们的框架，
        // 需要在调用NewSocket函数之后自行调用XSetSocketLinger(sock, false, 0),
        // 关闭linger选项
        //
        // TODO(wookin): 先在这里注释掉,设置linger = 0,会快速结束TIME_WAIT
        //               在有些应用里面快速结束TIME_WAIT可能会导致新创建的socket号快速重用,
        //               出现一些意想不到的问题.
        //
        //               对于listen端口因需要保证端口快速重用,
        //               用到的NewSocket()的地方可以手动设置linger = 0
        // XSetSocketLinger(sock, true, 0);

        // tcp keep alive
        XSetSocketTCPKeepAlive(sock, 300);

        // add info to pool
        InsertSocketInfoToInUsePool(sock, is_tcp , file, line);
    } else {
        VLOG(3) << "***NewSocket fail***, maybe out of handles limit";
    }
    return sock;
}

//
// Listen on port
//
bool ListenOnPort(SOCKET sock, const char* host, uint16_t port,
                  uint32_t listen_backlog) {
    bool b = false;
    if (sock == INVALID_SOCKET || !host)
        return b;

    // Set reuse
    XSetSocketReuseAddress(sock, true);

    // Bind
    VLOG(3) << "Try bind to addr:" << host << ":" << port;
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = H2NS(port);
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        LOG(ERROR) << "Bind to address fail. Err:" << errno << ":" << strerror(errno);
        return b;
    }

    // Listen
    if (listen(sock, listen_backlog) != 0) {
        LOG(ERROR) << "Listen fail. Err:" << errno << ":" << strerror(errno);
    } else {
        VLOG(3) << "Bind to port OK. host:" << host << ":" << port;
        b = true;
    }

    return b;
}

SOCKET  AcceptNewConnectionImp(const char* file, int32_t line,
                            SOCKET listen_sock,
                            struct sockaddr_in* from_addr,
                            bool directly_accept) {
    SOCKET new_sock = INVALID_SOCKET;
    if (listen_sock == INVALID_SOCKET)
        return new_sock;

    bool can_accept_now = false;
    if (directly_accept)
        can_accept_now = true;
    else {
        // 异步检测是否有句柄可读, 防止上层的listen句柄未设置成non-blocking
        int32_t ret = FD_Readable(listen_sock, 0);
        if (ret > 0)
            // New request
            can_accept_now = true;
        else if(ret < 0) {
            // Error
            LOG(ERROR) << "Error on listen socket:"
                       "listen_sock = " << listen_sock <<
                       ", errno:" << errno << ":" << strerror(errno);
        }
    }

    // Real accept
    if (can_accept_now) {
        struct sockaddr_in st_from_addr;
        struct sockaddr* peer = 0;
        if (!from_addr) {
            peer = (struct sockaddr*)&st_from_addr;
            from_addr = &st_from_addr;
        } else
            peer = (struct sockaddr*)from_addr;
        socklen_t len = sizeof(struct sockaddr);

        new_sock = accept(listen_sock, peer, &len);
        if (new_sock == INVALID_SOCKET) {
            // 判断是否因为惊群引起
            int32_t err = GetLastSocketError();
            if(!(err == EAGAIN || err == POSIX_EWOULDBLOCK)) {
                LOG(ERROR) << "Accept fail, errno:" << err << ":" << strerror(err);
            }
        }
    }

    // Init new fd
    if (new_sock != INVALID_SOCKET) {
#if defined(_DEBUG) || defined(_DEBUG_COUNT)
        VLOG(3) << "accept new socket fd:" << new_sock <<
                  ", from:" << inet_ntoa(from_addr->sin_addr) << ":"
                  << N2HS(from_addr->sin_port);
#endif // _DEBUG || _DEBUG_COUNT

        // linux下accept不继承listen句柄属性, 需要重新设置
        // 默认非阻塞, 方便类似EPOLL_ET模式接收数据
        XSetSocketBlockingMode(new_sock, false);

        // 默认收发大小buffer, 1024 k
        XSetSocketReceiveBuffSize(new_sock, 1024*1024);
        XSetSocketSendBuffSize(new_sock, 1024*1024);

        // No delay
        XSetSocketNoDelay(new_sock);

        // Quick ACK
        // XSetSocketQuickAck(hNewSock);

        // Tcp keep alive
        XSetSocketTCPKeepAlive(new_sock, 300);

        __AddRefAcceptedSockFDCount();

        // add info to pool
        InsertSocketInfoToInUsePool(new_sock, true , file, line);
    }

    return new_sock;
}

CXThreadMutex g_debug_sock_handle_leak_mutex;

void ListSocketsInUse(){
#ifdef _DEBUG
    return;
#endif

    CXThreadAutoLock auto_lock(&g_debug_sock_handle_leak_mutex);
    // socket in use
    std::map<SOCKET, std::string>::iterator it = g_debug_inuse_sockets.begin();
    for (; it != g_debug_inuse_sockets.end(); ++it) {
        LOG(ERROR) << "Maybe socket handle leak, " << it->second;
    }
}

void InsertSocketInfoToInUsePool(SOCKET sock, bool is_tcp, const char* file, int32_t line){
#ifndef _DEBUG
    return;
#endif

    CHECK(file);

    CXThreadAutoLock auto_lock(&g_debug_sock_handle_leak_mutex);

    // 获取短文件名
    const char* name = strrchr(file, '/');
    if (!name) name = strrchr(file, '\\');
    if (name)
        name++;
    else
        name = file;

    char info[256] = {0};
    safe_snprintf(info, sizeof(info), "sock: %d, %s, at %s:%d", sock, is_tcp ? "TCP" : "UDP",
                                      name, line);
    VLOG(3) << "insert socket info to in use pool: sock:" << info;

    std::pair<std::map<SOCKET, std::string>::iterator, bool> ret;
    ret = g_debug_inuse_sockets.insert(std::pair<SOCKET, std::string>(sock, info));
    if(!ret.second){
        // maybe double insert
        // LOG(FATAL) << "maybe double insert socket info to handles pool, " << info;
        LOG(ERROR) << "maybe double insert socket info to handles pool, " << info;
    }
}

void EraseSocketFromInUsePool(SOCKET sock){
#ifndef _DEBUG
    return;
#endif

    CXThreadAutoLock auto_lock(&g_debug_sock_handle_leak_mutex);
    VLOG(3) << "try erase sock:" << sock << " from in use socket handles pool.";
    if(g_debug_inuse_sockets.erase(sock) ==0){
        // maybe double erase
        LOG(FATAL) << "maybe double erase socket info from handles pool, sock: " << sock;
        LOG(ERROR) << "maybe double erase socket info from handles pool, sock: " << sock;
    }
}

//
// function:    CreateSocketPairAutoPort
// description: 创建一对TCP连接socket fd对, 如果失败自动增大端口
//
bool CreateSocketPairAutoPort(SOCKET* fd_read,
                              SOCKET* fd_write,
                              uint32_t  netorder_host,
                              uint16_t netorder_port,
                              uint16_t* success_port) {
    bool b = false;
    uint16_t port_start = N2HS(netorder_port);
    uint16_t port = 0;
    for(port = port_start; port <= 49150; ++port){
        b = CreateSocketPair(fd_read, fd_write, netorder_host, H2NS(port));
        if (b){
            if(success_port)
                *success_port = port;
            break;
        }
    }

    if (!b) {
        struct in_addr host;
        host.s_addr = netorder_host;
        char buff[32] = {0};
        LOG(FATAL) << "Create socket pair fail, try listen on port: "
                   << Inet_ntoa(host, buff, sizeof(buff)) << ":" << port;
    }
    return b;
}

//
// 创建socket pair的时候加锁,保证端口不会被占用
// 这种调用不会很频繁,不用考虑效率降低问题
//
CXThreadMutex g_create_socket_pair_mutex;

//
// function:    CreateSocketPair
// description: 创建一对TCP连接socket fd对
//
bool CreateSocketPair(SOCKET* fd_read,
                      SOCKET* fd_write,
                      uint32_t  netorder_host,
                      uint16_t netorder_listen_port) {
    CXThreadAutoLock auto_lock(&g_create_socket_pair_mutex);

    bool b = false;
    SOCKET listen_sock = NewSocket(true);
    SOCKET client_sock = NewSocket(true);
    SOCKET server_sock = INVALID_SOCKET;
    if (listen_sock != INVALID_SOCKET && client_sock != INVALID_SOCKET) {
        // Listen address
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = netorder_host;
        addr.sin_port = netorder_listen_port;
        if (bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
            LOG(ERROR) << "Create socket pair fail, bind on port:"
                       << ntohs(netorder_listen_port) << " fail, maybe it in use.";
        } else {
            listen(listen_sock, 256);

            // Connect to address
            struct sockaddr_in toaddr;
            memset(&toaddr, 0, sizeof(toaddr));
            toaddr.sin_family = AF_INET;
            toaddr.sin_addr.s_addr = netorder_host;
            toaddr.sin_port = netorder_listen_port;
            bool writeable = false;
            if (connect(client_sock,
                        (struct sockaddr*)&toaddr,
                        sizeof(toaddr)) == 0) {
                // linux垃圾本机连接马上返回
                writeable = true;
            } else {
                writeable = ConnWriteable(client_sock, 2, &toaddr);
            }

            if (writeable) {
                struct sockaddr_in fromaddr;
                server_sock = AcceptNewConnection(listen_sock, &fromaddr);
                if (server_sock != INVALID_SOCKET) {
                    b = true;
                    VLOG(3) << "Create socket pair ok, " << inet_ntoa(addr.sin_addr) << ":" <<
                              N2HS(netorder_listen_port);
                } else {
                    b = false;
                    VLOG(3) << "Create socket pair fail, connect to server fail";
                }
            } else {
                LOG(ERROR) << "Create socket pair fail, writeable = false";
            }
        }
    }

    CloseSocket(listen_sock);
    if (!b) {
        CloseSocket(client_sock);
    } else {
        XSetSocketBlockingMode(client_sock, false);
        XSetSocketBlockingMode(server_sock, false);
        XSetSocketSendBuffSize(client_sock, (512*1024));
        XSetSocketSendBuffSize(server_sock, (512*1024));
        *fd_write = client_sock;
        *fd_read = server_sock;
    }
    return b;
}

//
// Temporary local listen host, port
//
NotifyEvent* CreateNotifyEvent(uint32_t netorder_host, uint16_t netorder_port) {
    NotifyEvent* handle = new NotifyEvent;
    if (!handle) {
        LOG(ERROR) << "out of memory.";
    } else {
        handle->netorder_host = netorder_host;
        handle->netorder_port = netorder_port;
        if (!CreateSocketPairAutoPort(&handle->sock[0], &handle->sock[1],
                              handle->netorder_host, handle->netorder_port)) {
            CloseNotifyEvent(handle);
            VLOG(3) << "create socket pair fail, on host:" <<
                      inet_ntoa(*reinterpret_cast<in_addr*>(&netorder_host)) << ":" <<
                      ntohs(netorder_port);
        }
    }

    return handle;
}


// to_peer : Which side to receive message
void SetNotify(NotifyEvent* &handle,
               NOTIFY_PEER to_peer) {
    if (handle) {
        // ? Calc: send from which side
        NOTIFY_PEER send_peer = NOTIFY_PEER_A;
        if (to_peer == NOTIFY_PEER_A)
            send_peer = NOTIFY_PEER_B;
        if (to_peer == NOTIFY_PEER_B)
            send_peer = NOTIFY_PEER_A;

        // Try send data
        if (handle->sock[send_peer] != INVALID_SOCKET) {
            if (!SendFixedBytes(handle->sock[send_peer],
                                "x", 1, 0)) {
                LOG(ERROR) << "try post notify event fail.";
                // rebuild socket pair
                handle = RecreateNotifyEvent(handle);
                if (handle) {
                    // try post notify message again
                    SendFixedBytes(handle->sock[send_peer],
                                   "x", 1, 0);
                } else {
                    LOG(ERROR) << "try RecreateNotifyEvent() fail.";
                }
            }
        }
    }
}

//
// Delete old handle and create new handle
//
NotifyEvent* RecreateNotifyEvent(NotifyEvent*& old_handle) {
    if (!old_handle)
        return NULL;

    uint32_t    netorder_host = old_handle->netorder_host;
    uint16_t    netorder_port = old_handle->netorder_port;
    CloseNotifyEvent(old_handle);
    return CreateNotifyEvent(netorder_host, netorder_port);
}

void CloseNotifyEvent(NotifyEvent*& handle) {
    if (handle) {
        CloseSocket(handle->sock[0]);
        CloseSocket(handle->sock[1]);
        delete handle;
        handle = NULL;
    }
}

// host to net float
float H2NF(float fval) {
    uint32_t val;
    memcpy(&val, &fval, sizeof(val));
    val = H2NL(val);
    float rf;
    memcpy(&rf, &val, sizeof(val));
    return rf;
}

// net to host float
float N2HF(float fval) {
    uint32_t val;
    memcpy(&val, &fval, sizeof(val));
    val = N2HL(val);
    float rf;
    memcpy(&rf, &val, sizeof(val));
    return rf;
}


CStrBuff::CStrBuff() {
    m_extend_step = 0;

    m_buff = 0;
    m_buff_len = 0;
    m_valid_len = 0;
}


void CStrBuff::SetExtendStep(uint32_t extend_step) {
    // 每次新new内存的最小大小
    m_extend_step = extend_step;
}


CStrBuff::~CStrBuff() {
    mempool_DELETE(m_buff);
}

bool CStrBuff::AppendStr(const char* sz) {
    return AppendStr(sz, STRLEN(sz));
}

bool CStrBuff::AppendStr(unsigned char ch) {
    const char* ptr = reinterpret_cast<const char*>(&ch);
    return AppendStr(ptr, 1);
}

bool CStrBuff::AppendStr(unsigned char ch, int32_t repeats) {
    CHECK(m_valid_len + static_cast<uint32_t>(repeats) < kMaxSubStringLength)
        << "CStrBuff::AppendStr should not append huge string with repeats " << repeats
        << ", current length " << m_valid_len
        << ", max substring length " << kMaxSubStringLength;

    for(int32_t i = 0; i < repeats; ++i) {
        if (!AppendStr(ch)) return false;
    }

    return true;
}

bool CStrBuff::AppendStr(const char* sz, uint32_t len) {
    if (!sz || !len)
        return false;

    CHECK(len + m_valid_len < kMaxSubStringLength)
        << "CStrBuff::AppendStr should not append huge string with length " << len
        << ", current length " << m_valid_len
        << ", max substring length " << kMaxSubStringLength;

    if (!m_buff || (len + m_valid_len) >= m_buff_len) {
        // Check buffer
        uint32_t new_buff_len = len + m_valid_len +
                                (m_extend_step >0 ? m_extend_step : 1024);
        char* p = reinterpret_cast<char*>(mempool_NEW_BYTES(new_buff_len));
        if (!p){
            LOG(FATAL) << "out of memory.";
            return false; // Maybe out of memory
        }
        m_buff_len = new_buff_len;

        // Save old data
        if (m_buff) {
            if (m_valid_len)
                memcpy(p, m_buff, m_valid_len);
            mempool_DELETE(m_buff);
        }
        m_buff = p;
    }

    // Save new data
    memcpy(m_buff + m_valid_len, sz, len);
    m_valid_len += len;
    m_buff[m_valid_len] = 0;
    CHECK(m_valid_len < m_buff_len);

    return true;
}

void CStrBuff::ChangeNLRT2Space(){
    if (m_buff && m_valid_len &&
        m_valid_len <= m_buff_len){
            for(uint32_t u = 0; u < m_valid_len; ++u){
                if(m_buff[u] == '\r' || m_buff[u] == '\n')
                    m_buff[u] = ' ';
            }
    }
}

bool BufferV::CheckNewAppendBuffer(uint32_t new_append_data_len) {
    bool b = false;

    // 不可能这样
    assert(!(max_buff_len>0 && buff == NULL));

    if (buff && (valid_len+new_append_data_len) <= max_buff_len) {
        b = true;
    } else {
        // New temp buffer
        uint32_t new_max = valid_len+new_append_data_len+m_extend_step;
        char* tmp_data = (char*)(mempool_NEW_BYTES(new_max));
        if (tmp_data) {
            // save old valid data
            if (valid_len && buff)
                memcpy(tmp_data, buff, valid_len);

            // Release old buffer
            if (buff) {
                mempool_DELETE(buff);
            }

            // Update buffer length
            buff = tmp_data;
            max_buff_len = new_max;
            b = true;
        } else {
            LOG(ERROR) << "*****ERROR, Level 5*****, CheckBuffer fail, "
                       "new buffer fail, out of memory.";
        }
    }
    return b;
}

//
// 需要保存原来的数据
//
bool BufferV::CheckBuffer(uint32_t new_total_length) {
    bool b = false;

    // 不可能这样
    assert(!(max_buff_len>0 && buff == NULL));

    if (buff && new_total_length <= max_buff_len) {
        b = true;
    } else {
        new_total_length = (new_total_length >= valid_len) ?
                           new_total_length :
                           valid_len;
        // New temp buffer
        uint32_t new_max = new_total_length + m_extend_step;
        char* tmp_data = (char*)(mempool_NEW_BYTES(new_max));
        if (tmp_data) {
            // save old valid data
            if (valid_len && buff)
                memcpy(tmp_data, buff, valid_len);

            // Release old buffer
            if (buff) {
                mempool_DELETE(buff);
            }

            // Update buffer length
            buff = tmp_data;
            max_buff_len = new_max;
            b = true;
        } else {
            LOG(ERROR) << "*****ERROR, Level 5*****, CheckBuffer fail, "
                       "new buffer fail, out of memory.";
        }
    }
    return b;
}

int32_t GetLastSocketError(){
#ifdef WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

_END_XFS_BASE_NAMESPACE_
