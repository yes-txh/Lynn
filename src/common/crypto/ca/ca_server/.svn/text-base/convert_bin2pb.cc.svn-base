#include <string.h>
#include <map>
#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "thirdparty/gtest/gtest.h"
#include "common/crypto/ca/ca_public/ca_struct.h"
#include "common/crypto/ca/ca_server/role_manager.h"

using namespace std;
using namespace ca;

// old format
const char* const kOldRoleFileName     = "role.dat";
const char* const kOldIdentityFileName = "identity.dat";
const char* const kOldRelatedFileName  = "related.dat";

DEFINE_string(path, "htdocs/ca_dir/", "data file path");

#ifdef WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif
typedef struct GIDUID{
    uint32_t gid;
    uint32_t uid;
    bool     valid;
    GIDUID() {
        gid = 0;
        uid = 0;
        valid = true;
    };
}GIDUID;

typedef struct IDNAME {
    uint32_t id;
    uint32_t quota;
    uint32_t reserve2;
    bool     valid;
    char     name[kMaxNameLen];
    IDNAME() {
        valid = true;
        id    = 0;
        name[0] = 0;
        quota = 0;
        reserve2 = 0;
    } 
}IDNAME;
#ifdef WIN32
#pragma pack(pop) 
#else
#pragma pack() 
#endif

class ConvertDataFmt {
public:
    ConvertDataFmt() {
        GetModuleFileName(NULL, m_module_dir, sizeof(m_module_dir));
        char* p = strrchr(m_module_dir, '/');
        if (!p)
            p = strrchr(m_module_dir, '\\');
        CHECK(p);
        p++;
        CHECK_GT(sizeof(m_module_dir) - 1, STRLEN(m_module_dir) + FLAGS_path.size());
        safe_snprintf(p, sizeof(m_module_dir), "%s", FLAGS_path.c_str());

        // init filenames
        safe_snprintf(m_identity_filename, kMaxDirLen, "%s%s", m_module_dir, kOldIdentityFileName);
        safe_snprintf(m_role_filename, kMaxDirLen, "%s%s", m_module_dir, kOldRoleFileName);
        safe_snprintf(m_related_filename, kMaxDirLen, "%s%s", m_module_dir, kOldRelatedFileName);
    }
    void Init() {
        role_mgr = RoleManager::GetInstance();
    }

    ~ConvertDataFmt() {
        RoleManager::FreeInstance();
    }

    void Convert();
public:
    RoleManager* role_mgr;
    char m_module_dir[256];
    char m_identity_filename[256];
    char m_role_filename[256];
    char m_related_filename[256];
};


void ConvertDataFmt::Convert() {
    Init();
    // identity
    FILE* fp = fopen(m_identity_filename, "rb");
    if (!fp) {
        LOG(ERROR) << " can't open" << m_identity_filename;
        return;
    }
    IDNAME info;
    map<uint32_t, string> identity_names;
    while (1 == fread(&info, sizeof(info), 1, fp)) {
        if (info.valid) {
            role_mgr->AddIdentity(info.name);
            identity_names.insert(pair<uint32_t, string>(info.id, info.name));
        }
    }
    fclose(fp);

    // role
    fp = fopen(m_role_filename, "rb");
    if (!fp) {
        LOG(ERROR) << " can't open" << m_role_filename;
        return;
    }
    map<uint32_t, string> role_names;
    while (1 == fread(&info, sizeof(info), 1, fp)) {
        if (info.valid) {
            role_mgr->AddRole(info.name);
            role_names.insert(pair<uint32_t, string>(info.id, info.name));
        }
    }
    fclose(fp);

    // relation
    fp = fopen(m_related_filename, "rb");
    if (!fp) {
        LOG(ERROR) << " can't open" << m_related_filename;
        return;
    }
    GIDUID info1;
    while (1 == fread(&info1, sizeof(info1), 1, fp)) {
        if (info1.valid) {
            map<uint32_t, string>::iterator it1 = identity_names.find(info1.uid);
            map<uint32_t, string>::iterator it2 = role_names.find(info1.gid);
            if (it1 != identity_names.end() && it2 != role_names.end()) {
                const char* identity_name = identity_names.find(info1.uid)->second.c_str();
                const char* role_name = role_names.find(info1.gid)->second.c_str();
                LOG(INFO) << identity_name << " and " << role_name;
                role_mgr->AddIdentityToRole(role_name, identity_name);
            }
        }
    }
    fclose(fp);
}

int32_t main(int32_t argc, char* argv[]) {
    InitGoogleDefaultLogParam(0);
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, false);
    CXSocketLibAutoManage auto_sock_lib_mgr;
    AutoBaseLib auto_baselib;

    ConvertDataFmt convertor;
    convertor.Convert();

    return 0;
}
