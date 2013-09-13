// quota_manager.h
// fatliu@tencent.com

// htdocs/ca_dir/quota_pb.dat文件用于存放quota信息:role_id,cluster,num_chunks,num_files,num_dirs;

#ifndef COMMON_CRYPTO_CA_CA_SERVER_CA_QUOTA_MANAGER_H_
#define COMMON_CRYPTO_CA_CA_SERVER_CA_QUOTA_MANAGER_H_
#include <string>
#include <map>
#include "common/base/scoped_ptr.h"
#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/netframe/netframe.hpp"
#include "common/rpc/proto_rpc/rpc_service.h"
#include "common/crypto/ca/ca_public/ca_error_code.h"
#include "common/system/concurrency/rwlock.hpp"

#include "common/crypto/ca/ca_public/ca_rpc.pb.h"
#include "common/crypto/ca/ca_server/ca_storage.pb.h"

using namespace std;

DECLARE_string(ca_rpc_addr);

namespace ca {

typedef pair<string, string> RoleCluster;

class QuotaManager : public ca::CaServerRpcService {
private:
    QuotaManager();
    ~QuotaManager();
    // 单实例对象
    static QuotaManager* s_quota_manager;

public:
    static QuotaManager* GetInstance() {
        if (s_quota_manager == NULL) {
            // init rwlock
            m_rwlock.reset(new RWLock());
        }

        RWLock::WriterLocker locker(*m_rwlock);
        if (s_quota_manager == NULL) {
            s_quota_manager = new QuotaManager;
            s_quota_manager->LoadConfig();
        }
        return s_quota_manager;
    }

    static void FreeInstance() {
        RWLock::WriterLocker locker(*m_rwlock);
        if (s_quota_manager) {
            delete s_quota_manager;
            s_quota_manager = NULL;
        }
    }

    bool ReloadConfig();

    // 设置role的quota值,单位为M
    bool SetRoleQuota(const char* role, const char* cluster,
        const Quota quota, CA_ERROR_CODE* ptr_error_code = 0);

    // 查询role的quota值.
    // 输出参数quota,单位为M.若失败,返回false
    bool QueryRoleQuota(const char* role, const char* cluster, Quota* quota,
        CA_ERROR_CODE* ptr_error_code = 0);

    // 列出所有role的quota值
    void ListAllRolesQuota(map<RoleCluster, Quota>* roles_quota,
        CA_ERROR_CODE* ptr_error_code = 0);

    // 将quota信息传给master
    void GetQuota(
        ::google::protobuf::RpcController* controller,
        const ::ca::GetQuotaRequest* request,
        ::ca::GetQuotaResponse* response,
        ::google::protobuf::Closure* done);

private:

    bool LoadConfig();

    void CleanConfig();

    // 1.role长度在0-32B之间
    // 2.在role_manager中查询role是否存在
    bool IsValidRole(const char* role, uint32_t* id = NULL, CA_ERROR_CODE* ptr_error_code = 0);

    // 将所有role的quota值写入磁盘
    bool DumpQuota(CA_ERROR_CODE* ptr_error_code = 0);

    // 将所有role对应的cluster和quota从文件读出
    bool LoadQuota();

private:

    static scoped_ptr<RWLock> m_rwlock;           // 读写锁
    char m_quota_filename[kMaxDirLen];
    map<RoleCluster, Quota>    m_roles_quota;
    scoped_ptr<netframe::NetFrame> m_netframe;
    scoped_ptr<rpc::HttpServer> m_http_server;
    scoped_ptr<rpc::RpcService> m_rpc_service;
};

} // namespace ca
#endif // COMMON_CRYPTO_CA_CA_SERVER_CA_QUOTA_MANAGER_H_
