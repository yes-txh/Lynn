//  general_sock.h
//  wookin
// //////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_GENERAL_SOCK_H_
#define COMMON_BASELIB_SVRPUBLIB_GENERAL_SOCK_H_

#include "common/baselib/svrpublib/general_util.h"
#include "common/baselib/svrpublib/lite_mempool.h"
#include "common/baselib/svrpublib/log.h"

#include "common/baselib/svrpublib/base_config.h"

#ifdef WIN32
#pragma   warning(disable:4127)
#endif // WIN32

_START_XFS_BASE_NAMESPACE_

#ifndef N2HS
#define N2HS ntohs
#define N2HL (uint32_t)ntohl
#define H2NS (uint16_t)htons
#define H2NL (uint32_t)htonl
float H2NF(float fval);
float N2HF(float fval);
#endif // N2HS

#ifndef hton64
#define hton64(x) (((uint64_t)(htonl((uint32_t)((x) & 0xffffffff))) << 32) | \
                         htonl((uint32_t)(((x) >> 32) & 0xffffffff)))

#define ntoh64(x) (((uint64_t)(ntohl( (uint32_t)((x) & 0xffffffff))) << 32) | \
                         ntohl((uint32_t)(((x) >> 32) & 0xffffffff)))
#endif // hton64

extern const uint16_t kSocketPairStartPort;

struct XIPPort {
    char                ip[24];
    uint16_t            port;

    struct sockaddr_in    addr;
    void SetHost(char* host, uint16_t num_port) {
        memset(ip, 0, sizeof(ip));
        safe_snprintf(ip, sizeof(ip), "%s", host);
        port = num_port;
    }

    XIPPort() {
        memset(ip, 0, sizeof(ip));
        port = 0;
        memset((unsigned char*)&addr, 0, sizeof(addr));
    }

    void ToHostOrder() {
        port = N2HS(port);
        addr.sin_addr.s_addr = N2HL(addr.sin_addr.s_addr);
        addr.sin_port = N2HS(addr.sin_port);
    }

    void ToNetOrder() {
        port = H2NS(port);
        addr.sin_addr.s_addr = H2NL(addr.sin_addr.s_addr);
        addr.sin_port = H2NS(addr.sin_port);
    }
};

struct XIPPortOnly {
    uint32_t    num_ip;
    uint16_t    port;
    XIPPortOnly():num_ip(0), port(0) {
    }

    void ToNetOrder() {
        num_ip = H2NL(num_ip);
        port = H2NS(port);
    }

    void ToHostOrder() {
        num_ip = N2HL(num_ip);
        port = N2HS(port);
    }
};

#ifdef WIN32
#pragma pack(push, 1)
#else // linux
#pragma pack(1)
#endif // linux

// 要在网络上传递,需要字节对齐
struct ForwardIP {
    uint32_t            ip;             // ip, port网络字节保持一致
    uint16_t            port;
    bool                via;            // 是否经过该节点,
    // true:前面路由道路上已经出现过该节点

    bool                is_bad_node;    // true:前面有人尝试连接该节点不成功

    bool                is_last_host;   // 是否最后一个节点,
    // 不能往后连接的时候就为last one

    bool                is_first_one;   // 流水线里面的第一个节点,
    // 所有后续任务向第一个节点汇报

    uint32_t            local_id;       // 节点的LocalId
    unsigned char       seq;            // 几个备份节点中的序号
    unsigned char       reserved1;
    unsigned char       reserved2;
    ForwardIP() {
        ip = 0;
        port = 0;
        via = is_bad_node = is_last_host = is_first_one = false;
        reserved1 = reserved2 = 0;
        local_id = 0;
        seq = 0;
    }

    void ToNetOrder() {
        ip = H2NL(ip);
        port = H2NS(port);
        local_id = H2NL(local_id);
    }

    void ToHostOrder() {
        ip = N2HL(ip);
        port = N2HS(port);
        local_id = N2HL(local_id);
    }
};

#ifdef WIN32
#pragma pack(pop)
#else // linux
#pragma pack()
#endif // linux


//
// 自动内存管理
// 该版本取消内存大小限制
//
//
struct BufferV {
    uint32_t volatile       valid_len;
    char*                   buff;
    uint32_t volatile       max_buff_len;
    uint32_t                m_extend_step;
    //
    // get valid buffer length
    //
    inline uint32_t GetValidBufferLen() const {
        CHECK(valid_len <= max_buff_len);
        return valid_len;
    }

    //
    // get valid package length
    //
    inline uint32_t GetValidPackLen() {
        return (uint32_t)sizeof(valid_len)+valid_len;
    }

    //
    // get max buffer length
    //
    inline uint32_t GetMaxBufferLen() const {
        return max_buff_len;
    }

    //
    // set valid data length
    //
    inline void     SetValidDataLen(uint32_t len) {
        valid_len = (len <= max_buff_len) ? len:0;
        if (len>max_buff_len) {
            LOG(FATAL) << "Error, valid len:" << len << " > max_buff_len:" << max_buff_len;
        }
    }

    void ResetParam() {
        valid_len = 0;
    }

    void SetExtendStep(uint32_t extend_step) {
        m_extend_step = extend_step;
    }

    void SetData(const char* sz) {
        SetData((const unsigned char*)sz, STRLEN(sz) + 1);
    }

    void SetData(const unsigned char* data, uint32_t len) {
        if (data && CheckBuffer(len)) {
            memcpy(buff, data, len);
            valid_len = len;
        } else {
            valid_len = 0;
            VLOG(3) << "BufferV<MaxBufLen:" << max_buff_len << ">, SetData(0x" <<
                      reinterpret_cast<const void*>(data) << ", " << len << ") fail.";
        }
    }

    //
    // 需要保存原来的数据
    // 所有数据长度
    //
    bool CheckBuffer(uint32_t new_total_len);

    //
    // 新增数据长度
    //
    bool CheckNewAppendBuffer(uint32_t new_append_len);

    BufferV() {
        buff = NULL;
        max_buff_len = 0;
        valid_len = 0;
        m_extend_step = 32;
    }

    ~BufferV() {
        if (buff) {
            mempool_DELETE(buff);
            buff = 0;
        }
    }
};


//
// name:        CStrBuff
// description: string buffer
//
class CStrBuff {
public:
    CStrBuff();
    ~CStrBuff();   
    
    bool AppendStr(const char* str);
    bool AppendStr(const char* str, uint32_t len);

    bool AppendStr(unsigned char ch);    
    bool AppendStr(unsigned char ch, int32_t repeats); // 连续插入多少个相同的字符

    uint32_t GetBuffLen()const  {   return m_buff_len;  }
    uint32_t GetValidLen()const {   return m_valid_len; }

    unsigned char GetLastChar() const {
        return m_valid_len ? (unsigned char)(m_buff[m_valid_len-1]) : (unsigned char)0;
    }

    const char* GetString(){ return m_buff;}

    void Reset() { m_valid_len = 0; if (m_buff) m_buff[0] = 0; }

    void SetExtendStep(uint32_t extend_step);

    // \r \n 换成空格
    void ChangeNLRT2Space();
    
private:
    char*  m_buff;

    uint32_t    m_extend_step;
    uint32_t    m_buff_len;
    uint32_t    m_valid_len;
};

bool    XSetSocketReceiveBuffSize(SOCKET sock, int32_t size);
bool    XSetSocketSendBuffSize(SOCKET sock, int32_t size);
bool    XSetSocketBlockingMode(SOCKET sock, bool is_blocking);
bool    XSetSocketReuseAddress(SOCKET sock, bool is_reuse);
bool    XSetSocketLinger(SOCKET sock, bool is_linger_on, uint16_t delay_secs);
bool    XSetSocketNoDelay(SOCKET sock);
bool    XSetSocketAcceptFilter(SOCKET sock);
bool    XSetSocketQuickAck(SOCKET sock);
int32_t XGetSocketError(SOCKET sock);

int32_t GetLastSocketError();

// ToDo(wookin): 这里是支持可以传入域名或者ip都可以,
//               在R2统一所有glibc版本之前先不支持这个(静态编译警告)
// donniechen : 现在放开，支持域名解析
bool    GetHostByName(const char* name, char* ip_addr,
                      int32_t ip_addr_buff_len);

//
// Setting interval, nInterval: seconds
//
bool    XSetSocketTCPKeepAlive(SOCKET sock, int32_t interval);

int32_t    _CloseSocket(SOCKET s);
int32_t    _CloseSocket(SOCKET s, TCP_FD_TYPE type);

//
// Listen on port
//
bool ListenOnPort(SOCKET sock, const char* host, uint16_t port,
                  uint32_t listen_backlog);

// 下面三个函数方便对TCP fd进行全局平衡统计
SOCKET  NewSocketImp(const char* file, int32_t line, bool is_tcp);
#define NewSocket(is_tcp) NewSocketImp(__FILE__, __LINE__, is_tcp)

SOCKET  AcceptNewConnectionImp(const char* file, int32_t line,
                               SOCKET listen_sock, struct sockaddr_in* from_addr,
                               bool directly_accept = false);
#define AcceptNewConnection(listen_sock, from_addr, ...) AcceptNewConnectionImp(__FILE__, __LINE__, \
                                                         listen_sock, from_addr, ##__VA_ARGS__)


#define CloseSocket(x)        { xfs::base::_CloseSocket(x);       \
                                x = INVALID_SOCKET; }

#define CloseSocketA(x, type) { xfs::base::_CloseSocket(x, type); \
                                x = INVALID_SOCKET; }

#define CLOSESOCKET    CloseSocket

//
// 显示还有多少sockets 在使用中
//
void ListSocketsInUse();

//
// object:      NotifyEvent object
// description: notify message by connected tcp socket pair
//
enum NOTIFY_PEER {
    NOTIFY_PEER_A = 0,
    NOTIFY_PEER_B = 1
};

struct NotifyEvent {
    SOCKET     sock[2];
    uint32_t   netorder_host;      // Save host (network bytes order)
    uint16_t   netorder_port;
    SOCKET GetClientSock() const {
        return sock[NOTIFY_PEER_A];
    }

    SOCKET GetServerSock() const {
        return sock[NOTIFY_PEER_B];
    }

    NotifyEvent() {
        sock[0] = sock[1] = INVALID_SOCKET;
        char local_host[32]={0};
        // "127.0.0.1"
        safe_snprintf(local_host, sizeof(local_host), "127.0.%d.%d",
            safe_rand() % 256, xfs::base::GetPID() % 256);        
        netorder_host = inet_addr(local_host);
        netorder_port = uint16_t(kSocketPairStartPort + (safe_rand() % 100));
    }
};

bool CreateSocketPair(SOCKET* fd_read,              // peer A
                      SOCKET* fd_write,             // peer B
                      uint32_t  netorder_host,
                      uint16_t netorder_listen_port // Temporary local
                      // listen host, port
                     );

bool CreateSocketPairAutoPort(SOCKET* fd_read, SOCKET* fd_write,
                              uint32_t  netorder_host,
                              uint16_t netorder_listen_port,
                              uint16_t* success_port = 0);


int32_t FD_Readable(SOCKET fd,
                    int32_t millisecs_timeout,
                    bool test_connection = false,     // 测试是否网络中断
                    bool silence_when_timeout = false // 超时的时候不输出日志
                   );

int32_t FD_Writeable(SOCKET fd,
                     int32_t millisecs_timeout,
                     bool test_connection = false, // 测试是否网络中断
                     bool* readable = NULL,
                     bool* maybe_fail = NULL);

//
// 判断connect出去的socket handle是否可写
//
bool ConnWriteable(SOCKET sock, uint32_t timeout_secs, struct sockaddr_in* debug_to_addr = 0);
bool ConnReadable(SOCKET sock, uint32_t timeout_secs, bool silence_when_timeout = false);

// 和ConnReadable相同,超时使用millisecs
bool ConnReadableWithTimeout(SOCKET sock, uint32_t timeout_millisecs, bool silence_when_timeout = false);

//
// connect to host, 且判断是否连接成功
//
bool ConnectToHost(SOCKET sock, uint32_t netorder_host,
                   uint16_t netorder_port,
                   uint32_t timeout_secs);

NotifyEvent* CreateNotifyEvent(uint32_t netorder_host,
                               uint16_t netorder_port  // Temporary local
                               // listen host, port
                              );

//
// Delete old handle
// and create new handle
//
NotifyEvent* RecreateNotifyEvent(NotifyEvent*& old_handle);


// to_peer : Which side to receive message
void SetNotify(NotifyEvent* &handle, NOTIFY_PEER to_peer);

void CloseNotifyEvent(NotifyEvent*& handle);


#define RECV_TIMEOUT_INFINITE 2147483640

//
// TCP
//
// function:    SendFixedBytes, ReceiveFixedPackage
// description: send or receive fixed length data
//              send, receive TCP data
//
bool SendFixedBytes(SOCKET sock,
                    const char* buff,
                    int32_t fixed_len,
                    uint32_t* sent,
                    bool break_readable = false);

bool SendFixedBytesInTime(SOCKET sock,
                          const char* buff,
                          int32_t fixed_len,
                          int32_t timeout,
                          uint32_t* sent,
                          bool break_readable = false);

bool ReceiveFixedPackage(SOCKET sock,
                         char* buff,
                         int32_t len,
                         int32_t timeout,
                         int32_t* num_received);

//
// UDP
//
bool    SendDatagram(SOCKET sock,
                     const char* pack,
                     int32_t pack_len,
                     const char* to_host,
                     uint16_t to_port,
                     int32_t timeout_secs);

bool    ReceiveDatagram(SOCKET sock,
                        char* buff,
                        int32_t buff_len,
                        int32_t* received_len,
                        struct sockaddr_in* from_addr,
                        int32_t timeout_secs);

// 封装send()函数,自动retry EINTR
// return: >0 , OK
//         0  , timeout
//         <0 , fail
int32_t SendDat(SOCKET sock, const char* buff, int32_t len, int32_t flags);

// 封装recv()函数,自动retry EINTR
// return: >0 , OK
//         0  , remote closed
//         <0 , fail
int32_t RecvDat(SOCKET sock, char* buff, int32_t len, int32_t flags);

//
// inet_ntoa()使用一个公共内存,连续调用inet_ntoa会导致最后一个结果冲掉前面的结果
// Inet_ntoa()线程安全,结果保存到buff中,返回buff地址
//
char* Inet_ntoa(struct in_addr in, char* buff, int32_t buff_len);

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_GENERAL_SOCK_H_
