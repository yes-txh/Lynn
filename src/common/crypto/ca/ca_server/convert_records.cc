#ifndef WIN32
#include <sys/types.h>
#include <dirent.h>
#endif
#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"
#include "common/crypto/ca/ca_public/ca_struct.h"
#include "common/crypto/ca/ca_server/role_manager.h"

using namespace xfs::base;
using namespace ca;

const char* kTestCaDir            = "htdocs"SPLIT_SIGN"ca_dir"SPLIT_SIGN;
const char* kTestRoleFileName     = "role.dat";
const char* kTestUserFileName     = "identity.dat";
const char* kTestRelatedFileName  = "related.dat";

class ConvertUserToRoleMgr{
public:
    ConvertUserToRoleMgr() {
        GetModuleFileName(NULL, m_module_dir, sizeof(m_module_dir));
        char* p = strrchr(m_module_dir, '/');
        if (!p)
            p = strrchr(m_module_dir, '\\');
        CHECK(p);
        p++;
        CHECK_GT(sizeof(m_module_dir) - 1, STRLEN(m_module_dir) + STRLEN(kTestCaDir));
        safe_snprintf(p, sizeof(m_module_dir), "%s", kTestCaDir);

        // init filenames
        safe_snprintf(m_identity_filename, kMaxDirLen, "%s%s", m_module_dir, kTestUserFileName);
        safe_snprintf(m_role_filename, kMaxDirLen, "%s%s", m_module_dir, kTestRoleFileName);
        safe_snprintf(m_related_filename, kMaxDirLen, "%s%s", m_module_dir, kTestRelatedFileName);
    }
    void Init() {
        role_mgr = RoleManager::GetInstance();
        role_mgr->DelIdentity("bb");
    }

    ~ConvertUserToRoleMgr() {
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


void ConvertUserToRoleMgr::Convert() {
    Init();

#ifdef WIN32
    _finddata_t files;
    strcat(m_module_dir, "*");
    long hd = _findfirst(m_module_dir, &files);
    if (hd != -1) {
        do {
            if (files.attrib == _A_SUBDIR) {
                LOG(INFO) << files.name;
                if (strcmp(files.name, ".") && strcmp(files.name, "..")) {
                    role_mgr->AddIdentity(files.name);
                }
            }
        } while(_findnext(hd, &files) == 0);
    }
#else
    DIR* dp = NULL;
    if((dp = opendir(m_module_dir)) != NULL)
    {
        struct dirent* dirp= NULL;
        while ((dirp = readdir(dp)) != NULL) {
            if (dirp->d_type == DT_DIR) {
                LOG(INFO) << dirp->d_name;
                if (strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, "..")) {
                    role_mgr->AddIdentity(dirp->d_name);
                }
            }
        }
    }
#endif
}

int32_t main(int32_t argc, char* argv[]) {
    InitGoogleDefaultLogParam(0);
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, false);
    CXSocketLibAutoManage auto_sock_lib_mgr;
    AutoBaseLib auto_baselib;

    ConvertUserToRoleMgr convertor;
    convertor.Convert();

    return 0;
}
