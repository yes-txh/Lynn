#ifdef WIN32
#include "common/baselib/svrpublib/server_publib.h"
#include "common/file/file.h"
#include "thirdparty/gflags/gflags.h"
#include "thirdparty/gtest/gtest.h"
using namespace xfs::base;

DECLARE_USING_LOG_LEVEL_NAMESPACE;

int32_t main(int32_t argc, char** argv) {
    
    LOG(INFO) << "**********************CA Client Unit Test Start*********************";
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, false);
    AutoBaseLib auto_base_lib;
    CXSocketLibAutoManage sock_lib_mgr;
    SetCheckProgramName(argv[0]);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

    LOG(INFO) << "*************************Everything goes well****************************";
    return 0;
}
#endif
