#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "common/crypto/rsa2/rsa2.h"
#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/crypto/ca/ca_server/ca.h"

using namespace xfs::base;

namespace ca {

// 认证过的用户名可以cache的时间,默认20分钟
const time_t kVerifiedUserTimeout = 1200;

CA::CA(): m_ca_public_key_ready(0), m_ca_private_key_ready(0) {
    m_private_key_buffer[0] = 0;
    m_user_name_buffer[0] = 0;
    m_pub_sign_buffer[0] = 0;
    m_pri_sign_buffer[0] = 0;
    m_info_buffer[0] = 0;
    srand((uint32_t)time(0));
}

CA::~CA() {
}

// bits_count_keysize在96~1024之间,这里固定取值为600
bool CA::Init() {
    int32_t status = -1;

    m_rsa_obj.InitRandomStruct((int32_t)safe_rand());
    GetModuleFileName(NULL, m_ca_dir, sizeof(m_ca_dir));

    // linux and windows
    char* p = strrchr(m_ca_dir, '/');
    if ( !p )
        p = strrchr(m_ca_dir, '\\');

    if (NULL == p)
        return false;

    p++;

    // 追加目录htdocs
    int32_t len_remain = m_ca_dir + sizeof(m_ca_dir) - p - 1;
    // 边界检查
    int32_t len_add_dir = strlen(kApacheHtdocsName) + strlen(kCaDirName) + 2;
    if (len_remain < len_add_dir)
        return false;

    safe_snprintf(p, len_remain, kApacheHtdocsName);
    if (!m_dir_manage.IsDirExist(m_ca_dir)) {
        m_dir_manage.MkDir(m_ca_dir);
    }
    p += strlen(kApacheHtdocsName);

    // 追加目录ca_dir
    len_remain = m_ca_dir + sizeof(m_ca_dir) - p - 1;
    safe_snprintf(p, len_remain, "%s%s", SPLIT_SIGN, kCaDirName);
    if (!m_dir_manage.IsDirExist(m_ca_dir)) {
        m_dir_manage.MkDir(m_ca_dir);
    }
    p += strlen(kCaDirName) + 1;

    len_remain = m_ca_dir + sizeof(m_ca_dir) - p - 1;
    safe_snprintf(p, len_remain, SPLIT_SIGN);

    // 第一次启动CA中心的时候，将CA产生的密钥对保存到文件里，并存放到成员变量里
    // 如果不是第一次，则直接从文件里读出用户的密钥对，存放到成员变量里
    if (!IsExist("ca_root")) {
        // 生成CA自己的证书
        // 产生CA自己的公钥和私钥
        RsaKeyPair rsa_keypair;
        bool ret_gen = GenKeyPair(kRsaKeyBitsCount, &rsa_keypair);
        if (!ret_gen)
            return false;

        m_ca_public_key = rsa_keypair.rsa_public_key;
        m_ca_private_key = rsa_keypair.rsa_private_key;
        CACertificate ca_certificate;
        CA_ERROR_CODE error_code;
        int32_t ret_regist = RegistCertificate("ca_root", &ca_certificate, &error_code);
        if (!ret_regist)
            return false;

        status = 0;
    } else {
        // 如果存在ca的证书记录，则校验证书里的信息，然后读出密钥对
        CACertificate     ca_certificate;
        CAPrivate         ca_private_file;
        char ca_private_key_path[MAX_PATH] = {0};
        char ca_public_key_path[MAX_PATH] = {0};
        CA_ERROR_CODE error_code_pub;
        CA_ERROR_CODE error_code_pri;
        bool ret_get_pub = GetUserPublicInfo("ca_root",
                                             &ca_certificate,
                                             &m_ca_public_key,
                                             ca_public_key_path,
                                             MAX_PATH,
                                             &error_code_pub);
        bool ret_get_pri = GetUserPrivateInfo("ca_root",
                                              &ca_private_file,
                                              ca_private_key_path,
                                              MAX_PATH,
                                              &error_code_pri);
        if ((!ret_get_pub)
            || (!ret_get_pri)
            || (0 != strcmp("ca_root", ca_certificate.ca_certificate_data.user_name))) {
            return false;
        }
        memcpy(&m_ca_private_key, ca_private_file.private_key, ca_private_file.private_key_len);
        status = 0;
    }

    if (0 == status) {
        m_ca_public_key_ready = 1;
        m_ca_private_key_ready = 1;
        return true;
    }
    return false;
}


// 产生公钥/私钥对, bits_count_keysize的范围是96~1024
bool CA::GenKeyPair(int32_t bits_count_keysize, RsaKeyPair* rsa_keypair) {
    int32_t ret = m_rsa_obj.GenerateKeyPair(bits_count_keysize);
    CHECK_EQ(ret, 0);
    R_RSA_PUBLIC_KEY* rsa_public_key;
    R_RSA_PRIVATE_KEY* rsa_private_key;
    bool ret_getpub = m_rsa_obj.GetPublicKey(&rsa_public_key);
    bool ret_getpri = m_rsa_obj.GetPrivateKey(&rsa_private_key);
    if (ret_getpub && ret_getpri) {
        rsa_keypair->rsa_public_key  = *rsa_public_key;
        rsa_keypair->rsa_private_key = *rsa_private_key;
        return true;
    }
    return false;
}

// CA公布自己的公钥，每一个用户都可以得到CA的公钥，用以验证CA颁发的证书
bool CA::GetCAPublicKey(R_RSA_PUBLIC_KEY** ca_public_key) {
    if (!m_ca_public_key_ready)
        return false;

    *ca_public_key = &m_ca_public_key;
    return true;
}


// 这个函数测试时用, CA公布自己的私钥
// bool CA::GetCAPrivateKey(R_RSA_PRIVATE_KEY** ca_private_key) {
//     if (!m_ca_private_key_ready)
//         return false;
//
//     *ca_private_key = &m_ca_private_key;
//     return true;
// }



// 用于在用户注册时判断该用户是否注册过
bool CA::IsExist(const char* user_name) {
    char user_dir_name[MAX_PATH] = {0};
    safe_snprintf(user_dir_name, sizeof(user_dir_name), "%s%s", m_ca_dir, user_name);
    return m_dir_manage.IsDirExist(user_dir_name) ? true : false;
}

// 用户的注册函数，用户通过输入用户名和用户密码，在CA注册
// 并获得由CA颁发的证书
// 如果失败，返回false
bool CA::RegistCertificate(const char* user_name,
                           CACertificate* ca_certificate,
                           CA_ERROR_CODE* error_code) {
    if (!user_name || !ca_certificate) {
        SET_ERRORCODE(error_code, ERROR_CA_PARAMETER_ERROR);
        return false;
    }

    // 先判断用户是否曾经注册过，如果CA中心有该用户的证书
    // 则删除旧的证书，并重新注册
    if (IsExist(user_name)) {
        bool ret_revoke = RevokeCertificate(user_name);
        if (!ret_revoke) {
            SET_ERRORCODE(error_code, ERROR_CA_DELETE_CERTIFICATE_FAIL);
            return false;
        }
    }

    // ----------------------------
    // 生成证书
    //
    // 证书格式
    safe_snprintf(ca_certificate->fmt, kFmtLen, "%s", kCaPubFmt);
    ca_certificate->version = kCurrentVersion;

    // 用户名和长度
    ca_certificate->user_name_len = static_cast<uint32_t>(strlen(user_name) + 1);
    if (ca_certificate->user_name_len > kUserNameLen) {
        SET_ERRORCODE(error_code, ERROR_CA_REGISTER_FAIL);
        return false;
    }
    safe_snprintf(ca_certificate->ca_certificate_data.user_name,
                  ca_certificate->user_name_len, "%s", user_name);

    // 证书的有效期
    ca_certificate->ca_certificate_data.certificate_time_begin = time(NULL);
    ca_certificate->ca_certificate_data.certificate_time_end =
        ca_certificate->ca_certificate_data.certificate_time_begin + DEFAULT_CA_CERTIFICATE_TIMEOUT;

    // memo描述
    safe_snprintf(ca_certificate->memo, sizeof(ca_certificate->memo), "%s", CA_MEMO);

    RsaKeyPair user_rsa_keypair;
    // CA为用户产生一对rsa_keypair，如果是ca_root，直接读取初始化时保存的值
    if (strcmp(user_name, "ca_root") == 0) {
        ca_certificate->public_key_len =  sizeof(m_ca_public_key);
        if (ca_certificate->public_key_len > kMaxPublicKeyLen) {
            SET_ERRORCODE(error_code, ERROR_CA_REGISTER_FAIL);
            return false;
        }
        memcpy(ca_certificate->ca_certificate_data.user_public_key,
               &m_ca_public_key,
               ca_certificate->public_key_len);
        user_rsa_keypair.rsa_private_key = m_ca_private_key;
    } else {
        GenKeyPair(kRsaKeyBitsCount, &user_rsa_keypair);
        ca_certificate->public_key_len =  sizeof(user_rsa_keypair.rsa_public_key);
        if (ca_certificate->public_key_len > kMaxPublicKeyLen) {
            SET_ERRORCODE(error_code, ERROR_CA_REGISTER_FAIL);
            return false;
        }
        memcpy(ca_certificate->ca_certificate_data.user_public_key,
               &(user_rsa_keypair.rsa_public_key),
               ca_certificate->public_key_len);
    }

    const uint32_t info_len = sizeof(ca_certificate->ca_certificate_data);
    // 用ca的private_key为证书签名
    ca_certificate->ca_sign = m_pub_sign_buffer;
    bool ret_ca_sign =
        GenCASign(reinterpret_cast<const unsigned char*>(&(ca_certificate->ca_certificate_data)),
                  info_len, &m_ca_private_key,
                  reinterpret_cast<unsigned char*>(ca_certificate->ca_sign),
                  &(ca_certificate->sign_len));
    if (!ret_ca_sign) {
        SET_ERRORCODE(error_code, ERROR_CA_REGISTER_FAIL);
        return false;
    }


    // --------------------------------
    // 生成用户可下载的私钥文件
    CAPrivate user_private_file;
    safe_snprintf(user_private_file.fmt, kFmtLen, "%s", kCaPriFmt);

    user_private_file.version = kCurrentVersion;

    user_private_file.private_key_len =  sizeof(user_rsa_keypair.rsa_private_key);
    if (user_private_file.private_key_len > kMaxPrivateKeyLen) {
        SET_ERRORCODE(error_code, ERROR_CA_REGISTER_FAIL);
        return false;
    }

    user_private_file.private_key = m_private_key_buffer;
    memcpy(user_private_file.private_key,
           &user_rsa_keypair.rsa_private_key,
           user_private_file.private_key_len);

    user_private_file.user_name_len = static_cast<uint32_t>(strlen(user_name) + 1);
    if (user_private_file.user_name_len > kUserNameLen) {
        SET_ERRORCODE(error_code, ERROR_CA_REGISTER_FAIL);
        return false;
    }

    user_private_file.user_name = m_user_name_buffer;
    safe_snprintf(user_private_file.user_name, user_private_file.user_name_len, "%s", user_name);

    // 用用户的private_key来对用户名的hash进行加密，生成sign
    user_private_file.sign = m_pri_sign_buffer;
    bool ret_user_sign = GenCASign(reinterpret_cast<const unsigned char*>(user_name),
                                   user_private_file.user_name_len - 1,
                                   &(user_rsa_keypair.rsa_private_key),
                                   reinterpret_cast<unsigned char*>(user_private_file.sign),
                                   &(user_private_file.sign_len));
    if (!ret_user_sign) {
        SET_ERRORCODE(error_code, ERROR_CA_REGISTER_FAIL);
        return false;
    }

    // 将新注册的证书及信息归档
    bool ret_pri = CAPriArchive(user_name, &user_private_file);
    bool ret_pub = CAPubArchive(user_name, ca_certificate);
    if (!ret_pri || !ret_pub) {
        SET_ERRORCODE(error_code, ERROR_CA_REGISTER_FAIL);
        return false;
    }

    SET_ERRORCODE(error_code, ERROR_CA_OK);
    return true;
}


// 用户通过输入用户名，从CA查询，获得自己的public_key和证书
// 如果用户信息不存在，error_code=ERR_CA_UNREGISTERED_USER
// 如果用户证书过期，返回ERR_CA_OUT_OF_DATE
bool CA::GetUserPublicInfo(const char* user_name,
                           CACertificate* ca_certificate,
                           R_RSA_PUBLIC_KEY* user_public_key,
                           char* user_public_key_path,
                           uint32_t path_len,
                           CA_ERROR_CODE* error_code) {
    if (!user_name  || !ca_certificate || !user_public_key
            || !user_public_key_path || path_len < MAX_PATH) {
        SET_ERRORCODE(error_code, ERROR_CA_PARAMETER_ERROR);
        return false;
    }

    // 先判断用户是否曾经注册过，如果CA中心没有该用户的证书
    // 则返回，并提示用户，询问用户是否需要注册
    if (!IsExist(user_name)) {
        SET_ERRORCODE(error_code, ERROR_CA_UNREGISTERED_USER);
        return false;
    }

    char user_certificate_file_name[MAX_PATH] = {0};
    safe_snprintf(user_certificate_file_name,
                  sizeof(user_certificate_file_name),
                  "%s%s%s%s%s",
                  m_ca_dir,
                  user_name,
                  SPLIT_SIGN,
                  user_name,
                  kPubFileName);
    safe_snprintf(user_public_key_path,
                  path_len,
                  "ca_dir/%s/%s%s",
                  user_name,
                  user_name,
                  kPubFileName);

    if (!m_dir_manage.IsFileExist(user_certificate_file_name)) {
        SET_ERRORCODE(error_code, ERROR_CA_UNREGISTERED_USER);
        return false;
    }

    // 直接从磁盘读取用户public_key
    FILE*   fp_certificate = NULL;
    fp_certificate = fopen(user_certificate_file_name, "rb");

    if (!fp_certificate) {
        SET_ERRORCODE(error_code, ERROR_CA_OPEN_FAIL);
        return false;
    }

    // 一次读取整个文件到内存，再从内存中解析数据，减少读磁盘次数
    size_t ret = fread(m_info_buffer, 1, kMaxInfoLen, fp_certificate);
    if (0 == ret) {
        fclose(fp_certificate);
        SET_ERRORCODE(error_code, ERROR_CA_READ_FAIL);
        return false;
    }
    int32_t ret_fclose = fclose(fp_certificate);
    CHECK_EQ(0, ret_fclose);

    uint32_t off_set = 0;
    memcpy(ca_certificate->fmt, m_info_buffer, kFmtLen);
    if (strcmp(ca_certificate->fmt, kCaPubFmt) != 0) {
        SET_ERRORCODE(error_code, ERROR_CA_INVALID_CERTIFICATE);
        return false;
    }
    off_set += kFmtLen;

    memcpy(&(ca_certificate->version),
           m_info_buffer + off_set,  sizeof(ca_certificate->version));
    off_set += sizeof(ca_certificate->version);

    memcpy(&(ca_certificate->user_name_len),
           m_info_buffer + off_set, sizeof(ca_certificate->user_name_len));
    off_set += sizeof(ca_certificate->user_name_len);

    memcpy(ca_certificate->ca_certificate_data.user_name,
           m_info_buffer + off_set, ca_certificate->user_name_len);
    off_set += ca_certificate->user_name_len;

    memcpy(&(ca_certificate->public_key_len),
           m_info_buffer + off_set, sizeof(ca_certificate->public_key_len));
    off_set += sizeof(ca_certificate->public_key_len);

    memcpy(ca_certificate->ca_certificate_data.user_public_key,
           m_info_buffer + off_set, ca_certificate->public_key_len);
    off_set += ca_certificate->public_key_len;

    memcpy(&(ca_certificate->ca_certificate_data.certificate_time_begin),
           m_info_buffer + off_set,
           sizeof(ca_certificate->ca_certificate_data.certificate_time_begin));
    off_set += sizeof(ca_certificate->ca_certificate_data.certificate_time_begin);

    memcpy(&(ca_certificate->ca_certificate_data.certificate_time_end),
           m_info_buffer + off_set,
           sizeof(ca_certificate->ca_certificate_data.certificate_time_end));
    off_set += sizeof(ca_certificate->ca_certificate_data.certificate_time_begin);

    memcpy(&(ca_certificate->sign_len),
           m_info_buffer + off_set, sizeof(ca_certificate->sign_len));
    off_set += sizeof(ca_certificate->sign_len);

    ca_certificate->ca_sign = m_pub_sign_buffer;
    memcpy(ca_certificate->ca_sign,
           m_info_buffer + off_set, ca_certificate->sign_len);
    off_set += ca_certificate->sign_len;

    memcpy(ca_certificate->memo, m_info_buffer + off_set, kMemoLen);
    off_set += kMemoLen;
    // 比较读出的长度是否正确
    CHECK_EQ(ret, off_set);    

    // 判断用户的证书是否过期，过期则要求用户续期，或用户申请注销证书
    // 用户证书过期时，从m_certified_user_list中删除用户认证情况
    time_t time_now = time(NULL);

    if (time_now > ca_certificate->ca_certificate_data.certificate_time_end) {        
        SET_ERRORCODE(error_code, ERROR_CA_OUT_OF_DATE);
    } else {
        SET_ERRORCODE(error_code, ERROR_CA_OK);
    }

    memcpy(user_public_key,
           ca_certificate->ca_certificate_data.user_public_key,
           ca_certificate->public_key_len);
    SET_ERRORCODE(error_code, ERROR_CA_OK);
    return true;
}

// 用户通过输入用户名和用户密码，从CA查询，获得自己的private_key_file
// 如果用户信息不存在，error_code=ERR_CA_UNREGISTERED_USER
bool CA::GetUserPrivateInfo(const char* user_name,
                            CAPrivate* user_private_key_file,
                            char* user_private_key_path,
                            uint32_t path_len,
                            CA_ERROR_CODE* error_code) {
    if (!user_name || !user_private_key_file
            || !user_private_key_path || path_len < MAX_PATH) {
        SET_ERRORCODE(error_code, ERROR_CA_PARAMETER_ERROR);
        return false;
    }

    // 先判断用户是否曾经注册过，如果CA中心没有该用户的证书
    // 则返回，并提示用户，询问用户是否需要注册
    if (!IsExist(user_name)) {
        SET_ERRORCODE(error_code, ERROR_CA_UNREGISTERED_USER);
        return false;
    }

    char user_private_key_file_name[MAX_PATH] = {0};

    safe_snprintf(user_private_key_file_name,
                  sizeof(user_private_key_file_name),
                  "%s%s%s%s%s",
                  m_ca_dir,
                  user_name,
                  SPLIT_SIGN,
                  user_name,
                  kPriFileName);
    safe_snprintf(user_private_key_path,
                  path_len,
                  "ca_dir/%s/%s%s",
                  user_name,
                  user_name,
                  kPriFileName);
    // 返回用户的private_key
    FILE*   fp_private_key = NULL;
    fp_private_key = fopen(user_private_key_file_name, "rb");

    if (!fp_private_key) {
        SET_ERRORCODE(error_code, ERROR_CA_UNREGISTERED_USER);
        return false;
    }

    // 一次读取整个文件到内存，再从内存中解析数据，减少读磁盘次数
    size_t ret = fread(m_info_buffer, 1, kMaxInfoLen, fp_private_key);
    if (0 == ret) {
        fclose(fp_private_key);
        SET_ERRORCODE(error_code, ERROR_CA_READ_FAIL);
        return false;
    }
    int32_t ret_fclose = fclose(fp_private_key);
    CHECK_EQ(0, ret_fclose);

    uint32_t off_set = 0;
    memcpy(user_private_key_file->fmt, m_info_buffer, kFmtLen);
    if (strcmp(user_private_key_file->fmt, kCaPriFmt) != 0) {
        SET_ERRORCODE(error_code, ERROR_CA_INVALID_PRIVATEKEY);
        return false;
    }
    off_set += kFmtLen;

    memcpy(&(user_private_key_file->version),
           m_info_buffer + off_set, sizeof(user_private_key_file->version));
    off_set += sizeof(user_private_key_file->version);

    memcpy(&(user_private_key_file->private_key_len),
           m_info_buffer + off_set, sizeof(user_private_key_file->private_key_len));
    off_set += sizeof(user_private_key_file->private_key_len);

    user_private_key_file->private_key = m_private_key_buffer;
    memcpy(user_private_key_file->private_key,
           m_info_buffer + off_set, user_private_key_file->private_key_len);
    off_set += user_private_key_file->private_key_len;

    memcpy(&(user_private_key_file->user_name_len),
           m_info_buffer + off_set, sizeof(user_private_key_file->user_name_len));    
    if (user_private_key_file->user_name_len > kUserNameLen) {
        SET_ERRORCODE(error_code, ERROR_CA_INVALID_PRIVATEKEY);
        return false;
    }
    off_set += sizeof(user_private_key_file->user_name_len);

    user_private_key_file->user_name = m_user_name_buffer;
    memcpy(user_private_key_file->user_name,
           m_info_buffer + off_set, user_private_key_file->user_name_len);
    off_set += user_private_key_file->user_name_len;

    memcpy(&(user_private_key_file->sign_len), 
           m_info_buffer + off_set, sizeof(user_private_key_file->sign_len));
    off_set += sizeof(user_private_key_file->sign_len);

    user_private_key_file->sign = m_pri_sign_buffer;
    memcpy(user_private_key_file->sign,
           m_info_buffer + off_set, user_private_key_file->sign_len);
    off_set += user_private_key_file->sign_len;

    // 比较读出的长度是否正确
    CHECK_EQ(ret, off_set);
    
    SET_ERRORCODE(error_code, ERROR_CA_OK);
    return true;
}

// 用户通过输入用户名,向CA申请续期数字证书
bool CA::ApplyRenewKey(const char* user_name) {
    if (!user_name)
        return false;

    CACertificate ca_certificate;
    R_RSA_PUBLIC_KEY user_public_key;
    char user_public_key_path[MAX_PATH] = {0};
    CA_ERROR_CODE error_code;

    bool ret_get_pub = GetUserPublicInfo(user_name, &ca_certificate, &user_public_key,
                                         user_public_key_path, MAX_PATH, &error_code);
    if (!ret_get_pub) {
        return false;
    }


    // 更新用户证书中的time_begin和time_end
    ca_certificate.ca_certificate_data.certificate_time_begin = time(NULL);
    ca_certificate.ca_certificate_data.certificate_time_end =
        ca_certificate.ca_certificate_data.certificate_time_begin + DEFAULT_CA_CERTIFICATE_TIMEOUT;

    // CA对证书的重新签名
    const uint32_t info_len = sizeof(ca_certificate.ca_certificate_data);
    // 用ca的private_key为证书签名
    ca_certificate.ca_sign = m_pub_sign_buffer;
    bool ret_ca_sign =
        GenCASign(reinterpret_cast<unsigned char*>(&(ca_certificate.ca_certificate_data)),
                  info_len, &m_ca_private_key,
                  reinterpret_cast<unsigned char*>(ca_certificate.ca_sign),
                  &(ca_certificate.sign_len));
    if (!ret_ca_sign) {
        return false;
    }

    // 用户证书重新归档, 并把新的用户信息放入map中
    if (!CAPubArchive(user_name, &ca_certificate))
        return false;   
    
    return true;
}

// 用户通过输入用户名，向CA申请撤销数字证书
bool  CA::RevokeCertificate(const char* user_name) {
    if (!user_name)
        return false;

    char user_dir_name[MAX_PATH] = {0};
    safe_snprintf(user_dir_name,
                  sizeof(user_dir_name),
                  "%s%s%s",
                  m_ca_dir,
                  user_name,
                  SPLIT_SIGN);    
    
    return m_dir_manage.RecursivelyRmDir(user_dir_name);
}

// 为每个用户建立一个目录，存放两个文件: private key file和证书
// 用户私钥文件归档
bool CA::CAPriArchive(const char* user_name, const CAPrivate* user_private_file) {
    if (!user_name || !user_private_file)
        return false;

    // 建立用户目录
    char user_dir_name[MAX_PATH] = {0};
    safe_snprintf(user_dir_name,
                  sizeof(user_dir_name),
                  "%s%s%s",
                  m_ca_dir,
                  user_name,
                  SPLIT_SIGN);

    if (!m_dir_manage.IsDirExist(user_dir_name)) {
        m_dir_manage.MkDir(user_dir_name);
    }

    // 建立文件，private key
    char file_name[MAX_PATH] = {0};
    safe_snprintf(file_name, sizeof(file_name), "%s", user_dir_name);
    char user_private_key_file_name[MAX_PATH] = {0};
    safe_snprintf(user_private_key_file_name,
                  sizeof(user_private_key_file_name),
                  "%s%s%s",
                  file_name,
                  user_name,
                  kPriFileName);

    // -------------------------------------------------
    // private_key_file归档
    //
    // 先将所有内容写到内存里，然后一次写入磁盘文件，减少读盘次数
    // 写入文件格式头
    uint32_t off_set = 0;
    memcpy(m_info_buffer, user_private_file->fmt, kFmtLen);
    off_set += kFmtLen;

    // 写入文件版本号
    memcpy(m_info_buffer + off_set,
           &(user_private_file->version), sizeof(user_private_file->version));
    off_set += sizeof(user_private_file->version);

    // 写入privatekey的长度
    memcpy(m_info_buffer + off_set,
           &(user_private_file->private_key_len), sizeof(user_private_file->private_key_len));
    off_set += sizeof(user_private_file->private_key_len);

    // 写入privatekey
    memcpy(m_info_buffer + off_set,
           user_private_file->private_key, user_private_file->private_key_len);
    off_set += user_private_file->private_key_len;

    // 写入username的长度
    memcpy(m_info_buffer + off_set,
           &(user_private_file->user_name_len), sizeof(user_private_file->user_name_len));
    off_set += sizeof(user_private_file->user_name_len);

    // 写入username
    memcpy(m_info_buffer + off_set,
           user_private_file->user_name, user_private_file->user_name_len);
    off_set += user_private_file->user_name_len;

    // 写入signature的长度
    memcpy(m_info_buffer + off_set,
           &(user_private_file->sign_len), sizeof(user_private_file->sign_len));
    off_set += sizeof(user_private_file->sign_len);

    // 写入signature
    memcpy(m_info_buffer + off_set,
           user_private_file->sign, user_private_file->sign_len);
    off_set += user_private_file->sign_len;

    if (off_set > kMaxInfoLen)
        return false;

    FILE*   fp_private_key = NULL;
    fp_private_key = fopen(user_private_key_file_name, "wb");
    if (!fp_private_key)
        return false;

    int32_t ret = fwrite(m_info_buffer, off_set, 1, fp_private_key);
    if (1 != ret) {
        fclose(fp_private_key);
        unlink(user_private_key_file_name);
        return false;
    }
    int32_t  ret_fclose = fclose(fp_private_key);
    CHECK_EQ(0, ret_fclose);
    return true;
}

// 为每个用户建立一个目录，存放两个文件: private key file和证书
// 用户证书文件归档
bool CA::CAPubArchive(const char* user_name, const CACertificate* user_certificate) {
    if (!user_name || !user_certificate)
        return false;

    // 建立用户目录
    char user_dir_name[MAX_PATH] = {0};
    safe_snprintf(user_dir_name, sizeof(user_dir_name), "%s%s%s", m_ca_dir, user_name, SPLIT_SIGN);

    if (!m_dir_manage.IsDirExist(user_dir_name)) {
        m_dir_manage.MkDir(user_dir_name);
    }

    // 建立文件，存放证书
    char file_name[MAX_PATH] = {0};
    safe_snprintf(file_name, sizeof(file_name), "%s", user_dir_name);
    char user_certificate_file_name[MAX_PATH] = {0};
    safe_snprintf(user_certificate_file_name,
                  sizeof(user_certificate_file_name),
                  "%s%s%s",
                  file_name,
                  user_name,
                  kPubFileName);


    // -------------------------------------------------
    // 用户证书归档
    //
    // 先将所有内容写到内存里，然后一次写入磁盘文件，减少读盘次数
    uint32_t off_set = 0;
    // 写入文件格式头
    memcpy(m_info_buffer, user_certificate->fmt, kFmtLen);
    off_set += kFmtLen;

    // 写入文件版本号
    memcpy(m_info_buffer + off_set,
           &(user_certificate->version), sizeof(user_certificate->version));
    off_set += sizeof(user_certificate->version);

    // 写入username的长度
    memcpy(m_info_buffer + off_set,
           &(user_certificate->user_name_len), sizeof(user_certificate->user_name_len));
    off_set += sizeof(user_certificate->user_name_len);

    // 写入username
    memcpy(m_info_buffer + off_set,
           user_certificate->ca_certificate_data.user_name, user_certificate->user_name_len);
    off_set += user_certificate->user_name_len;

    // 写入publickey的长度
    memcpy(m_info_buffer + off_set,
           &(user_certificate->public_key_len), sizeof(user_certificate->public_key_len));
    off_set += sizeof(user_certificate->public_key_len);

    // 写入publickey
    memcpy(m_info_buffer + off_set, user_certificate->ca_certificate_data.user_public_key,
           user_certificate->public_key_len);
    off_set += user_certificate->public_key_len;

    // 写入certificate-begin
    memcpy(m_info_buffer + off_set,
           &(user_certificate->ca_certificate_data.certificate_time_begin),
           sizeof(user_certificate->ca_certificate_data.certificate_time_begin));
    off_set += sizeof(user_certificate->ca_certificate_data.certificate_time_begin);

    // 写入certificate-end
    memcpy(m_info_buffer + off_set,
           &(user_certificate->ca_certificate_data.certificate_time_end),
           sizeof(user_certificate->ca_certificate_data.certificate_time_end));
    off_set += sizeof(user_certificate->ca_certificate_data.certificate_time_end);

    // 写入signature的长度
    memcpy(m_info_buffer + off_set,
           &(user_certificate->sign_len), sizeof(user_certificate->sign_len));
    off_set += sizeof(user_certificate->sign_len);

    // 写入signature
    memcpy(m_info_buffer + off_set,
           user_certificate->ca_sign, user_certificate->sign_len);
    off_set += user_certificate->sign_len;

    // 写入证书memo
    memcpy(m_info_buffer + off_set, user_certificate->memo, kMemoLen);
    off_set += kMemoLen;

    if (off_set > kMaxInfoLen)
        return false;

    FILE*   fp_certificate = NULL;
    fp_certificate = fopen(user_certificate_file_name, "wb");

    if (!fp_certificate)
        return false;

    int32_t ret = fwrite(m_info_buffer, off_set, 1, fp_certificate);
    if (1 != ret) {
        fclose(fp_certificate);
        unlink(user_certificate_file_name);
        return false;
    }
    int32_t ret_fclose = fclose(fp_certificate);
    CHECK_EQ(0, ret_fclose);
    return true;
}

// 签名函数
// CA为证书产生数字签名，即算出user_name+user_publice_key+time_begin+time_end的hash值
// 并用CA的私钥加密
// CA为私钥文件产生数字签名，即算出user_name的hash值，并用用户的私钥加密
bool CA::GenCASign(const unsigned char* info,
                   uint32_t info_len,
                   R_RSA_PRIVATE_KEY* private_key,
                   unsigned char* ca_sign,
                   uint32_t* sign_len) {
    if (!info || !private_key)
        return false;

    // 计算要加密的src的hash值
    uint32_t hash = 0;
    hash = GetCRC32(0xffffffff, info, info_len);

    // 用private_key来对hash进行签名，即对其加密
    m_rsa_obj.SetPrivateKey(private_key);
    uint32_t hash_len = sizeof(uint32_t);
    int32_t ret_sign =
        m_rsa_obj.PrivateEncrypt(reinterpret_cast<unsigned char*>(&hash), hash_len,
                                 ca_sign, sign_len);
    return (0 == ret_sign) ? true : false;
}



bool CA::IsValidUser(const char* user_name, char* sign, uint32_t sign_len,
                     CA_ERROR_CODE* error_code) {
    if (!user_name || !sign) {
        SET_ERRORCODE(error_code, ERROR_CA_PARAMETER_ERROR);
        return false;
    }
    
    // 第一次请求验证时才进行解密验证，当验证通过时，将该用户添加到已认证用户列表里
    std::map<std::string, timeval>::iterator it = m_verified_users.find(user_name);
    if(it != m_verified_users.end()){
        timeval t_last = it->second;
        // 使用相对时间
        timeval t_now;
        fast_getrelativetimeofday(&t_now);
        
        if(t_now.tv_sec - t_last.tv_sec <= kVerifiedUserTimeout){
            // 在cache里面找到用户,且未超时
            SET_ERRORCODE(error_code, ERROR_CA_OK);
            return true;
        }else{
            // 在cache里找到用户,但超时了
            m_verified_users.erase(it);
        }        
    }

    // 到这里肯定是cache里找不到结果的用户名
    // 重新对用户进行认证

    // 先对sign解密，得出hash
    R_RSA_PUBLIC_KEY  user_public_key;
    char              user_public_key_path[MAX_PATH] = {0};
    CACertificate     ca_certificate;
   
    bool ret_getkey = GetUserPublicInfo(user_name,
                                        &ca_certificate,
                                        &user_public_key,
                                        user_public_key_path,
                                        MAX_PATH,
                                        error_code);

    if (!ret_getkey)
        return false;

    m_rsa_obj.SetPublicKey(&user_public_key);
    uint32_t user_name_len = static_cast<uint32_t>(strlen(user_name));

    uint32_t hash_orignal = 0;  
    hash_orignal = GetCRC32(0xffffffff,
        reinterpret_cast<const unsigned char*>(user_name),
        user_name_len);      

    uint32_t hash_decrypt = 0;
    uint32_t hash_decrypt_len = sizeof(uint32_t);
    int32_t ret_decrpyt = 
        m_rsa_obj.PublicDecrypt(reinterpret_cast<unsigned char*>(sign),
                                sign_len,
                                reinterpret_cast<unsigned char*>(&hash_decrypt),
                                &hash_decrypt_len);
    if (0 != ret_decrpyt) {
        SET_ERRORCODE(error_code, ERROR_CA_INVALID_CERTIFICATE);
        LOG(ERROR) << "ERR_CA_INVALID_CERTIFICATE, PublicDecrypt error";
        LOG(ERROR) << "user_name = " << user_name;
        LOG(ERROR) << "ca_sign = " << sign;
        LOG(ERROR) << "sign_len = " << sign_len;
        return false;
    }

    if (hash_orignal != hash_decrypt) {
        SET_ERRORCODE(error_code, ERROR_CA_INVALID_USER_SIGN);
        LOG(ERROR) << "ERR_CA_INVALID_CERTIFICATE, hash_orignal != hash_decrypt";
        LOG(ERROR) << "hash_orignal = " << hash_orignal << " hash_decrypt = " << hash_decrypt;
        LOG(ERROR) << "user_name = " << user_name;
        LOG(ERROR) << "ca_sign = " << sign;
        LOG(ERROR) << "sign_len = " << sign_len;
        return false;
    }

    SET_ERRORCODE(error_code, ERROR_CA_OK);

    // 第一次验证时放入set中

    // 使用相对时间
    struct timeval t;
    fast_getrelativetimeofday(&t);
    m_verified_users.insert(std::pair<std::string,timeval>(user_name, t));
    return true;
}

} // namespace ca
