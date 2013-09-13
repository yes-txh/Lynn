//////////////////////////////////////////////////////////////////////////
// general_util_test.cc
// @brief:     test fuctions in general_util.cc
// @author:  fatliu@tencent
// @time:     2010-10-3
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
int32_t TestGeneralUtil(int32_t argc, char** argv)
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

// @brief:     test fuction safe_snprintf
TEST(TestSafeSnprintf, SafeSnprintf) {
    // normal
    const char* src = "Hello World!";
    char dst1[15] = {0};
    int32_t bytes = 0;
    bytes = safe_snprintf(dst1, sizeof(dst1), "%s", src);
    CHECK_EQ(12, bytes);

    // buffer too small
    // char dst2[10] = {0};
    // bytes = safe_snprintf(dst2, sizeof(dst2), "%s", src);
#ifdef WIN32
    // CHECK_EQ(0, bytes);
#else
    // CHECK_EQ(9, bytes);
#endif
}

// @brief:     test fuction safe_snprintf
TEST(TestLiteTime, LiteTime) {
    time_t t;
    time_t t1 = lite_time(&t);
    EXPECT_EQ(t, t1);

    time_t begin_time;
    time_t end_time;
    lite_time(&begin_time);
    XSleep(1100);
    lite_time(&end_time);
    EXPECT_TRUE(abs(static_cast<long>(1 - (end_time - begin_time))) <= 1);
}

// @brief:     test fuction  lite_gettimeofday
TEST(TestLiteGetTimeofDay, LiteGetTimeofDay) {
    // gettimeofday has been tested in TestGetTimeofDay()
#ifndef WIN32
    // undo
    struct timeval begin_time;
    struct timeval end_time;

    // threshold
    lite_gettimeofday(&begin_time, NULL);
    XSleep(1000);
    lite_gettimeofday(&end_time, NULL);
    int32_t interval = (end_time.tv_sec-begin_time.tv_sec) * 1000 +
                       (end_time.tv_usec-begin_time.tv_usec) / 1000;
    EXPECT_TRUE(abs(1000 - interval) < 10);

    // ??
    lite_gettimeofday(&begin_time, NULL);
    XSleep(100);
    lite_gettimeofday(&end_time, NULL);
    interval = (end_time.tv_sec-begin_time.tv_sec) * 1000 +
               (end_time.tv_usec-begin_time.tv_usec) / 1000;
    EXPECT_TRUE(abs(100-interval) < 10);

    // ???
    lite_gettimeofday(&begin_time, NULL);
    XSleep(1009);
    lite_gettimeofday(&end_time, NULL);
    interval = (end_time.tv_sec-begin_time.tv_sec) * 1000 +
               (end_time.tv_usec-begin_time.tv_usec)/1000;
    EXPECT_TRUE(abs(1009 - interval) < 10);
#endif
}

// @brief:     test fuction  Sleep/XSleep/XUSleep
TEST(TestSleep, Sleep) {
    // test XSleep
    struct timeval begin_time;
    struct timeval end_time;

    //  accurate to seconds
    lite_gettimeofday(&begin_time, NULL);
    XSleep(1000);
    lite_gettimeofday(&end_time, NULL);
    int32_t interval = (end_time.tv_sec-begin_time.tv_sec) * 1000 +
                       (end_time.tv_usec-begin_time.tv_usec) / 1000;
    EXPECT_TRUE(abs(1000 - interval) < 100);

    // threshold, > threshold, accurate to 10%
    lite_gettimeofday(&begin_time, NULL);
    XSleep(100);
    lite_gettimeofday(&end_time, NULL);
    interval = (end_time.tv_sec-begin_time.tv_sec) * 1000 +
               (end_time.tv_usec-begin_time.tv_usec)/1000;
    EXPECT_TRUE(abs(100 - interval) < 10);

    // test XUSleep ??? 5000 vs 31251
    lite_gettimeofday(&begin_time, NULL);
    XUSleep(500000);
    lite_gettimeofday(&end_time, NULL);
    interval = (end_time.tv_sec-begin_time.tv_sec) * 1000 +
               (end_time.tv_usec-begin_time.tv_usec)/1000;
    EXPECT_TRUE(abs(500 - interval) < 10);
}

// @brief:     test class AutoLibMgr
TEST(TestAutoLibMgr, AutoLibMgr) {
    CAutoLibGlobalVars auto_lib_mgr;
#if defined(_DEBUG) || defined(_DEBUG_COUNT)
    for (uint32_t u = 0; u < 3; u++) {
        auto_lib_mgr.AddRefAcceptedSockFDCount();
    }
    CHECK_EQ((uint32_t)3, auto_lib_mgr.m_num_accepted_tcp_count);

    for (uint32_t u = 0; u < 7; u++) {
        auto_lib_mgr.AddRefNewTCPFDCount();
    }
    CHECK_EQ((uint32_t)7, auto_lib_mgr.m_num_new_tcp_count);

    for (uint32_t u = 0; u < 5; u++) {
        auto_lib_mgr.AddRefNewUDPFDCount();
    }
    CHECK_EQ((uint32_t)5, auto_lib_mgr.m_num_new_udp_count);

    auto_lib_mgr.AddRefCloseFDCount(true, TCP_FD_NEW, (SOCKET)1234);
    CHECK_EQ((uint32_t)1, auto_lib_mgr.m_num_close_tcp_socket_count);
    CHECK_EQ((uint32_t)1, auto_lib_mgr.m_num_close_new_tcp_count);

    auto_lib_mgr.AddRefCloseFDCount(true, TCP_FD_ACCEPTED, (SOCKET)1234);
    CHECK_EQ((uint32_t)2, auto_lib_mgr.m_num_close_tcp_socket_count);
    CHECK_EQ((uint32_t)1, auto_lib_mgr.m_num_close_accepted_tcp_count);

    auto_lib_mgr.AddRefCloseFDCount(true, TCP_FD_UNKNOWN, (SOCKET)1234);
    CHECK_EQ((uint32_t)3, auto_lib_mgr.m_num_close_tcp_socket_count);
    CHECK_EQ((uint32_t)1, auto_lib_mgr.m_num_close_unknown_tcp_count);

    auto_lib_mgr.AddRefCloseFDCount(true, (TCP_FD_TYPE)5, (SOCKET)1234);
    CHECK_EQ((uint32_t)4, auto_lib_mgr.m_num_close_tcp_socket_count);

    auto_lib_mgr.AddRefCloseFDCount(false, TCP_FD_NEW, (SOCKET)1234);
    CHECK_EQ((uint32_t)1, auto_lib_mgr.m_num_close_udp_fd_count);

    auto_lib_mgr.AddRefCloseFDCount(false, (TCP_FD_TYPE)5, (SOCKET)1234);
    CHECK_EQ((uint32_t)2, auto_lib_mgr.m_num_close_udp_fd_count);
#endif
}

// @brief:     test class CStr
TEST(TestCStr, CStr) {
    //  test "Hello World!"
    CStr obj_str;
    const char* src = "Hello World!";
    const int32_t len = static_cast<int32_t>(strlen(src));

    obj_str.SetStr(src);
    const char* dst = obj_str.Value();
    CHECK_EQ(0, strcmp(src, dst));
    CHECK_EQ((uint32_t)len, obj_str.GetValidLen());

    obj_str.SetStr(src, len);
    dst = obj_str.Value();
    CHECK_EQ(0, strcmp(src, dst));
    CHECK_EQ((uint32_t)len, obj_str.GetValidLen());

    // test ""
    const char* src1 = "";
    const int32_t len1 = static_cast<int32_t>(strlen(src1));

    obj_str.SetStr(src1);
    const char* dst1 = obj_str.Value();
    CHECK_EQ(0, strcmp(src1, dst1));
    CHECK_EQ((uint32_t)len1, obj_str.GetValidLen());

    // a larger length
    obj_str.SetStr(src1, len1+100);
    dst1 = obj_str.Value();
    CHECK_EQ(0, strcmp(src1, dst1));
    CHECK_EQ((uint32_t)len1, obj_str.GetValidLen());
}
#ifdef WIN32
// @brief:     test functions shmget/shmat/shmdt
TEST(TestShareMemoryFuncs, ShareMemoryFuncs) {
    // file1
    char* filename = "e:\\test_file";
    key_t key0 = ftok(filename, 8);

    int32_t shmid0 = shmget(key0, 128, IPC_CREAT);
    CHECK_NE(0, shmid0);

    // unknow shmid
    void* shmaddr0 = shmat(123, &shmaddr0, IPC_CREAT);
    CHECK(!shmaddr0);

    shmaddr0 = shmat(shmid0, &shmaddr0, IPC_CREAT);
    CHECK(shmaddr0);

    int32_t ret0 = shmdt(shmaddr0);
    CHECK_EQ(0, ret0);

    // file2
    filename = "e:\\file_test";
    key_t key1 = ftok(filename, 8);
    CHECK_NE(key1, key0);

    int32_t shmid1 = shmget(key1, 128, IPC_CREAT);
    CHECK_NE(0, shmid1);
    CHECK_NE(shmid1, shmid0);

    void* shmaddr1 = shmat(shmid1, &shmaddr1, IPC_CREAT);
    CHECK(shmaddr1);
    CHECK_NE(shmaddr1, shmaddr0);

    int32_t ret1 = shmdt(shmaddr1);
    CHECK_EQ(0, ret1);
}
#else
// @brief:     test function GetModuleFileName
TEST(TestGetModuleFileName, GetModuleFileName) {
    char name_module[256]= {0};
    uint32_t ret = GetModuleFileName(NULL, name_module, sizeof(name_module));
    CHECK(ret);
    CHECK_EQ(ret, static_cast<uint32_t>(strlen(name_module)));
}

// @brief:     test function SetFDLimit
TEST(TestSetFDLimit, SetFDLimit) {
    bool b = false;
    uint32_t max_fds = 256;
    struct rlimit rlim0= {0};
    if (getrlimit(RLIMIT_NOFILE, &rlim0) != 0)
        return;

    b = SetFDLimit(max_fds);
    CHECK_EQ(true, b);
    struct rlimit rlim1= {0};
    if (getrlimit(RLIMIT_NOFILE, &rlim1) != 0)
        return;
    CHECK_EQ(rlim1.rlim_cur, rlim0.rlim_cur);
    CHECK_EQ(rlim1.rlim_max, rlim0.rlim_max);

    max_fds = 1024;
    b = SetFDLimit(max_fds);
    EXPECT_TRUE(b);
    struct rlimit rlim2= {0};
    CHECK_EQ (getrlimit(RLIMIT_NOFILE, &rlim2), 0);

    // 系统默认的最大文件句柄数是1024，所以上面的单元测试可以跑过(设置最大的句柄都<=1024)
    // 假如要把最大文件句柄设置成2000，那么在有些系统上已经大于2000，比如65536(有运维修改过了),
    // 那么返回成功，但在CI上面默认就是1024，所以要调用系统的setrlimit，这个接口要求是root用户
    // 如果不是root用户，则失败了，CI不过，正是这个原因
    // 这个单测没有通用性，先注释之
    /*max_fds = 2000;
    b = SetFDLimit(max_fds);
#ifdef WIN32
    EXPECT_FALSE(b);
#else
EXPECT_TRUE(b);
#endif*/

    setrlimit(RLIMIT_NOFILE, &rlim0);
}

// @brief:     test function SetCoreLimit
TEST(TestSetCoreLimit, SetCoreLimit) {
    bool b = false;
    b = SetCoreLimit();
    CHECK(b);
    struct rlimit rlim= {0};
    rlim.rlim_cur = RLIM_INFINITY;
    rlim.rlim_max = RLIM_INFINITY;
    if (getrlimit(RLIMIT_CORE, &rlim) != 0)
        return;
    CHECK_EQ(rlim.rlim_cur, RLIM_INFINITY);
    CHECK_EQ(rlim.rlim_max, RLIM_INFINITY);
}
#endif
