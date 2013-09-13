// role_manager.h
// fatliu@tencent.com

// 1. htdocs/ca_dir/role_pb.dat�ļ����ڴ��role��id,name,valid����Ϣ;
// 2. htdocs/ca_dir/identity_pb.dat�ļ����ڴ��identity��id,name,valid����Ϣ;
// 3. htdocs/ca_dir/related_pb.dat�ļ����ڴ������role_id,identity_id,valid�Ķ�Ӧ��Ϣ:
// 4. ��������ʱ�����ļ�role_pb.dat���ڴ�map<uint32_t, string> m_role_names,
//    �����ļ�identity_pb.dat���ڴ�map<uint32_t, string> m_identity_names,
//    �����ļ�related_pb.dat���ڴ�map<string, Item*> m_identitys��m_roles,
//    �ֱ�����identityname������Ӧrole id�б��ӳ�����rolename������Ӧidentity id�б��ӳ��.

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
    // ��ʵ������
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
    // ����û�
    bool AddIdentity(const char* identity_name, CA_ERROR_CODE* ptr_error_code = 0);

    // ɾ���û�
    bool DelIdentity(const char* identity_name, CA_ERROR_CODE* ptr_error_code = 0);

    // ��ѯ�û�������
    bool QueryIdentityRoles(const char* identity_name, vector<string>* role_list,
                            CA_ERROR_CODE* ptr_error_code = 0);

    // role interface :
    // �����
    bool AddRole(const char* role_name, CA_ERROR_CODE* ptr_error_code = 0);

    // ɾ����
    bool DelRole(const char* role_name, CA_ERROR_CODE* ptr_error_code = 0);

    // ����û�����
    bool AddIdentityToRole(const char* role_name, const char* identity_name,
                           CA_ERROR_CODE* ptr_error_code = 0);

    // ����ɾ���û�
    bool DelIdentityFromRole(const char* role_name, const char* identity_name,
                             CA_ERROR_CODE* ptr_error_code = 0);

    // ��ѯ���������û�
    bool QueryRoleIdentities(const char* role_name, vector<string>* identity_list,
                             CA_ERROR_CODE* ptr_error_code = 0);

    // ��֤�û�
    bool VerifyIdentity(const char* role_name, const char* identity_name,
                        CA_ERROR_CODE* ptr_error_code = 0);

    // ��ʾ�����û�
    void PrintAllIdentities(vector<string>* identity_list);

    // ��ʾ������
    void PrintAllRoles(vector<string>* role_list);

    // ����role����ѯrole id, ����role id
    uint32_t GetRoleIdByName(const char* role_name);

    // ����role id��ѯrole��, ����role��
    const char* GetRoleNameById(uint32_t role_id);

    // ����identity����ѯidentity id, ����identity id
    uint32_t GetIdentityIdByName(const char* identity_name);

    // ����identity id��ѯidentity��, ����identity��
    const char* GetIdentityNameById(uint32_t identity_id);

    bool ReloadConfig();

private:

    bool LoadConfig();
    void CleanConfig();

    // ����û����������Ƿ�ﵽ����
    inline bool CheckRoleNum(const char* identity_name);

    // ����identity names
    bool LoadIdentityNames();

    // ����role names
    bool LoadRoleNames();

    // ����relations
    bool LoadRelations();

    // �����ϴε��ļ���filename_time
    bool SaveOldFile(const char* filename, CA_ERROR_CODE* ptr_error_code = 0);

    // ��identity names��Ϣд�����
    bool DumpIdentityNames(CA_ERROR_CODE* ptr_error_code = 0);

    // ��role names��Ϣд�����
    bool DumpRoleNames(CA_ERROR_CODE* ptr_error_code = 0);

    // ��relations��Ϣд�����
    bool DumpRelations(CA_ERROR_CODE* ptr_error_code = 0);

    // ����û�
    bool AddIdentityWithoutLock(const char* identity_name,
                                CA_ERROR_CODE* ptr_error_code = 0);

    // ɾ���û�
    bool DelIdentityWithoutLock(const char* identity_name,
                                CA_ERROR_CODE* ptr_error_code = 0);

    // �����
    bool AddRoleWithoutLock(const char* role_name,
                            CA_ERROR_CODE* ptr_error_code = 0);

    // ɾ����
    bool DelRoleWithoutLock(const char* role_name,
                            CA_ERROR_CODE* ptr_error_code = 0);

    // ����û�����
    bool AddIdentityToRoleWithoutLock(const char* role_name, const char* identity_name,
                                      CA_ERROR_CODE* ptr_error_code = 0);

    // ����ɾ���û�
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
