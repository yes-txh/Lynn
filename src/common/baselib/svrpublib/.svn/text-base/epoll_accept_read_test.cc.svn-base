//////////////////////////////////////////////////////////////////////////
// epoll_accept_read_test.cc
// @brief:     Test base class CEpollAcceptRead_T
// @author:  fatliu@tencent
// @time:     2010-10-27
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
class MyEpollSvr:public CEpollAcceptRead_T<uint32_t>, public CXThreadBase {
public:
    explicit MyEpollSvr(uint32_t max_fds) :
        CEpollAcceptRead_T<uint32_t>(max_fds) {}

    bool Init(const char* host, uint16_t port,
              uint32_t max_recv_request_timeout,
              uint16_t min_binary_head,
              uint16_t listen_backlog);

    void Uninit();

    virtual void Routine() {
        AcceptReadRoutine();
    }

    // send back whatever received
    virtual bool OnReceivedRequest(SOCKET fd,
                                   const char* request_pack,
                                   uint32_t len,
                                   time_t t0);
private:
};

#ifdef WIN32
int32_t TestEpollAcceptRead(int32_t argc, char** argv)
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

// @brief:     Test epoll sever thread
TEST(CEpollAcceptRead, EpollSvrThread) {
    bool b = false;
    const char* ip = "127.0.0.1";
    uint16_t port = 50010;

    // server thread
    MyEpollSvr epoll_svr(10);
    b = epoll_svr.Init(ip, port, 1, 0, 5);
    CHECK(b);

    epoll_svr.StartThread();
    VLOG(3) << "start server thread...";

    // client
    // connet to server
    SOCKET fd           = NewSocket(true);
    CHECK_NE((uint32_t)fd, INVALID_SOCKET);
    b = ConnectToHost(fd, inet_addr(ip), H2NS(port), 1);
    CHECK(b);
    VLOG(3) << "connect to server...";

    // send data to server
    const char* data    = "test send data...\r\n";
    uint32_t len        = static_cast<uint32_t>(strlen(data)) - 1;
    uint32_t sent = 0;
    b = SendFixedBytes(fd,
                       data,
                       len,
                       &sent);
    CHECK(b);
    CHECK_EQ(len, sent);

    // receive data
    char buff[256] = {0};
    int32_t ret = 0;
    while ((ret = recv(fd, buff, 256, 0)) == -1) {
        XSleep(1000);
    }

    CHECK_EQ(len, ret);
    CHECK_EQ(0, strncmp(reinterpret_cast<const char*>(buff),
                        reinterpret_cast<const char*>(data),
                        len));

    CloseSocket(fd);

    epoll_svr.EndThread();
    epoll_svr.Uninit();
}

bool MyEpollSvr::Init(const char* host, uint16_t port,
                      uint32_t max_recv_request_timeout,
                      uint16_t min_binary_head,
                      uint16_t listen_backlog) {
    bool b = false;
    b = InitListen(host, port,
                   max_recv_request_timeout,
                   min_binary_head,
                   listen_backlog);
    CHECK(b);
    return true;
}

void MyEpollSvr::Uninit() {
    CEpollAcceptRead_T<uint32_t>::Uninit();
}

bool MyEpollSvr::OnReceivedRequest(SOCKET fd,
                                   const char* request_pack,
                                   uint32_t len,
                                   time_t t0) {
    bool b = false;

    // send data
    b = SendFixedBytes(fd, request_pack, len, 0, false);
    CHECK(b);

    return true;
}
