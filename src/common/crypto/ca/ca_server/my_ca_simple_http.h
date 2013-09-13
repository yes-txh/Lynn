//////////////////////////////////////////////////////////////////////////
// @file:   my_ca_simle_http
// @brief:  处理并显示ca页面
// @author: joeytian@tencent.com
// @time:   2010-1-4
// 修改历史:
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

    // 用户可以继承这个函数实现具体响应
    virtual bool OnUserHTTPRequest(const BufferV* received_buff,
                                   CHttpBuff* http_response);


private:
    // 显示CA页面用户证书注册状况
    void DisplayUserInfo(CACertificate* ca_certificate, const char* user_name,
                         CA_ERROR_CODE certificate_state, const char* ca_token_encrypt,
                         uint32_t len_encrypt, CHttpBuff* http_response);
    bool String2Hex(const char* str, char* hex, int len);

    // url地址里找不到用户的token信息时，跳转到OA登陆
    void TurnToOA(CHttpBuff* http_response);

    // CA的主页面
    void Index(const BufferV* received_buff, CHttpBuff* http_response);

    // 显示用户证书注册状况
    void Welcome(const BufferV* received_buff, CHttpBuff* http_response);

    // 注册证书
    void Register(const BufferV* received_buff, CHttpBuff* http_response);

    // 更新到期的用户证书
    void Renew(const BufferV*  received_buff, CHttpBuff* http_response);

    // 删除用户证书
    void Revoke(const BufferV* received_buff, CHttpBuff* http_response);

    // 下载用户证书
    void Download(const BufferV* received_buff, CHttpBuff* http_response);

    // 检验用户合法性
    void CheckIdentity(const BufferV* received_buff, CHttpBuff* http_response);

    // 发送ticket信息到OA，解析user_name
    bool ParseOAName(const char* user_ticket, char* user_name);

    // 校验url里面的加密串(用户名+token+有效期)
    bool CheckCaToken(const char* user_name, const char* ca_token_encrypt, uint32_t len_encrypt);

    // 显示banner信息
    void SetBanner(const char* user_name, CHttpBuff* http_response);

    // 简单的加密
    bool SimpleEncrypt(unsigned char* plaintext, uint32_t len_plaintext,
                       unsigned char* str_encrypted, uint32_t len_str_encrypted);
    // 解密
    bool SimpleDecrypt(const char* str_encrypted, uint32_t len_str_encrypted,
                       unsigned char* plaintext, uint32_t len_plaintext);

    // 添加Identity
    void AddIdentity(const BufferV* received_buff, CHttpBuff* http_response);

    // 删除Identity
    void DelIdentity(const BufferV* received_buff, CHttpBuff* http_response);

    // 查询Identity属于哪些role
    void QueryIdentityRoles(const BufferV* received_buff, CHttpBuff* http_response);

    // 添加role
    void AddRole(const BufferV* received_buff, CHttpBuff* http_response);

    // 删除role
    void DelRole(const BufferV* received_buff, CHttpBuff* http_response);

    // 查询role下包含哪些Identity
    void QueryRoleIdentities(const BufferV* received_buff, CHttpBuff* http_response);

    // 添加指定Identity到指定role
    void AddIdentityToRole(const BufferV* received_buff, CHttpBuff* http_response);

    // 从指定role删除指定Identity
    void DelIdentityFromRole(const BufferV* received_buff, CHttpBuff* http_response);

    // 设置role_manager页面的url链接
    void SetRoleBottomLink(const char* user_name, const char* ca_token_encrypt,
                       const uint32_t len_encrypt, CHttpBuff* http_response);

    // 显示增删Identity和role等的post输入框
    void PrintReqPage(const char* page_name, uint32_t req_type,
                      const char* user_name, const char* ca_token_encrypt,
                      uint32_t len_encrypt, CHttpBuff* http_response);

    // 显示所有的Identity
    void PrintAllIdentities(const BufferV* received_buff, CHttpBuff* http_response);

    // 显示所有role
    void PrintAllRoles(const BufferV* received_buff, CHttpBuff* http_response);

    // 显示post输入框，输入密码以进入role_manager页面
    void PrintRoleManagerPage(const char* user_name, const char* ca_token_encrypt,
                              uint32_t len_encrypt, CHttpBuff* http_response);

    // 处理跳转到role_manager页面的请求
    void DealRoleManagerReq(const BufferV* received_buff, CHttpBuff* http_response);

    // 设置role的quota值
    void SetRoleQuota(const BufferV* received_buff, CHttpBuff* http_response);

    // 查询role的quota值
    void QueryRoleQuota(const BufferV* received_buff, CHttpBuff* http_response);

    // 列出所有role的quota值
    void ListAllRolesQuota(const BufferV* received_buff, CHttpBuff* http_response);

    // 设置quota_manager页面的url链接
    void SetQuotaBottomLink(const char* user_name, const char* ca_token_encrypt,
        const uint32_t len_encrypt, CHttpBuff* http_response);

    // 显示post输入框，输入密码以进入quota_manager页面
    void PrintQuotaManagerPage(const char* user_name, const char* ca_token_encrypt,
        uint32_t len_encrypt, CHttpBuff* http_response);

    // 设置QuotaManager首页
    void DealQuotaManagerReq(const BufferV* received_buff, CHttpBuff* http_response);


    // 显示post输入框，输入密码以进入authenticate_record页面
    void AccessAuthenticateRecordPage(const char* user_name, const char* ca_token_encrypt,
                                      uint32_t len_encrypt, CHttpBuff* http_response);

    // 处理跳转到authenticate_record页面的请求
    void DealAuthenticateRecordReq(const BufferV* received_buff, CHttpBuff* http_response);

    // 显示日期下拉选择框
    // show_detail: 是否显示右边的详细记录查询的下拉框, =false不显示，=true显示
    // 当需要显示右边的日期选择框时，需要传入目前左边已显示的日期的值，不然右边区域点击按钮跳转刷新后
    // 左右区域因取不到日期值，会默认显示当天的曲线图，用户体验性不好
    void PrintDaySlect(bool show_detail, const char* user_name, const char* ca_token_encrypt,
                       uint32_t len_encrypt, const char* pwd, int32_t year_chart,
                       int32_t month_chart, int32_t day_chart, CHttpBuff* http_response);

    // 读取数据并格式化
    void PrepareChartData(time_t req_chart_epoch_time, time_t req_text_epoch_time,
                          int32_t req_day_num, int32_t req_page_num, int32_t* total_page_num,
                          std::string& xml_data, std::string& auth_record_file);

    // 从磁盘重新读取文件数据
    void Reload(const BufferV* received_buff, CHttpBuff* http_response);

    // 如果有改动且离上次CheckPoint超过了5分钟,则写新的CheckPoint文件
    void WriteCheckPoint();

    // 复制文件
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
