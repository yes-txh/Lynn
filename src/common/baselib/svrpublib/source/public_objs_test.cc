//////////////////////////////////////////////////////////////////////////
// public_objs_test.cc
// @brief:      Test functions in public_objs.cc
// @author:     fatliu@tencent
// @time:       2010-11-01
// @version:    1.0
//////////////////////////////////////////////////////////////////////////

#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

#ifdef WIN32
int32_t TestPublicObjs(int32_t argc, char** argv)
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

// @brief:      Test functions InitBaseLib & ShutdownBaseLib
TEST(TestBaseLib, InitAndShutdown) {
#ifdef WIN32
    InitBaseLib();
    // init once in main...
    EXPECT_EQ(2, g_lib_vars.m_num_init_base_lib_count);
    InitBaseLib();
    EXPECT_EQ(3, g_lib_vars.m_num_init_base_lib_count);

    ShutdownBaseLib();
    EXPECT_EQ(2, g_lib_vars.m_num_init_base_lib_count);
    ShutdownBaseLib();
    EXPECT_EQ(1, g_lib_vars.m_num_init_base_lib_count);
#endif
}

// @brief:      Test function InitGoogleDefaultLogParam
TEST(Glog, InitGoogleDefaultLogParam) {
    InitGoogleDefaultLogParam();

    char module_name[MAX_PATH] = {0};
    GetModuleFileName(NULL, module_name, sizeof(module_name));

    // linux and windows
    char* p = strrchr(module_name, '/');
    if ( !p )
        p = strrchr(module_name, '\\');
    if (p)
        *p = 0;

	printf("log_dir=%s, module name:%s\r\n", FLAGS_log_dir.c_str(), module_name);
    EXPECT_STREQ(FLAGS_log_dir.c_str(), module_name);

	printf("FLAGS_stderrthreshold=%d\r\n", FLAGS_stderrthreshold)		;

#ifdef NDEBUG
    EXPECT_EQ(FLAGS_stderrthreshold, ERROR);
#else
	EXPECT_EQ(FLAGS_stderrthreshold, INFO);
#endif //
}


