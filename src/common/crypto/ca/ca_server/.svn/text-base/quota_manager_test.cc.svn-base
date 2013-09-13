#ifdef WIN32
#include <winsock2.h>
#endif
#include "thirdparty/gtest/gtest.h"
#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/rpc/proto_rpc/rpc_service.h"
#include "common/crypto/ca/ca_public/ca_struct.h"
#include "common/crypto/ca/ca_public/ca_rpc.pb.h"
#include "common/crypto/ca/ca_server/role_manager.h"
#include "common/crypto/ca/ca_server/quota_manager.h"

using namespace xfs::base;
using namespace ca;

#ifdef WIN32
int32_t TestQuotaManager(int32_t argc, char** argv)
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

class QuotaManagerTest: public testing::Test {
public:
    QuotaManagerTest() {
		FLAGS_ca_rpc_addr = "127.0.43.60:30090";
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
        safe_snprintf(filename, kMaxDirLen, "%s%s", module_dir, kQuotaFileName);

        role_mgr = RoleManager::GetInstance();
        CHECK(role_mgr->AddRole("test_role"));
    }
    void Init() {
        quota_mgr = QuotaManager::GetInstance();
    }

    ~QuotaManagerTest() {
        QuotaManager::FreeInstance();
        CHECK(role_mgr->DelRole("test_role"));
        RoleManager::FreeInstance();
    }
public:
    uint32_t count;
    QuotaManager* quota_mgr;
    RoleManager* role_mgr;
};

TEST_F(QuotaManagerTest, LoadAndDump) {
    Init();

    Quota quota;
    quota.num_chunks = 123;
    quota.num_files = 456;
    quota.num_directories = 789;
    CHECK(quota_mgr->SetRoleQuota("test_role", "test_cluster", quota));

    // reload
    QuotaManager::FreeInstance();
    quota_mgr = QuotaManager::GetInstance();
    map<RoleCluster, Quota> roles_quota;
    quota_mgr->ListAllRolesQuota(&roles_quota);
    CHECK_EQ(1, roles_quota.size());
}

TEST_F(QuotaManagerTest, SetAndQueryQuota) {
    Init();

    Quota quota;
    quota.num_chunks = 123;
    quota.num_files = 456;
    quota.num_directories = 789;
    CHECK(quota_mgr->SetRoleQuota("test_role", "test_cluster", quota));
    CHECK(!quota_mgr->SetRoleQuota("test_role", NULL, quota));
    CHECK(!quota_mgr->SetRoleQuota("test_role", "", quota));

    Quota quota1;
    CHECK(quota_mgr->QueryRoleQuota("test_role", "test_cluster", &quota1));
    CHECK_EQ(quota.num_chunks, quota1.num_chunks);
    CHECK_EQ(quota.num_files, quota1.num_files);
    CHECK_EQ(quota.num_directories, quota1.num_directories);
    CHECK(!quota_mgr->QueryRoleQuota("test_role1", "test_cluster", &quota1));
}

void CallBack(rpc::RpcController* controller,
              GetQuotaRequest* request,
              GetQuotaResponse* response) {
    LOG(INFO) << "call back here...";
    CHECK_EQ(1, response->quota_record_size());
    CHECK_EQ(0, response->quota_record(0).role_name().compare("test_role"));
    CHECK_EQ(123, response->quota_record(0).chunks_count());
    CHECK_EQ(456, response->quota_record(0).files_count());
    CHECK_EQ(789, response->quota_record(0).directories_count());
}

TEST_F(QuotaManagerTest, GetQuota) {
    Init();

    Quota quota;
    quota.num_chunks = 123;
    quota.num_files = 456;
    quota.num_directories = 789;
    CHECK(quota_mgr->SetRoleQuota("test_role", "test_cluster", quota));
    // Create instances for proto_rpc.
    rpc::RpcController* controller = new rpc::RpcController();
    GetQuotaRequest* request = new GetQuotaRequest();
    GetQuotaResponse* response = new GetQuotaResponse();
    request->set_cluster_identifier("test_cluster");
    google::protobuf::Closure* get_quota_callback =
        NewClosure(&CallBack, controller, request, response);

    quota_mgr->GetQuota(controller, request, response, get_quota_callback);
}
