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

    // ��������: ������
    static Certifier* GetInstance() { return &Instance(); }
    static void FreeInstance() { }

    // ��������: ����û�ID�ͽ�ɫ�Ƿ�����Ȩ����Ӱ�����еĵ�½
    // �������:
    //           identity ����identity
    //           rolename ����role������ΪNULL����identity����rolename
    // ����ֵ:   �ɹ��򷵻�true,ʧ���򷵻�false.
    bool CheckUser(const char* identity,
                   const char* rolename = NULL) const;

    // ��������: ��֤�û�����ӿ�, ��ʱ�����ӿ������ݣ��Ժ���Զ���GFLAGS�ж�������ticket����δ��
    // �������:
    //           identity ����identity,Ϊ��ʱ������flag identity�ͻ�������LOGNAME(WIN32�µ�USERNAME),
    //                    ��Ϊ���򷵻�ʧ��;
    //           rolename ����role��,Ϊ��ʱ��flag role,��ҲΪ��������Ϊidentity;
    // ����ֵ:   �ɹ��򷵻�ʵ��role,ʧ���򷵻ؿ�.
    const std::string VerifyUser(const char* identity = NULL,
                                 const char* rolename = NULL);

    // ��������: �Ƿ��Ѿ���֤;
    // ����ֵ:   �ɹ��򷵻�true,ʧ���򷵻�false.
    bool IsVerified(void) const { return m_certified; }

    // ��������: ��ȡĬ��username(Ĭ��Ϊ��ʱ,����gflags��env����û�����Ϣ);
    // �������:
    //           real_identity      ʵ���û���;
    // ����ֵ:   �ɹ�true,ʧ��false.
    bool GetIdentity(std::string *real_identity) const;

    // ��������: ��ȡĬ��username(Ĭ��Ϊ��ʱ,����gflags��env����û�����Ϣ);
    // �������:
    //           real_role          ʵ��role��
    // ����ֵ:   �ɹ�true,ʧ��false.
    bool GetRole(std::string *real_role) const;

    // ��������: ����ticket
    // �������:
    //           ticket          �����û����ͽ�ɫ����ticket
    // ����ֵ:   �ɹ�true,ʧ��false.
    bool CreateTicket(std::string *ticket) const;

    // ��������: �����ã����ú�����ٴε���VerifyUser
    void Reset(void) { m_certified = false; };

private:

    // ��������: ��ticket�л�ȡid�ͽ�ɫ;
    // �������:
    //           ticket ������ticket
    // �������:
    //           id   �û���;
    //           role ��ɫ
    // ����ֵ:   �ɹ�true,ʧ��false.
    bool GetIdentityRoleFromTicket(const std::string& ticket, std::string* id, std::string* role) const;

    // ��������: ��֤�û�;
    // �������:
    //           identity �û���;
    //           rolename role��
    // ����ֵ:   �ɹ��򷵻�role,ʧ���򷵻ؿ�.
    bool IsValidUser(const char* identity = NULL, const char* rolename = NULL) const;


    // ��������: ��ȡǩ��;
    // �������:
    //           username �û���;
    //           sign_len ǩ��buffer��С
    // �������:
    //           sign     ǩ����buffer;
    //           sign_len ����ǩ������
    // ����ֵ:   �ɹ�true,ʧ��false.
    bool GetSign(const char* identity, char* sign, uint32_t* sign_len) const;

    // ��������: Encodeǩ��,���޷����ַ�תΪ16���Ʊ���;
    // �������:
    //           hex      Encodeǰ���޷����ַ���;
    //           hex_len  Encodeǰ���޷����ַ�������
    // �������:
    //           buff     Encodeǩ�����buffer;
    //           buff_len Encodeǩ����ĳ���
    // ����ֵ:   �ɹ�true,ʧ��false.
    bool Hex2String(const unsigned char* hex, int hex_len, char* buff, int *buff_len) const;

    // ��������: ����private key file, ��ȡǩ��;
    // �������:
    //           identity      ʵ���û���
    //           filename      key�ļ���;
    // �������:
    //           sign          ǩ��buffer;
    //           sign_len      ǩ������
    // ����ֵ:   �ɹ�true,ʧ��false.
    // key file format :
    // | version | len_privatekey | privatekey | len_username | username | len_sign | sign |
    // |----2B---|-------4B-------|----len-----|-----4B-------|---len----|----4B----|--len-|
    bool ParseKeyFile(const char* identity, const char* filename,
                      char* sign, uint32_t* sign_len,
                      CA_ERROR_CODE* err = NULL) const;


    // ��������: ����CA��Ӧ��;
    // ����ֵ:   �ɹ�true,ʧ��false.
    bool ParseResponse(const char* http_response) const;

    // ��������: ��ȡĬ��username(Ĭ��Ϊ��ʱ,����gflags��env����û�����Ϣ);
    // �������:
    //           real_identity      ʵ���û���buffer;
    //           identity_len       ʵ���û���buffer����;
    // ����ֵ:   �ɹ�true,ʧ��false.
    bool GetRealIdentity(char* real_identity, uint32_t identity_len) const;

    // ��������: ��ȡĬ��username(Ĭ��Ϊ��ʱ,����gflags��env����û�����Ϣ);
    // �������:
    //           real_identity      ʵ���û���;
    // �������:
    //           real_role          ʵ��role��buffer
    //           identity_len       ʵ��role��buffer����;
    // ����ֵ:   �ɹ�true,ʧ��false.
    bool GetRealRole(char* real_identity, char* real_role, uint32_t role_len) const;


    // ��������: ���name�еķǷ��ַ�;
    // ����ֵ:   �ɹ�true,ʧ��false.
    bool IsValidName(const char* name) const;

    std::string                   m_ca_ip;

    bool          m_certified;
    std::string   m_identity;
    std::string   m_role;
};
} // namespace ca

#endif //COMMON_CA_CA_CLIENT_CERTIFICATION_H_

