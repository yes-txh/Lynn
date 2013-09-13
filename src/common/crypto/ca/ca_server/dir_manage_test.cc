#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "thirdparty/gtest/gtest.h"
#include "common/crypto/ca/ca_public/ca_error_code.h"
#include "common/crypto/ca/ca_server/dir_manage.h"
#include "common/crypto/ca/ca_server/role_manager.h"

using namespace xfs::base;
using namespace ca;

const char* kTestFileName = "test_file.dat";
extern const char* kTestCaDir;

#ifdef WIN32
int32_t TestDirManage(int32_t argc, char** argv)
#else
int32_t main(int32_t argc, char** argv)
#endif
{
#ifndef WIN32
    InitGoogleDefaultLogParam(0);
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, true);
    AutoBaseLib auto_baselib();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#else
    return 0;
#endif
}

/*
class TestEnvironment : public testing::Environment {
public:
    virtual void SetUp() {
        LOG(INFO) << "TestEnvironment SetUp";
        char module_dir[kMaxDirLen] = {0};
        GetModuleFileName(NULL, module_dir, sizeof(module_dir));

        // linux and windows
        char* p = strrchr(module_dir, '/');
        if ( !p )
            p = strrchr(module_dir, '\\');
        CHECK(p);
        p++;

        // 追加目录htdocs
        int32_t len_remain = module_dir + sizeof(module_dir) - p - 1;
        // 边界检查
        int32_t len_add_dir = sizeof("htdocs/") + sizeof("ca_dir/") - 2;
        CHECK_GT(len_remain, len_add_dir);

        safe_snprintf(p, len_remain, "htdocs");

        DirManage dir_manage;
        CHECK(dir_manage.RecursivelyRmDir(module_dir));
    }

    virtual void TearDown() {
        LOG(INFO) << "TestEnvironment TearDown";
    }
};
*/



class DirManageTest: public testing::Test
{
public:
        DirManage dir_manage;
protected:
    static void SetUpTestCase() {
        LOG(INFO) << "TestEnvironment SetUp";
        char module_dir[kMaxDirLen] = {0};
        GetModuleFileName(NULL, module_dir, sizeof(module_dir));

        // linux and windows
        char* p = strrchr(module_dir, '/');
        if ( !p )
            p = strrchr(module_dir, '\\');
        CHECK(p);
        p++;

        // 追加目录htdocs
        int32_t len_remain = module_dir + sizeof(module_dir) - p - 1;
        // 边界检查
        int32_t len_add_dir = sizeof("htdocs/") + sizeof("ca_dir/") - 2;
        CHECK_GT(len_remain, len_add_dir);

        safe_snprintf(p, len_remain, "htdocs");

        DirManage dir_manage;
        dir_manage.RecursivelyRmDir(module_dir);
	}

    static void TearDownTestCase() {
        LOG(INFO) << "TestEnvironment TearDown";
    }
};

TEST_F(DirManageTest, TestDirAndFile) {
    char module_dir[kMaxDirLen] = {0};
    GetModuleFileName(NULL, module_dir, sizeof(module_dir));

    // linux and windows
    char* p = strrchr(module_dir, '/');
    if ( !p )
        p = strrchr(module_dir, '\\');
    CHECK(p);
    p++;

    // 追加目录htdocs
    int32_t len_remain = module_dir + sizeof(module_dir) - p - 1;
    // 边界检查
    int32_t len_add_dir = sizeof("htdocs/") + sizeof("ca_dir/") - 2;
    CHECK_GT(len_remain, len_add_dir);

    safe_snprintf(p, len_remain, "htdocs");
    char tmp_dir[kMaxDirLen] = {0};
    safe_snprintf(tmp_dir, kMaxDirLen, module_dir);

    // -------------------------------------
    // test mkdir
    // dir not exist
    CHECK(!dir_manage.IsDirExist(module_dir));
    // mkdir
    dir_manage.MkDir(module_dir);
    // dir already exist
    CHECK(dir_manage.IsDirExist(module_dir));
    p += 6;
    len_remain = module_dir + sizeof(module_dir) - p - 1;
    safe_snprintf(p, len_remain, "%s%s", SPLIT_SIGN, "ca_dir");
    // dir not exist
    CHECK(!dir_manage.IsDirExist(module_dir));
    // mkdir
    dir_manage.MkDir(module_dir);
    // dir already exist
    CHECK(dir_manage.IsDirExist(module_dir));
    p += 7;
    len_remain = module_dir + sizeof(module_dir) - p - 1;

    // -------------------------------------
    // test mkfile
    safe_snprintf(p, len_remain, "%s%s", SPLIT_SIGN, kTestFileName);
    // file not exist
    CHECK(!dir_manage.IsFileExist(module_dir));
    // mkfile
    FILE* fp = fopen(module_dir, "wb");
    CHECK(fp);
    fclose(fp);
    // file already exist
    CHECK(dir_manage.IsFileExist(module_dir));

    // -------------------------------------
    // test recursively rmdir
    dir_manage.RecursivelyRmDir(tmp_dir);
    CHECK(!dir_manage.IsDirExist(tmp_dir));
}
