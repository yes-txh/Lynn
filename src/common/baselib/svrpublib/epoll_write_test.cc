//////////////////////////////////////////////////////////////////////////
// epoll_write_test.cc
// @brief:     Test class CEpollWrite_T as client write server thread
// @author:  fatliu@tencent
// @time:    2010-10-27
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
class MySvrThread:public CEpoll, public CXThreadBase {
public:
    MySvrThread(uint32_t epoll_size,
                const char* listen_host,
                uint16_t listen_port);

    virtual ~MySvrThread();

    // inherit from CXThreadBase
    virtual void Routine(uint32_t epoll_wait_millesecs) {
        CEpoll::Routine(epoll_wait_millesecs);
    }

    virtual void Routine() {
        Routine(1000);
    }

    virtual void    OnAccept(SOCKET listen_sock);
    virtual void    OnEvent(const epoll_event* ev);

private:
    // send back whatever received
    bool    OnUserRequest(SOCKET sock, const unsigned char* data, uint32_t len);

    CBaseProtocolPack         m_pack;
    CBaseProtocolUnpack     m_unpack;
    char*                           m_data;
    uint32_t                       m_data_len;
};

#ifdef WIN32
int32_t TestEpollWrite(int32_t argc, char** argv)
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

// @brief:     Test epoll server thread with client objects CEpollWrite_T
TEST(TestEpollWrite, EpollWrite) {
    const char* ip = "127.0.0.1";
    uint16_t port = 50012;

    MySvrThread thread(5, ip, port);
    thread.StartThread();
    VLOG(3) << "start server...";

    bool b = false;
    CEpollWrite_T<20, 256, uint32_t> epoll_write;

    b = epoll_write.Init(10, 1);
    CHECK(b);

    // test function IsBufferReady
    b = epoll_write.IsBufferReady();
    CHECK(b);

    SOCKET fd = NewSocket(true);
    CHECK_NE((uint32_t)fd, INVALID_SOCKET);
    b = ConnectToHost(fd, inet_addr(ip), H2NS(port), 1);
    CHECK(b);
    VLOG(3) << "connect to server...";

    // prepare package data
    time_t t0 = time(0);
    const char* data = "test send data...\r\n";

    CBaseProtocolPack pack;
    CBaseProtocolUnpack unpack;
    pack.Init();
    CHECK(unpack.Init());
    pack.ResetContent();
    pack.SetSeq(0); // set seq
    pack.SetServiceType(1028); // SS_SEARCH2SB_RSP
    pack.SetKey(100, data);

    // get response package
    unsigned char* pack_data = 0;
    uint32_t pack_len = 0;
    pack.GetPackage(&pack_data, &pack_len);
    b = epoll_write.SetOutputData(fd, t0, true, pack_data, pack_len);
    CHECK(b);

    epoll_write.Routine();

    // receive data
    char buff[256] = {0};
    int32_t ret = 0;
    while ((ret = recv(fd, buff, 256, 0)) == -1) {
        XSleep(1000);
    }

    unpack.AttachPackage((unsigned char*)buff, ret);
    b = unpack.Unpack();
    CHECK(b);
    unsigned char* val = 0;
    uint32_t val_len = 0;
    unpack.GetVal(123, &val, &val_len);

    // compare data
    CHECK_EQ(0, strcmp(reinterpret_cast<const char*>(val),
                       reinterpret_cast<const char*>(data)));

    pack.Uninit();
    unpack.Uninit();

    // core...
    // epoll_write.Uninit();
    thread.EndThread();
    CloseSocket(fd);
}

MySvrThread::MySvrThread(uint32_t epoll_size,
                         const char* listen_host,
                         uint16_t listen_port)
    :m_data_len(0) {
    m_pack.Init();
    CHECK(m_unpack.Init());
    m_data = new char[256];
    memset(m_data, 0, 256);
    CEpoll::EpollInit(epoll_size, listen_host, listen_port);
}

MySvrThread::~MySvrThread() {
    delete[] m_data;
    m_pack.Uninit();
    m_unpack.Uninit();
}

void MySvrThread::OnAccept(SOCKET listen_sock) {
    struct sockaddr_in from_addr;
    SOCKET new_sock = AcceptNewConnection(listen_sock, &from_addr);
    CHECK_NE((uint32_t)listen_sock, INVALID_SOCKET);

    epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
    ev.data.ptr = reinterpret_cast<void*>(new_sock);
    CHECK(AddToEpoll(new_sock, &ev));
}

void MySvrThread::OnEvent(const epoll_event* ev) {
    uint32_t bytes = 0;
    while ((bytes = recv(ev->data.fd, m_data, 256, 0)) == -1) {
        XSleep(1000);
    }
    CHECK_NE(-1, bytes);
    m_data_len = bytes;

    OnUserRequest(ev->data.fd, (const unsigned char*)m_data, m_data_len);
}

bool MySvrThread::OnUserRequest(SOCKET sock,
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
