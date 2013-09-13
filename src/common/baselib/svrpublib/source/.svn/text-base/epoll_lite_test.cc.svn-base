//////////////////////////////////////////////////////////////////////////
// epoll_lite_test.cc
// @brief:     Test base class CEpoll
// @author:  fatliu@tencent
// @time:    2010-09-28
// @version: 1.0
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#define _FAKE_EPOLL
#endif

#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

// @brief:     Server thread
class MyEpollSvrThread:public CEpoll, public CXThreadBase {
public:
    MyEpollSvrThread(uint32_t epoll_size,
                     const char* listen_host,
                     uint16_t listen_port);

    virtual ~MyEpollSvrThread();

    virtual void    Routine(uint32_t epoll_wait_millesecs) {
        CEpoll::Routine(epoll_wait_millesecs);
    }

    // inherit from CXThreadBase
    virtual void    Routine() {
        Routine(1000);
    }

    virtual void    OnAccept(SOCKET listen_sock);
    virtual void    OnEvent(const epoll_event* ev);

private:
    // send back whatever received
    bool    OnUserRequest(SOCKET sock,
                          const unsigned char* data,
                          uint32_t len);

    CBaseProtocolPack         m_pack;
    CBaseProtocolUnpack     m_unpack;
    char*                           m_data;
    uint32_t                       m_data_len;
};

// @brief:     Client object
class MyClientObj {
public:
    MyClientObj(SOCKET sock, const char* listen_host, uint16_t listen_port);
    ~MyClientObj();
    void    SendData(const unsigned char* data, uint32_t len, uint32_t seq);
    bool    ReceiveData();

private:
    // compare receive data to send data
    bool    OnUserRequest(const unsigned char* data, uint32_t len);

    CBaseProtocolPack         m_pack;
    CBaseProtocolUnpack     m_unpack;
    char*                           m_data;
    uint32_t                       m_data_len;
    SOCKET                       m_sock;
};

#ifdef WIN32
int32_t TestEpollLite(int32_t argc, char** argv)
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

// @brief:     Test basic operations in epoll, such as add¡¢modify and delete
TEST(TestEpoll, BasicOperations) {
    CEpoll epoll;
    uint32_t epoll_size = 10;
    const char* listen_host = "127.0.0.1";
    uint16_t listen_port = 55000;
    uint16_t listen_baklog = 1024;
    bool break_epoll_wait = 0;
    bool b = false;

    // create a listen socket
    SOCKET listen_sock = NewSocket(true);
    CHECK_NE((uint32_t)listen_sock, INVALID_SOCKET);
    b = ListenOnPort(listen_sock, listen_host, listen_port, listen_baklog);
    CHECK(b);
    b = epoll.EpollInit(epoll_size, listen_sock, &break_epoll_wait);
    CHECK(b);

    // test get listen handle
    SOCKET sock2 = epoll.GetListenHandle();
    CHECK_EQ((int32_t)sock2, (int32_t)listen_sock);

    SOCKET sock[10];
    for (uint32_t u = 0; u < 10; u++) {
        sock[u] = NewSocket(true);
        CHECK_NE((uint32_t)sock[u], INVALID_SOCKET);
        epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.ptr = &sock[u];
        // test add to epoll
        b = epoll.AddToEpoll(sock[u], &ev);
        CHECK(b);

        // test delete from epoll
        if (u % 2) {
            b = epoll.DeleteFromEpoll(sock[u]);
            CHECK(b);
        } else {// test modify from epoll
            ev.events = EPOLLIN | EPOLLOUT;
            b = epoll.ModifyFromEpoll(sock[u], &ev);
            CHECK(b);
        }
    }

    // undo...client project

    SOCKET client_sock = NewSocket(true);
    CHECK_NE((uint32_t)client_sock, INVALID_SOCKET);
    b = ConnectToHost(client_sock,
                      inet_addr(listen_host),
                      H2NS(listen_port),
                      1);
    CHECK(b);

    SOCKET* sock_pair = 0;
    epoll.GetNotifyWriteSockOfSocketPair(&sock_pair);
    CHECK_NE((int32_t)*sock_pair, (int32_t)INVALID_SOCKET);

    CLOSESOCKET(sock2);
    CLOSESOCKET(client_sock);
}

// @brief:     Test epoll sever thread
TEST(TestEpoll, MyEpollSvrThread) {
    const char* ip = "127.0.0.1";
    uint16_t port = 50011;

    MyEpollSvrThread thread(5, ip, port);
    thread.StartThread();

    SOCKET socket_handle = INVALID_SOCKET;
    socket_handle = NewSocket(true);
    CHECK_NE(static_cast<uint32_t>(socket_handle), INVALID_SOCKET);
    CHECK(ConnectToHost(socket_handle, inet_addr(ip), H2NS(port), 1));

    // send data
    CBaseProtocolPack pack;
    pack.ResetContent();
    pack.SetSeq(0);
    pack.SetServiceType(500);

    char buff[128]= {0};
    safe_snprintf(buff, sizeof(buff), "test send ok");
    pack.SetKey(100, buff);

    const char* data = NULL;
    uint32_t len = 0;
    pack.GetPackage(&data, &len);

    bool b = SendFixedBytes(socket_handle, data, len, 0, false);
    CHECK(b);

    char buff_rcv[256] = {0};
    uint32_t len_rcv = 256;

    // receive data
    int32_t ret = 0;
    while ((ret = recv(socket_handle, buff_rcv, len_rcv, 0)) == -1) {
        XSleep(1000);
    }

    CBaseProtocolUnpack unpack;
    CHECK(unpack.Init());
    unpack.AttachPackage((unsigned char*)buff_rcv, len_rcv);
    b = unpack.Unpack();
    CHECK(b);
    char* val = NULL;
    uint32_t val_len = 0;
    unpack.GetVal(123, &val, &val_len);

    // compare
    CHECK_EQ(0, strcmp("test send ok", (const char*)val));
    unpack.Uninit();

    XSleep(500);
    thread.EndThread();
    CLOSESOCKET(socket_handle);
}

// @brief:     Test epoll server thread with multi client objects
TEST(TestEpoll, MyEpollSvrAndClient) {
    const char* ip = "127.0.0.1";
    uint16_t port = 50010;

    MyEpollSvrThread thread(5, ip, port);
    thread.StartThread();
    VLOG(3) << "start server...";

    SOCKET socket_handle1 = NewSocket(true);
    CHECK_NE(static_cast<uint32_t>(socket_handle1), INVALID_SOCKET);
    MyClientObj client_obj1(socket_handle1, ip, port);

    SOCKET socket_handle2 = NewSocket(true);
    CHECK_NE(static_cast<uint32_t>(socket_handle2), INVALID_SOCKET);
    MyClientObj client_obj2(socket_handle2, ip, port);
    VLOG(3) << "start client...";

    uint32_t seq = 0;
    for (uint32_t i = 0; i < 10; i++) {
        seq++;
        char buff[128] = {0};

        // client1 send data
        safe_snprintf(buff, sizeof(buff), "[1] test send ok count:%u :)", i);
        client_obj1.SendData(reinterpret_cast<const unsigned char*>(buff),
                             static_cast<uint32_t>(strlen(buff) + 1),
                             seq);

        // client2 send data
        safe_snprintf(buff, sizeof(buff), "[2] test send ok count:%u :)", i);
        client_obj2.SendData(reinterpret_cast<const unsigned char*>(buff),
                             static_cast<uint32_t>(strlen(buff) + 1),
                             seq);

        // client1 receive data
        while (!client_obj1.ReceiveData()) {
            XSleep(1000);
        }

        // client2 receive data
        while (!client_obj2.ReceiveData()) {
            XSleep(1000);
        }
    }
    XSleep(500);
    thread.EndThread();
}

MyEpollSvrThread::MyEpollSvrThread(uint32_t epoll_size,
                                   const char* listen_host,
                                   uint16_t listen_port)
    :m_data_len(0) {
    m_pack.Init();
    CHECK(m_unpack.Init());
    m_data = new char[256];
    memset(m_data, 0, 256);
    CEpoll::EpollInit(epoll_size, listen_host, listen_port);
}
MyEpollSvrThread::~MyEpollSvrThread() {
    delete[] m_data;
    m_pack.Uninit();
    m_unpack.Uninit();
}

void MyEpollSvrThread::OnAccept(SOCKET listen_sock) {
    struct sockaddr_in from_addr;
    SOCKET new_sock = AcceptNewConnection(listen_sock, &from_addr);
    CHECK_NE((uint32_t)listen_sock, INVALID_SOCKET);

    epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
    ev.data.ptr = reinterpret_cast<void*>(new_sock);
    CHECK(AddToEpoll(new_sock, &ev));
}

void MyEpollSvrThread::OnEvent(const epoll_event* ev) {
    int32_t bytes = recv(ev->data.fd, m_data, 256, 0);
    CHECK_NE(0, bytes);
    CHECK_NE(-1, bytes);
    m_data_len = bytes;

    OnUserRequest(ev->data.fd, (const unsigned char*)m_data, m_data_len);
}

bool MyEpollSvrThread::OnUserRequest(SOCKET sock,
                                     const unsigned char* data,
                                     uint32_t len) {
    bool b = false;

    // receive data from client
    m_unpack.AttachPackage((unsigned char*)data, len);
    b = m_unpack.Unpack();
    CHECK(b);
    unsigned char* val = 0;
    uint32_t val_len = 0;
    m_unpack.GetVal(100, &val, &val_len);

    // prepare response data
    // prepare data
    uint32_t seq = m_unpack.GetSeq();
    m_pack.ResetContent();
    m_pack.SetSeq(seq); // set seq
    m_pack.SetServiceType(1028); // SS_SEARCH2SB_RSP
    m_pack.SetKey(123, val);

    // get response package
    const char* pack_data = NULL;
    uint32_t pack_len = 0;
    m_pack.GetPackage(&pack_data, &pack_len);

    // send data
    b = SendFixedBytes(sock, pack_data, pack_len, 0, false);
    CHECK(b);

    return true;
}

MyClientObj::MyClientObj(SOCKET sock,
                         const char* listen_host,
                         uint16_t listen_port)
    :m_data_len(0) {
    m_pack.Init();
    CHECK(m_unpack.Init());
    m_data = new char[256];
    memset(m_data, 0, 256);
    CHECK_NE((uint32_t)sock, INVALID_SOCKET);
    m_sock = sock;

    bool b = ConnectToHost(m_sock,
                           inet_addr(listen_host),
                           H2NS(listen_port),
                           1);
    CHECK(b);
}

MyClientObj::~MyClientObj() {
    delete[] m_data;
    m_pack.Uninit();
    m_unpack.Uninit();
    CLOSESOCKET(m_sock);
}

void MyClientObj::SendData(const unsigned char* data,
                           uint32_t len,
                           uint32_t seq) {
    CHECK(len);
    CHECK(data);
    memset(m_data, 0, 256);
    memcpy(m_data, data, len);
    m_data_len = len;

    // prepare data
    m_pack.ResetContent();
    m_pack.SetSeq(seq); // set seq
    m_pack.SetServiceType(1028); // SS_SEARCH2SB_RSP
    m_pack.SetKey(100, m_data);

    // get response package
    const char* pack_data = NULL;
    uint32_t pack_len = 0;
    m_pack.GetPackage(&pack_data, &pack_len);

    // send data
    uint32_t send = 0;
    bool b = SendFixedBytes(m_sock, pack_data, pack_len, &send, false);
    CHECK_EQ(send, pack_len);
    CHECK(b);
}

bool MyClientObj::ReceiveData() {
    char buff[256] = {0};
    int32_t ret = recv(m_sock, buff, 256, 0);
    if (ret == -1) {
        return false;
    } else {
        CHECK(OnUserRequest((const unsigned char*)buff, ret));
    }
    return true;
}

bool MyClientObj::OnUserRequest(const unsigned char* data, uint32_t len) {
    m_unpack.AttachPackage((unsigned char*)data, len);
    bool b = m_unpack.Unpack();
    CHECK(b);
    unsigned char* val = 0;
    uint32_t val_len = 0;
    m_unpack.GetVal(123, &val, &val_len);
    CHECK_EQ(val_len, m_data_len);
    CHECK_EQ(0, strcmp(reinterpret_cast<const char*>(val),
                       reinterpret_cast<const char*>(m_data)));
    VLOG(3) << val;
    return true;
}
