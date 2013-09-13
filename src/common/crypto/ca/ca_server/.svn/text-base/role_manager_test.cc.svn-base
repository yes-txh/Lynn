#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/crypto/ca/ca_server/ca.h"
#include "thirdparty/gtest/gtest.h"
#include "common/crypto/ca/ca_public/ca_struct.h"
#include "common/crypto/ca/ca_server/role_manager.h"

using namespace xfs::base;
using namespace ca;

#ifdef WIN32
int32_t TestRoleManager(int32_t argc, char** argv)
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

class TestEnvironment : public testing::Environment {
public:
    virtual void SetUp() {
        LOG(INFO) << "TestEnvironment SetUp";
    }

    virtual void TearDown() {
        LOG(INFO) << "TestEnvironment TearDown";
    }
};

class RoleManagerTest: public testing::Test {
public:
    RoleManagerTest() {
        char module_dir[kMaxDirLen] = {0};
        GetModuleFileName(NULL, module_dir, sizeof(module_dir));
        char* p = strrchr(module_dir, '/');
        if (!p)
            p = strrchr(module_dir, '\\');
        CHECK(p);
        p++;
        CHECK_GT(sizeof(module_dir) - 1, STRLEN(module_dir) + STRLEN(kCaDir));
        safe_snprintf(p, sizeof(module_dir), "%s", kCaDir);

        char cmd[256] = {0};
#ifdef WIN32
        safe_snprintf(cmd, sizeof(cmd), "md %s", module_dir);
#else
        safe_snprintf(cmd, sizeof(cmd), "mkdir -p %s", module_dir);
#endif
        system(cmd);
        // init filenames
        char filename[256] = {0};
        safe_snprintf(filename, kMaxDirLen, "%s%s", module_dir, kRoleFileName);
        remove(filename);
        safe_snprintf(filename, kMaxDirLen, "%s%s", module_dir, kIdentityFileName);
        remove(filename);
        safe_snprintf(filename, kMaxDirLen, "%s%s", module_dir, kRelatedFileName);
        remove(filename);
    }
    void Init() {
        role_mgr = RoleManager::GetInstance();
    }

    ~RoleManagerTest() {
        RoleManager::FreeInstance();
    }
public:
    RoleManager* role_mgr;
};

TEST_F(RoleManagerTest, AddAndDelIdentity) {
    Init();
    
    // add then del
    CHECK(role_mgr->AddIdentity("fatliu"));
    CHECK(role_mgr->DelIdentity("fatliu"));

    CHECK(role_mgr->AddIdentity("fatliu"));
    // exist, add fail
    CHECK(!role_mgr->AddIdentity("fatliu"));
    CHECK(role_mgr->DelIdentity("fatliu"));

    // not exist, del fail
    CHECK(!role_mgr->DelIdentity("fatliu"));
}

TEST_F(RoleManagerTest, AddAndDelRole) {
    Init();

    // add then del
    CHECK(role_mgr->AddRole("liu"));
    CHECK(role_mgr->DelRole("liu"));

    CHECK(role_mgr->AddRole("liu"));
    // exist, add fail
    CHECK(!role_mgr->AddRole("liu"));
    CHECK(role_mgr->DelRole("liu"));

    // not exist, del fail
    CHECK(!role_mgr->DelRole("liu"));
}

TEST_F(RoleManagerTest, AddAndDelRelation) {
    Init();

    vector<string> list;

    CHECK(role_mgr->AddIdentity("a"));
    CHECK(role_mgr->AddRole("1"));

    // add then del
    CHECK(role_mgr->AddIdentityToRole("1", "a"));
    CHECK(role_mgr->DelIdentityFromRole("1", "a"));

    CHECK(role_mgr->AddIdentityToRole("1", "a"));
    // exist, add fail
    CHECK(!role_mgr->AddIdentityToRole("1", "a"));
    CHECK(role_mgr->DelIdentityFromRole("1", "a"));

    // not exist, del fail
    CHECK(!role_mgr->DelIdentityFromRole("1", "a"));

    CHECK(role_mgr->DelIdentity("a"));
    CHECK(role_mgr->DelRole("1"));
}

TEST_F(RoleManagerTest, Query) {
    Init();
    vector<string> list;

    CHECK(role_mgr->AddIdentity("a"));
    CHECK(role_mgr->AddIdentity("bb"));
    CHECK(role_mgr->AddIdentity("ccc"));
    CHECK(role_mgr->AddIdentity("dddd"));
    CHECK(role_mgr->AddIdentity("eeeee"));

    CHECK(role_mgr->AddRole("1"));
    CHECK(role_mgr->AddRole("22"));
    CHECK(role_mgr->AddRole("333"));
    CHECK(role_mgr->AddRole("4444"));
    CHECK(role_mgr->AddRole("55555"));

    // add relations : add user a to role 1 22 333 4444 55555
    CHECK(role_mgr->AddIdentityToRole("1", "a"));
    CHECK(role_mgr->AddIdentityToRole("22", "a"));
    CHECK(role_mgr->AddIdentityToRole("333", "a"));
    CHECK(role_mgr->AddIdentityToRole("4444", "a"));
    CHECK(role_mgr->AddIdentityToRole("55555", "a"));

    CHECK(role_mgr->VerifyIdentity("22", "a"));
    CHECK(role_mgr->VerifyIdentity("ccc", "ccc"));
    CHECK(role_mgr->QueryIdentityRoles("a", &list));
    CHECK_EQ(6, list.size());

    CHECK(role_mgr->QueryRoleIdentities("1", &list));
    CHECK_EQ(1, list.size());
    CHECK_EQ(0, strcmp(list[0].c_str(), "a"));

    // del one relation: del user a from role 333
    CHECK(role_mgr->DelIdentityFromRole("333", "a"));
    CHECK(role_mgr->QueryIdentityRoles("a", &list));
    CHECK_EQ(5, list.size());

    CHECK(role_mgr->QueryRoleIdentities("333", &list));
    CHECK_EQ(0, list.size());

    // not exist
    CHECK(!role_mgr->QueryIdentityRoles("ffffff", &list));
    CHECK(!role_mgr->QueryRoleIdentities("666666", &list));

    // add relations : add user a bb ccc dddd eeeee to role 1
    CHECK(!role_mgr->AddIdentityToRole("1", "a"));
    CHECK(role_mgr->AddIdentityToRole("1", "bb"));
    CHECK(role_mgr->AddIdentityToRole("1", "ccc"));
    CHECK(role_mgr->AddIdentityToRole("1", "dddd"));
    CHECK(role_mgr->AddIdentityToRole("1", "eeeee"));

    CHECK(role_mgr->QueryRoleIdentities("1", &list));
    CHECK_EQ(5, list.size());

    CHECK(role_mgr->QueryIdentityRoles("ccc", &list));
    CHECK_EQ(2, list.size());
    CHECK_EQ(0, strcmp(list[0].c_str(), "ccc"));
    CHECK_EQ(0, strcmp(list[1].c_str(), "1"));

    // del all relations
    CHECK(role_mgr->DelIdentityFromRole("1", "a"));
    CHECK(role_mgr->DelIdentityFromRole("22", "a"));
    CHECK(role_mgr->DelIdentityFromRole("4444", "a"));
    CHECK(role_mgr->DelIdentityFromRole("55555", "a"));
    CHECK(role_mgr->DelIdentityFromRole("1", "bb"));
    CHECK(role_mgr->DelIdentityFromRole("1", "ccc"));
    CHECK(role_mgr->DelIdentityFromRole("1", "dddd"));
    CHECK(role_mgr->DelIdentityFromRole("1", "eeeee"));

    // del roles and Identities
    CHECK(role_mgr->DelIdentity("a"));
    CHECK(role_mgr->DelIdentity("bb"));
    CHECK(role_mgr->DelIdentity("ccc"));
    CHECK(role_mgr->DelIdentity("dddd"));
    CHECK(role_mgr->DelIdentity("eeeee"));

    CHECK(role_mgr->DelRole("1"));
    CHECK(role_mgr->DelRole("22"));
    CHECK(role_mgr->DelRole("333"));
    CHECK(role_mgr->DelRole("4444"));
    CHECK(role_mgr->DelRole("55555"));
}

