// my_ca_simple_http.cc: interface for the MySimpleHttpThread class.
// joeytian@tencent.com
//////////////////////////////////////////////////////////////////////
#include <string.h>
#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/system/time/time_utils.hpp"
#include "common/base/string/string_number.hpp"
#include "common/crypto/ca/ca_server/my_ca_simple_http.h"
//////////////////////////////////////////////////////////////////////
using namespace xfs::base;

// 用于onebox测试
DEFINE_bool(ca_onebox, false, "if real server, ca_onebox=false; else ca_onebox=true");
DEFINE_string(ca_onebox_user_name, "onebox_test", "user_name for onebox test");
// 主备CA Sever
DEFINE_string(ca_server_role, "Primary", "Primary or Secondary");

namespace ca {

const char* kAdminPwd = "xfs_admin";
const char* const kFusionChartJs = "http://xfs.soso.oa.com/xfs/xfs_chart/JSClass/FusionCharts.js";
const char* const kFusionChart3D = "http://xfs.soso.oa.com/xfs/xfs_chart/Charts/Column3D.swf";
const char* const kFusionChart2D = "http://xfs.soso.oa.com/xfs/xfs_chart/Charts/Area2D.swf";
const char* const kNotFoundPage = "http://xfs.soso.oa.com/xfs/xfs_chart/Charts/not_found_page.jpg";
// 超过5分钟且有修改,则写CheckPoint文件.
const int64_t kReserveTime = 5 * 60;
bool g_has_modified = false;
time_t g_last_modified = 0;
CXThreadMutex g_http_mutex;

MySimpleHttpThread::MySimpleHttpThread() {
    m_module_name[0] = 0;

    char module_file_name[MAX_PATH] = {0};
    GetModuleFileName(NULL, module_file_name, sizeof(module_file_name));
    // linux and windows
    char* p = strrchr(module_file_name, '/');
    if ( !p )
        p = strrchr(module_file_name, '\\');

    CHECK(p) << "bad file name: " << module_file_name;
    safe_snprintf(m_module_name, sizeof(m_module_name), p + 1);
    CHECK(m_ca.Init()) << "init ca fail";

}


// 用户可以继承这个函数实现具体响应
bool MySimpleHttpThread::OnUserHTTPRequest(const BufferV* received_buff,
                                            CHttpBuff* http_response) {
    // 每次收到请求,都先判断是否需要做CheckPoint
    WriteCheckPoint();

    // 校验用户有效性
    char check_user[] = "/ca_check_user.html";
    int32_t len_check_user = sizeof(check_user) - 1;
    if (0 ==
        strncmp(reinterpret_cast<const char*>(received_buff->buff), check_user, len_check_user)) {
        CheckIdentity(received_buff, http_response);
        return true;
    }

    char ca_index[] = "/index.html";
    char ca_home[] = "/";
    int32_t len_ca_index = sizeof(ca_index) - 1;
    if (0 == strncmp(reinterpret_cast<const char*>(received_buff->buff), ca_index, len_ca_index) ||
        0 == strcmp(reinterpret_cast<const char*>(received_buff->buff), ca_home)) {
        Index(received_buff, http_response);
        return true;
    }

    // 欢迎界面
    char ca_welcome[] = "/ca_welcome.html";
    int32_t len_ca_welcome = sizeof(ca_welcome) - 1;
    if (0 ==
        strncmp(reinterpret_cast<const char*>(received_buff->buff),  ca_welcome, len_ca_welcome)) {
        Welcome(received_buff, http_response);
        return true;
    }

    // 注册证书
    char ca_register[] = "/ca_register.html";
    int32_t len_ca_register = sizeof(ca_register) - 1;
    if (0 ==
        strncmp(reinterpret_cast<const char*>(received_buff->buff), ca_register, len_ca_register)) {
        Register(received_buff, http_response);
        return true;
    }

    // 证书续期，更新
    char ca_renew[] = "/ca_renew.html";
    int32_t len_ca_renew = sizeof(ca_renew) - 1;
    if (0 == strncmp(reinterpret_cast<const char*>(received_buff->buff), ca_renew, len_ca_renew)) {
        Renew(received_buff, http_response);
        return true;
    }

    // 删除证书
    char ca_revoke[] = "/ca_revoke.html";
    int32_t len_ca_revoke = sizeof(ca_revoke) - 1;
    if (0 ==
        strncmp(reinterpret_cast<const char*>(received_buff->buff), ca_revoke, len_ca_revoke)) {
        Revoke(received_buff, http_response);
        return true;
    }

    // 下载私钥
    char download[] = "/ca_download.html";
    int32_t len_download = sizeof(download) - 1;
    if (0 == strncmp(reinterpret_cast<const char*>(received_buff->buff), download, len_download)) {
        Download(received_buff, http_response);
        return true;
    }

    // --------------begin role manager------------------
    // role manager index page
    const char page_role_manager[] = "/role_manager.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_role_manager,
        sizeof(page_role_manager) - 1) == 0) {
        http_response->SetAttr("role_manager");
        DealRoleManagerReq(received_buff, http_response);
        return true;
    }

    // add identity
    const char page_add_identity[] = "/add_identity.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_add_identity,
        sizeof(page_add_identity) - 1) == 0) {
        AddIdentity(received_buff, http_response);
        return true;
    }

    // del identity
    const char page_del_identity[] = "/del_identity.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_del_identity,
        sizeof(page_del_identity) - 1) == 0) {
        DelIdentity(received_buff, http_response);
        return true;
    }

    // query identity roles
    const char page_query_identity_roles[] = "/query_identity_roles.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_query_identity_roles,
        sizeof(page_query_identity_roles) - 1) == 0) {
        QueryIdentityRoles(received_buff, http_response);
        return true;
    }

    // add role
    const char page_add_role[] = "/add_role.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_add_role,
        sizeof(page_add_role) - 1) == 0) {
        AddRole(received_buff, http_response);
        return true;
    }

    // del role
    const char page_del_role[] = "/del_role.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_del_role,
        sizeof(page_del_role) - 1) == 0) {
        DelRole(received_buff, http_response);
        return true;
    }

    // query role identities
    const char page_query_role_identities[] = "/query_role_identities.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_query_role_identities,
        sizeof(page_query_role_identities) - 1) == 0) {
        QueryRoleIdentities(received_buff, http_response);
        return true;
    }

    // add identity to role
    const char page_add_relation[] = "/add_relation.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_add_relation,
        sizeof(page_add_relation) - 1) == 0) {
        AddIdentityToRole(received_buff, http_response);
        return true;
    }

    // del identity from role
    const char page_del_relation[] = "/del_relation.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_del_relation,
        sizeof(page_del_relation) - 1) == 0) {
        DelIdentityFromRole(received_buff, http_response);
        return true;
    }

    // display all identities
    const char page_display_identities[] = "/display_identities.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_display_identities,
        sizeof(page_display_identities) - 1) == 0) {
        http_response->SetAttr("display_identities");
        PrintAllIdentities(received_buff, http_response);
        return true;
    }

    // display all roles
    const char page_display_roles[] = "/display_roles.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_display_roles,
        sizeof(page_display_roles) - 1) == 0) {
        http_response->SetAttr("display_roles");
        PrintAllRoles(received_buff, http_response);
        return true;
    }

    // --------------begin quota manager------------------
    // quota manager index page
    const char page_quota_manager[] = "/quota_manager.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_quota_manager,
        sizeof(page_quota_manager) - 1) == 0) {
        http_response->SetAttr("quota_manager");
        DealQuotaManagerReq(received_buff,http_response);
        return true;
    }

    // set role quota page
    const char page_set_role_quota[] = "/set_role_quota.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_set_role_quota,
        sizeof(page_set_role_quota) - 1) == 0) {
        http_response->SetAttr("set_role_quota");
        SetRoleQuota(received_buff, http_response);
        return true;
    }

    // query role quota page
    const char page_query_role_quota[] = "/query_role_quota.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_query_role_quota,
        sizeof(page_query_role_quota) - 1) == 0) {
        http_response->SetAttr("query_role_quota");
        QueryRoleQuota(received_buff, http_response);
        return true;
    }

    // list all roles quota page
    const char page_list_all_roles_quota[] = "/list_all_roles_quota.html";
    if (STRNCASECMP(reinterpret_cast<const char*>(received_buff->buff), page_list_all_roles_quota,
        sizeof(page_list_all_roles_quota) - 1) == 0) {
        http_response->SetAttr("list_all_roles_quota");
        ListAllRolesQuota(received_buff, http_response);
        return true;
    }


    // --------------begin authenticate record------------------
    // 查看用户认证历史记录
    char auth_record[] = "/authenticate_record.html";
    int32_t len_auth = sizeof(auth_record) - 1;
    if (0 == strncmp(reinterpret_cast<const char*>(received_buff->buff), auth_record, len_auth)) {
        DealAuthenticateRecordReq(received_buff, http_response);
        return true;
    }

    // -------------------backup CA Data------------------------
    // 如果是备,有reload功能
    if (FLAGS_ca_server_role == "Secondary") {
        char reload[] = "/reload.html";
        int32_t len_reload = sizeof(reload) - 1;
        if (0 == strncmp(reinterpret_cast<const char*>(received_buff->buff), reload, len_reload)) {
            Reload(received_buff, http_response);
            return true;
        }
    }

    return false;
}

void MySimpleHttpThread::Index(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;
    http_response->SetAttr("ca_index", m_module_name, false);
    // 如果是ca_onebox，则无需token登录, 用户名默认为onebox_test，可用FLAGS更改
    if (FLAGS_ca_onebox) {
        // 跳转到欢迎界面
        char html_buff[1024] = {0};
        const char* proxy = GetProxyRequest();
        safe_snprintf(html_buff, sizeof(html_buff),
            "<meta http-equiv='Refresh'"
            "content='0;"
            "URL=%s%shttp://%s/ca_welcome.html?"
            "CA_USERNAME=%s&home=http://xfs.soso.oa.com/index.html' />",
            proxy, STRLEN(proxy)? "?realserver=":"",
            GetListenHostPort(), FLAGS_ca_onebox_user_name.c_str());
        http_response->AddKey("", html_buff, "");
        return;
    }

    // 如果不是ca_onebox，则跳转到OA登录，带回ticket信息
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        // 如果？后面没有参数，自动跳转
        TurnToOA(http_response);
        return;
    }

    // 从OA登录后转回，带回ticket信息，从中解析出用户名
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_ticket = NULL;
    m_cgi_parser.GetParameter("ticket", &user_ticket);
    if (!user_ticket) {
        // 第一次时转到OA登录，带回ticket
        TurnToOA(http_response);
        return;
    }
    // 如果有ticket信息，则从中解析出用户名
    char user_name[kUserNameLen] = {0};
    // 自己封包发送ticket，替代使用soap
    if (!ParseOAName(user_ticket, reinterpret_cast<char*>(user_name))) {
        http_response->AddKey("", "get username error, please relogin", "");
        return;
    }
    // 从ticket解析出用户名后
    // 将用户名+一个随机数+和timeout=1天，加密，便于之后通信中的身份确认

    time_t time_out = time(NULL) + DEFAULT_CA_TICKET_TIMEOUT;
    srand((uint32_t)time(0));
    uint32_t randkey = 1 + safe_rand() % 100;
    unsigned char tmp[128] = {0};
    safe_snprintf(reinterpret_cast<char*>(tmp), sizeof(tmp),
                  "ENCRYPT_NAME=%s&ENCRYPT_TIMEOUT=%lld&ENCRYPT_RANDKEY=%d",
                  user_name,  time_out, randkey);
    unsigned char str_encrypted[kEncryptCaTokenLen] = {0};
    if (!SimpleEncrypt(tmp, 128, str_encrypted, kEncryptCaTokenLen))
        return;

    // 携带用户唯一性标志的加密信息跳转到欢迎界面
    char html_buff[1024] = {0};
    const char* proxy = GetProxyRequest();
    safe_snprintf(html_buff,
                  sizeof(html_buff),
                  "<meta http-equiv='Refresh'"
                  "content='0;"
                  "URL=%s%shttp://%s/ca_welcome.html?"
                  "CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d&home=http://xfs.soso.oa.com/index.html' />",
                  proxy,
                  STRLEN(proxy)? "?realserver=":"",
                  GetListenHostPort(),
                  user_name, str_encrypted, kEncryptCaTokenLen);
    http_response->AddKey("", html_buff, "");
}

void MySimpleHttpThread::Welcome(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    http_response->SetAttr("ca_welcome", m_module_name, false);

    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
    m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
    m_cgi_parser.GetParameter("LEN", &len_encrypt);
    if (NULL == user_name) {
        http_response->AddKey("", "parameter missing: CA_USERNAME=xxx");
        return;
    }

    if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
        http_response->AddKey("", "ca_token check failed", "");
        if (NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
        }
        return;
    }

    SetBanner(user_name, http_response);
    CACertificate ca_certificate;
    char link_addr[1024] = {0};
    // 用户验证OK，则看用户是否注册过，显示不同的内容
    if (!m_ca.IsExist(user_name)) { // 如果磁盘上没有用户的证书
        // 让用户注册
        http_response->BeginGroup("user state");
        http_response->AddKey("Certifacte State", "Unregistered", "");
        http_response->EndGroup("user state");

        http_response->BeginGroup("user certificate info");

        safe_snprintf(link_addr, sizeof(link_addr),
                      "http://%s/ca_register.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                      GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);

        http_response->AddInternalHrefKey("Register Certificate", link_addr, "", "");
        http_response->EndGroup("user certificate info");
    } else { // 如果磁盘上有用户的证书
        // 已经注册过，则直接显示用户证书
        R_RSA_PUBLIC_KEY public_key;
        char             public_key_path[MAX_PATH] = {0};
        CA_ERROR_CODE error_code_pub;

        if (m_ca.GetUserPublicInfo(user_name, &ca_certificate, &public_key,
                                   public_key_path, MAX_PATH, &error_code_pub)) {
            DisplayUserInfo(&ca_certificate, user_name, error_code_pub,
                            ca_token_encrypt, len_encrypt, http_response);
        } else {
            // 存在用户证书，但证书异常
            http_response->AddKey("", "get certificate error,please delete and register again", "");

            safe_snprintf(link_addr, sizeof(link_addr),
                          "http://%s/ca_revoke.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                          GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
            http_response->AddInternalHrefKey("Delete Certificate", link_addr, "", "");
        }
    }
}

void MySimpleHttpThread::Register(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    http_response->SetAttr("ca_register", m_module_name, false);

    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
    m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
    m_cgi_parser.GetParameter("LEN", &len_encrypt);
    if (NULL == user_name) {
        http_response->AddKey("", "parameter missing: CA_USERNAME=xxx");
        return;
    }

    if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
        http_response->AddKey("", "ca_token check failed", "");
        if (NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
        }
        return;
    }

    SetBanner(user_name, http_response);
    CACertificate ca_certificate;
    CA_ERROR_CODE error_code;
    int32_t ret_regist = m_ca.RegistCertificate(user_name, &ca_certificate, &error_code);
    if (ret_regist && (ERROR_CA_OK == error_code)) { // 注册成功
        CA_ERROR_CODE add_user_error_code;
        // 添加用户
        RoleManager::GetInstance()->AddIdentity(user_name, &add_user_error_code);

        // display certificate info
        DisplayUserInfo(&ca_certificate, user_name, error_code,
                        ca_token_encrypt, len_encrypt, http_response);
    }
}


void MySimpleHttpThread::Renew(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    http_response->SetAttr("ca_renew", m_module_name, false);
    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
    m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
    m_cgi_parser.GetParameter("LEN", &len_encrypt);
    if (NULL == user_name) {
        http_response->AddKey("", "parameter missing: CA_USERNAME=xxx");
        return;
    }

    if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
        http_response->AddKey("", "ca_token check failed", "");
        if (NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
        }
        return;
    }

    SetBanner(user_name, http_response);
    // get certificate info
    CACertificate ca_certificate;
    R_RSA_PUBLIC_KEY public_key;
    char             public_key_path[MAX_PATH];
    m_ca.ApplyRenewKey(user_name);
    CA_ERROR_CODE error_code;
    m_ca.GetUserPublicInfo(user_name, &ca_certificate, &public_key,
                           public_key_path, MAX_PATH, &error_code);
    DisplayUserInfo(&ca_certificate, user_name, error_code,
                    ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::Revoke(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    http_response->SetAttr("ca_revoke", m_module_name, false);

    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
    m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
    m_cgi_parser.GetParameter("LEN", &len_encrypt);
    if (NULL == user_name) {
        http_response->AddKey("", "parameter missing: CA_USERNAME=xxx");
        return;
    }

    if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
        http_response->AddKey("", "ca_token check failed", "");
        if (NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
        }
        return;
    }

    SetBanner(user_name, http_response);
    if (m_ca.RevokeCertificate(user_name)) {
        // 删除证书后，显示让用户注册的页面
        CACertificate ca_certificate;
        http_response->BeginGroup("user state");
        http_response->AddKey("Certifacte State", "Unregistered", "");
        http_response->EndGroup("user state");

        http_response->BeginGroup("user certificate info");
        char link_addr[1024] = {0};
        safe_snprintf(link_addr, sizeof(link_addr),
                      "http://%s/ca_register.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                      GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
        http_response->AddInternalHrefKey("Register Certificate", link_addr, "", "");
        http_response->EndGroup("user certificate info");
    }
}

void MySimpleHttpThread::Download(const BufferV* received_buff,
                                   CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    http_response->SetAttr("ca_download", m_module_name, false);

    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
    m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
    m_cgi_parser.GetParameter("LEN", &len_encrypt);
    if (NULL == user_name) {
        http_response->AddKey("", "parameter missing: CA_USERNAME=xxx");
        return;
    }

    if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
        http_response->AddKey("", "ca_token check failed", "");
        if (NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
        }
        return;
    }
    SetBanner(user_name, http_response);
    CAPrivate user_private_key_file;
    char user_private_key_path[MAX_PATH];
    CA_ERROR_CODE error_code;
    m_ca.GetUserPrivateInfo(user_name, &user_private_key_file, user_private_key_path,
                            MAX_PATH, &error_code);
    char html_buff[1024] = {0};
    const char* proxy = GetProxyRequest();
    safe_snprintf(html_buff, sizeof(html_buff),
                  "<meta http-equiv='Refresh' content='0; URL=%s%shttp://%s/%s' />",
                  proxy, STRLEN(proxy)? "?realserver=":"",
                  GetListenHostPort(), user_private_key_path);
    http_response->AddKey("", html_buff, "");
}


void MySimpleHttpThread::CheckIdentity(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response) {
        return;
    }

    const char* key_check_user_ret = "check_user_result";
    CA_ERROR_CODE error_code = ERROR_CA_OK;
    char check_user_result[512];
    http_response->SetAttr("ca_check_user", m_module_name, false);
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        error_code = ERROR_CA_PAPRAM_INVALID;
        safe_snprintf(check_user_result, sizeof(check_user_result),
                      "CheckUserResult=FAIL ErrorCode=%d", error_code);
        LOG(ERROR) << "ERR_CA_PAPRAM_INVALID can't find ? in parameter";
        http_response->AddKey(key_check_user_ret, check_user_result, "");
        return;
    }

    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* ca_username = NULL;
    m_cgi_parser.GetParameter("CA_USERNAME", &ca_username);
    const char* ca_rolename = NULL;
    m_cgi_parser.GetParameter("CA_ROLENAME", &ca_rolename);
    if (!ca_username) {
        error_code = ERROR_CA_PAPRAM_INVALID;
        safe_snprintf(check_user_result, sizeof(check_user_result),
                      "CheckUserResult=FAIL ErrorCode=%d", error_code);
        LOG(ERROR) << "ERR_CA_PAPRAM_INVALID can't find user_name or ca_rolename in parameter";
        LOG(ERROR) << "user_name = " << ca_username << " role_name = " << ca_rolename;
        http_response->AddKey(key_check_user_ret, check_user_result, "");
        return;
    }
    // 记录用户认证信息
    // TODO:(joeytian)
    // ip = xxx.xxx.xxx.xxx 这里暂时获取不到ip，等待wookin修改simple_http或者由ca_client传ip后再来完善
    const char* client_ip = "";
    AuthRecord::GetInstance()->AppendRecord(ca_username, client_ip);
    const char* sign = NULL;
    m_cgi_parser.GetParameter("SIGN", &sign);
    uint32_t sign_len = 0;
    m_cgi_parser.GetParameter("SIGN_LEN", &sign_len);
    if (!sign || !sign_len) {
        error_code = ERROR_CA_PAPRAM_INVALID;
        safe_snprintf(check_user_result, sizeof(check_user_result),
                      "CheckUserResult=FAIL ErrorCode=%d", error_code);
        LOG(ERROR) << "ERR_CA_PAPRAM_INVALID can't find sign or sign_len in parameter";
        LOG(ERROR) << "sign = " << sign << "sign_len = " << sign_len;
        http_response->AddKey(key_check_user_ret, check_user_result, "");
        return;
    }

    char str_sign[256] = {0};
    if (String2Hex(sign, str_sign, sign_len)) {
        bool ret_valid = m_ca.IsValidUser(ca_username, str_sign, sign_len, &error_code);
        if (!ret_valid)
            LOG(ERROR) << "IsValidUser error";
        // TODO : compatible for old user checker
        if (ret_valid && ca_rolename) {
            ret_valid =
                RoleManager::GetInstance()->VerifyIdentity(ca_rolename, ca_username, &error_code);
            if (!ret_valid)
                LOG(ERROR) << "VerifyIdentity error";
        }
         safe_snprintf(check_user_result, sizeof(check_user_result),
                       "CheckUserResult=%s ErrorCode=%d", ret_valid ? "OK" : "FAIL", error_code);
        http_response->AddKey(key_check_user_ret, check_user_result, "");
    } else {
        error_code = ERROR_CA_PAPRAM_INVALID;
        safe_snprintf(check_user_result, sizeof(check_user_result),
                      "CheckUserResult=FAIL ErrorCode=%d", error_code);
        http_response->AddKey(key_check_user_ret, check_user_result, "");
    }
}


void MySimpleHttpThread::DisplayUserInfo(CACertificate* ca_certificate,
                                          const char*  user_name,
                                          CA_ERROR_CODE  certificate_state,
                                          const char*  ca_token_encrypt,
                                          uint32_t len_encrypt,
                                          CHttpBuff* http_response) {
    if (!ca_certificate || !user_name) {
        http_response->AddKey("", "parameter error", "");
        return;
    }
    // get certificate info
    http_response->BeginGroup("user state");
    http_response->AddKey("Certificate State", "Registered", "");
    http_response->EndGroup("user state");


    http_response->BeginGroup("user certificate info");

    char link_addr[1024] = {0};
    if (certificate_state == ERROR_CA_OK || certificate_state == ERROR_CA_OUT_OF_DATE) {
        if (certificate_state == ERROR_CA_OUT_OF_DATE)
            http_response->AddKey("", "certificate out of date, please renew", "");
        http_response->AddKey("UserName", user_name, "");
        http_response->AddKey("Memo", ca_certificate->memo, "");
        char version[16];
        safe_snprintf(version, 16, "%d", static_cast<int32_t>(ca_certificate->version));
        http_response->AddKey("Version", version, "");
        char gmt_time_start[128];
        char gmt_time_end[128];

        safe_ctime(&(ca_certificate->ca_certificate_data.certificate_time_begin),
                   sizeof(gmt_time_start), gmt_time_start);
        http_response->AddKey("BeginTime", gmt_time_start, "");

        safe_ctime(&(ca_certificate->ca_certificate_data.certificate_time_end),
                   sizeof(gmt_time_end), gmt_time_end);
        http_response->AddKey("EndTime", gmt_time_end, "");
        http_response->EndGroup("user certificate info");


        http_response->BeginGroup("operation");

        safe_snprintf(link_addr, sizeof(link_addr),
                      "http://%s/ca_renew.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                      GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
        http_response->AddInternalHrefKey("Renew Certificate", link_addr, "", "");
        safe_snprintf(link_addr, sizeof(link_addr),
                      "http://%s/ca_revoke.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                      GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
        http_response->AddInternalHrefKey("Delete Certificate", link_addr, "", "");
        safe_snprintf(link_addr, sizeof(link_addr),
                      "http://%s/ca_download.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                      GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
        http_response->AddInternalHrefKey("Download_Private_key", link_addr, "", "");
        http_response->EndGroup("operation");

    } else {
        http_response->AddKey("", "get certificate error,please delete and register again", "");

        safe_snprintf(link_addr, sizeof(link_addr),
                      "http://%s/ca_revoke.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                      GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
        http_response->AddInternalHrefKey("Delete Certificate", link_addr, "", "");
        http_response->EndGroup("user certificate info");
    }

    // show role manager page
    safe_snprintf(link_addr, sizeof(link_addr),
                  "http://%s/role_manager.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                  GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);

    http_response->BeginGroup("role_manager");
    http_response->AddInternalHrefKey("RoleManager", link_addr, "", "");
    http_response->EndGroup("role_manager");

    // show quota manager page
    safe_snprintf(link_addr, sizeof(link_addr),
        "http://%s/quota_manager.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
        GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);

    http_response->BeginGroup("quota_manager");
    http_response->AddInternalHrefKey("QuotaManager", link_addr, "", "");
    http_response->EndGroup("quota_manager");

    // 默认显示当天的统计情况
    safe_snprintf(link_addr, sizeof(link_addr),
        "http://%s/authenticate_record.html?CA_USERNAME=%s&LEN=%d&CA_TOKEN=%s",
         GetListenHostPort(), user_name, len_encrypt, ca_token_encrypt);

    http_response->BeginGroup("authenticate_record");
    http_response->AddInternalHrefKey("AuthenticateRecordManager", link_addr, "", "");
    http_response->EndGroup("authenticate_record");
}

bool MySimpleHttpThread::String2Hex(const char* str, char* hex, int len) {
    if (!str || !hex)
        return false;

    int i = 0;
    for ( ; i < len; i ++) {
        char one_byte[3];
        char* stop = NULL;
        memcpy(one_byte, &str[i * 2], 2);
        one_byte[2] = '\0';
        hex[i] = strtol(one_byte, &stop, 16);
    }
    return true;
}

void MySimpleHttpThread::TurnToOA(CHttpBuff* http_response) {
    if (!http_response)
        return;

    char html_buff[1024] = {0};
    const char* proxy = GetProxyRequest();
    safe_snprintf(html_buff, sizeof(html_buff),
                  "<meta http-equiv='Refresh'"
                  "content='0;"
                  "URL=http://passport.oa.com/modules/passport/signin.ashx?"
                  "url=%s%shttp://%s/index.html' />",
                  proxy ,
                  STRLEN(proxy)? "?realserver=":"",
                  GetListenHostPort());
    http_response->AddKey("", html_buff, "");
}

bool MySimpleHttpThread::ParseOAName(const char* user_ticket, char* user_name) {
    if (!user_ticket || !user_name)
        return false;

    const char* host_name = "passport.oa.com";
    struct hostent* ht = gethostbyname(host_name);
    if (!ht || ht->h_addr_list == NULL || ht->h_length <= 0 || ht->h_addr == NULL)
        return false;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    memcpy(&addr.sin_addr.s_addr, ht->h_addr, sizeof(addr.sin_addr.s_addr));
    char* host_addr = inet_ntoa(addr.sin_addr);

    const char* cgi = "/services/passportservice.asmx";
    uint16_t port = 80;

    char http_param[2048] = {0};
    int32_t len_param =
        safe_snprintf(http_param, sizeof(http_param),
                      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                      "<SOAP-ENV:Envelope "
                      "xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" "
                      "xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" "
                      "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
                      "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
                      "xmlns:ns1=\"http://indigo.oa.com/services/\">"
                      "<SOAP-ENV:Body>"
                      "<ns1:DecryptTicket><ns1:encryptedTicket>"
                      "%s"
                      "</ns1:encryptedTicket></ns1:DecryptTicket>"
                      "</SOAP-ENV:Body>"
                      "</SOAP-ENV:Envelope>",
                      user_ticket);

    char http_head[1000] = {0};
    safe_snprintf(http_head, sizeof(http_head),
                  "POST %s HTTP/1.1\r\n"
                  "Host: %s\r\n"
                  "User-Agent: gSOAP/2.7\r\n"
                  "Content-Type: text/xml; charset=utf-8\r\n"
                  "Content-Length: %d\r\n"
                  "Connection: close\r\n"
                  "SOAPAction: \"http://indigo.oa.com/services/DecryptTicket\"\r\n\r\n",
                  cgi,  host_name, len_param);
    HTTP_CGI_ERROR ret_post = m_get_http_response.GetResponse(host_addr, port, cgi, http_param,
                                                              true, 5, http_head);
    if (ret_post != ERROR_HTTP_OK)
        return false;

    const char* parse_result = m_get_http_response.GetHttpContent();
    const char* p = strstr(parse_result, "<LoginName>");
    const char* q = strstr(parse_result, "</LoginName>");
    char oa_login_name[] = "<LoginName>";
    int32_t pad_len = sizeof(oa_login_name) - 1;
    if (!p || !q)
        return false;
    memcpy(user_name, p + pad_len, (q - p - pad_len));
    return true;
}

bool MySimpleHttpThread::CheckCaToken(const char* user_name, const char* ca_token_encrypt,
                                       uint32_t len_encrypt) {
    if (FLAGS_ca_onebox)
        return true;

    if (!user_name || !ca_token_encrypt)
        return false;


    // 解密ca_tocken，验证
    uint32_t       len_plaintext = (len_encrypt - 4) / 2;
    unsigned char* plaintext = new unsigned char[len_plaintext];
    if (!SimpleDecrypt(ca_token_encrypt, len_encrypt,
                       plaintext, len_plaintext)) {
        delete []plaintext;
        plaintext = NULL;
        return false;
    }

    const char*    encrypt_name = NULL;
    time_t   encrypt_timeout = 0;
    uint32_t encrypt_randkey = 0;
    CParserCGIParameter cgi_parser;
    cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(plaintext));
    cgi_parser.GetParameter("ENCRYPT_NAME", &encrypt_name);
    cgi_parser.GetParameter("ENCRYPT_TIMEOUT", reinterpret_cast<uint32_t*>(&encrypt_timeout));
    cgi_parser.GetParameter("ENCRYPT_RANDKEY", &encrypt_randkey);
    if (!encrypt_name || 0 == encrypt_timeout || 0 == encrypt_randkey) {
        delete []plaintext;
        plaintext = NULL;
        return false;
    }
    int32_t ret_cmp = strcmp(encrypt_name, user_name);
    time_t time_now = time(NULL);
    int32_t ret_timeout = (time_now < encrypt_timeout) ? 0 : -1;

    if (0 != ret_cmp || 0 != ret_timeout) {
        delete []plaintext;
        plaintext = NULL;
        return false;
    }
    delete []plaintext;
    plaintext = NULL;
    return true;
}


// 简单的加密
// 加密是将单个字符的ASCII码数字存入内存,如a,其存入内存是61,占2个char:'6''1'
bool MySimpleHttpThread::SimpleEncrypt(unsigned char* plaintext, uint32_t  len_plaintext,
                                        unsigned char* str_encrypted, uint32_t len_str_encrypted) {
    if (!plaintext || !str_encrypted || len_str_encrypted < 2*len_plaintext+2)
        return false;

    unsigned char* ptr = str_encrypted;
    for (uint32_t i = 0; i < len_plaintext; ++i) {
        safe_snprintf(reinterpret_cast<char*>(ptr), 3, "%x", plaintext[i]);

        ptr += 2;
    }
    return true;
}

// 简单解密
// 解密则需要每2个char组合,才能得到一个原始的字符
bool MySimpleHttpThread::SimpleDecrypt(const char* str_encrypted, uint32_t  len_str_encrypted,
                                        unsigned char* plaintext, uint32_t len_plaintext) {
    if (!str_encrypted || !plaintext || len_str_encrypted < 2*len_plaintext+2 )
        return false;

    char tmp[8] = {0};
    uint32_t len = 0;
    for (uint32_t i = 0; i < len_str_encrypted - 4; i++) {
        tmp[0] = str_encrypted[i];
        tmp[1] = str_encrypted[++i];
        plaintext[len] = (unsigned char)strtol(tmp, reinterpret_cast<char**>(NULL), 16);
        len++;
    }
    return true;
}

void MySimpleHttpThread::SetBanner(const char* user_name, CHttpBuff* http_response) {
    if (!user_name || !http_response)
        return;
    const char* proxy = GetProxyRequest();
    char login_page[256] = {0};
    safe_snprintf(login_page, sizeof(login_page),
                  "http://passport.oa.com/modules/passport/signout.ashx?"
                  "url=%s%shttp://%s/index.html",
                  proxy,
                  STRLEN(proxy)? "?realserver=":"",
                  GetListenHostPort());
    http_response->SetBanner(user_name, login_page);
}

void MySimpleHttpThread::AddIdentity(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    CXThreadAutoLock auto_lock(&g_http_mutex);
    http_response->SetAttr("add_identity", m_module_name, false);
    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);
    }

    const char* identity = NULL;
    m_cgi_parser.GetParameter("identity_name", &identity);
    if (!identity) {
        PrintReqPage("add_identity", NEED_USER_NAME,
                      user_name, ca_token_encrypt, len_encrypt, http_response);
        SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
        return;
    }

    CA_ERROR_CODE err;
    if (RoleManager::GetInstance()->AddIdentity(identity, &err)) {
        http_response->SetAttr("AddIdentityOK");
        http_response->AddKey("AddIdentityOK", "", "");
    } else {
        http_response->SetAttr("AddIdentityFail");
        http_response->AddKey("AddIdentityFail", CaGetErrorCodeStr(err), "");
    }

    g_has_modified = true;
    SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::DelIdentity(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;
    CXThreadAutoLock auto_lock(&g_http_mutex);
    http_response->SetAttr("del_identity", m_module_name, false);

    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);
    }

    const char* identity = NULL;
    m_cgi_parser.GetParameter("identity_name", &identity);
    if (!identity) {
        PrintReqPage("del_identity", NEED_USER_NAME,
                      user_name, ca_token_encrypt, len_encrypt, http_response);
        SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
        return;
    }

    CA_ERROR_CODE err;
    if (RoleManager::GetInstance()->DelIdentity(identity, &err)) {
        http_response->SetAttr("DelIdentityOK");
        http_response->AddKey("DelIdentityOK", "", "");
    } else {
        http_response->SetAttr("DelIdentityFail");
        http_response->AddKey("DelIdentityFail", CaGetErrorCodeStr(err), "");
    }

    g_has_modified = true;
    SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::QueryIdentityRoles(const BufferV* received_buff,
                                             CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    http_response->SetAttr("query_identity_roles", m_module_name, false);
    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);
    }

    const char* identity = NULL;
    m_cgi_parser.GetParameter("identity_name", &identity);
    if (!identity) {
        PrintReqPage("query_identity_roles", NEED_USER_NAME,
                      user_name, ca_token_encrypt, len_encrypt, http_response);
        SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
        return;
    }

    CA_ERROR_CODE err;
    vector<string> list;
    if (RoleManager::GetInstance()->QueryIdentityRoles(identity, &list, &err)) {
        http_response->SetAttr("QueryIdentityRolesOK");
        http_response->BeginGroup(identity);
        for (vector<string>::iterator it = list.begin(); it != list.end(); ++it) {
            http_response->AddKey("role", it->c_str());
        }
        http_response->EndGroup(identity);
        http_response->AddKey("QueryIdentityRolesOK", "", "");
    } else {
        http_response->SetAttr("QueryIdentityRolesFail");
        http_response->AddKey("QueryIdentityRolesFail", CaGetErrorCodeStr(err), "");
    }
    SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::AddRole(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    CXThreadAutoLock auto_lock(&g_http_mutex);
    http_response->SetAttr("add_role", m_module_name, false);

    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);
    }

    const char* role_name = NULL;
    m_cgi_parser.GetParameter("role_name", &role_name);

    if (!role_name) {
        PrintReqPage("add_role", NEED_ROLE_NAME,
                      user_name, ca_token_encrypt, len_encrypt, http_response);
        SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
        return;
    }

    CA_ERROR_CODE err;
    if (RoleManager::GetInstance()->AddRole(role_name, &err)) {
        http_response->SetAttr("AddRoleOK");
        http_response->AddKey("AddRoleOK", "", "");
    } else {
        http_response->SetAttr("AddRoleFail");
        http_response->AddKey("AddRoleFail", CaGetErrorCodeStr(err), "");
    }

    g_has_modified = true;
    SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::DelRole(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    CXThreadAutoLock auto_lock(&g_http_mutex);
    http_response->SetAttr("del_role", m_module_name, false);
    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);
    }

    const char* role_name = NULL;
    m_cgi_parser.GetParameter("role_name", &role_name);

    if (!role_name) {
        PrintReqPage("del_role", NEED_ROLE_NAME,
                      user_name, ca_token_encrypt, len_encrypt, http_response);
        SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
        return;
    }

    CA_ERROR_CODE err;
    if (RoleManager::GetInstance()->DelRole(role_name, &err)) {
        http_response->SetAttr("DelRoleOK");
        http_response->AddKey("DelRoleOK", "", "");
    } else {
        http_response->SetAttr("DelRoleFail");
        http_response->AddKey("DelRoleFail", CaGetErrorCodeStr(err), "");
    }

    g_has_modified = true;
    SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::QueryRoleIdentities(const BufferV* received_buff,
                                              CHttpBuff* http_response) {
   if (!received_buff || !http_response)
        return;

    http_response->SetAttr("query_role_identities", m_module_name, false);
    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);
    }

    const char* role_name = NULL;
    m_cgi_parser.GetParameter("role_name", &role_name);

    if (!role_name) {
        PrintReqPage("query_role_identities", NEED_ROLE_NAME,
                      user_name, ca_token_encrypt, len_encrypt, http_response);
        SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
        return;
    }

    CA_ERROR_CODE err;
    vector<string> list;
    if (RoleManager::GetInstance()->QueryRoleIdentities(role_name, &list, &err)) {
        http_response->SetAttr("QueryRoleIdentitiesOK");
        http_response->BeginGroup(role_name);
        for (vector<string>::iterator it = list.begin(); it != list.end(); ++it) {
            http_response->AddKey("identity", it->c_str());
        }
        http_response->EndGroup(role_name);
        http_response->AddKey("QueryRoleIdentitiesOK", "", "");
    } else {
        http_response->SetAttr("QueryRoleIdentitiesFail");
        http_response->AddKey("QueryRoleIdentitiesFail", CaGetErrorCodeStr(err), "");
    }
    SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::AddIdentityToRole(const BufferV* received_buff,
                                            CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;
    
    CXThreadAutoLock auto_lock(&g_http_mutex);
    http_response->SetAttr("add_relation", m_module_name, false);
    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);
    }

    const char* identity = NULL;
    const char* role_name = NULL;
    m_cgi_parser.GetParameter("identity_name", &identity);
    m_cgi_parser.GetParameter("role_name", &role_name);

    if (!identity || !role_name) {
        PrintReqPage("add_relation", NEED_ROLE_NAME | NEED_USER_NAME,
                      user_name, ca_token_encrypt, len_encrypt, http_response);
        SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
        return;
    }

    CA_ERROR_CODE err;
    if (RoleManager::GetInstance()->AddIdentityToRole(role_name, identity, &err)) {
        http_response->SetAttr("AddRelationOK");
        http_response->AddKey("AddRelationOK", "", "");
    } else {
        http_response->SetAttr("AddRelationFail");
        http_response->AddKey("AddRelationFail", CaGetErrorCodeStr(err), "");
    }

    g_has_modified = true;
    SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::DelIdentityFromRole(const BufferV* received_buff,
                                              CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    CXThreadAutoLock auto_lock(&g_http_mutex);
    http_response->SetAttr("del_relation", m_module_name, false);
    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);
    }

    const char* identity = NULL;
    const char* role_name = NULL;
    m_cgi_parser.GetParameter("identity_name", &identity);
    m_cgi_parser.GetParameter("role_name", &role_name);

    if (!identity || !role_name) {
        PrintReqPage("del_relation", NEED_ROLE_NAME | NEED_USER_NAME,
                      user_name, ca_token_encrypt, len_encrypt, http_response);
        SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
        return;
    }

    CA_ERROR_CODE err;
    if (RoleManager::GetInstance()->DelIdentityFromRole(role_name, identity, &err)) {
       http_response->SetAttr("DelRelationOK");
       http_response->AddKey("DelRelationOK", "", "");
    } else {
       http_response->SetAttr("DelRelationFail");
       http_response->AddKey("DelRelationFail", CaGetErrorCodeStr(err), "");
    }

    g_has_modified = true;
    SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::PrintReqPage(const char* page_name, uint32_t req_type,
                                       const char* user_name, const char* ca_token_encrypt,
                                       uint32_t len_encrypt, CHttpBuff* http_response) {
    bool need_role_name = (req_type & NEED_ROLE_NAME) == 0 ? false : true;
    bool need_identity  = (req_type & NEED_USER_NAME) == 0 ? false : true;
    bool need_quota     = (req_type & NEED_QUOTA) == 0 ? false : true;
    bool need_cluster   = (req_type & NEED_CLUSTER) == 0 ? false : true;

    const char* proxy = GetProxyRequest();
    char java_script[1024] = {0};
    safe_snprintf(java_script, sizeof(java_script),
        "<script> "
        "function f_submit() { "
        "var act='%s%shttp://%s/%s.html?'"
        "%s%s%s%s%s"
        "+'&home=http://xfs.soso.oa.com/index.html';"
        "document.getElementById('req_list').action=act;"
        "}"
        "</script>",
        proxy,
        strlen(proxy)? "?realserver=":"",
        GetListenHostPort(),
        page_name,
        need_identity ?
        "+'identity_name='+document.getElementById('identity_name').value+'&'" : "",
        need_role_name ?
        "+'role_name='+document.getElementById('role_name').value+'&'" : "",
        need_cluster ?
        "+'cluster='+document.getElementById('cluster').value+'&'" : "",
        need_quota ?
        "+'num_chunks='+document.getElementById('num_chunks').value+'&'"
        "+'num_files='+document.getElementById('num_files').value+'&'"
        "+'num_directories='+document.getElementById('num_directories').value+'&'" : "",
        FLAGS_ca_onebox ? "" :
        "+'CA_USERNAME='+document.getElementById('CA_USERNAME').value"
        "+'&CA_TOKEN='+document.getElementById('CA_TOKEN').value"
        "+'&LEN='+document.getElementById('LEN').value"
        );
    http_response->AddKey(java_script, "", "");

    char form[1500] = {0};
    safe_snprintf(form,
        sizeof(form),
        "<form method='post' id='req_list'>"
        "<input type='hidden' name='CA_USERNAME' id='CA_USERNAME' value='%s'>"
        "<input type='hidden' name='CA_TOKEN' id='CA_TOKEN' value='%s'>"
        "<input type='hidden' name='LEN' id='LEN' value='%d'>"
        "<p>"
        "%s%s%s%s"
        "<input type='submit' value='submit' onclick='f_submit()'></p></form>",
        user_name,
        ca_token_encrypt,
        len_encrypt,
        need_identity ? "please input identity_name:"
        "<input type='text' name='identity_name' id='identity_name' size='20'>" : "",
        need_role_name ? "please input role_name:"
        "<input type='text' name='role_name' id='role_name' size='20'>" : "",
        need_cluster?
        "cluster:<input type='text' name='cluster' id='cluster' size='20'>" : "",
        need_quota ? "<br>please input quota:<br>"
        "chunks number:<input type='text' name='num_chunks' id='num_chunks' size='20'><br>"
        "files number:<input type='text' name='num_files' "
        "id='num_files' size='20'><br>"
        "directories number:<input type='text' name='num_directories' "
        "id='num_directories' size='20'>" : ""
        );

    //设置统计项 key - value 及描述信息
    http_response->AddKey(form, "", "");
}

void MySimpleHttpThread::SetRoleBottomLink(const char* user_name, const char* ca_token_encrypt,
                                        const uint32_t len_encrypt, CHttpBuff* http_response) {
    char link[1024] = {0};

    http_response->BeginGroup("Identity");
    safe_snprintf(link, sizeof(link),
                  "http://%s/add_identity.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                  GetListenHostPort(), user_name,  ca_token_encrypt, len_encrypt);
    http_response->AddInternalHrefKey("Add", link, "", "add an identity");

    safe_snprintf(link, sizeof(link),
                  "http://%s/del_identity.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                  GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
    http_response->AddInternalHrefKey("Delete", link, "", "delete an identity");

    safe_snprintf(link, sizeof(link),
                  "http://%s/query_identity_roles.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                  GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
    http_response->AddInternalHrefKey("Query", link, "", "query an identity's role list");

    safe_snprintf(link, sizeof(link),
        "http://%s/display_identities.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
        GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
    http_response->AddInternalHrefKey("ListAll", link, "", "list all identity names");
    http_response->EndGroup("Identity");

    http_response->BeginGroup("Role");
    safe_snprintf(link, sizeof(link),
                  "http://%s/add_role.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                  GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
    http_response->AddInternalHrefKey("Add", link, "", "add a role");

    safe_snprintf(link, sizeof(link),
                  "http://%s/del_role.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                  GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
    http_response->AddInternalHrefKey("Delete", link, "", "delete a role");

    safe_snprintf(link, sizeof(link),
                  "http://%s/query_role_identities.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                  GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
    http_response->AddInternalHrefKey("Query", link, "", "query a role's identity list");

    safe_snprintf(link, sizeof(link),
        "http://%s/display_roles.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
        GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
    http_response->AddInternalHrefKey("ListAll", link, "", "list all role names");
    http_response->EndGroup("Role");

    http_response->BeginGroup("Relation");
    safe_snprintf(link, sizeof(link),
                  "http://%s/add_relation.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                  GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
    http_response->AddInternalHrefKey("Add", link, "", "add an identity to a role");

    safe_snprintf(link, sizeof(link),
                  "http://%s/del_relation.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
                  GetListenHostPort(), user_name, ca_token_encrypt, len_encrypt);
    http_response->AddInternalHrefKey("Delete", link, "", "delete an identity from a role");
    http_response->EndGroup("Relation");
}

void MySimpleHttpThread::PrintAllIdentities(const BufferV* received_buff,
                                            CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    http_response->SetAttr("print_all_identities", m_module_name, false);
    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);
    }

    vector<string> list;
    RoleManager::GetInstance()->PrintAllIdentities(&list);
    http_response->SetAttr("PrintAllIdentitiesOK");
    http_response->BeginGroup("Result");
    for (vector<string>::iterator it = list.begin(); it != list.end(); ++it) {
        http_response->AddKey("identity", it->c_str());
    }
    http_response->EndGroup("Result");
    http_response->AddKey("PrintAllIdentitiesOK", "", "");
    SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::PrintAllRoles(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    http_response->SetAttr("print_all_roles", m_module_name, false);
    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);
    }

    vector<string> list;
    RoleManager::GetInstance()->PrintAllRoles(&list);
    http_response->SetAttr("PrintAllRolesOK");
    http_response->BeginGroup("Result");
    for (vector<string>::iterator it = list.begin(); it != list.end(); ++it) {
        http_response->AddKey("role", it->c_str());
    }
    http_response->EndGroup("Result");
    http_response->AddKey("PrintAllRolesOK", "", "");
    SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::PrintRoleManagerPage(const char* user_name, const char* ca_token_encrypt,
                                               uint32_t len_encrypt, CHttpBuff* http_response) {
    http_response->SetAttr("CaRoleMagnagerPage");


    char java_script[1024] = {0};
#ifdef WIN32
    safe_snprintf(java_script, sizeof(java_script),
        "<script> "
        "function f_submit() { "
        "var act='http://%s/role_manager.html';"
        "document.getElementById('dump_pwd_list').action=act;}"
        "</script>", GetListenHostPort());
#else // linux
    const char* proxy = GetProxyRequest();
    safe_snprintf(java_script, sizeof(java_script),
        "<script> "
        "function f_submit() { "
        "var act='%s%shttp://%s/role_manager.html?'"
        "+'pwd='+document.getElementById('pwd').value"
        "+'&home='+document.getElementById('home').value"
        "+'&CA_USERNAME='+document.getElementById('CA_USERNAME').value"
        "+'&CA_TOKEN='+document.getElementById('CA_TOKEN').value"
        "+'&LEN='+document.getElementById('LEN').value;"
        "document.getElementById('dump_pwd_list').action=act;}"
        "</script>",
        proxy,
        strlen(proxy)? "?realserver=":"",
        GetListenHostPort());
#endif
    http_response->AddKey(java_script, "", "");

    char form[1024] = {0};
    safe_snprintf(form, sizeof(form),
        "<form method='post' id='dump_pwd_list'>"
        "<input type='hidden' name='CA_USERNAME' id='CA_USERNAME' value='%s'>"
        "<input type='hidden' name='LEN' id='LEN' value='%d'>"
        "<input type='hidden' name='CA_TOKEN' id='CA_TOKEN' value='%s'>"
        "<input type='hidden' name='home' id='home' value='http://xfs.soso.oa.com/index.html'>"
        "<p>please input password:<input type='password' name='pwd' id='pwd' size='20'>"
        "<input type='submit' value='submit' onclick='f_submit()'></p>"
        "</form>",
        user_name, len_encrypt, ca_token_encrypt);

    http_response->AddKey(form, "", "");
}

void MySimpleHttpThread::DealRoleManagerReq(const BufferV* received_buff,
                                             CHttpBuff* http_response) {
    //dump master
     if (!received_buff || !http_response)
        return;

    http_response->SetAttr("ca_role_manager", m_module_name, false);

    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;

    // 在onebox环境中，跳过输入密码这一步骤，直接显示
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);

        const char* dump_pwd = NULL;
        m_cgi_parser.GetParameter("pwd", &dump_pwd);
        if (!dump_pwd) {
            PrintRoleManagerPage(user_name, ca_token_encrypt, len_encrypt, http_response);
            return;
        }
        if (0 != strncmp(dump_pwd, kAdminPwd, strlen(kAdminPwd))) {
            http_response->AddKey("PassWord incorrect", "", "");
            return;
        }
    }

    SetRoleBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::SetRoleQuota(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    CXThreadAutoLock auto_lock(&g_http_mutex);
    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);
    }

    const char* role_name = NULL;
    m_cgi_parser.GetParameter("role_name", &role_name);
    Quota quota;
    const char* cluster = 0;
    m_cgi_parser.GetParameter("cluster", &cluster);
    m_cgi_parser.GetParameter("num_chunks", &quota.num_chunks);
    m_cgi_parser.GetParameter("num_files", &quota.num_files);
    m_cgi_parser.GetParameter("num_directories", &quota.num_directories);

    if (!role_name) {
        PrintReqPage("set_role_quota", NEED_ROLE_NAME | NEED_QUOTA | NEED_CLUSTER,
            user_name, ca_token_encrypt, len_encrypt, http_response);
        SetQuotaBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
        return;
    }

    CA_ERROR_CODE err;
    if (QuotaManager::GetInstance()->SetRoleQuota(role_name, cluster, quota, &err)) {
        http_response->SetAttr("SetRoleQuotaOK");
        http_response->AddKey("SetRoleQuotaOK", "", "");
    } else {
        http_response->SetAttr("SetRoleQuotaFail");
        http_response->AddKey("SetRoleQuotaFail", CaGetErrorCodeStr(err), "");
    }

    g_has_modified = true;
    SetQuotaBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::QueryRoleQuota(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);
    }

    const char* role_name = NULL;
    const char* cluster = 0;
    m_cgi_parser.GetParameter("cluster", &cluster);
    m_cgi_parser.GetParameter("role_name", &role_name);

    if (!role_name) {
        PrintReqPage("query_role_quota", NEED_ROLE_NAME | NEED_CLUSTER,
            user_name, ca_token_encrypt, len_encrypt, http_response);
        SetQuotaBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
        return;
    }

    CA_ERROR_CODE err;
    Quota quota;
    if (QuotaManager::GetInstance()->QueryRoleQuota(role_name, cluster, &quota, &err)) {
        http_response->SetAttr("QueryRoleQuotaOK");
        http_response->AddKey("QueryRoleQuotaOK", "", "");
        http_response->BeginGroup(role_name);
        http_response->AddKey("cluster", cluster, "");
        http_response->AddKey("num_chunks", quota.num_chunks, "");
        http_response->AddKey("num_files", quota.num_files, "");
        http_response->AddKey("num_directories", quota.num_directories, "");
        http_response->EndGroup(role_name);
    } else {
        http_response->SetAttr("QueryRoleQuotaFail");
        http_response->AddKey("QueryRoleQuotaFail", CaGetErrorCodeStr(err), "");
    }

    SetQuotaBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::ListAllRolesQuota(const BufferV* received_buff, CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);
    }

    CA_ERROR_CODE err;
    std::map<RoleCluster, Quota> roles_quota;
    QuotaManager::GetInstance()->ListAllRolesQuota(&roles_quota, &err);
    http_response->SetAttr("ListAllRolesQuotaOK");
    http_response->AddKey("ListAllRolesQuotaOK", "", "");
    std::map<RoleCluster, Quota>::iterator it = roles_quota.begin();
    for (; it != roles_quota.end(); ++it) {
        http_response->BeginGroup(it->first.first.c_str());
        http_response->AddKey("cluster", it->first.second.c_str(), "");
        http_response->AddKey("num_chunks", it->second.num_chunks, "");
        http_response->AddKey("num_files", it->second.num_files, "");
        http_response->AddKey("num_directories", it->second.num_directories, "");
        http_response->EndGroup(it->first.first.c_str());
    }

    SetQuotaBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}

void MySimpleHttpThread::SetQuotaBottomLink(const char* user_name,
    const char* ca_token_encrypt, const uint32_t len_encrypt, CHttpBuff* http_response) {
    char link[1024] = {0};

    http_response->BeginGroup("QuotaManager");
    safe_snprintf(link, sizeof(link),
        "http://%s/set_role_quota.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
        GetListenHostPort(), user_name,  ca_token_encrypt, len_encrypt);
    http_response->AddInternalHrefKey("Set", link, "", "set a role's quota");

    safe_snprintf(link, sizeof(link),
        "http://%s/query_role_quota.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
        GetListenHostPort(), user_name,  ca_token_encrypt, len_encrypt);
    http_response->AddInternalHrefKey("Query", link, "", "query a role's quota");

    safe_snprintf(link, sizeof(link),
        "http://%s/list_all_roles_quota.html?CA_USERNAME=%s&CA_TOKEN=%s&LEN=%d",
        GetListenHostPort(), user_name,  ca_token_encrypt, len_encrypt);
    http_response->AddInternalHrefKey("List", link, "", "list all roles' quota");
    http_response->EndGroup("QuotaManager");
}

void MySimpleHttpThread::PrintQuotaManagerPage(const char* user_name, const char* ca_token_encrypt,
                                              uint32_t len_encrypt, CHttpBuff* http_response) {
    http_response->SetAttr("CaQuotaMagnagerPage");


    char java_script[1024] = {0};
#ifdef WIN32
    safe_snprintf(java_script, sizeof(java_script),
      "<script> "
      "function f_submit() { "
      "var act='http://%s/quota_manager.html';"
      "document.getElementById('dump_pwd_list').action=act;}"
      "</script>", GetListenHostPort());
#else // linux
    const char* proxy = GetProxyRequest();
    safe_snprintf(java_script, sizeof(java_script),
      "<script> "
      "function f_submit() { "
      "var act='%s%shttp://%s/quota_manager.html?'"
      "+'pwd='+document.getElementById('pwd').value"
      "+'&home='+document.getElementById('home').value"
      "+'&CA_USERNAME='+document.getElementById('CA_USERNAME').value"
      "+'&CA_TOKEN='+document.getElementById('CA_TOKEN').value"
      "+'&LEN='+document.getElementById('LEN').value;"
      "document.getElementById('dump_pwd_list').action=act;}"
      "</script>",
      proxy,
      strlen(proxy)? "?realserver=":"",
      GetListenHostPort());
#endif
    http_response->AddKey(java_script, "", "");

    char form[1024] = {0};
    safe_snprintf(form, sizeof(form),
        "<form method='post' id='dump_pwd_list'>"
        "<input type='hidden' name='CA_USERNAME' id='CA_USERNAME' value='%s'>"
        "<input type='hidden' name='LEN' id='LEN' value='%d'>"
        "<input type='hidden' name='CA_TOKEN' id='CA_TOKEN' value='%s'>"
        "<input type='hidden' name='home' id='home' value='http://xfs.soso.oa.com/index.html'>"
        "<p>please input password:<input type='password' name='pwd' id='pwd' size='20'>"
        "<input type='submit' value='submit' onclick='f_submit()'></p>"
        "</form>",
        user_name, len_encrypt, ca_token_encrypt);

    http_response->AddKey(form, "", "");
}

void MySimpleHttpThread::DealQuotaManagerReq(const BufferV* received_buff,
                                              CHttpBuff* http_response) {
    if (!received_buff || !http_response)
        return;

    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;

    // 在onebox环境中，跳过输入密码这一步骤，直接显示
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);

        const char* dump_pwd = NULL;
        m_cgi_parser.GetParameter("pwd", &dump_pwd);
        if (!dump_pwd) {
            PrintQuotaManagerPage(user_name, ca_token_encrypt, len_encrypt, http_response);
            return;
        }
        if (0 != strncmp(dump_pwd, kAdminPwd, strlen(kAdminPwd))) {
            http_response->AddKey("PassWord incorrect", "", "");
            return;
        }
    }

    SetQuotaBottomLink(user_name, ca_token_encrypt, len_encrypt, http_response);
}



void MySimpleHttpThread::AccessAuthenticateRecordPage(const char* user_name,
                                                       const char* ca_token_encrypt,
                                                       uint32_t len_encrypt,
                                                       CHttpBuff* http_response) {
    if (!user_name || !ca_token_encrypt || !http_response)
        return;

    const char* proxy = GetProxyRequest();
    char java_script[1024] = {0};
#ifdef WIN32
    safe_snprintf(java_script, sizeof(java_script),
        "<script> "
        "function f_submit() { "
        "var act='http://%s/authenticate_record.html';"
        "document.getElementById('dump_pwd_list').action=act;}"
        "</script>", GetListenHostPort());
#else // linux
    safe_snprintf(java_script, sizeof(java_script),
        "<script> "
        "function f_submit() { "
        "var act='%s%shttp://%s/authenticate_record.html?'"
        "+'pwd='+document.getElementById('pwd').value"
        "+'&home='+document.getElementById('home').value"
        "+'&CA_USERNAME='+document.getElementById('CA_USERNAME').value"
        "+'&CA_TOKEN='+document.getElementById('CA_TOKEN').value"
        "+'&LEN='+document.getElementById('LEN').value;"
        "document.getElementById('dump_pwd_list').action=act;}"
        "</script>",
        proxy,
        strlen(proxy)? "?realserver=":"",
        GetListenHostPort());
#endif
    http_response->AddKey(java_script, "", "");

    char form[1024] = {0};
    safe_snprintf(form, sizeof(form),
        "<form method='post' id='dump_pwd_list'>"
        "<input type='hidden' name='CA_USERNAME' id='CA_USERNAME' value='%s'>"
        "<input type='hidden' name='LEN' id='LEN' value='%d'>"
        "<input type='hidden' name='CA_TOKEN' id='CA_TOKEN' value='%s'>"
        "<input type='hidden' name='home' id='home' value='http://xfs.soso.oa.com/index.html'>"
        "<p>please input password:<input type='password' name='pwd' id='pwd'size='20'>"
        "<input type='submit' value='submit' onclick='f_submit()'></p>"
        "</form>",
        user_name, len_encrypt, ca_token_encrypt);

    http_response->AddKey(form, "", "");
}

void MySimpleHttpThread::DealAuthenticateRecordReq(const BufferV* received_buff,
                                                    CHttpBuff* http_response) {
    //dump master
     if (!received_buff || !http_response)
        return;

    http_response->SetAttr("CaAuthenticateRecordPage", m_module_name, false);

    // 解析received_buff中和ca_token相关的参数
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* user_name = NULL;
    const char* ca_token_encrypt = NULL;
    uint32_t len_encrypt = 0;
    const char* pwd = NULL;

    // 在onebox环境中，跳过输入密码这一步骤，直接显示
    if (!FLAGS_ca_onebox) {
        m_cgi_parser.GetParameter("CA_USERNAME", &user_name);
        m_cgi_parser.GetParameter("CA_TOKEN", &ca_token_encrypt);
        m_cgi_parser.GetParameter("LEN", &len_encrypt);
        if (NULL == user_name || NULL == ca_token_encrypt || 0 == len_encrypt) {
            http_response->AddKey("", "parameter missing: CA_USERNAME=xxx&CA_TOKEN=xxx&LEN=xxx");
            return;
        }

        if (!CheckCaToken(user_name, ca_token_encrypt, len_encrypt)) {
            http_response->AddKey("", "ca_token check failed", "");
            return;
        }
        SetBanner(user_name, http_response);

        m_cgi_parser.GetParameter("pwd", &pwd);
        if (!pwd) {
            AccessAuthenticateRecordPage(user_name, ca_token_encrypt, len_encrypt, http_response);
            return;
        }
        if (0 != strncmp(pwd, kAdminPwd, strlen(kAdminPwd))) {
            http_response->AddKey("PassWord Incorrect", "", "");
            return;
        }
    }


    // 显示用户认证历史记录的曲线
    // 默认显示当天的记录，用户可选择日期
    int32_t year_chart = 0;
    int32_t month_chart = 0;
    int32_t day_chart = 0;
    int32_t req_day_num = 0;

    m_cgi_parser.GetParameter("year_chart", &year_chart);
    m_cgi_parser.GetParameter("month_chart", &month_chart);
    m_cgi_parser.GetParameter("day_chart", &day_chart);
    m_cgi_parser.GetParameter("req_day_num", &req_day_num);

    struct tm req_local_time = {0};
    time_t req_chart_epoch_time = 0;

    if (0 == day_chart || 0 == month_chart || 0 == year_chart || 0 == req_day_num) {
        req_chart_epoch_time = time(NULL);
        req_day_num = 1;
        safe_localtime(&req_chart_epoch_time, &req_local_time);
    } else {
        req_local_time.tm_year = year_chart - 1900;
        req_local_time.tm_mon = month_chart - 1;
        req_local_time.tm_mday = day_chart;
        req_chart_epoch_time = mktime(&req_local_time);
    }

    // 解析显示记录detail相关的参数
    const char* show_detail = NULL;
    bool is_show_detail = false;
    m_cgi_parser.GetParameter("show_detail", &show_detail);
    if (show_detail && (0 == strcmp(show_detail, "true")))
        is_show_detail = true;

    int32_t year_text = 0;
    int32_t month_text = 0;
    int32_t day_text = 0;
    int32_t req_page_num = 0;
    if (is_show_detail) {
        // 解析另外的三个时间参数和page_num
        m_cgi_parser.GetParameter("year_text", &year_text);
        m_cgi_parser.GetParameter("month_text", &month_text);
        m_cgi_parser.GetParameter("day_text", &day_text);
        m_cgi_parser.GetParameter("req_page_num", &req_page_num);
    }

    time_t req_text_epoch_time = 0;
    if (0 == year_text || 0 == month_text || 0 == day_text || 0 == req_page_num) {
        req_text_epoch_time = req_chart_epoch_time;
    } else {
        struct tm tmp_local_time = {0};
        tmp_local_time.tm_year = year_text - 1900;
        tmp_local_time.tm_mon = month_text - 1;
        tmp_local_time.tm_mday = day_text;
        req_text_epoch_time = mktime(&tmp_local_time);
    }


    int32_t total_page_num = 0;
    std::string xml_data;
    std::string auth_record_file;
    // 获取数据
    PrepareChartData(req_chart_epoch_time, req_text_epoch_time, req_day_num, req_page_num,
                     &total_page_num, xml_data, auth_record_file);

    if (auth_record_file.empty()) {
        auth_record_file = "there is no record";
    }

    // ----------------------
    // 显示各模块   
    const char* proxy = GetProxyRequest();
    char page_index_submit[1024] = {0};
    safe_snprintf(page_index_submit, sizeof(page_index_submit),
        "<script> function page_index_submit() {"
        "var act='%s%shttp://%s/authenticate_record.html?'"
        "+'pwd=%s&CA_USERNAME=%s&LEN=%d&CA_TOKEN=%s&year_chart=%d&month_chart=%d&day_chart=%d'"
        "+'&req_day_num=%d&year_text=%d&month_text=%d&day_text=%d&show_detail=true'"
        "+'&req_page_num='+document.getElementById('page_index').value;"
        "document.getElementById('page_index_req').action=act;}</script>",
        proxy, strlen(proxy)? "?realserver=":"", GetListenHostPort(),
        pwd, user_name, len_encrypt, ca_token_encrypt, year_chart, month_chart, day_chart,
        req_day_num, year_text, month_text, day_text);
    http_response->AddKey(page_index_submit, "", "");


    // 显示日期下拉选择框，用户可选择日期来显示那天的曲线
    PrintDaySlect(is_show_detail, user_name, ca_token_encrypt, len_encrypt, pwd, year_chart,
                  month_chart, day_chart, http_response);

    // 显示show_detail的按钮
    // 未显示textarea时，点击按钮可多提交下面这些参数
    char text_area_date_param[1024] = {0};
    safe_snprintf(text_area_date_param, sizeof(text_area_date_param),
        "<input type='hidden' name='year_text' id='year_text' value='%d'>"
        "<input type='hidden' name='month_text' id='month_text' value='%d'>"
        "<input type='hidden' name='day_text' id='day_text' value='%d'>"
        "<input type='hidden' name='req_page_num' id='req_page_num' value='1'>",
        year_chart, month_chart, day_chart);   


    // 处理显示分页选择的url
    char page_url[4096] = {0};
    if (is_show_detail) {
        char prev[1024] = {0};
        safe_snprintf(prev, sizeof(prev),"<a href=\""
            "%s%shttp://%s/authenticate_record.html?pwd=%s&CA_USERNAME=%s&LEN=%d&CA_TOKEN=%s&"
            "year_chart=%d&month_chart=%d&day_chart=%d&req_day_num=%d&year_text=%d&month_text=%d&"
            "day_text=%d&show_detail=true&req_page_num=%d\""
            ">&lt;&ltprev&nbsp;&nbsp;</a>",
            proxy, strlen(proxy)? "?realserver=":"", GetListenHostPort(),
            pwd, user_name, len_encrypt, ca_token_encrypt, year_chart, month_chart, day_chart,
            req_day_num, year_text, month_text, day_text, req_page_num - 1);

        char next[1024] = {0};
        safe_snprintf(next, sizeof(next),"<a href=\""
            "%s%shttp://%s/authenticate_record.html?pwd=%s&CA_USERNAME=%s&LEN=%d&CA_TOKEN=%s&"
            "year_chart=%d&month_chart=%d&day_chart=%d&req_day_num=%d&year_text=%d&month_text=%d&"
            "day_text=%d&show_detail=true&req_page_num=%d\""
            ">&nbsp;&nbsp;next&gt;&gt;</a>",
            proxy, strlen(proxy)? "?realserver=":"", GetListenHostPort(),
            pwd, user_name, len_encrypt, ca_token_encrypt, year_chart, month_chart, day_chart,
            req_day_num, year_text, month_text, day_text, req_page_num + 1);

        char first[1024] = {0};
        safe_snprintf(first, sizeof(first),"<a href=\""
            "%s%shttp://%s/authenticate_record.html?pwd=%s&CA_USERNAME=%s&LEN=%d&CA_TOKEN=%s&"
            "year_chart=%d&month_chart=%d&day_chart=%d&req_day_num=%d&year_text=%d&month_text=%d&"
            "day_text=%d&show_detail=true&req_page_num=%d\""
            ">[1]</a>",
            proxy, strlen(proxy)? "?realserver=":"", GetListenHostPort(),
            pwd, user_name, len_encrypt, ca_token_encrypt, year_chart, month_chart, day_chart,
            req_day_num, year_text, month_text, day_text, 1);

        char last[1024] = {0};
        safe_snprintf(last, sizeof(last),"<a href=\""
            "%s%shttp://%s/authenticate_record.html?pwd=%s&CA_USERNAME=%s&LEN=%d&CA_TOKEN=%s&"
            "year_chart=%d&month_chart=%d&day_chart=%d&req_day_num=%d&year_text=%d&month_text=%d&"
            "day_text=%d&show_detail=true&req_page_num=%d\""
            ">[%d]</a>",
            proxy, strlen(proxy)? "?realserver=":"", GetListenHostPort(),
            pwd, user_name, len_encrypt, ca_token_encrypt, year_chart, month_chart, day_chart,
            req_day_num, year_text, month_text, day_text, total_page_num, total_page_num);

        safe_snprintf(page_url, sizeof(page_url), "<td>&nbsp;</td>"
            "<td><span class='url_in_foot'><form method='post' id='page_index_req'>"
            "%s%s%s%s%s"            
            "%s</form></span></td></tr></table>",
            (req_page_num <= 1 ||  req_page_num > total_page_num) ? "" : prev,
            (total_page_num < 1 ) ? "" : first,
            (total_page_num <= 1) ? "" : "...",
            (total_page_num <= 1) ? "" : last,
            (req_page_num >= total_page_num) ? "" : next,
            (total_page_num < 1) ? ""
            : "<input type='text' name='page_index' id='page_index' size='2'>"
            "<input type='submit' value='Go' onclick='page_index_submit()'>");

    }

    // 显示show_detail的按钮
    char show_detail_button[4096] = {0};
    safe_snprintf(show_detail_button, sizeof(show_detail_button),
        "<tr><td><form method='get' id='show_detail_btn' action='%s'>"
        "<input type='hidden' name='realserver' id='realserver' "
        "value='http://%s/authenticate_record.html'>"
        "<input type='hidden' name='home' id='home' value='http://xfs.soso.oa.com/index.html'>"
        "<input type='hidden' name='pwd' id='pwd' value='%s'>"
        "<input type='hidden' name='CA_USERNAME' id='CA_USERNAME' value='%s'>"
        "<input type='hidden' name='LEN' id='LEN' value='%d'>"
        "<input type='hidden' name='CA_TOKEN' id='CA_TOKEN' value='%s'>"
        "<input type='hidden' name='year_chart' id='year_chart' value='%d'>"
        "<input type='hidden' name='month_chart' id='month_chart' value='%d'>"
        "<input type='hidden' name='day_chart' id='day_chart' value='%d'>"
        "<input type='hidden' name='req_day_num' id='req_day_num' value='%d'>"
        "<input type='hidden' name='show_detail' id='show_detail' value='%s'>"
        "%s"
        "<input type='submit' value='%s'>"
        "</form>"
        "</td>%s",
        proxy, GetListenHostPort(), pwd, user_name, len_encrypt, ca_token_encrypt,
        year_chart, month_chart, day_chart, req_day_num,
        is_show_detail ? "false" : "true",
        is_show_detail ? "" : text_area_date_param,
        is_show_detail ? "Hide Detail of Authentication" : "Show Detail of Authentication",
        is_show_detail ? page_url : "</tr></table>");

    http_response->AddKey(show_detail_button, "", "");

    // 设置x轴的显示，如果是显示一天的记录，则x轴的单位为hour，如果是多天，则单位为day
    char x_axis_name[8];
    safe_snprintf(x_axis_name, sizeof(x_axis_name), "%s",
        (1 == req_day_num) ? "hour" : "day");

    // 设置图标标题上的日期范围显示如：from 2011-5-7 to 2011-8-16
    time_t pre_req_epoch_time =  req_chart_epoch_time - (req_day_num - 1) * 24 * 60* 60;
    struct tm local_from_time = {0};
    safe_localtime(&pre_req_epoch_time, &local_from_time);
    char day_rang[32];
    safe_snprintf(day_rang, sizeof(day_rang), "from %d-%d-%d   to   %d-%d-%d",
        local_from_time.tm_year + 1900, local_from_time.tm_mon + 1,
        local_from_time.tm_mday, req_local_time.tm_year + 1900,
        req_local_time.tm_mon + 1, req_local_time.tm_mday);

    // 设置图标的显示类型，如果请求显示的天数>30天，则显示为2D的曲线
    // 如果<30天，则显示为3D的柱状图
    const char* chart_type = (req_day_num > 30) ? kFusionChart2D : kFusionChart3D;


    // 显示图表
    // 显示一天、最近7天、最近30天，最近90天的url链接
    char url_one[512] = {0};
    // 显示一天
    req_day_num = 1;
    safe_snprintf(url_one, sizeof(url_one),
        "%s%shttp://%s/authenticate_record.html?pwd=%s&CA_USERNAME=%s&LEN=%d&CA_TOKEN=%s&"
        "year_chart=%d&month_chart=%d&day_chart=%d&req_day_num=%d&show_detail=false",
        proxy, strlen(proxy)? "?realserver=":"", GetListenHostPort(),
        pwd, user_name, len_encrypt, ca_token_encrypt, req_local_time.tm_year + 1900,
        req_local_time.tm_mon + 1, req_local_time.tm_mday, req_day_num);


    // 显示最近7天
    char url_seven[512] = {0};
    req_day_num = 7;
    safe_snprintf(url_seven, sizeof(url_seven),
        "%s%shttp://%s/authenticate_record.html?pwd=%s&CA_USERNAME=%s&LEN=%d&CA_TOKEN=%s&"
        "year_chart=%d&month_chart=%d&day_chart=%d&req_day_num=%d&show_detail=false",
        proxy, strlen(proxy)? "?realserver=":"", GetListenHostPort(),
        pwd, user_name, len_encrypt, ca_token_encrypt, req_local_time.tm_year + 1900,
        req_local_time.tm_mon + 1, req_local_time.tm_mday, req_day_num);


    // 显示最近30天
    char url_thirty[512] = {0};
    req_day_num = 30;
    safe_snprintf(url_thirty, sizeof(url_thirty),
        "%s%shttp://%s/authenticate_record.html?pwd=%s&CA_USERNAME=%s&LEN=%d&CA_TOKEN=%s&"
        "year_chart=%d&month_chart=%d&day_chart=%d&req_day_num=%d&show_detail=false",
        proxy, strlen(proxy)? "?realserver=":"", GetListenHostPort(),
        pwd, user_name, len_encrypt, ca_token_encrypt, req_local_time.tm_year + 1900,
        req_local_time.tm_mon + 1, req_local_time.tm_mday, req_day_num);

    // 显示最近60天
    char url_sixty[512] = {0};
    req_day_num = 60;
    safe_snprintf(url_sixty, sizeof(url_sixty),
        "%s%shttp://%s/authenticate_record.html?pwd=%s&CA_USERNAME=%s&LEN=%d&CA_TOKEN=%s&"
        "year_chart=%d&month_chart=%d&day_chart=%d&req_day_num=%d&show_detail=false",
        proxy, strlen(proxy)? "?realserver=":"", GetListenHostPort(),
        pwd, user_name, len_encrypt, ca_token_encrypt, req_local_time.tm_year + 1900,
        req_local_time.tm_mon + 1, req_local_time.tm_mday, req_day_num);


    // 显示最近90天
    char url_ninety[512] = {0};
    req_day_num = 90;
    safe_snprintf(url_ninety, sizeof(url_ninety),
        "%s%shttp://%s/authenticate_record.html?pwd=%s&CA_USERNAME=%s&LEN=%d&CA_TOKEN=%s&"
        "year_chart=%d&month_chart=%d&day_chart=%d&req_day_num=%d&show_detail=false",
        proxy, strlen(proxy)? "?realserver=":"", GetListenHostPort(),
        pwd, user_name, len_encrypt, ca_token_encrypt, req_local_time.tm_year + 1900,
        req_local_time.tm_mon + 1, req_local_time.tm_mday, req_day_num);

    std::string html_data;
    html_data = html_data +
        "<div class=\"dispaly_ca_auth_block\">"
        "<div id= \"dispaly_ca_auth_block_chart_area\">"
        "<div class=\"ca_div_body\" id = divUrl>"
        "<a href=\"" + string(url_one) + "\">[1-day]</a>"
        "<a href=\"" + string(url_seven) + "\">[7-days]</a>"
        "<a href=\"" + string(url_thirty) + "\">[30-days]</a>"
        "<a href=\"" + string(url_sixty) + "\">[60-days]</a>"
        "<a href=\"" + string(url_ninety) + "\">[90-days]</a>"
        "</div>"
        "<div class=\"ca_div_body\" id=\"divChart\">"
        "<img src=\"" + string(kNotFoundPage) + "\"/></div></div>";
    if (is_show_detail) {
        html_data = html_data +
        "<textarea id='dispaly_ca_auth_block_textarea' name=\"auth_record_file\" "
        "readonly=\"readonly\" cols=\"56\">"
        + auth_record_file + "</textarea></div>";
    } else {
        html_data = html_data + "</div>";
    }
    http_response->AddKey("", html_data.c_str(), "");

    if (xml_data.empty())
        return;

    std::string js_str;
    js_str = js_str +
        "<script language = \"javascript\" src = \"" + string(kFusionChartJs) + "\"></script>"
        "<script type=\"text/javascript\">"
        "var record_chart = new FusionCharts(\"" + string(chart_type) + "\","
        " \"record_char_id\", \"700\", \"500\");"
        "record_chart.setDataXML(\"<graph caption = 'Authenticate Record Chart "
        + string(day_rang) + "' xAxisName = '" + string(x_axis_name) + "'yAxisName = 'counts' "
        "showNames = '1' decimalPrecision = '0' formatNumberScale = '0' >"
        + xml_data + "</graph> \");"
        "record_chart.render(\"divChart\");</script>";
    http_response->AddKey("", js_str.c_str(), "");

}



void MySimpleHttpThread::PrintDaySlect(bool show_detail, const char* user_name,
               const char* ca_token_encrypt, uint32_t len_encrypt, const char* pwd,
               int32_t year_chart, int32_t month_chart, int32_t day_chart,CHttpBuff* http_response){
    if (!http_response)
        return;

    char form[3072] = {0};
    const char* proxy = GetProxyRequest();
    if (!show_detail) { // 只输出左边的选择框
        safe_snprintf(form, sizeof(form),
            "<table class='dispaly_ca_dayselect_block'><tr><td>"
            "<form method='get' id='dump_day_req_list' action='%s'>"
            "<select id='year_chart' name='year_chart'></select>"
            "<select id='month_chart' name='month_chart'></select>"
            "<select id='day_chart' name='day_chart'></select>"
            "<input type='hidden' name='realserver' id='realserver' "
            "value='http://%s/authenticate_record.html'>"
            "<input type='hidden' name='req_day_num' id='req_day_num' value='1'>"
            "<input type='hidden' name='show_detail' id='show_detail' value='false'>"
            "<input type='hidden' name='pwd' id='pwd' value='%s'>"
            "<input type='hidden' name='home' id='home' value='http://xfs.soso.oa.com/index.html'>"
            "<input type='hidden' name='CA_USERNAME' id='CA_USERNAME' value='%s'>"
            "<input type='hidden' name='LEN' id='LEN' value='%d'>"
            "<input type='hidden' name='CA_TOKEN' id='CA_TOKEN' value='%s'>"
            "<input type='submit' value='submit'>"
            "</form><script>CreateDateSelect(\"year_chart\", \"month_chart\", \"day_chart\");"
            "</script></td></tr>",
            proxy, GetListenHostPort(), pwd, user_name, len_encrypt, ca_token_encrypt);
    } else { // 右边查询详细记录的下拉框也输出
        safe_snprintf(form, sizeof(form),
            "<table class='dispaly_ca_dayselect_block'><tr><td>"
            "<form method='get' id='dump_day_req_list' action='%s'>"
            "<select id='year_chart' name='year_chart'></select>"
            "<select id='month_chart' name='month_chart'></select>"
            "<select id='day_chart' name='day_chart'></select>"
            "<input type='hidden' name='realserver' id='realserver' "
            "value='http://%s/authenticate_record.html'>"
            "<input type='hidden' name='req_day_num' id='req_day_num' value='1'>"
            "<input type='hidden' name='show_detail' id='show_detail' value='false'>"
            "<input type='hidden' name='pwd' id='pwd' value='%s'>"
            "<input type='hidden' name='home' id='home' value='http://xfs.soso.oa.com/index.html'>"
            "<input type='hidden' name='CA_USERNAME' id='CA_USERNAME' value='%s'>"
            "<input type='hidden' name='LEN' id='LEN' value='%d'>"
            "<input type='hidden' name='CA_TOKEN' id='CA_TOKEN' value='%s'>"
            "<input type='submit' value='submit'>"
            "</form><script>CreateDateSelect(\"year_chart\", \"month_chart\", \"day_chart\");"
            "</script></td><td>&nbsp;</td><td>"
            "<form method='get' id='dump_day_req_list' action='%s'>"
            "<select id='year_text' name='year_text'></select>"
            "<select id='month_text' name='month_text'></select>"
            "<select id='day_text' name='day_text'></select>"
            "<input type='hidden' name='realserver' id='realserver' "
            "value='http://%s/authenticate_record.html'>"
            "<input type='hidden' name='req_day_num' id='req_day_num' value='1'>"
            "<input type='hidden' name='show_detail' id='show_detail' value='true'>"
            "<input type='hidden' name='req_page_num' id='req_page_num' value='1'>"
            "<input type='hidden' name='pwd' id='pwd' value='%s'>"
            "<input type='hidden' name='home' id='home' value='http://xfs.soso.oa.com/index.html'>"
            "<input type='hidden' name='CA_USERNAME' id='CA_USERNAME' value='%s'>"
            "<input type='hidden' name='LEN' id='LEN' value='%d'>"
            "<input type='hidden' name='CA_TOKEN' id='CA_TOKEN' value='%s'>"
            "<input type='hidden' name='year_chart' id='year_chart' value='%d'>"
            "<input type='hidden' name='month_chart' id='month_chart' value='%d'>"
            "<input type='hidden' name='day_chart' id='day_chart' value='%d'>"
            "<input type='submit' value='submit'>"
            "</form><script>CreateDateSelect(\"year_text\", \"month_text\", \"day_text\");"
            "</script></td></tr>",
            proxy, GetListenHostPort(), pwd, user_name, len_encrypt, ca_token_encrypt,
            proxy, GetListenHostPort(), pwd, user_name, len_encrypt, ca_token_encrypt, year_chart,
            month_chart, day_chart);
    }

    http_response->AddKey(form, "", "");
}

void MySimpleHttpThread::PrepareChartData(time_t req_chart_epoch_time, time_t req_text_epoch_time,
                                 int32_t req_day_num, int32_t req_page_num, int32_t* total_page_num,
                                 std::string& xml_data, std::string& auth_record_file) {
    std::string count_statistics_file;
    AuthRecord::GetInstance()->ReadRecord(req_chart_epoch_time, req_text_epoch_time, req_day_num,
        req_page_num, total_page_num, &auth_record_file, &count_statistics_file);
    if (count_statistics_file.empty())
        return;

    CKeyValueParser key_value_parse_obj;
    if (!key_value_parse_obj.ParserFromBuffer(
        reinterpret_cast<const unsigned char*>(count_statistics_file.c_str()),
        count_statistics_file.length()))
        return;

    // 如果是显示一天的记录，则曲线的x轴按0-23小时显示，如：0 1 2 3 ...
    if (1 == req_day_num) {
        char key_hour[3] = {0};
        for (int32_t i = 0; i < 24; ++i) {
            char value_hourly_count[6] = {0};
            safe_snprintf(key_hour, sizeof(key_hour), "%d", i);
            key_value_parse_obj.GetValue(key_hour,
                reinterpret_cast<unsigned char*>(value_hourly_count), sizeof(value_hourly_count));
            xml_data = xml_data + "<set name = '" + key_hour + "' value = '"
                + value_hourly_count + "' color = '8BBA00' />";
        }
    } else { // 如果是显示多天的记录，则曲线的x轴按天显示
        char req_day[16] = {0}; // 指定的日期，格式为2011_5_15
        char day_x[8] = {0}; // x轴显示的日期
        struct tm req_local_time = {0};
        time_t pre_req_epoch_time = 0;
        for (int32_t i = req_day_num - 1; i >= 0; --i) {
            pre_req_epoch_time = req_chart_epoch_time - i * 24 * 60* 60;
            safe_localtime(&pre_req_epoch_time, &req_local_time);
            safe_snprintf(req_day, sizeof(req_day), "%d_%d_%d", req_local_time.tm_year + 1900,
                          req_local_time.tm_mon + 1, req_local_time.tm_mday);

            char daily_count[6] = {0}; // 从数据串里解析得到每天认证统计的累计和
            key_value_parse_obj.GetValue(req_day, reinterpret_cast<unsigned char*>(daily_count),
                                         sizeof(daily_count));


            // 每月的1号和14号才显示出月份，如：5-1 2 3 .....5-14 16 17....
            if (1 == req_local_time.tm_mday || 14 == req_local_time.tm_mday) {
                safe_snprintf(day_x, sizeof(day_x), "%d-%d",
                               req_local_time.tm_mon + 1, req_local_time.tm_mday);
            } else {
                safe_snprintf(day_x, sizeof(day_x), "%d", req_local_time.tm_mday);
            }

            if (req_day_num < 30) {
                xml_data = xml_data + "<set name = '" + day_x + "' value = '"
                           + daily_count + "' color = '8BBA00' />";
            }
            else { // 如果要显示的天数大于1个月，则按每7天显示一个x坐标，不然会看不清
                   // 且每个月的第一天都显示出月份，所以显示出坐标的天为1,7,14,21,28
                if (1 != req_local_time.tm_mday && 7 != req_local_time.tm_mday
                    && 14 != req_local_time.tm_mday && 21 != req_local_time.tm_mday
                    && 28 != req_local_time.tm_mday)
                    xml_data = xml_data + "<set name = '' value = '"
                               + daily_count + "' color = '8BBA00' />";
                else
                    xml_data = xml_data + "<set name = '" + day_x + "' value = '"
                               + daily_count + "' color = '8BBA00' />";
            }
        }
    }
}

void MySimpleHttpThread::Reload(const BufferV* received_buff, CHttpBuff* http_response) {
    CXThreadAutoLock auto_lock(&g_http_mutex);
    const char* ptr = strchr(reinterpret_cast<const char*>(received_buff->buff), '?');
    if (!ptr) {
        http_response->AddKey("", "parameter missing in url, please relogin", "");
        return;
    }
    m_cgi_parser.AttachEnvironmentString(reinterpret_cast<const char*>(ptr + 1));
    const char* dump_pwd = NULL;
    m_cgi_parser.GetParameter("pwd", &dump_pwd);
    if (!dump_pwd || 0 != strncmp(dump_pwd, kAdminPwd, strlen(kAdminPwd))) {
        http_response->AddKey("ReloadFail", "");
        http_response->AddKey("PassWord incorrect", "", "");
        return;
    }

    bool b = RoleManager::GetInstance()->ReloadConfig();
    b &= QuotaManager::GetInstance()->ReloadConfig();
    if (b) {
        http_response->SetAttr("ReloadOK");
        http_response->AddKey("ReloadOK", "", "");
    } else {
        http_response->SetAttr("ReloadFail");
        http_response->AddKey("ReloadFail", "");
    }
}

void MySimpleHttpThread::WriteCheckPoint() {
    if (FLAGS_ca_server_role == "Secondary")
        return;

    // 是否需要写CheckPoint
    time_t now = time(0);
    if (!g_has_modified || (now - g_last_modified < kReserveTime)) {
        LOG(INFO) << "No Need To Write CheckPoint.";
        return;
    }

    CXThreadAutoLock auto_lock(&g_http_mutex);
    LOG(INFO) << "Begin Write CheckPoint.";
    char module_dir[MAX_PATH] = {0};
    GetModuleFileName(NULL, module_dir, sizeof(module_dir));
    // 创建新文件夹
    std::string cur_dir = module_dir;
    std::string dir_name;
    std::string tmp_dir;
    int32_t pos = cur_dir.find_last_of('/');
    if (pos != -1) {
        cur_dir = cur_dir.substr(0, pos + 1);
        cur_dir += kCaDir;
        dir_name += cur_dir + NumberToString(now);
        tmp_dir = dir_name + "_tmp";
    } else {
        // module name无效时,使用当前运行路径
        cur_dir = "./";
        cur_dir = kCaDir;
        dir_name += cur_dir + NumberToString(now);
        tmp_dir = dir_name + "_tmp";
    }

    bool b = false;
    if (mkdir(tmp_dir.c_str(), 0777) == 0) {
        std::string filename = cur_dir + kIdentityFileName;
        std::string new_filename = tmp_dir + "/" + kIdentityFileName;
        b = CopyFile(filename, new_filename);
        filename = cur_dir +  kRoleFileName;
        new_filename = tmp_dir + "/" + kRoleFileName;
        b &= CopyFile(filename, new_filename);
        filename = cur_dir + kRelatedFileName;
        new_filename = tmp_dir + "/" + kRelatedFileName;
        b &= CopyFile(filename, new_filename);
        filename = cur_dir + kQuotaFileName;
        new_filename = tmp_dir + "/" + kQuotaFileName;
        b &= CopyFile(filename, new_filename);
        if (b) {
            b = (rename(tmp_dir.c_str(), dir_name.c_str()) == 0);
            if (b) {
                g_has_modified = false;
                g_last_modified = now;
                LOG(INFO) << "End Write CheckPoint.";
            }
        }
    }

    // 不成功,则删除tmp目录
    if (!b) {
        std::string cmd = "rm ";
        cmd += tmp_dir + "/*";
        system(cmd.c_str());
        cmd = "rmdir ";
        cmd += tmp_dir;
        system(cmd.c_str());
        LOG(ERROR) << "Fail to Write CheckPoint.";
    }
}

bool MySimpleHttpThread::CopyFile(std::string& filename, std::string& new_filename) {
    bool b = false;
    FILE* fp = fopen(filename.c_str(), "rb");
    FILE* fp_new = fopen(new_filename.c_str(), "wb");
    if (fp && fp_new) {
        char buffer[1024] = {0};
        uint32_t len = 1024;
        do {
            b = false;
            int32_t ret = fread(buffer, len, 1, fp);
            if (ret == 1 || (ret == 0 && feof(fp))) {
                int64_t tail = ftell(fp);
                ret = fwrite(buffer, tail, 1, fp_new);
                if (ret == 1)
                    b = true;
                else
                    LOG(ERROR) << "Write file " << new_filename << "FAIL";
            } else
                LOG(ERROR) << "Read file " << filename << "FAIL";
            if (feof(fp)){
                break;
            }
        } while(b);
        fclose(fp);
        fclose(fp_new);
    } else {
        if (fp) {
            LOG(ERROR) << "Open file " << new_filename << "FAIL";
            fclose(fp);
        }
        if (fp_new) {
            LOG(ERROR) << "Open file " << filename << "FAIL";
            fclose(fp_new);
        }    
    }

    return b;    
}

} // namespace ca
