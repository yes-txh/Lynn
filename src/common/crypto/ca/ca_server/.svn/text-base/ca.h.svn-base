//////////////////////////////////////////////////////////////////////////
// @file:   ca
// @brief:  模拟实现CA认证，伪造数字签名证书
// @author: joeytian@tencent.com
// @time:   2010-11-24
// 修改历史:
//          <author>    <time>
//////////////////////////////////////////////////////////////////////////

// 模拟实现CA认证，伪造数字签名证书
// 用户向CA申请证书的步骤

// 1.CA以用户名为ca_root产生自己的证书和私钥文件。CA服务器保存。
// 2.ca为每个用户建立一个目录，存放两个文件:：用户的证书文件和包含用户私钥的私钥文件。
//
//   证书内容为：
//
//        +-----------------------------------------------------+
//        | kCaPubFmt = "CA_PBULIC_KEY"                         |
//        | version                                             |
//        |-----------------------------------------------------|
//        | len_user_name                                       |
//        | user_name                                           |
//        | len_user_public_key                                 |
//        | user_public_key                                     |
//        | certificate-begin:***                               |
//        | certificate-end:  ***                               |
//        | len_sign                                            |
//        | Sign=Encrypt(ca_private_key, hash of name/key/time) |
//        |-----------------------------------------------------|
//        | memo                                                |
//        +_____________________________________________________+
//   用户私钥文件内容为：
//
//        +--------------------------------------------------+
//        | kCaPriFmt = "CA_PRIVATE_KEY"                     |
//        | version                                          |
//        |--------------------------------------------------|
//        | len_user_private_key                             |
//        | user_private_key                                 |
//        | len_user_name                                    |
//        | user_name                                        |
//        | len_sign                                         |
//        |--------------------------------------------------|
//        | Sign=Encrypt(user_private_key,hash of user_name) |
//        +__________________________________________________+
//
// 3：搭建web服务器(apache)。需要tokin卡登录认证。
//
// 使用：
// 1：开发客户端程序，可以从CA获取证书和用户自己的私钥（需要输入密码）文件存放到用户当前目录~/
// 2：SDK client File::Init()的时候根据输入的用户名查找用户自身私钥文件，
//     从私钥文件中读取签名(签名为用户私钥机密过的用户名hash值)，把用户名+签名一起发送到ca中心，
//     ca中心根据用户名找到对应的证书，从证书中获得用户的公钥，用用户公钥对签名进行解密。
//     解密成功后验证hash的一致性。



#ifndef COMMON_CRYPTO_CA_CA_SERVER_CA_H_
#define COMMON_CRYPTO_CA_CA_SERVER_CA_H_

#include <map>
#include <string>
#include <set>
#include "common/crypto/rsa2/rsa2.h"
#include "common/crypto/ca/ca_public/ca_error_code.h"
#include "common/crypto/ca/ca_public/ca_struct.h"
#include "common/crypto/ca/ca_server/dir_manage.h"

namespace ca {
//
// CA类,用于用户证书注册,撤销,续期
// 不支持多线程调用
// bits_count_keysize的值固定为600，为CA用户产生自己的一对密钥
//
class CA {
public:  
    CA();
    ~CA();
    
    bool Init();


    // 用户的注册函数，用户通过输入用户名和用户密码，在CA注册
    // 并获得由CA颁发的证书
    bool RegistCertificate(const char* user_name,
                           CACertificate* ca_certificate,
                           CA_ERROR_CODE* error_code);


    // 用户通过输入用户名，从CA查询，获得自己的public_key和证书
    bool GetUserPublicInfo(const char* user_name,
                           CACertificate* ca_certificate,
                           R_RSA_PUBLIC_KEY* user_public_key,
                           char* user_public_key_path,
                           uint32_t path_len,
                           CA_ERROR_CODE* error_code);

    // 用户通过输入用户名，从CA查询，可以下载自己的private_key
    bool GetUserPrivateInfo(const char* user_name,
                            CAPrivate* user_private_key_file,
                            char* user_private_key_path,
                            uint32_t path_len,
                            CA_ERROR_CODE* error_code);

    // 用户通过输入用户名,向CA申请续期数字证书
    bool ApplyRenewKey(const char* user_name);


    // 用户通过输入用户名,向CA申请撤销数字证书
    bool RevokeCertificate(const char* user_name);


    // CA公布自己的公钥，每一个用户都可以得到CA的公钥，用以验证CA颁发的证书
    bool GetCAPublicKey(R_RSA_PUBLIC_KEY** ca_public_key);

    //  这个函数测试时用, CA公布自己的私钥
    // bool GetCAPrivateKey(R_RSA_PRIVATE_KEY** ca_private_key);

    // 用用户的公钥解密sign，
    bool IsValidUser(const char* user_name, char* sign, uint32_t sign_len,
                     CA_ERROR_CODE* error_code);
    // 用于在用户注册时判断该用户是否注册过.
    bool IsExist(const char* user_name);


private:
    // 产生公钥/私钥对, bits_count_keysize的范围是96~1024
    bool GenKeyPair(int32_t bits_count_keysize, RsaKeyPair* rsa_keypair);

    // 签名函数
    // CA为证书产生数字签名，即算出user_name+user_publice_key+time_begin+time_end的hash值
    // 并用CA的私钥加密
    // CA为私钥文件产生数字签名，即算出user_name的hash值，并用用户的私钥加密
    bool GenCASign(const unsigned char* info,
                   uint32_t info_len,
                   R_RSA_PRIVATE_KEY* private_key,
                   unsigned char* ca_sign,
                   uint32_t* sign_len);




    // 为每个用户建立一个目录，存放两个文件: private key，证书
    bool CAPriArchive(const char* user_name, const CAPrivate* user_private_file);
    bool CAPubArchive(const char* user_name, const CACertificate* user_certificate);

private:
    int32_t m_ca_public_key_ready;
    int32_t m_ca_private_key_ready;
    R_RSA_PUBLIC_KEY  m_ca_public_key;
    R_RSA_PRIVATE_KEY m_ca_private_key;
    CRSACrypt m_rsa_obj;
    DirManage m_dir_manage;
    int32_t m_num_certificate;
    // 本地文件存储目录
    char m_ca_dir[MAX_PATH];

    // 定义三个buferr，供private_key, user_name, sign使用
    char m_private_key_buffer[kMaxPrivateKeyLen];
    char m_user_name_buffer[kUserNameLen];
    char m_pub_sign_buffer[kCaSignLen];
    char m_pri_sign_buffer[kCaSignLen];
    char m_info_buffer[kMaxInfoLen];    
    
    // 记录已经通过认证的用户名和当时时间
    std::map<std::string, timeval> m_verified_users;
};
} // namespace ca

#endif // COMMON_CRYPTO_CA_CA_SERVER_CA_H_
