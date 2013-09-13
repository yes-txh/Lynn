#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

int32_t main(int32_t argc, char** argv) {
#ifdef WIN32
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, true);

    AutoBaseLib auto_baselib;
    SetCheckErrLevel(ERR);
    SetCheckProgramName(argv[0]);
    CXSocketLibAutoManage sock_mgr;

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

#else
    return 0;
#endif
}

