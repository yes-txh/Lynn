// Copyright 2010, Tencent Inc.
// Author: fatliu(fatliu@tencent.com)
#ifndef COMMON_CA_CA_CLIENT_CERTIFICATION_INL_
#define COMMON_CA_CA_CLIENT_CERTIFICATION_INL_

#include "common/crypto/ca/ca_public/certification.h"
#include "common/crypto/hash/crc.hpp"
#include "common/base/string/string_algorithm.hpp"
#include "common/base/string/string_number.hpp"
#include "common/encoding/base64.hpp"
#include "common/base/module.hpp"

// using --ticket=xxx to set ticket
DEFINE_string(ticket, "", "base64 encoded ticket with CRC, use case:\n"
              "\t1. Authenticate through -identity and -role\n"
              "\t2. Create ticket using CreateTicket() and save it\n"
              "\t3. Pass saved ticket to -ticket for future authentication");
// using --identity=xxx to set identity(user) name
DEFINE_string(identity, "", "rtx user_name");
// using --role=xxx to set role(group) name
DEFINE_string(role, "", "role(group) name for ca");
// using --ca_server="mockca.soso.oa.com" to skip Verify User
DEFINE_string(ca_server, "ca.soso.oa.com", "cautious! xfs internal log server");
DEFINE_int32(ca_port, 10080, "ca server port");

namespace ca {

#ifdef WIN32
const char* kEnvIdentity = "USERNAME";
#else
const char* kEnvIdentity = "USER";
#endif

const char* kCaCgi = "/ca_check_user.html";
const char* kMockMode1 = "mockca.soso.oa.com";
const char* kMockMode2 = "mock.soso.oa.com";

CXThreadMutex g_ca_mutex;

Certifier::Certifier():m_certified(false) {
    //if use mockca, no need to get ip
    if(strcmp(FLAGS_ca_server.c_str(), kMockMode1) == 0 ||
       strcmp(FLAGS_ca_server.c_str(), kMockMode2) == 0){
        m_ca_ip = "127.0.0.1";
        return;
    }

    // get CA host by name
    uint32_t times = 3;
    while (times--) {
        struct hostent *hp = gethostbyname(FLAGS_ca_server.c_str());
        if (hp) {
            m_ca_ip = inet_ntoa(*reinterpret_cast<struct in_addr*>(*hp->h_addr_list));
            return;
        }
        LOG(FATAL) << "Get ip of server : " << FLAGS_ca_server << " FAIL!";
    }
}

Certifier::~Certifier() {
}

bool Certifier::CheckUser(const char* identity, const char* rolename) const {

    CXThreadAutoLock auto_lock(&g_ca_mutex);

    CHECK(identity);

    uint32_t identity_len = static_cast<uint32_t>(STRLEN(identity));
    uint32_t rolename_len = static_cast<uint32_t>(STRLEN(rolename));
    if (identity_len > kUserNameLen || rolename_len > kUserNameLen) {
        LOG(ERROR) << "identity : " << identity << " or rolename : " << rolename
                   << "longer than " << kUserNameLen;
        return false;
    }

    if (!IsValidName(identity)) {
        LOG(ERROR) << "Identity name contains invalid char!";
        return false;
    }
    LOG(INFO) << "identity is " << identity;

    const char *real_role;
    // get real role name
    if (rolename_len == 0) {
        real_role = identity;
    } else {
        real_role = rolename;
    }

    if (!IsValidName(real_role)) {
        LOG(ERROR) << "Role name contains invalid char!";
        return false;
    }

    bool ret = IsValidUser(identity, real_role);
    LOG(INFO) << "verify role: " << real_role << " of identity: " << identity
              << " ,result: " << (ret ? "OK" : "FAIL");

    return ret;
}

const std::string Certifier::VerifyUser(const char* identity, const char* rolename) {

    CXThreadAutoLock auto_lock(&g_ca_mutex);

    if (m_certified) {
        CHECK(!m_role.empty() && !m_identity.empty()) << "m_role=" << m_role << "; m_identity=" << m_identity;
        if (identity != NULL) {
            CHECK_EQ(identity, m_identity);
        }
        if (rolename != NULL) {
            CHECK_EQ(rolename, m_role);
        }
        return m_role;
    }

    m_certified = false;
    m_role = "";
    m_identity = "";

    CHECK(FLAGS_ticket.empty() || (!identity && !rolename));

    if (!FLAGS_ticket.empty())
    {
        if ( !GetIdentityRoleFromTicket(FLAGS_ticket, &m_identity, &m_role) )
        {
            LOG(WARNING) << "Corrupted ticket : " << FLAGS_ticket;
            return "";
        }
        else //use identity and role from the ticket
        {
            m_certified = true;
            return m_role;
        }
    }

    uint32_t identity_len = static_cast<uint32_t>(STRLEN(identity));
    uint32_t rolename_len = static_cast<uint32_t>(STRLEN(rolename));
    if (identity_len > kUserNameLen || rolename_len > kUserNameLen) {
        LOG(ERROR) << "identity : " << identity << " or rolename : " << rolename
                   << "longer than " << kUserNameLen;
        return "";
    }

    // get real identity
    char real_identity[kUserNameLen + 1] = {0};
    char real_role[kUserNameLen + 1] = {0};
    if (identity_len == 0) {
        if (!GetRealIdentity(real_identity, kUserNameLen)) {
            LOG(ERROR) << "Try get identity name from gflags or environment FAIL";
            return "";
        }
    } else {
        memcpy(real_identity, identity, identity_len);
    }

    if (!IsValidName(real_identity)) {
        LOG(ERROR) << "Identity name contains invalid char!";
        return "";
    }
    m_identity = real_identity;
    LOG(INFO) << "identity is " << real_identity;

    // get real role name
    if (rolename_len == 0) {
        if (!GetRealRole(real_identity, real_role, kUserNameLen)) {
            LOG(ERROR) << "Try get role name from gflags FAIL";
            return "";
        }
    } else {
        memcpy(real_role, rolename, rolename_len);
    }

    if (!IsValidName(real_role)) {
        LOG(ERROR) << "Role name contains invalid char!";
        return "";
    }

    m_certified = IsValidUser(real_identity, real_role);
    LOG(INFO) << "verify role: " << real_role << " of identity: " << real_identity
              << " ,result: " << (m_certified ? "OK" : "FAIL");

    if (m_certified)
        m_role = real_role;
    else
        m_role = "";
    return m_role;
}

bool Certifier::IsValidUser(const char* identity, const char* rolename) const {
    CHECK(identity && rolename);
    // mock
    if (FLAGS_ca_server.compare(kMockMode1) == 0
        || FLAGS_ca_server.compare(kMockMode2) == 0) {
        LOG(INFO) << "MockCA mode, skip verify user";
        return true;
    }

    // get sign from private key file
    char sign[kCaSignLen] = {0};
    uint32_t sign_len = kCaSignLen;
    if (!GetSign(identity, sign, &sign_len)) {
        LOG(ERROR) << "can't get sign from key file in current or home dir! please visit "
                   << "http://tapd.oa.com/v3/infra/wikis/view/XFS_SDK_Develop_FAQ";
        return false;
    }

    // encode sign
    char buff[2*kCaSignLen] = {0};
    int32_t buff_len = sizeof(buff);
    if (!Hex2String(reinterpret_cast<const unsigned char*>(sign),
                    sign_len, buff, &buff_len)) {
        LOG(ERROR) << "encode sign error, dest buffer too short!";
        return false;
    }

    char ptr_params[512] = {0};
    safe_snprintf(ptr_params, sizeof(ptr_params),
                  "CA_USERNAME=%s&CA_ROLENAME=%s&SIGN=%s&SIGN_LEN=%u",
                  identity, rolename, buff, sign_len);

    // send HTTP request
    // use GET
    CGetHttpResponse response;
    HTTP_CGI_ERROR err = response.GetResponse(
        m_ca_ip.c_str(), static_cast<uint16_t>(FLAGS_ca_port),
        kCaCgi, ptr_params, false, 5);

    if (err != ERROR_HTTP_OK) {
        LOG(ERROR) << "Verify user : " << identity << " role : " << rolename
                   << " FAIL! get http response error:" << GetHttpErrString(err);
        return false;
    }

    // parse http response
    return ParseResponse(response.GetHttpContent());
}

bool Certifier::Hex2String(const unsigned char* hex, int32_t hex_len,
                               char* buff, int32_t *buff_len) const {
    if(*buff_len < (hex_len * 2) + 1)
        return false;

    for (int32_t i = 0; i < hex_len; i++) {
        safe_snprintf(buff + i*2, 3, "%02x", hex[i]);
    }
    *buff_len = hex_len * 2;

    return true;
}

bool Certifier::GetSign(const char* identity, char* sign, uint32_t* sign_len) const {
    char cur_path[MAX_PATH] = {0};
    GetModuleFileName(NULL, cur_path, sizeof(cur_path));
    char* p = strrchr(cur_path, '/');
    if (!p)
        p = strrchr(cur_path, '\\');

    if (p)
        *(p+1) = 0;

    char private_filename[MAX_PATH] = {0};
    safe_snprintf(private_filename, sizeof(private_filename),
                  "%s%s_private_key.dat", cur_path, identity);
    CA_ERROR_CODE err = ERROR_CA_OK;
#ifdef WIN32
    if (!ParseKeyFile(identity, private_filename, sign, sign_len, &err)) {
        LOG(ERROR) << "try get private key from: " << private_filename
                   << " fail. Err:" << CaGetErrorCodeStr(err);
        return false;
    }
#else
    if (!ParseKeyFile(identity, private_filename, sign, sign_len, &err)) {
        char* home_path = getenv("HOME");
        safe_snprintf(private_filename, sizeof(private_filename),
                      "%s/.private_ca/%s_private_key.dat", home_path, identity);
        CA_ERROR_CODE err2 = ERROR_CA_OK;
        if (!ParseKeyFile(identity, private_filename, sign, sign_len, &err2)) {
            safe_snprintf(private_filename, sizeof(private_filename),
                          "./%s_private_key.dat", identity);
            CA_ERROR_CODE err3 = ERROR_CA_OK;
            if (!ParseKeyFile(identity, private_filename, sign, sign_len, &err3)) {
                LOG(ERROR) << "try get private key from: " << cur_path << identity
                           << "_private_key.dat fail. Err:" << CaGetErrorCodeStr(err);
                LOG(ERROR) << "try get private key from: " << home_path << "/.private_ca/"
                           << identity << "_private_key.dat fail. Err:" << CaGetErrorCodeStr(err2);
                LOG(ERROR) << "try get private key from cur cmd path"
                           << " fail. Err:" << CaGetErrorCodeStr(err3);
                return false;
            }
        }
    }
#endif

    return true;
}

#define CHECK_RET_AND_RETURN(b, err_ptr, code) \
{if (!(b)) {SET_ERRORCODE(err_ptr, code);fclose(fp);return false;}}

#define CHECK_RET_AND_DELETE_AND_RETURN(b, err_ptr, code) \
    {if (!(b)) {SET_ERRORCODE(err_ptr, code);fclose(fp);    \
    delete[] key_file_buffer; return false;}}

bool Certifier::ParseKeyFile(const char* identity, const char* filename,
                             char* sign, uint32_t* sign_len, CA_ERROR_CODE *err) const {
    FILE* fp = fopen(filename, "rb");

    if (!fp) {
        SET_ERRORCODE(err, ERROR_CA_OPEN_FAIL);
        return false;
    }

    CHECK_RET_AND_RETURN(0 == fseek(fp, 0, SEEK_END), err, ERROR_CA_INVALID_FORMAT);
    uint32_t file_size = ftell(fp);
    CHECK_RET_AND_RETURN(0 == fseek(fp, 0, SEEK_SET), err, ERROR_CA_INVALID_FORMAT);

    char* key_file_buffer = new char[file_size + 1];
    CHECK_RET_AND_DELETE_AND_RETURN(
        1 == static_cast<int32_t>(fread(key_file_buffer, file_size, 1, fp)),
        err, ERROR_CA_INVALID_FORMAT);

    // compare format
    uint32_t len = kFmtLen;
    CHECK_RET_AND_DELETE_AND_RETURN(0 == strncmp(key_file_buffer, kCaPriFmt, len),
                                    err, ERROR_CA_INVALID_FORMAT);

    // compare version
    uint16_t ver = 0;
    CHECK_RET_AND_DELETE_AND_RETURN((len + sizeof(uint16_t) < file_size),
                                    err, ERROR_CA_INVALID_VERSION);
    memcpy(&ver, key_file_buffer + len, sizeof(uint16_t));
    CHECK_RET_AND_DELETE_AND_RETURN(ver == kCurrentVersion, err, ERROR_CA_INVALID_VERSION);
    len = len + sizeof(uint16_t);

    // skip private key
    uint32_t pri_key_len = 0;
    CHECK_RET_AND_DELETE_AND_RETURN((len + sizeof(uint32_t) < file_size),
                                    err, ERROR_CA_INVALID_PRIVATEKEY);
    memcpy(&pri_key_len, key_file_buffer + len, sizeof(uint32_t));
    len = len + sizeof(uint32_t) + pri_key_len;

    // compare identity name
    uint32_t idenity_len = 0;
    CHECK_RET_AND_DELETE_AND_RETURN((len + sizeof(uint32_t) < file_size),
                                    err, ERROR_CA_INVALID_IDENTIY);
    memcpy(&idenity_len, key_file_buffer + len, sizeof(uint32_t));
    len = len + sizeof(uint32_t);
    CHECK_RET_AND_DELETE_AND_RETURN(0 == strncmp(key_file_buffer + len, identity, idenity_len),
                                    err, ERROR_CA_INVALID_IDENTIY);
    len = len + idenity_len;

    // get sign len
    uint32_t cur_sign_len = 0;
    CHECK_RET_AND_DELETE_AND_RETURN((len + sizeof(uint32_t) < file_size),
                                    err, ERROR_CA_INVALID_USER_SIGN);
    memcpy(&cur_sign_len, key_file_buffer + len, sizeof(uint32_t));
    len = len + sizeof(uint32_t);
    CHECK_RET_AND_DELETE_AND_RETURN(cur_sign_len != 0 && cur_sign_len < *sign_len &&
                                    (len + cur_sign_len <= file_size),
                                    err, ERROR_CA_INVALID_USER_SIGN);
    *sign_len = cur_sign_len;
    memcpy(sign, key_file_buffer + len, cur_sign_len);

    delete[] key_file_buffer;
    fclose(fp);
    return true;
}

bool Certifier::GetIdentity(std::string* identity) const {
    CXThreadAutoLock auto_lock(&g_ca_mutex);

    if (!IsVerified())
        return false;

    *identity = m_identity;
    return true;
}

bool Certifier::GetRealIdentity(char* real_identity, uint32_t identiy_len) const {
    // get user name from gflags
    uint32_t len = static_cast<uint32_t>(FLAGS_identity.size());
    if (len > 0 && len < identiy_len) {
        memcpy(real_identity, FLAGS_identity.c_str(), len);
        real_identity[len]=0;
        return true;
    }

    // get user name from environment
#ifdef WIN32
    len = GetEnvironmentVariable(kEnvIdentity, real_identity, kUserNameLen);
    if (len > 0 && len < identiy_len) {
        real_identity[len] = 0;
        return true;
    }
#else
    char* env = getenv(kEnvIdentity);
    len = STRLEN(env);
    if (len > 0 && len < identiy_len) {
        memcpy(real_identity, env, len);
        real_identity[len]=0;
        return true;
    }
#endif

    return false;
}

bool Certifier::GetRole(std::string* role) const {
    CXThreadAutoLock auto_lock(&g_ca_mutex);

    if (!IsVerified())
        return false;

    *role = m_role;
    return true;
}

bool Certifier::GetRealRole(char* real_identity, char* real_role, uint32_t role_len) const {
    if (FLAGS_role.empty())
        memcpy(real_role, real_identity, STRLEN(real_identity));
    else if (FLAGS_role.size() < role_len) {
        memcpy(real_role, FLAGS_role.c_str(), FLAGS_role.size());
    } else {
        LOG(ERROR) << "role in gflags is longer than max role len : " << role_len;
        return false;
    }

    return true;
}

bool Certifier::CreateTicket(std::string* ticket) const {
    if (!IsVerified())
        return false;

    std::string id;
    std::string role;

    if (GetIdentity(&id) && GetRole(&role)) {
        std::string crc_ticket = id + "\t" + role;
        uint32_t crc = CRC32Hash32(crc_ticket.c_str(), crc_ticket.length());
        std::string raw_ticket = crc_ticket + "\t" + NumberToString(crc);
        if (!Base64::Encode(raw_ticket, ticket))
            return false;
        return true;
    }
    return false;
}

bool Certifier::GetIdentityRoleFromTicket(const std::string& ticket, std::string* id, std::string* role) const {

    std::string raw_ticket;
    if (!Base64::Decode(ticket, &raw_ticket))
    {
        LOG(ERROR) << "Failed decoding ticket: " << ticket;
        return false;
    }

    std::vector<std::string> arr;
    SplitString(raw_ticket, "\t", &arr);
    if (arr.size() != 3)
    {
        LOG(ERROR) << "ill formated ticket(" << ticket << "): [" << raw_ticket << "]";
        return false;
    }

    uint32_t crc;
    if (!StringToNumber(arr[2] ,&crc ,10))
    {
        LOG(ERROR) << "Invalid CRC number! ticket(" << ticket << "): [" << raw_ticket << "]";
        return false;
    }

    std::string crc_ticket = arr[0]+"\t"+arr[1];
    if (CRC32Hash32(crc_ticket.c_str(), crc_ticket.length())!=crc)
    {
        LOG(ERROR) << "CRC failed! ticket(" << ticket << "): [" << raw_ticket << "]";
        return false;
    }

    *id = arr[0];
    *role = arr[1];

    return true;
}

bool Certifier::ParseResponse(const char* http_response) const {
    const char* p = strstr(http_response, "CheckUserResult=OK");
    LOG(INFO) << "CheckUserResult=" << (p ? "OK" : "FAIL");

    if (!p) {
        const char* err_pos = strstr(http_response, "ErrorCode=");
        if (err_pos) {
            uint32_t err_code = ATOI(err_pos + strlen("ErrorCode="));
            LOG(ERROR) << "Get CA response, ErrorCode=" << err_code << ", ErrorString:"
                       << CaGetErrorCodeStr(err_code);
        } else {
            LOG(ERROR) << "Parse CA response FAIL! try get ErrorCode fail...";
        }
    }
    return (p != NULL);
}

bool Certifier::IsValidName(const char* name) const {
    if (!name || !(*name) || isdigit(*name)) return false;
    const char* p = name;
    while (*p != '\0') {
        if (!isalpha(*p) && !isdigit(*p) && *p != '_') return false;
        ++p;
    }
    return true;
}

} // namespace ca

DEFINE_MODULE(Certifier)
{
    // invoked by InitAllModule
    ca::Certifier::Instance().VerifyUser();
    if ( !ca::Certifier::Instance().IsVerified() )
    {
        LOG(ERROR) << "Authentication failed!";
        return false; // will be aborbed by the module framework
    }
    else
        return true;
}

#endif // COMMON
