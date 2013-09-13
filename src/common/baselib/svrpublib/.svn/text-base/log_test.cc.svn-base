//////////////////////////////////////////////////////////////////////////
// log_test.cc
// @brief:     Test functions in log.cc
// @author:  fatliu@tencent
// @time:     2010-10-28
// @version: 1.0
//////////////////////////////////////////////////////////////////////////

#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

#ifdef WIN32
int32_t TestLog(int32_t argc, char** argv)
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

// @brief:     Test functions MemDebug
TEST(TestLog, MemDebug) {
    // test MemDebug
    const char* p = "hello world! :p\r\ntest =.=|| \r\n";
    Xprintf_MemDebug(p, static_cast<uint32_t>(strlen(p)));
}
