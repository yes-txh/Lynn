////////////////////////////////////////////////////////////////////////
// fake_epoll_test.cc
// @brief:     Test operations in fake_epoll
// @author:  fatliu@tencent
// @time:     2010-10-20
// @version: 1.0
////////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

#ifdef WIN32
int32_t TestFakeEpoll(int32_t argc, char** argv)
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

#ifdef WIN32
// @brief:     test create fake_epoll
TEST(TestFakeEpollCreate, FakeEpollCreate) {
    EPOLLHANDLE handle = 0;
    int32_t size = 5;
    handle = fake_epoll_create(size);
    CHECK_NE(reinterpret_cast<THANDLE>(handle), INVALID_HANDLE);
}

// @brief:     test close fake_epoll
TEST(TestFakeEpollClose, FakeEpollClose) {
    EPOLLHANDLE handle = 0;
    int32_t size = 5;
    handle = fake_epoll_create(size);
    CHECK_NE(reinterpret_cast<THANDLE>(handle), INVALID_HANDLE);

    int32_t ret = fake_epoll_close(handle);
    CHECK_EQ(0, ret);
}

// @brief:     test add event to fake_epoll
TEST(TestFakeEpollCtl, Add) {
    EPOLLHANDLE handle = 0;
    int32_t size = 5;
    handle = fake_epoll_create(size);
    CHECK_NE(reinterpret_cast<THANDLE>(handle), INVALID_HANDLE);

    int32_t ret = -1;
    // test add
    int32_t op = EPOLL_CTL_ADD;
    SOCKET sock_add = NewSocket(true);
    CHECK_NE((uint32_t)sock_add, INVALID_SOCKET);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sock_add;
    ret = fake_epoll_ctl(handle, op, sock_add, &ev);
    CHECK_EQ(0, ret);
    CHECK(epoll_fd_exist(handle, sock_add));

    // add sock_add again, fail
    ret = fake_epoll_ctl(handle, op, sock_add, &ev);
    CHECK_EQ(-1, ret);

    // add another sock
    SOCKET sock_add1 = NewSocket(true);
    CHECK_NE((uint32_t)sock_add1, INVALID_SOCKET);
    struct epoll_event ev1;
    ev1.events = EPOLLIN;
    ev1.data.fd = sock_add1;
    ret = fake_epoll_ctl(handle, op, sock_add1, &ev1);
    CHECK_EQ(0, ret);
    CHECK(epoll_fd_exist(handle, sock_add1));

    CloseSocket(sock_add1);
    CloseSocket(sock_add);
    ret = fake_epoll_close(handle);
    CHECK_EQ(0, ret);
}

// @brief:     test modify event from fake_epoll
TEST(TestFakeEpollCtl, Modify) {
    EPOLLHANDLE handle = 0;
    int32_t size = 5;
    handle = fake_epoll_create(size);
    CHECK_NE(reinterpret_cast<THANDLE>(handle), INVALID_HANDLE);

    int32_t ret = -1;
    // add
    int32_t op = EPOLL_CTL_ADD;
    SOCKET sock_add = NewSocket(true);
    CHECK_NE((uint32_t)sock_add, INVALID_SOCKET);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sock_add;
    ret = fake_epoll_ctl(handle, op, sock_add, &ev);
    CHECK_EQ(0, ret);
    CHECK(epoll_fd_exist(handle, sock_add));

    // test modify
    op = EPOLL_CTL_MOD;
    SOCKET sock_mod = sock_add;
    ev.events = EPOLLOUT;
    ret = fake_epoll_ctl(handle, op, sock_mod, &ev);
    CHECK_EQ(0, ret);
    FakeEPOLL_HANDLE* fake_handle = static_cast<FakeEPOLL_HANDLE*>(handle);
    CHECK_EQ(fake_handle->m_valid_head->node.epollevent.data.u32, ev.data.u32);
    CHECK(epoll_fd_exist(handle, sock_mod));

    // modify ev point to other data
    struct epoll_event ev1;
    ev1.events = EPOLLIN;
    ev1.data.fd = 1;
    ret = fake_epoll_ctl(handle, op, sock_mod, &ev1);
    CHECK_EQ(-1, ret);

    CloseSocket(sock_add);
    ret = fake_epoll_close(handle);
    CHECK_EQ(0, ret);
}

// @brief:     test delete event from fake_epoll
TEST(TestFakeEpollCtl, Delete) {
    EPOLLHANDLE handle = 0;
    int32_t size = 5;
    handle = fake_epoll_create(size);
    CHECK_NE(reinterpret_cast<THANDLE>(handle), INVALID_HANDLE);

    int32_t ret = -1;
    // add
    int32_t op = EPOLL_CTL_ADD;
    SOCKET sock_add = NewSocket(true);
    CHECK_NE((uint32_t)sock_add, INVALID_SOCKET);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sock_add;
    ret = fake_epoll_ctl(handle, op, sock_add, &ev);
    CHECK_EQ(0, ret);
    CHECK(epoll_fd_exist(handle, sock_add));

    // add another sock
    SOCKET sock_add1 = NewSocket(true);
    CHECK_NE((uint32_t)sock_add1, INVALID_SOCKET);
    struct epoll_event ev1;
    ev1.events = EPOLLIN;
    ev1.data.fd = sock_add1;
    ret = fake_epoll_ctl(handle, op, sock_add1, &ev1);
    CHECK_EQ(0, ret);
    CHECK(epoll_fd_exist(handle, sock_add1));

    // test del, nothing about ev, just del sock
    op = EPOLL_CTL_DEL;
    SOCKET sock_del = sock_add1;
    ret = fake_epoll_ctl(handle, op, sock_del, &ev1);
    CHECK_EQ(0, ret);
    CHECK(!epoll_fd_exist(handle, sock_del));

    // del again, fail
    ret = fake_epoll_ctl(handle, op, sock_del, &ev1);
    CHECK_EQ(-1, ret);

    sock_del = sock_add;
    ret = fake_epoll_ctl(handle, op, sock_del, &ev);
    CHECK_EQ(0, ret);

    CloseSocket(sock_add1);
    CloseSocket(sock_add);
    ret = fake_epoll_close(handle);
    CHECK_EQ(0, ret);
}

// @brief:     test fake_epoll wait
TEST(TestFakeEpollWait, FakeEpollWait) {
    EPOLLHANDLE handle = 0;
    int32_t size = 5;
    handle = fake_epoll_create(size);
    CHECK_NE(reinterpret_cast<THANDLE>(handle), INVALID_HANDLE);
    int32_t ret = -1;

    // add
    int32_t op = EPOLL_CTL_ADD;
    SOCKET sock_add = NewSocket(true);
    CHECK_NE(static_cast<uint32_t>(sock_add), INVALID_SOCKET);
    struct epoll_event ev[10];
    for (int32_t i = 0; i < 10; i++) {
        ev[i].events = EPOLLIN;
        ev[i].data.fd = i;
    }

    ret = fake_epoll_ctl(handle, op, sock_add,
                         reinterpret_cast<epoll_event*>(&ev));
    CHECK_EQ(0, ret);
    CHECK(epoll_fd_exist(handle, sock_add));

    // test fake epoll wait
    ret = fake_epoll_wait(handle, reinterpret_cast<epoll_event*>(ev), 10, 5);
    CHECK_EQ(0, ret);
    ret = fake_epoll_close(handle);
    CHECK_EQ(0, ret);
}

// @brief:     test function get_valid_count
TEST(TestGetValidCount, GetValidCount) {
    EPOLLHANDLE handle = 0;
    int32_t size = 5;
    handle = fake_epoll_create(size);
    CHECK_NE(reinterpret_cast<THANDLE>(handle), INVALID_HANDLE);
    int32_t ret = -1;

    struct epoll_event ev[10];
    for (int32_t i = 0; i < 10; i++) {
        if (i % 2) {
            ev[i].events = EPOLLIN;
        } else if (i % 3) {
            ev[i].events = 0;
        } else {
            ev[i].events = EPOLLOUT;
        }
    }

    ret = get_valid_count(reinterpret_cast<epoll_event*>(&ev), 10);
    CHECK_EQ(2, ret);

    ev[2].events = EPOLLIN;
    ret = get_valid_count(reinterpret_cast<epoll_event*>(&ev), 10);
    CHECK_EQ(4, ret);

    ret = fake_epoll_close(handle);
    CHECK_EQ(0, ret);
}

// @brief:     test function epoll_FD_SET
TEST(TestEpollFdSet, EpollFdSet) {
    epoll_fd_set    set;
    set.num_fd_count = FD_SETSIZE;
    for (int32_t i = 0; i < FD_SETSIZE; i++) {
        set.fd_array[i] = i;
        set.data_array[i].fd = i;
    }

    for (int32_t i = 0; i < FD_SETSIZE; i++) {
        CHECK_EQ(i, static_cast<int32_t>(set.data_array[i].fd));
    }

    epoll_FD_SET(3, &set, set.data_array[7]);
    CHECK_EQ(3, static_cast<int32_t>(set.data_array[3].fd));

    set.num_fd_count = 3;
    epoll_FD_SET(3, &set, set.data_array[7]);
    CHECK_EQ(7, static_cast<int32_t>(set.data_array[3].fd));
    CHECK_EQ(4, static_cast<int32_t>(set.num_fd_count));

    epoll_FD_SET(5, &set, set.data_array[7]);
    CHECK_EQ(5, static_cast<int32_t>(set.data_array[5].fd));
    CHECK_EQ(7, static_cast<int32_t>(set.data_array[4].fd));
    CHECK_EQ(5, static_cast<int32_t>(set.num_fd_count));
}
#endif
