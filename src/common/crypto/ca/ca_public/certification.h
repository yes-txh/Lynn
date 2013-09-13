// Copyright 2010, Tencent Inc.
// Author: fatliu(fatliu@tencent.com)

#ifndef COMMON_CA_CA_CLIENT_CERTIFICATION_H_
#define COMMON_CA_CA_CLIENT_CERTIFICATION_H_
#include <map>

#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/crypto/ca/ca_public/ca_struct.h"
#include "common/crypto/ca/ca_public/ca_error_code.h"
#include "common/base/singleton.hpp"

namespace ca {

class Certifier : public SingletonBase<Certifier>
{
public:
    Certifier();
    ~Certifier();

    // 功能描述: 向后兼容
    static Certifier* GetInstance() { return &Instance(); }
    static void FreeInstance() { }

    // 功能描述: 检查用户ID和角色是否有授权，不影响现有的登陆
    // 输入参数:
    //           identity 输入identity
    //           rolename 输入role名，若为NULL则用identity代替rolename
    // 返回值:   成功则返回true,失败则返回false.
    bool CheckUser(const char* identity,
                   const char* rolename = NULL) const;

    // 功能描述: 验证用户对外接口, 暂时保留接口向后兼容，以后可以都从GFLAGS中读，所以ticket参数未加
    // 输入参数:
    //           identity 输入identity,为空时依次找flag identity和环境变量LOGNAME(WIN32下的USERNAME),
    //                    都为空则返回失败;
    //           rolename 输入role名,为空时找flag role,若也为空则设置为identity;
    // 返回值:   成功则返回实际role,失败则返回空.
    const std::string VerifyUser(const char* identity = NULL,
                                 const char* rolename = NULL);

    // 功能描述: 是否已经验证;
    // 返回值:   成功则返回true,失败则返回false.
    bool IsVerified(void) const { return m_certified; }

    // 功能描述: 获取默认username(默认为空时,需找gflags和env里的用户名信息);
    // 输出参数:
    //           real_identity      实际用户名;
    // 返回值:   成功true,失败false.
    bool GetIdentity(std::string *real_identity) const;

    // 功能描述: 获取默认username(默认为空时,需找gflags和env里的用户名信息);
    // 输出参数:
    //           real_role          实际role名
    // 返回值:   成功true,失败false.
    bool GetRole(std::string *real_role) const;

    // 功能描述: 生成ticket
    // 输出参数:
    //           ticket          包含用户名和角色名的ticket
    // 返回值:   成功true,失败false.
    bool CreateTicket(std::string *ticket) const;

    // 功能描述: 测试用，重置后可以再次调用VerifyUser
    void Reset(void) { m_certified = false; };

private:

    // 功能描述: 从ticket中获取id和角色;
    // 输入参数:
    //           ticket 编码后的ticket
    // 输出参数:
    //           id   用户名;
    //           role 角色
    // 返回值:   成功true,失败false.
    bool GetIdentityRoleFromTicket(const std::string& ticket, std::string* id, std::string* role) const;

    // 功能描述: 验证用户;
    // 输入参数:
    //           identity 用户名;
    //           rolename role名
    // 返回值:   成功则返回role,失败则返回空.
    bool IsValidUser(const char* identity = NULL, const char* rolename = NULL) const;


    // 功能描述: 获取签名;
    // 输入参数:
    //           username 用户名;
    //           sign_len 签名buffer大小
    // 输出参数:
    //           sign     签名的buffer;
    //           sign_len 返回签名长度
    // 返回值:   成功true,失败false.
    bool GetSign(const char* identity, char* sign, uint32_t* sign_len) const;

    // 功能描述: Encode签名,将无符号字符转为16进制编码;
    // 输入参数:
    //           hex      Encode前的无符号字符串;
    //           hex_len  Encode前的无符号字符串长度
    // 输出参数:
    //           buff     Encode签名后的buffer;
    //           buff_len Encode签名后的长度
    // 返回值:   成功true,失败false.
    bool Hex2String(const unsigned char* hex, int hex_len, char* buff, int *buff_len) const;

    // 功能描述: 解析private key file, 获取签名;
    // 输入参数:
    //           identity      实际用户名
    //           filename      key文件名;
    // 输出参数:
    //           sign          签名buffer;
    //           sign_len      签名长度
    // 返回值:   成功true,失败false.
    // key file format :
    // | version | len_privatekey | privatekey | len_username | username | len_sign | sign |
    // |----2B---|-------4B-------|----len-----|-----4B-------|---len----|----4B----|--len-|
    bool ParseKeyFile(const char* identity, const char* filename,
                      char* sign, uint32_t* sign_len,
                      CA_ERROR_CODE* err = NULL) const;


    // 功能描述: 解析CA回应包;
    // 返回值:   成功true,失败false.
    bool ParseResponse(const char* http_response) const;

    // 功能描述: 获取默认username(默认为空时,需找gflags和env里的用户名信息);
    // 输出参数:
    //           real_identity      实际用户名buffer;
    //           identity_len       实际用户名buffer长度;
    // 返回值:   成功true,失败false.
    bool GetRealIdentity(char* real_identity, uint32_t identity_len) const;

    // 功能描述: 获取默认username(默认为空时,需找gflags和env里的用户名信息);
    // 输入参数:
    //           real_identity      实际用户名;
    // 输出参数:
    //           real_role          实际role名buffer
    //           identity_len       实际role名buffer长度;
    // 返回值:   成功true,失败false.
    bool GetRealRole(char* real_identity, char* real_role, uint32_t role_len) const;


    // 功能描述: 检测name中的非法字符;
    // 返回值:   成功true,失败false.
    bool IsValidName(const char* name) const;

    std::string                   m_ca_ip;

    bool          m_certified;
    std::string   m_identity;
    std::string   m_role;
};
} // namespace ca

#endif //COMMON_CA_CA_CLIENT_CERTIFICATION_H_

