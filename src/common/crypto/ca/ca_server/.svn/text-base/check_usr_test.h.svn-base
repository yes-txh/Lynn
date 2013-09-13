#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/crypto/ca/ca_server/ca.h"
using namespace ca;

static const uint32_t SIGN_LEN = 256;
static const uint32_t USERNAME_LEN = 256;

DECLARE_int32(ca_port);
DECLARE_string(ca_host);


class CheckUsrTest {
public:
    CheckUsrTest() {};
    ~CheckUsrTest() {};
    bool IsValidUser(const char* user_name);

private:
    bool GetPrivateKey(const char* user_name);
    bool GenSign(const char* user_name);
    bool Hex2String(const unsigned char* hex, int hexLen, char* str, int *len);

    unsigned char       m_sign[kCaSignLen];
    uint32_t            m_sign_len;
    CAPrivate           m_private_key;
    CStrBuff            m_obj_buff;
    CGetHttpResponse    m_response;
};


bool CheckUsrTest::Hex2String(const unsigned char* hex, int hex_len, char* str, int *len) {
    if(*len < (hex_len * 2) + 1) {
        return false;
    }

    int i, o;
    for (i = 0, o = 0; i < hex_len; ++i, o += 2) {
        snprintf(&str[o], 3, "%02X", hex[i]);
    }
    *len = hex_len * 2;

    return true;
}


bool CheckUsrTest::IsValidUser(const char* user_name) {
    uint32_t username_len = static_cast<uint32_t>(strlen(user_name));
    if (username_len > USERNAME_LEN) {
        return false;
    }

    CA ca;
    ca.Init();
    CAPrivate user_private_key_file;
    char user_private_key_path[MAX_PATH] = {0};
    CA_ERROR_CODE error_code;
    if (!ca.GetUserPrivateInfo(user_name, &user_private_key_file,
                               user_private_key_path, MAX_PATH, &error_code)) {
        LOG(ERROR) << "get user: " << user_name << " 's private_key fail";
        return false;
    }

    memcpy(m_sign, user_private_key_file.sign, user_private_key_file.sign_len);
    m_sign_len = user_private_key_file.sign_len;

    char domain[128] = {0};
    safe_snprintf(domain,sizeof(domain), "%s:%d", FLAGS_ca_host.c_str(), FLAGS_ca_port);
    const char* cgi = "/ca_check_user.html";

    // encode encrypt string
    char buff[1024] = {0};
    int32_t buff_len = 1024;
    if (!Hex2String((const unsigned char*)m_sign, m_sign_len, buff, &buff_len)) {
        LOG(ERROR) << "encode encrypt string error, dest buffer too short!";
        return false;
    }
    char params[512] = {0};

    safe_snprintf(params, sizeof(params), "CA_USERNAME=%s&SIGN=%s&SIGN_LEN=%u",
                  user_name, buff, m_sign_len);

    HTTP_CGI_ERROR err = m_response.GetResponse(domain, cgi, params, false,  5);

    if (err != ERROR_HTTP_OK) {
        LOG(ERROR) << "get http response error, ErrorString: " << GetHttpErrString(err);
        return false;
    }

    char* http_content = const_cast<char*>(m_response.GetHttpContent());

    char* p = strstr(http_content, "CheckUserResult=OK");
    if (p) {
        char* err_pos = strstr(const_cast<char*>(http_content), "ErrorCode=");
        if (err_pos) {
            uint32_t err_code = ATOI(err_pos + strlen("ErrorCode="));
            LOG(INFO) << "Get CA response, ErrorCode=" << err_code
                      << ",ErrorString: " << CaGetErrorCodeStr(err_code);
        }
        return true;
    }

    char* q = strstr(http_content, "CheckUserResult=FAIL");
    if (q) {
        char* err_pos = strstr(const_cast<char*>(http_content), "ErrorCode=");
        if (err_pos) {
            uint32_t err_code = ATOI(err_pos + strlen("ErrorCode="));
            LOG(ERROR) << "Get CA response, ErrorCode=" << err_code
                       << ",ErrorString :" << CaGetErrorCodeStr(err_code);
        }
        return false;
    }
    return false;
}

// bool CheckUsrTest::GetPrivateKey(const char* user_name) {
//     // cur dir
//     char cur_file[256] = {0};
//     GetModuleFileName(NULL, cur_file, sizeof(cur_file));
//
//     // linux and windows
//     char* p = strrchr(cur_file, '/');
//     if ( !p )
//         p = strrchr(cur_file, '\\');
//
//     if (NULL == p)
//         return false;
//
//     p++;
//
//     // 追加目录htdocs
//     int32_t len_remain = cur_file + sizeof(cur_file) - p - 1;
//     // 边界检查
//     int32_t len_add_dir = strlen(kApacheHtdocsName) + strlen(kCaDirName) + 2;
//     if (len_remain < len_add_dir)
//         return false;
//
//     safe_snprintf(p, len_remain, kApacheHtdocsName);
//     p += strlen(kApacheHtdocsName);
//
//     // 追加目录ca_dir
//     len_remain = m_ca_dir + sizeof(m_ca_dir) - p - 1;
//     safe_snprintf(p, len_remain, "%s%s", SPLIT_SIGN, kCaDirName);
//     p += strlen(kCaDirName) + 1;
//
//     len_remain = m_ca_dir + sizeof(m_ca_dir) - p - 1;
//     safe_snprintf(p, len_remain, "%s%s%s%s_private_key.dat",
//                   SPLIT_SIGN, user_name, SPLIT_SIGN, user_name);
//
//     struct stat buf;
//     if (stat(cur_file, &buf) == 0) {
//         FILE* fp = fopen(cur_file, "rb");
//         if (!fp) {
//             return false;
//         }
//         fclose(fp);
//         return true;
//     }
//     return false;
// }
