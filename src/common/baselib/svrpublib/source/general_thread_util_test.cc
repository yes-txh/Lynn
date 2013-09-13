//////////////////////////////////////////////////////////////////////////
// general_thread_util_test.cc
// @brief:     Test base class CXThreadBase
// @author:  fatliu@tencent
// @time:     2010-10-20
// @version: 1.0
//////////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

// 调用一次Routine,times+1
class TestThread:public CXThreadBase {
public:
    TestThread():times(0) {
    };

    ~TestThread() {
    };
    virtual void    Routine() {
        times++;
    }
    uint32_t GetTimes()const {
        return times;
    }
private:
    uint32_t volatile times;
};

// 调用一次Routine,等待5微秒,再count+1
class TestThreadWithXsleep:public CXThreadBase {
public:
    explicit TestThreadWithXsleep(uint32_t &count) {
        ptimes = &count;
    }
    ~TestThreadWithXsleep() {
    };
    virtual void    Routine() {
        XUSleep(5000);
        (*ptimes)++;
        StopRoutine();
    }
private:
    uint32_t volatile *ptimes;
};

#ifdef WIN32
int32_t TestGeneralThreadUtil(int32_t argc, char** argv)
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

// @brief:     Test base class CXThreadBase by TestThread:
//                 count times of using Routine
TEST(TestThread, Thread) {
    TestThread thread;
    // 调用10次Routine
    for (int32_t i = 0; i < 10; i++) {
        thread.Routine();
        uint32_t times = thread.GetTimes();
        CHECK_EQ(times, i+1);
    }
}

// @brief:     Test base class CXThreadBase by 2 TestThreadWithXsleep:
//                 count times of using Routine
TEST(TestThread, TestThreadWithXsleep) {
    uint32_t count = 0;
    TestThreadWithXsleep thread1(count);
    TestThreadWithXsleep thread2(count);
    // 调用20次Routine
    for (int32_t i = 0; i < 10; i++) {
        thread1.StartThread();
        thread2.StartThread();
    }
    // 部分线程并没XUSleep结束,所以count<20
    CHECK_NE(count, 20);
    // 等候所有线程结束
    XSleep(1000);
    CHECK_EQ(count, 20);
}

