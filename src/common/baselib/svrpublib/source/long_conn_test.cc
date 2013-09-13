//////////////////////////////////////////////////////////////////////////
// long_conn_test.cc
// @brief:      Test class CLongConn & CLongConnTask
// @author:     fatliu@tencent
// @time:       2010-10-14
// @version:    1.0
//////////////////////////////////////////////////////////////////////////
#include <string.h>
#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

#ifdef WIN32
int32_t TestLongConn(int32_t argc, char** argv)
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

// @brief:  test method Init & Uninit in class CLongConn
TEST(CLongConn, Init) {
    CLongConn lc;

    // test Init(uint32_t max_sessions, const char* listen_host,
    //           uint16_t port, float timeout, bool* break_epoll_wait = 0)
    uint32_t max_sessions = 5;
    const char* listen_host = "127.0.0.1";
    uint16_t port = 50030;
    float timeout = static_cast<float>(0.01);
    bool break_epoll_wait = true;
    bool b = lc.Init(max_sessions, listen_host, port,
                     timeout, NULL,  &break_epoll_wait);
    EXPECT_TRUE(b);

    // test Init(uint32_t max_sessions, SOCKET listen_sock,
    //          float timeout, bool* break_epoll_wait = 0)
    SOCKET listen_sock = NewSocket(true);
    EXPECT_NE(INVALID_SOCKET, listen_sock);

    // Init twice, fail
    b = lc.Init(max_sessions, listen_sock,
                timeout, NULL, &break_epoll_wait);
    EXPECT_FALSE(b);

    lc.Uninit();
    b = lc.Init(max_sessions, listen_sock,
                timeout, NULL, &break_epoll_wait);
    EXPECT_TRUE(b);

    lc.Uninit();
    CLOSESOCKET(listen_sock);
}

// @brief:  test method CreateLongConnSession &
//          CloseLongConnSession in class CLongConn
TEST(CLongConn, CreateLongConnSession) {
    // start server
    SOCKET  listen_sock = NewSocket(true);
    EXPECT_NE(INVALID_SOCKET, listen_sock);

    const char* host_ip = "127.0.0.1";
    uint16_t host_port = 50020;
    bool b = ListenOnPort(listen_sock, host_ip, host_port, 5);

    // init lc
    CLongConn lc;
    uint32_t max_sessions = 5;
    const char* host = "127.0.0.1";
    uint16_t port = 50021;
    float timeout = static_cast<float>(0.01);
    bool break_epoll_wait = true;
    b = lc.Init(max_sessions, host, port,
                timeout, NULL, &break_epoll_wait);
    EXPECT_TRUE(b);

    // create long conn session
    LongConnHandle hSession = lc.CreateLongConnSession(host_ip, host_port);
    LongConnSession* session =
        reinterpret_cast<LongConnSession*>(hSession.handle);
    EXPECT_NE(INVALID_SOCKET, session->sock);
    EXPECT_NE((uint32_t)0, hSession.serial_num);

    // close long conn session
    lc.CloseLongConnSession(hSession);
    session = reinterpret_cast<LongConnSession*>(hSession.handle);
    EXPECT_EQ(INVALID_SOCKET, session->sock);
    EXPECT_NE(hSession.serial_num, session->long_conn_serial_num);

    lc.Uninit();
    CLOSESOCKET(listen_sock);
}

// @brief:  test method GetPeerNameOfLongConn in class CLongConn
TEST(CLongConn, GetPeerNameOfLongConn) {
    // start server
    SOCKET  listen_sock = NewSocket(true);
    EXPECT_NE(INVALID_SOCKET, listen_sock);

    const char* host_ip = "127.0.0.1";
    uint16_t host_port = 50020;
    bool b = ListenOnPort(listen_sock, host_ip, host_port, 5);

    // init lc
    CLongConn lc;
    uint32_t max_sessions = 5;
    const char* host = "127.0.0.1";
    uint16_t port = 50021;
    float timeout = static_cast<float>(0.01);
    bool break_epoll_wait = true;
    b = lc.Init(max_sessions, host, port,
                timeout, NULL, &break_epoll_wait);
    EXPECT_TRUE(b);

    // create long conn session
    LongConnHandle hSession = lc.CreateLongConnSession(host_ip, host_port);
    LongConnSession* session =
        reinterpret_cast<LongConnSession*>(hSession.handle);
    EXPECT_NE(INVALID_SOCKET, session->sock);
    EXPECT_NE((uint32_t)0, hSession.serial_num);

    // get peer name of long conn
    uint32_t peer_host = 0;
    uint16_t peer_port = 0;
    b = lc.GetPeerNameOfLongConn(hSession, &peer_host, &peer_port);
    EXPECT_TRUE(b);
    EXPECT_EQ(inet_addr(host_ip), peer_host);
    // EXPECT_EQ(H2NS(host_port), peer_port);

    // close long conn session
    lc.CloseLongConnSession(hSession);
    session = reinterpret_cast<LongConnSession*>(hSession.handle);
    EXPECT_EQ(INVALID_SOCKET, session->sock);
    EXPECT_NE(hSession.serial_num, session->long_conn_serial_num);

    lc.Uninit();
    CLOSESOCKET(listen_sock);
}

// @brief:  test method GetSockHandleOfLongConn in class CLongConn
TEST(CLongConn, GetSockHandleOfLongConn) {
    // start server
    SOCKET  listen_sock = NewSocket(true);
    EXPECT_NE(INVALID_SOCKET, listen_sock);

    const char* host_ip = "127.0.0.1";
    uint16_t host_port = 50020;
    bool b = ListenOnPort(listen_sock, host_ip, host_port, 5);

    // init lc
    CLongConn lc;
    uint32_t max_sessions = 5;
    const char* host = "127.0.0.1";
    uint16_t port = 50021;
    float timeout = static_cast<float>(0.01);
    bool break_epoll_wait = true;
    b = lc.Init(max_sessions, host, port,
                timeout, NULL, &break_epoll_wait);
    EXPECT_TRUE(b);

    // create long conn session
    LongConnHandle hSession = lc.CreateLongConnSession(host_ip, host_port);
    LongConnSession* session =
        reinterpret_cast<LongConnSession*>(hSession.handle);
    EXPECT_NE(INVALID_SOCKET, session->sock);
    EXPECT_NE((uint32_t)0, hSession.serial_num);

    // get sock handle of long conn
    SOCKET sock = lc.GetSockHandleOfLongConn(hSession);
    EXPECT_EQ(session->sock, sock);

    // close long conn session
    lc.CloseLongConnSession(hSession);
    session = reinterpret_cast<LongConnSession*>(hSession.handle);
    EXPECT_EQ(INVALID_SOCKET, session->sock);
    EXPECT_NE(hSession.serial_num, session->long_conn_serial_num);

    lc.Uninit();
    CLOSESOCKET(listen_sock);
}

// @brief:  test method SendNodeData in class CLongConn
TEST(CLongConn, SendNodeData) {
    // start server
    SOCKET  listen_sock = NewSocket(true);
    EXPECT_NE(INVALID_SOCKET, listen_sock);

    const char* host_ip = "127.0.0.1";
    uint16_t host_port = 50020;
    bool b = ListenOnPort(listen_sock, host_ip, host_port, 5);

    // init lc
    CLongConn lc;
    uint32_t max_sessions = 5;
    const char* host = "127.0.0.1";
    uint16_t port = 50021;
    float timeout = static_cast<float>(0.01);
    bool break_epoll_wait = true;
    b = lc.Init(max_sessions, host, port,
                timeout, NULL, &break_epoll_wait);
    EXPECT_TRUE(b);

    // create long conn session
    LongConnHandle hSession = lc.CreateLongConnSession(host_ip, host_port);
    LongConnSession* session =
        reinterpret_cast<LongConnSession*>(hSession.handle);
    EXPECT_NE(INVALID_SOCKET, session->sock);
    EXPECT_NE((uint32_t)0, hSession.serial_num);

    // create Long conn node data
    LongConnNode* data_node = mempool_NEW(LongConnNode);
    data_node->data.SetData("send data");

    // send node data
    LONGCONN_ERR err = lc.SendNodeData(hSession, data_node);
    EXPECT_EQ(LERR_OK, err);

    // close long conn session
    lc.CloseLongConnSession(hSession);
    session = reinterpret_cast<LongConnSession*>(hSession.handle);
    EXPECT_EQ(INVALID_SOCKET, session->sock);
    EXPECT_NE(hSession.serial_num, session->long_conn_serial_num);

    lc.Uninit();
    CLOSESOCKET(listen_sock);
}

// @brief:  test method InitLongConn in class CLongConnTasks
TEST(CLongConnTasks, InitLongConn) {
    CLongConnTasks tasks;
    uint32_t max_sessions = 5;
    const char* listen_host = "127.0.0.1";
    uint16_t port = 50031;
    float timeout = static_cast<float>(0.01);
    bool b = tasks.InitLongConn(0, max_sessions, listen_host, port, timeout);
    EXPECT_TRUE(b);

    SOCKET listen_sock = NewSocket(true);
    EXPECT_NE(INVALID_SOCKET, listen_sock);

    // Init twice, fail
    b = tasks.InitLongConn(0, max_sessions, listen_sock, timeout);
    EXPECT_FALSE(b);

    tasks.Uninit();
    b = tasks.InitLongConn(0, max_sessions, listen_sock, timeout);
    EXPECT_TRUE(b);

    tasks.Uninit();
    CLOSESOCKET(listen_sock);
}

// @brief:  test method SendData in class CLongConnTasks
TEST(CLongConnTasks, SendData) {
    // start server
    SOCKET  listen_sock = NewSocket(true);
    EXPECT_NE(INVALID_SOCKET, listen_sock);

    const char* host_ip = "127.0.0.1";
    uint16_t host_port = 50020;
    bool b = ListenOnPort(listen_sock, host_ip, host_port, 5);

    // init lc
    CLongConn lc;
    uint32_t max_sessions = 5;
    float timeout = static_cast<float>(0.01);

    // init long conn tasks
    CLongConnTasks lc_tasks;
    const char* listen_host = "127.0.0.1";
    uint16_t listen_port = 50022;
    b = lc_tasks.InitLongConn(0, max_sessions,
                              listen_host, listen_port, timeout);
    EXPECT_TRUE(b);

    // create long conn session
    LongConnHandle hSession = lc_tasks.CreateLongConnSession(host_ip,
                              host_port);
    LongConnSession* session =
        reinterpret_cast<LongConnSession*>(hSession.handle);

    EXPECT_NE(INVALID_SOCKET, session->sock);
    EXPECT_NE((uint32_t)0, hSession.serial_num);

    LTasksGroup tasks;

    CBaseProtocolPack pack;
    pack.Init();
    uint32_t seq = 0;
    // prepare request package
    pack.ResetContent();
    pack.SetSeq(seq);
    pack.SetServiceType(500);

    char buff[128]= {0};
    safe_snprintf(buff, sizeof(buff), "test send ok! :)\r\n");
    pack.SetKey(100, reinterpret_cast<char*>(buff));

    unsigned char* data = 0;
    uint32_t len = 0;
    pack.GetPackage(&data, &len);

    // set task parameter
    // set send package
    // set task 1
    tasks.m_tasks[0].SetSendData(data, len);
    // set session
    tasks.m_tasks[0].SetConnSession(hSession);
    // need recv response
    tasks.m_tasks[0].SetNeedResponse(1);

    // set task 2
    tasks.m_tasks[1].SetSendData(data, len);
    // set session
    tasks.m_tasks[1].SetConnSession(hSession);
    // need recv response
    tasks.m_tasks[1].SetNeedResponse(1);

    // set group parameters
    tasks.SetValidTasks(2);
    b = lc_tasks.SendData(&tasks);
    EXPECT_TRUE(b);

    // more tasks than actual, fail
    tasks.SetValidTasks(3);
    b = lc_tasks.SendData(&tasks);
    EXPECT_FALSE(b);

    // less tasks than actual, ok
    tasks.SetValidTasks(1);
    b = lc_tasks.SendData(&tasks);
    EXPECT_TRUE(b);

    lc_tasks.CloseLongConnSession(hSession);

    // lc_tasks.RoutineLongConn(100);
    lc_tasks.UninitLongConn();
    CLOSESOCKET(listen_sock);
    pack.Uninit();
}

// @brief:  test method CreateLongConnSession &
//          RemoveLongConnSession in class CLongConnTasks
TEST(CLongConnTasks, CreateLongConnSession) {
    // start server
    SOCKET  listen_sock = NewSocket(true);
    EXPECT_NE(INVALID_SOCKET, listen_sock);

    const char* host_ip = "127.0.0.1";
    uint16_t host_port = 50020;
    bool b = ListenOnPort(listen_sock, host_ip, host_port, 5);

    // init lc
    CLongConnTasks lc_tasks;
    uint32_t max_sessions = 5;
    const char* host = "127.0.0.1";
    uint16_t port = 50021;
    float timeout = static_cast<float>(0.01);
    b = lc_tasks.InitLongConn(0, max_sessions, host, port, timeout);
    EXPECT_TRUE(b);

    // create long conn session
    LongConnHandle hSession = lc_tasks.CreateLongConnSession(host_ip,
                              host_port);
    LongConnSession* session =
        reinterpret_cast<LongConnSession*>(hSession.handle);
    EXPECT_NE(INVALID_SOCKET, session->sock);
    EXPECT_NE((uint32_t)0, hSession.serial_num);

    // close long conn session
    // lc_tasks.RemoveLongConnSession(hSession);
    lc_tasks.CloseLongConnSession(hSession);
    session = reinterpret_cast<LongConnSession*>(hSession.handle);

    lc_tasks.RoutineLongConn(100);
    lc_tasks.UninitLongConn();
    lc_tasks.Release();

    EXPECT_NE(hSession.serial_num, session->long_conn_serial_num);

    CLOSESOCKET(listen_sock);
}

// @brief:  test method GetPeerName in class CLongConnTasks
TEST(CLongConnTasks, GetPeerName) {
    // start server
    SOCKET  listen_sock = NewSocket(true);
    EXPECT_NE(INVALID_SOCKET, listen_sock);

    const char* host_ip = "127.0.0.1";
    uint16_t host_port = 50020;
    bool b = ListenOnPort(listen_sock, host_ip, host_port, 5);

    // init lc
    CLongConnTasks lc;
    uint32_t max_sessions = 5;
    const char* host = "127.0.0.1";
    uint16_t port = 50021;
    float timeout = static_cast<float>(0.01);
    b = lc.InitLongConn(0, max_sessions, host, port, timeout);
    EXPECT_TRUE(b);

    // create long conn session
    LongConnHandle hSession = lc.CreateLongConnSession(host_ip, host_port);
    LongConnSession* session =
        reinterpret_cast<LongConnSession*>(hSession.handle);
    EXPECT_NE(INVALID_SOCKET, session->sock);
    EXPECT_NE((uint32_t)0, hSession.serial_num);

    // get peer name of long conn
    uint32_t peer_host = 0;
    uint16_t peer_port = 0;
    b = lc.GetPeerName(hSession, &peer_host, &peer_port);
    EXPECT_TRUE(b);
    EXPECT_EQ(inet_addr(host_ip), peer_host);
    // EXPECT_EQ(H2NS(host_port), peer_port);

    // close long conn session
    lc.CloseLongConnSession(hSession);
    session = reinterpret_cast<LongConnSession*>(hSession.handle);

    lc.RoutineLongConn(100);
    lc.UninitLongConn();
    lc.Release();
    EXPECT_NE(hSession.serial_num, session->long_conn_serial_num);
    CLOSESOCKET(listen_sock);
}

// @brief:  test method GetSockHandle in class CLongConnTasks
TEST(CLongConnTasks, GetSockHandle) {
    // start server
    SOCKET  listen_sock = NewSocket(true);
    EXPECT_NE(INVALID_SOCKET, listen_sock);

    const char* host_ip = "127.0.0.1";
    uint16_t host_port = 50020;
    bool b = ListenOnPort(listen_sock, host_ip, host_port, 5);

    // init lc
    CLongConnTasks lc;
    uint32_t max_sessions = 5;
    const char* host = "127.0.0.1";
    uint16_t port = 50021;
    float timeout = static_cast<float>(0.01);
    b = lc.InitLongConn(0, max_sessions, host, port, timeout);
    EXPECT_TRUE(b);

    // create long conn session
    LongConnHandle hSession = lc.CreateLongConnSession(host_ip, host_port);
    LongConnSession* session =
        reinterpret_cast<LongConnSession*>(hSession.handle);
    EXPECT_NE(INVALID_SOCKET, session->sock);
    EXPECT_NE((uint32_t)0, hSession.serial_num);

    // get sock handle of long conn
    SOCKET sock = lc.GetSockHandle(hSession);
    EXPECT_EQ(session->sock, sock);

    // close long conn session
    lc.CloseLongConnSession(hSession);
    session = reinterpret_cast<LongConnSession*>(hSession.handle);

    lc.RoutineLongConn(100);
    lc.UninitLongConn();
    lc.Release();
    EXPECT_NE(hSession.serial_num, session->long_conn_serial_num);
    CLOSESOCKET(listen_sock);
}
