#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "thirdparty/gtest/gtest.h"

using namespace xfs::base;

int32_t main(int32_t argc, char* argv[]) {
#ifdef WIN32
    InitGoogleDefaultLogParam(0);
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, false);
    CXSocketLibAutoManage auto_sock_lib_mgr;
    AutoBaseLib auto_baselib;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#else
    return 0;
#endif
}