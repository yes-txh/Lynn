// role_manager.h
// fatliu@tencent.com

// 1. htdocs/ca_dir/role_pb.dat文件用于存放role的id,name,valid的信息;
// 2. htdocs/ca_dir/identity_pb.dat文件用于存放identity的id,name,valid的信息;
// 3. htdocs/ca_dir/related_pb.dat文件用于存放所有role_id,identity_id,valid的对应信息:
// 4. 程序启动时加载文件role_pb.dat进内存map<uint32_t, string> m_role_names,
//    加载文件identity_pb.dat进内存map<uint32_t, string> m_identity_names,
//    加载文件related_pb.dat进内存map<string, Item*> m_identitys和m_roles,
//    分别是由identityname到所对应role id列表的映射和由rolename到所对应identity id列表的映射.

#ifndef COMMON_CRYPTO_CA_CA_SERVER_CA_ROLE_MANAGER_H_
#define COMMON_CRYPTO_CA_CA_SERVER_CA_ROLE_MANAGER_H_

#include <string>
#include <map>
#include <set>
#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/crypto/ca/ca_public/ca_error_code.h"

using namespace std;

namespace ca {

class RoleManager {
private:
    RoleManager();
    ~RoleManager();
    // 单实例对象
    static RoleManager*  s_role_manager;

public:
    static RoleManager* GetInstance() {
        CXThreadAutoLock auto_lock(&s_mutex);
        if (s_role_manager == NULL) {
            s_role_manager = new RoleManager;
            s_role_manager->LoadConfig();
        }
        return s_role_manager;
    }

    static void FreeInstance() {
        CXThreadAutoLock auto_lock(&s_mutex);
        if (s_role_manager) {
            delete s_role_manager;
            s_role_manager = NULL;
        }
    }

    // identity interface :
    // 添加用户
    bool AddIdentity(const char* identity_name, CA_ERROR_CODE* ptr_error_code = 0);

    // 删除用户
    bool DelIdentity(const char* identity_name, CA_ERROR_CODE* ptr_error_code = 0);

    // 查询用户所在组
    bool QueryIdentityRoles(const char* identity_name, vector<string>* role_list,
                            CA_ERROR_CODE* ptr_error_code = 0);

    // role interface :
    // 添加组
    bool AddRole(const char* role_name, CA_ERROR_CODE* ptr_error_code = 0);

    // 删除组
    bool DelRole(const char* role_name, CA_ERROR_CODE* ptr_error_code = 0);

    // 添加用户到组
    bool AddIdentityToRole(const char* role_name, const char* identity_name,
                           CA_ERROR_CODE* ptr_error_code = 0);

    // 从组删除用户
    bool DelIdentityFromRole(const char* role_name, const char* identity_name,
                             CA_ERROR_CODE* ptr_error_code = 0);

    // 查询组里所有用户
    bool QueryRoleIdentities(const char* role_name, vector<string>* identity_list,
                             CA_ERROR_CODE* ptr_error_code = 0);

    // 验证用户
    bool VerifyIdentity(const char* role_name, const char* identity_name,
                        CA_ERROR_CODE* ptr_error_code = 0);

    // 显示所有用户
    void PrintAllIdentities(vector<string>* identity_list);

    // 显示所有组
    void PrintAllRoles(vector<string>* role_list);

    // 根据role名查询role id, 返回role id
    uint32_t GetRoleIdByName(const char* role_name);

    // 根据role id查询role名, 返回role名
    const char* GetRoleNameById(uint32_t role_id);

    // 根据identity名查询identity id, 返回identity id
    uint32_t GetIdentityIdByName(const char* identity_name);

    // 根据identity id查询identity名, 返回identity名
    const char* GetIdentityNameById(uint32_t identity_id);

    bool ReloadConfig();

private:

    bool LoadConfig();
    void CleanConfig();

    // 检查用户所属组数是否达到上限
    inline bool CheckRoleNum(const char* identity_name);

    // 加载identity names
    bool LoadIdentityNames();

    // 加载role names
    bool LoadRoleNames();

    // 加载relations
    bool LoadRelations();

    // 保存上次的文件至filename_time
    bool SaveOldFile(const char* filename, CA_ERROR_CODE* ptr_error_code = 0);

    // 将identity names信息写入磁盘
    bool DumpIdentityNames(CA_ERROR_CODE* ptr_error_code = 0);

    // 将role names信息写入磁盘
    bool DumpRoleNames(CA_ERROR_CODE* ptr_error_code = 0);

    // 将relations信息写入磁盘
    bool DumpRelations(CA_ERROR_CODE* ptr_error_code = 0);

    // 添加用户
    bool AddIdentityWithoutLock(const char* identity_name,
                                CA_ERROR_CODE* ptr_error_code = 0);

    // 删除用户
    bool DelIdentityWithoutLock(const char* identity_name,
                                CA_ERROR_CODE* ptr_error_code = 0);

    // 添加组
    bool AddRoleWithoutLock(const char* role_name,
                            CA_ERROR_CODE* ptr_error_code = 0);

    // 删除组
    bool DelRoleWithoutLock(const char* role_name,
                            CA_ERROR_CODE* ptr_error_code = 0);

    // 添加用户到组
    bool AddIdentityToRoleWithoutLock(const char* role_name, const char* identity_name,
                                      CA_ERROR_CODE* ptr_error_code = 0);

    // 从组删除用户
    bool DelIdentityFromRoleWithoutLock(const char* role_name, const char* identity_name,
                                        CA_ERROR_CODE* ptr_error_code = 0);

private:    
    static CXThreadMutex s_mutex;

    char m_role_filename[kMaxDirLen];
    char m_identity_filename[kMaxDirLen];
    char m_related_filename[kMaxDirLen];

    map<uint32_t, string> m_identity_names;
    map<uint32_t, string> m_role_names;
    map<string, Item*> m_identities;
    map<string, Item*> m_roles;

    uint32_t m_max_identity_id;
    uint32_t m_max_role_id;
};

} // namespace ca
#endif // COMMON_CRYPTO_CA_CA_SERVER_CA_ROLE_MANAGER_H_
