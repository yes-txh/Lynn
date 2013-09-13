//////////////////////////////////////////////////////////////////////////
// @file:   my_ca_simle_http
// @brief:  ������ʾcaҳ��
// @author: joeytian@tencent.com
// @time:   2010-1-4
// �޸���ʷ:
//          <author>    <time>
//////////////////////////////////////////////////////////////////////////
#ifndef COMMON_CRYPTO_CA_CA_SERVER_MY_CA_SIMPLE_HTTP_H_
#define COMMON_CRYPTO_CA_CA_SERVER_MY_CA_SIMPLE_HTTP_H_

#include "common/crypto/ca/ca_server/ca.h"
#include "common/crypto/ca/ca_server/role_manager.h"
#include "common/crypto/ca/ca_server/quota_manager.h"
#include "common/crypto/ca/ca_server/authenticate_record_manager.h"

namespace ca {

class MySimpleHttpThread: public CBaseHttpProcThread {
public:
    enum InputType{
        NEED_USER_NAME   = 0x00000001,
        NEED_ROLE_NAME   = 0x00000010,
        NEED_QUOTA       = 0x00000100,
        NEED_CLUSTER     = 0x00001000
    };

    MySimpleHttpThread();
    virtual ~MySimpleHttpThread() {}

    // �û����Լ̳��������ʵ�־�����Ӧ
    virtual bool OnUserHTTPRequest(const BufferV* received_buff,
                                   CHttpBuff* http_response);


private:
    // ��ʾCAҳ���û�֤��ע��״��
    void DisplayUserInfo(CACertificate* ca_certificate, const char* user_name,
                         CA_ERROR_CODE certificate_state, const char* ca_token_encrypt,
                         uint32_t len_encrypt, CHttpBuff* http_response);
    bool String2Hex(const char* str, char* hex, int len);

    // url��ַ���Ҳ����û���token��Ϣʱ����ת��OA��½
    void TurnToOA(CHttpBuff* http_response);

    // CA����ҳ��
    void Index(const BufferV* received_buff, CHttpBuff* http_response);

    // ��ʾ�û�֤��ע��״��
    void Welcome(const BufferV* received_buff, CHttpBuff* http_response);

    // ע��֤��
    void Register(const BufferV* received_buff, CHttpBuff* http_response);

    // ���µ��ڵ��û�֤��
    void Renew(const BufferV*  received_buff, CHttpBuff* http_response);

    // ɾ���û�֤��
    void Revoke(const BufferV* received_buff, CHttpBuff* http_response);

    // �����û�֤��
    void Download(const BufferV* received_buff, CHttpBuff* http_response);

    // �����û��Ϸ���
    void CheckIdentity(const BufferV* received_buff, CHttpBuff* http_response);

    // ����ticket��Ϣ��OA������user_name
    bool ParseOAName(const char* user_ticket, char* user_name);

    // У��url����ļ��ܴ�(�û���+token+��Ч��)
    bool CheckCaToken(const char* user_name, const char* ca_token_encrypt, uint32_t len_encrypt);

    // ��ʾbanner��Ϣ
    void SetBanner(const char* user_name, CHttpBuff* http_response);

    // �򵥵ļ���
    bool SimpleEncrypt(unsigned char* plaintext, uint32_t len_plaintext,
                       unsigned char* str_encrypted, uint32_t len_str_encrypted);
    // ����
    bool SimpleDecrypt(const char* str_encrypted, uint32_t len_str_encrypted,
                       unsigned char* plaintext, uint32_t len_plaintext);

    // ���Identity
    void AddIdentity(const BufferV* received_buff, CHttpBuff* http_response);

    // ɾ��Identity
    void DelIdentity(const BufferV* received_buff, CHttpBuff* http_response);

    // ��ѯIdentity������Щrole
    void QueryIdentityRoles(const BufferV* received_buff, CHttpBuff* http_response);

    // ���role
    void AddRole(const BufferV* received_buff, CHttpBuff* http_response);

    // ɾ��role
    void DelRole(const BufferV* received_buff, CHttpBuff* http_response);

    // ��ѯrole�°�����ЩIdentity
    void QueryRoleIdentities(const BufferV* received_buff, CHttpBuff* http_response);

    // ���ָ��Identity��ָ��role
    void AddIdentityToRole(const BufferV* received_buff, CHttpBuff* http_response);

    // ��ָ��roleɾ��ָ��Identity
    void DelIdentityFromRole(const BufferV* received_buff, CHttpBuff* http_response);

    // ����role_managerҳ���url����
    void SetRoleBottomLink(const char* user_name, const char* ca_token_encrypt,
                       const uint32_t len_encrypt, CHttpBuff* http_response);

    // ��ʾ��ɾIdentity��role�ȵ�post�����
    void PrintReqPage(const char* page_name, uint32_t req_type,
                      const char* user_name, const char* ca_token_encrypt,
                      uint32_t len_encrypt, CHttpBuff* http_response);

    // ��ʾ���е�Identity
    void PrintAllIdentities(const BufferV* received_buff, CHttpBuff* http_response);

    // ��ʾ����role
    void PrintAllRoles(const BufferV* received_buff, CHttpBuff* http_response);

    // ��ʾpost��������������Խ���role_managerҳ��
    void PrintRoleManagerPage(const char* user_name, const char* ca_token_encrypt,
                              uint32_t len_encrypt, CHttpBuff* http_response);

    // ������ת��role_managerҳ�������
    void DealRoleManagerReq(const BufferV* received_buff, CHttpBuff* http_response);

    // ����role��quotaֵ
    void SetRoleQuota(const BufferV* received_buff, CHttpBuff* http_response);

    // ��ѯrole��quotaֵ
    void QueryRoleQuota(const BufferV* received_buff, CHttpBuff* http_response);

    // �г�����role��quotaֵ
    void ListAllRolesQuota(const BufferV* received_buff, CHttpBuff* http_response);

    // ����quota_managerҳ���url����
    void SetQuotaBottomLink(const char* user_name, const char* ca_token_encrypt,
        const uint32_t len_encrypt, CHttpBuff* http_response);

    // ��ʾpost��������������Խ���quota_managerҳ��
    void PrintQuotaManagerPage(const char* user_name, const char* ca_token_encrypt,
        uint32_t len_encrypt, CHttpBuff* http_response);

    // ����QuotaManager��ҳ
    void DealQuotaManagerReq(const BufferV* received_buff, CHttpBuff* http_response);


    // ��ʾpost��������������Խ���authenticate_recordҳ��
    void AccessAuthenticateRecordPage(const char* user_name, const char* ca_token_encrypt,
                                      uint32_t len_encrypt, CHttpBuff* http_response);

    // ������ת��authenticate_recordҳ�������
    void DealAuthenticateRecordReq(const BufferV* received_buff, CHttpBuff* http_response);

    // ��ʾ��������ѡ���
    // show_detail: �Ƿ���ʾ�ұߵ���ϸ��¼��ѯ��������, =false����ʾ��=true��ʾ
    // ����Ҫ��ʾ�ұߵ�����ѡ���ʱ����Ҫ����Ŀǰ�������ʾ�����ڵ�ֵ����Ȼ�ұ���������ť��תˢ�º�
    // ����������ȡ��������ֵ����Ĭ����ʾ���������ͼ���û������Բ���
    void PrintDaySlect(bool show_detail, const char* user_name, const char* ca_token_encrypt,
                       uint32_t len_encrypt, const char* pwd, int32_t year_chart,
                       int32_t month_chart, int32_t day_chart, CHttpBuff* http_response);

    // ��ȡ���ݲ���ʽ��
    void PrepareChartData(time_t req_chart_epoch_time, time_t req_text_epoch_time,
                          int32_t req_day_num, int32_t req_page_num, int32_t* total_page_num,
                          std::string& xml_data, std::string& auth_record_file);

    // �Ӵ������¶�ȡ�ļ�����
    void Reload(const BufferV* received_buff, CHttpBuff* http_response);

    // ����иĶ������ϴ�CheckPoint������5����,��д�µ�CheckPoint�ļ�
    void WriteCheckPoint();

    // �����ļ�
    bool CopyFile(std::string& filename, std::string& new_filename);

private:
    CA m_ca;    
    CStrBuff m_obj_buff;
    CParserCGIParameter m_cgi_parser;
    CGetHttpResponse m_get_http_response;
    char m_module_name[kModuleNameLen];
};

} // namespace ca
#endif // COMMON_CRYPTO_CA_CA_SERVER_MY_CA_SIMPLE_HTTP_H_
