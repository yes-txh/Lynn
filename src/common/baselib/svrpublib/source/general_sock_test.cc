//////////////////////////////////////////////////////////////////////////
// general_sock_test.cc
// @brief:     Test functions in general_sock.cc
// @author:  fatliu@tencent
// @time:     2010-10-11
// @version: 1.0
//////////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/general_head.h"
#include "common/baselib/svrpublib/general_type_def.h"
#include "common/baselib/svrpublib/thread_mutex.h"
#include "common/baselib/svrpublib/general_util.h"

#include "common/baselib/svrpublib/log.h"
#include "common/baselib/svrpublib/lite_mempool.h"
#include "common/baselib/svrpublib/general_sock.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

#ifdef WIN32
int32_t TestGeneralSock(int32_t argc, char** argv)
#else
int32_t main(int32_t argc, char** argv)
#endif
{
#ifndef WIN32
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, true);

    AutoBaseLib auto_baselib;
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
#else
    return 0;
#endif
}

// @brief:     Test StrBuff with default extend step
TEST(TestStrBuff, DefaultExtendStep) {
    bool b = false;
    CStrBuff str_buff;
    str_buff.Reset();

    // default extend step 1024
    const char* str = "test1";
    uint32_t str_len = static_cast<uint32_t>(strlen(str));
    b = str_buff.AppendStr(str, str_len);
    CHECK(b);
    const char* ustr1 = "test2";
    uint32_t ustr1_len = static_cast<uint32_t>(strlen((const char*)ustr1));
    b = str_buff.AppendStr(ustr1);
    CHECK(b);
    b = str_buff.AppendStr(ustr1, ustr1_len);
    CHECK(b);
    unsigned char ch = 'c';
    b = str_buff.AppendStr(ch);
    CHECK(b);

    uint32_t total_valid_len = str_buff.GetValidLen();
    CHECK_EQ(total_valid_len, str_len + 2*ustr1_len + 1);

    uint32_t total_buff_len = str_buff.GetBuffLen();
    CHECK_EQ(total_buff_len, static_cast<uint32_t>(1029));
}

// @brief:     Test StrBuff with extend step 8
TEST(TestStrBuff, ExtendStep8) {
    bool b = false;
    CStrBuff str_buff;
    str_buff.Reset();

    const char* str = "test1";
    uint32_t str_len = static_cast<uint32_t>(strlen(str));
    const char* ustr1 = "test2";
    uint32_t ustr1_len = static_cast<uint32_t>(strlen((const char*)ustr1));
    unsigned char ch = 'c';

    // change extend step to 8, buffer len never changed
    str_buff.Reset();
    str_buff.SetExtendStep(8);
    b = str_buff.AppendStr(str, str_len);
    CHECK(b);
    b = str_buff.AppendStr(ustr1);
    CHECK(b);
    b = str_buff.AppendStr(ustr1, ustr1_len);
    CHECK(b);
    b = str_buff.AppendStr(ch);
    CHECK(b);
    CHECK_EQ(ch, str_buff.GetLastChar());

    uint32_t total_valid_len = str_buff.GetValidLen();
    CHECK_EQ(total_valid_len, str_len + 2*ustr1_len + 1);

    uint32_t total_buff_len = str_buff.GetBuffLen();
    CHECK_EQ(total_buff_len, static_cast<uint32_t>(23));
}

// @brief:     Test StrBuff with extend step 4
TEST(TestStrBuff, ExtendStep4) {
    bool b = false;
    CStrBuff str_buff;
    str_buff.Reset();

    const char* str = "test1";
    uint32_t str_len = static_cast<uint32_t>(strlen(str));
    const char* ustr1 = "test2";
    uint32_t ustr1_len = static_cast<uint32_t>(strlen((const char*)ustr1));
    unsigned char ch = 'c';

    // change extend step to 4, clean buff len
    CStrBuff str_buff1;
    str_buff1.SetExtendStep(4);
    b = str_buff1.AppendStr(str, str_len);
    CHECK(b);
    b = str_buff1.AppendStr(ustr1);
    CHECK(b);
    b = str_buff1.AppendStr(ustr1, ustr1_len);
    CHECK(b);
    b = str_buff1.AppendStr(ch);
    CHECK(b);

    uint32_t total_valid_len = str_buff1.GetValidLen();
    CHECK_EQ(total_valid_len, static_cast<uint32_t>(16));

    uint32_t total_buff_len = str_buff1.GetBuffLen();
    CHECK_EQ(total_buff_len, static_cast<uint32_t>(19));
}

// @brief:     Test operations in BufferV
TEST(TestBufferV, BufferV) {
    bool b = false;
    BufferV buffer;

    // test set data
    const unsigned char* data1 = (const unsigned char*)"test1";
    uint32_t data1_len = static_cast<uint32_t>(strlen((const char*)data1));
    buffer.SetData(data1, data1_len);
    const char* data2 = "test2";
    uint32_t data2_len = static_cast<uint32_t>(strlen(data2));
    buffer.SetData(data2);
    uint32_t total_len = data2_len + 1;
    CHECK_EQ(buffer.GetValidBufferLen(), total_len);
    CHECK_EQ(buffer.GetValidPackLen(),
             total_len + static_cast<uint32_t>(sizeof(uint32_t)));

    CHECK_EQ(buffer.GetMaxBufferLen(), static_cast<uint32_t>(32) + data1_len);

    // test set valid data len
    buffer.SetValidDataLen(static_cast<uint32_t>(3));
    CHECK_EQ(buffer.GetValidBufferLen(), static_cast<uint32_t>(3));
    CHECK_EQ(buffer.GetValidPackLen(),
             static_cast<uint32_t>(sizeof(uint32_t)) + 3);

    CHECK_EQ(buffer.GetMaxBufferLen(), static_cast<uint32_t>(32) + data1_len);

    // test check buffer
    buffer.ResetParam();
    buffer.SetExtendStep(static_cast<uint32_t>(8));
    b = buffer.CheckBuffer(40);
    CHECK(b);
    CHECK_EQ(buffer.GetMaxBufferLen(), static_cast<uint32_t>(40 + 8));
    CHECK_EQ(buffer.GetValidBufferLen(), static_cast<uint32_t>(0));
    buffer.CheckBuffer(100);
    CHECK(b);
    CHECK_EQ(buffer.GetMaxBufferLen(), static_cast<uint32_t>(100 + 8));

    // another buffer, clean max buffer len
    BufferV buffer1;
    buffer1.SetExtendStep(static_cast<uint32_t>(8));
    const unsigned char* data3 = (const unsigned char*)"tst3";
    uint32_t data3_len = static_cast<uint32_t>(strlen((const char*)data3));
    buffer1.SetData(data3, data3_len);
    CHECK_EQ(buffer1.GetMaxBufferLen(), data3_len + static_cast<uint32_t>(8));
    CHECK_EQ(buffer1.GetValidBufferLen(), data3_len);

    // test check new append buffer
    buffer1.CheckNewAppendBuffer(10);
    CHECK_EQ(buffer1.GetMaxBufferLen(),
             data3_len + static_cast<uint32_t>(10 + 8));

    buffer1.CheckNewAppendBuffer(4);
    CHECK_EQ(buffer1.GetMaxBufferLen(),
             data3_len + static_cast<uint32_t>(10 + 8));
}

// @brief:     Test function SetSocketBlockingMode
TEST(TestSetSocketBlockingMode, SetSocketBlockingMode) {
    bool b = false;
    SOCKET sock = NewSocket(true);
    bool is_blocking = true;

#ifdef WIN32
    b = XSetSocketBlockingMode(sock, is_blocking);
    CHECK(b);
    is_blocking = false;
    b = XSetSocketBlockingMode(sock, is_blocking);
    CHECK(b);
#else
    b = XSetSocketBlockingMode(sock, is_blocking);
    CHECK(b);
    int32_t old_flag = fcntl(sock, F_GETFL, 0);
    CHECK_EQ(0, old_flag & O_NONBLOCK);

    is_blocking = false;
    b = XSetSocketBlockingMode(sock, is_blocking);
    CHECK(b);
    old_flag = fcntl(sock, F_GETFL, 0);
    CHECK(old_flag & O_NONBLOCK);
#endif

    CLOSESOCKET(sock);
}

// @brief:     Test function SetSocketReuseAddress
TEST(TestSetSocketReuseAddress, SetSocketReuseAddress) {
    bool b = false;
    bool is_reuse = true;
    SOCKET sock = NewSocket(true);
    b = XSetSocketReuseAddress(sock, is_reuse);
    CHECK(b);

    int32_t val1;
    socklen_t len = sizeof(int32_t);
    int32_t ret = getsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                             reinterpret_cast<char*>(&val1), &len);
    CHECK_NE(-1, ret);
    CHECK_EQ(1, val1);

    is_reuse = false;
    int32_t val2;
    b = XSetSocketReuseAddress(sock, is_reuse);
    CHECK(b);
    ret = getsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<char*>(&val2), &len);
    CHECK_NE(-1, ret);
    CHECK_EQ(0, val2);

    CLOSESOCKET(sock);
}

// @brief:     Test function SetSocketLinger
TEST(TestSetSocketLinger, SetSocketLinger) {
    bool b = false;
    SOCKET sock = NewSocket(true);
    bool is_linger_on = true;
    uint16_t delay_secs = 2;
    b = XSetSocketLinger(sock, is_linger_on, delay_secs);
    CHECK(b);

    struct linger linger_val;
    socklen_t len = sizeof(linger_val);
    int32_t ret = getsockopt(sock, SOL_SOCKET, SO_LINGER,
                             reinterpret_cast<char*>(&linger_val), &len);
    CHECK_NE(-1, ret);
    CHECK_EQ(1, linger_val.l_onoff);
    CHECK_EQ(2, linger_val.l_linger);

    is_linger_on = false;
    delay_secs = 1;
    b = XSetSocketLinger(sock, is_linger_on, delay_secs);
    CHECK(b);
    ret = getsockopt(sock, SOL_SOCKET, SO_LINGER,
                     reinterpret_cast<char*>(&linger_val), &len);
    CHECK_NE(-1, ret);
    CHECK_EQ(0, linger_val.l_onoff);
#ifdef WIN32
    CHECK_EQ(1, linger_val.l_linger);
#else
    CHECK_EQ(2, linger_val.l_linger);
#endif

    CLOSESOCKET(sock);
}

// @brief:     Test function SetSocketReceiveBuffSize
TEST(TestSetSocketReceiveBuffSize, SetSocketReceiveBuffSize) {
    bool b = false;
    SOCKET sock = NewSocket(true);
    int32_t val1;
    socklen_t len = sizeof(int32_t);
    int32_t ret = getsockopt(sock, SOL_SOCKET, SO_RCVBUF,
                             reinterpret_cast<char*>(&val1), &len);
    EXPECT_NE(-1, ret);
#ifdef WIN32
    EXPECT_EQ(1024 * 1024, val1);
#else
    CHECK_EQ(262142, val1);
#endif

    int32_t size = 1024;
    b = XSetSocketReceiveBuffSize(sock, size);
    EXPECT_TRUE(b);
    int32_t val2;
    ret = getsockopt(sock, SOL_SOCKET, SO_RCVBUF,
                     reinterpret_cast<char*>(&val2), &len);
    EXPECT_NE(-1, ret);
    EXPECT_NE(val1, val2);
#ifdef WIN32
    EXPECT_EQ(1024, val2);
#else
    EXPECT_EQ(1024 * 2, val2);
#endif

    CLOSESOCKET(sock);
}

// @brief:     Test function SetSocketSendBuffSize
TEST(TestSetSocketSendBuffSize, SetSocketSendBuffSize) {
    bool b = false;
    SOCKET sock = NewSocket(true);
    int32_t val1;
    socklen_t len = sizeof(int32_t);
    int32_t ret = getsockopt(sock, SOL_SOCKET, SO_SNDBUF,
                             reinterpret_cast<char*>(&val1), &len);
    EXPECT_NE(-1, ret);
#ifdef WIN32
    EXPECT_EQ(1024 * 1024, val1);
#else
    EXPECT_EQ(262142, val1);
#endif

    int32_t size = 1024;
    b = XSetSocketSendBuffSize(sock, size);
    EXPECT_TRUE(b);
    int32_t val2;
    ret = getsockopt(sock, SOL_SOCKET, SO_SNDBUF,
                     reinterpret_cast<char*>(&val2), &len);
    EXPECT_NE(-1, ret);
    EXPECT_NE(val1, val2);
#ifdef WIN32
    EXPECT_EQ(1024, val2);
#else
    EXPECT_EQ(1024 * 2, val2);
#endif

    CLOSESOCKET(sock);
}

// @brief:     Test function SetSocketNoDelay
TEST(TestSetSocketNoDelay, SetSocketNoDelay) {
    bool b = false;
    SOCKET sock = NewSocket(true);
    int32_t val1;
    socklen_t len = sizeof(int32_t);
    b = XSetSocketNoDelay(sock);
    CHECK(b);
    int32_t ret = getsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                             reinterpret_cast<char*>(&val1), &len);
    CHECK_NE(-1, ret);
    CHECK_EQ(1, val1);

    CLOSESOCKET(sock);
}

// @brief:     Test function SetSocketAcceptFilter
TEST(TestSetSocketAcceptFilter, SetSocketAcceptFilter) {
    bool b = false;
    SOCKET sock = NewSocket(true);
#ifdef WIN32
    b = XSetSocketAcceptFilter(sock);
    EXPECT_TRUE(b);
#else
    int32_t val1;
    socklen_t len = sizeof(int32_t);
    int32_t ret = getsockopt(sock, IPPROTO_TCP, TCP_DEFER_ACCEPT,
                             reinterpret_cast<char*>(&val1), &len);
    EXPECT_NE(-1, ret);
    EXPECT_EQ(0, val1);

    b = XSetSocketAcceptFilter(sock);
    EXPECT_TRUE(b);
    int32_t val2;
    ret = getsockopt(sock, IPPROTO_TCP, TCP_DEFER_ACCEPT,
                     reinterpret_cast<char*>(&val2), &len);
    EXPECT_NE(-1, ret);
    EXPECT_NE(0, val2);

#endif
    CLOSESOCKET(sock);
}

// @brief:     Test function SetSocketQuickAck
TEST(TestSetSocketQuickAck, SetSocketQuickAck) {
    bool b = false;
    SOCKET sock = NewSocket(true);
#ifdef WIN32
    b = XSetSocketQuickAck(sock);
    CHECK(b);
#else
    b = XSetSocketQuickAck(sock);
    CHECK(b);
    int32_t val2;
    socklen_t len = sizeof(int32_t);
    int32_t ret = getsockopt(sock, IPPROTO_TCP, TCP_QUICKACK,
                             reinterpret_cast<char*>(&val2), &len);
    CHECK_NE(-1, ret);
    CHECK_EQ(1, val2);
#endif
    CLOSESOCKET(sock);
}

// @brief:     Test function SetSocketTCPKeepAlive
TEST(TestSetSocketTCPKeepAlive, SetSocketTCPKeepAlive) {
    bool b = false;
    SOCKET sock = NewSocket(true);
    int32_t interval = 1;
#ifdef WIN32
    b = XSetSocketTCPKeepAlive(sock, interval);
    CHECK(b);
    // undo...
#else
    b = XSetSocketTCPKeepAlive(sock, interval);
    CHECK(b);

    int32_t val;
    socklen_t len = sizeof(int32_t);
    int32_t ret = getsockopt(sock, SOL_SOCKET, SO_KEEPALIVE,
                             reinterpret_cast<char*>(&val), &len);
    CHECK_NE(-1, ret);
    CHECK_EQ(1, val);

    ret = getsockopt(sock, SOL_TCP, TCP_KEEPIDLE,
                     reinterpret_cast<char*>(&val), &len);
    CHECK_NE(-1, ret);
    CHECK_EQ(5, val);

    ret = getsockopt(sock, SOL_TCP, TCP_KEEPINTVL,
                     reinterpret_cast<char*>(&val), &len);
    CHECK_NE(-1, ret);
    CHECK_EQ(interval, val);

    ret = getsockopt(sock, SOL_TCP, TCP_KEEPCNT,
                     reinterpret_cast<char*>(&val), &len);
    CHECK_NE(-1, ret);
    CHECK_EQ(3, val);
#endif

    CLOSESOCKET(sock);
}

// @brief:     Test function GetHostByName
// TEST(TestGetHostByName, GetHostByName)
// {
//    char ip[64] = {0};
//    int32_t ip_addr_buff_len = 64;
//    bool b = false;
//
// #ifdef WIN32
//    const char* name = "om.soso.oa.com";
//    b = GetHostByName(name, ip, ip_addr_buff_len);
//    EXPECT_TRUE(b);
//    EXPECT_EQ(0, strcmp(ip, "172.24.28.192"));
// #endif
//
//    const char* name1 = "172.24.28.192";
//    b = GetHostByName(name1, ip, ip_addr_buff_len);
//    CHECK(b);
//    CHECK_EQ(0, strcmp(ip, "172.24.28.192"));
//
//    const char* name2 = "localhost";
//    b = GetHostByName(name2, ip, ip_addr_buff_len);
//    CHECK(b);
//    CHECK_EQ(0, strcmp(ip, "127.0.0.1"));
// }

// @brief:     Test function SendFixedBytes
TEST(TestSendFixedBytes, SendFixedBytes) {
    SOCKET sock_listen = NewSocket(true);
    const char* host = "127.0.0.1";
    uint16_t port = 40015;
    uint32_t listen_backlog = 5;
    bool b = ListenOnPort(sock_listen, host, port, listen_backlog);
    CHECK(b);

    SOCKET sock_client = NewSocket(true);
    uint32_t timeout = 1;
    b = ConnectToHost(sock_client, inet_addr(host), H2NS(port), timeout);
    CHECK(b);

    const char* buff = "test";
    int32_t fix_len = static_cast<int32_t>(strlen((const char*)buff)) + 1;
    uint32_t sent = 0;
    bool break_readable = false;
    b = SendFixedBytes(sock_client, buff, fix_len, &sent, break_readable);
    CHECK(b);
    CHECK_EQ(fix_len, (int32_t)sent);

    CLOSESOCKET(sock_client);
    CLOSESOCKET(sock_listen);
}

// @brief:     Test function ReceiveFixedPackage
TEST(TestReceiveFixedPackage, ReceiveFixedPackage) {
    // listen
    SOCKET sock_listen = NewSocket(true);
    XSetSocketBlockingMode(sock_listen, false);
    const char* host_listen = "127.0.0.1";
    uint16_t port_listen = 40015;
    uint32_t listen_backlog = 5;
    bool b = ListenOnPort(sock_listen,
                          host_listen, port_listen,
                          listen_backlog);
    CHECK(b);

    // connect
    SOCKET sock_client = NewSocket(true);
    uint32_t timeout = 5;
    b = ConnectToHost(sock_client,
                      inet_addr(host_listen), H2NS(port_listen),
                      timeout);
    CHECK(b);

    // accept
    struct sockaddr_in from_addr;
    memset(&from_addr, 0, sizeof(from_addr));
    SOCKET new_sock = AcceptNewConnection(sock_listen,
                                          &from_addr,
                                          false);

    // send
    const char* buff_snd = "test";
    int32_t fix_len = static_cast<int32_t>(strlen((const char*)buff_snd)) + 1;
    uint32_t sent = 0;
    bool break_readable = false;
    b = SendFixedBytes(sock_client, buff_snd, fix_len, &sent, break_readable);
    CHECK(b);
    CHECK_EQ(fix_len, (int32_t)sent);

    // receive
    char buff_rcv[128] = {0};
    int32_t received = 0;
    b = ReceiveFixedPackage(new_sock, buff_rcv, fix_len, timeout, &received);
    CHECK(b);
    CHECK_EQ((uint32_t)received, sent);
    CHECK_EQ(0, strcmp(buff_rcv, (const char*)buff_snd));

    CLOSESOCKET(new_sock);
    CLOSESOCKET(sock_client);
    CLOSESOCKET(sock_listen);
}

// @brief:     Test function SendDatagram
TEST(TestSendDatagram, SendDatagram) {
    SOCKET sock_listen = NewSocket(true);
    const char* host = "127.0.0.1";
    uint16_t port = 40015;
    uint32_t listen_backlog = 5;
    bool b = ListenOnPort(sock_listen, host, port, listen_backlog);
    CHECK(b);

    SOCKET sock_client = NewSocket(true);
    uint32_t timeout = 1;
    b = ConnectToHost(sock_client, inet_addr(host), H2NS(port), timeout);
    CHECK(b);

    const char* pack = "test";
    int32_t pack_len = static_cast<int32_t>(strlen((const char*)pack + 1));
    b = SendDatagram(sock_client,
                     pack,
                     pack_len,
                     host,
                     port,
                     static_cast<int32_t>(timeout));
    CHECK(b);

    CLOSESOCKET(sock_client);
    CLOSESOCKET(sock_listen);
}

// @brief:     Test function ReceiveDatagram
TEST(TestReceiveDatagram, ReceiveDatagram) {
    // listen
    SOCKET sock_listen = NewSocket(true);
    const char* host = "127.0.0.1";
    uint16_t port = 40015;
    uint32_t listen_backlog = 5;
    bool b = ListenOnPort(sock_listen, host, port, listen_backlog);
    CHECK(b);

    // connect
    SOCKET sock_client = NewSocket(true);
    uint32_t timeout = 1;
    b = ConnectToHost(sock_client, inet_addr(host), H2NS(port), timeout);
    CHECK(b);

    // accept
    struct sockaddr_in from_addr;
    memset(&from_addr, 0, sizeof(from_addr));
    SOCKET new_sock = AcceptNewConnection(sock_listen,
                                          &from_addr,
                                          false);

    // send
    const char* pack = "test";
    int32_t pack_len = static_cast<int32_t>(strlen((const char*)pack)) + 1;
    b = SendDatagram(sock_client,
                     pack,
                     pack_len,
                     host,
                     port,
                     static_cast<int32_t>(timeout));
    CHECK(b);

    // receive
    char buff[128] = {0};
    int32_t received_len;
    b = ReceiveDatagram(new_sock,
                        buff,
                        128,
                        &received_len,
                        &from_addr,
                        timeout);
    CHECK(b);
    CHECK_EQ(0, strcmp((const char*)buff, (const char*)pack));

    CLOSESOCKET(new_sock);
    CLOSESOCKET(sock_client);
    CLOSESOCKET(sock_listen);
}

// @brief:     Test function ListenOnPort
TEST(TestListenOnPort, ListenOnPort) {
    SOCKET sock_listen = NewSocket(true);
    const char* host = "127.0.0.1";
    uint16_t port = 40015;
    uint32_t listen_backlog = 5;
    bool b = ListenOnPort(sock_listen, host, port, listen_backlog);
    CHECK(b);

    CLOSESOCKET(sock_listen);
}

// @brief:     Test function AcceptNewConnection
TEST(TestAcceptNewConnection, AcceptNewConnection) {
    SOCKET sock_listen = NewSocket(true);
    const char* host = "127.0.0.1";
    uint16_t port = 40015;
    uint32_t listen_backlog = 5;
    bool b = ListenOnPort(sock_listen, host, port, listen_backlog);
    CHECK(b);

    SOCKET sock_client = NewSocket(true);
    uint32_t timeout = 1;
    b = ConnectToHost(sock_client, inet_addr(host), H2NS(port), timeout);
    CHECK(b);

    struct sockaddr_in from_addr;
    memset(&from_addr, 0, sizeof(from_addr));
    from_addr.sin_family = AF_INET;
    from_addr.sin_addr.s_addr = inet_addr(host);
    from_addr.sin_port = H2NS(50116);
    bool directly_accept = false;
    SOCKET new_sock = AcceptNewConnection(sock_listen,
                                          &from_addr,
                                          directly_accept);
    CHECK_NE(static_cast<uint32_t>(new_sock),
             static_cast<uint32_t>(INVALID_SOCKET));

    CLOSESOCKET(sock_client);
    CLOSESOCKET(sock_listen);
}

// @brief:     Test function CreateSocketPairAutoPort
TEST(TestCreateSocketPairAutoPort, CreateSocketPairAutoPort) {
    SOCKET fd_read, fd_write;
    uint32_t netorder_host = inet_addr("127.0.0.1");
    uint16_t netorder_listen_port = htons(40015);
    uint16_t success_port = 0;
    bool b = CreateSocketPairAutoPort(&fd_read,
                                      &fd_write,
                                      netorder_host,
                                      netorder_listen_port,
                                      &success_port);
    CHECK(b);
    CHECK_NE(static_cast<uint32_t>(fd_read),
             static_cast<uint32_t>(INVALID_SOCKET));

    CHECK_NE(static_cast<uint32_t>(fd_write),
             static_cast<uint32_t>(INVALID_SOCKET));
    CHECK_GE(success_port, netorder_listen_port);

    CLOSESOCKET(fd_write);
    CLOSESOCKET(fd_read);
}

// @brief:     Test function CreateSocketPair
TEST(TestCreateSocketPair, CreateSocketPair) {
    SOCKET fd_read, fd_write;
    uint32_t netorder_host = inet_addr("127.0.0.1");
    uint16_t netorder_listen_port = htons(30015);
    bool b = CreateSocketPair(&fd_read,
                              &fd_write,
                              netorder_host,
                              netorder_listen_port);
    CHECK(b);
    CHECK_NE(static_cast<uint32_t>(fd_read),
             static_cast<uint32_t>(INVALID_SOCKET));

    CHECK_NE(static_cast<uint32_t>(fd_write),
             static_cast<uint32_t>(INVALID_SOCKET));

    CLOSESOCKET(fd_write);
    CLOSESOCKET(fd_read);
}

// @brief:     Test function ConnWriteable
TEST(TestConnWriteable, ConnWriteable) {
    SOCKET sock_listen = NewSocket(true);
    const char* host = "127.0.0.1";
    uint16_t port = 40015;
    uint32_t listen_backlog = 5;
    bool b = ListenOnPort(sock_listen, host, port, listen_backlog);
    CHECK(b);

    uint32_t timeout_secs = 1;
    struct sockaddr_in to_addr;
    to_addr.sin_family = AF_INET;
    to_addr.sin_addr.s_addr = inet_addr(host);
    to_addr.sin_port = htons(port);

    SOCKET sock_client = NewSocket(true);
    connect(sock_client, reinterpret_cast<struct sockaddr*>(&to_addr), sizeof(to_addr));
    b = ConnWriteable(sock_client, timeout_secs, &to_addr);
    CHECK(b);

    CLOSESOCKET(sock_client);
    CLOSESOCKET(sock_listen);
}

// @brief:     Test function ConnReadable
TEST(TestConnReadable, ConnReadable) {
    SOCKET sock_listen = NewSocket(true);
    const char* host = "127.0.0.1";
    uint16_t port = 40015;
    uint32_t listen_backlog = 5;
    bool b = ListenOnPort(sock_listen, host, port, listen_backlog);
    CHECK(b);

    uint32_t timeout_secs = 1;
    struct sockaddr_in to_addr;
    to_addr.sin_family = AF_INET;
    to_addr.sin_addr.s_addr = inet_addr(host);
    to_addr.sin_port = htons(port);

    SOCKET sock_client = NewSocket(true);
    connect(sock_client, reinterpret_cast<struct sockaddr*>(&to_addr), sizeof(to_addr));
    b = ConnReadable(sock_listen, timeout_secs, false);
    CHECK(b);

    CLOSESOCKET(sock_client);
    CLOSESOCKET(sock_listen);
}

// @brief:     Test function ConnectToHost
TEST(TestConnectToHost, ConnectToHost) {
    SOCKET sock_listen = NewSocket(true);
    const char* host = "127.0.0.1";
    uint16_t port = 40015;
    uint32_t listen_backlog = 5;
    bool b = ListenOnPort(sock_listen, host, port, listen_backlog);
    CHECK(b);

    SOCKET sock_client = NewSocket(true);
    uint32_t timeout = 1;
    b = ConnectToHost(sock_client, inet_addr(host), H2NS(port), timeout);
    CHECK(b);
    CLOSESOCKET(sock_client);
    CLOSESOCKET(sock_listen);
}

// @brief:     Test function CreateNotifyEvent
TEST(TestCreateNotifyEvent, CreateNotifyEvent) {
    uint32_t netorder_host = inet_addr("127.0.0.1");
    uint16_t netorder_listen_port = htons(30115);
    NotifyEvent* evt = CreateNotifyEvent(netorder_host,
                                         netorder_listen_port);
    CHECK(evt);
    CHECK_EQ(evt->netorder_host, netorder_host);
    CHECK_EQ(evt->netorder_port, netorder_listen_port);
    CloseNotifyEvent(evt);
}

// @brief:     Test function RecreateNotifyEvent
TEST(TestRecreateNotifyEvent, RecreateNotifyEvent) {
    uint32_t netorder_host = inet_addr("127.0.0.1");
    uint16_t netorder_listen_port = htons(30215);
    NotifyEvent* evt = CreateNotifyEvent(netorder_host,
                                         netorder_listen_port);
    CHECK(evt);

    NotifyEvent* evt_new = RecreateNotifyEvent(evt);
    CHECK(evt_new);
    EXPECT_TRUE(evt == NULL);
    
    CloseNotifyEvent(evt_new);
}

// @brief:     Test function H2NF
TEST(TestH2NF, H2NF) {
    float fval = static_cast<float>(3.14);
    float fh2nf = H2NF(fval);
    float fn2hf = N2HF(fh2nf);
    CHECK_EQ(0, static_cast<int32_t>(fval-fn2hf));
    CHECK_EQ(0, static_cast<int32_t>(fh2nf-H2NF(fn2hf)));
}
