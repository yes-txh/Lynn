// http_buff.h

#ifndef COMMON_BASELIB_SVRPUBLIB_HTTP_BUFF_H_
#define COMMON_BASELIB_SVRPUBLIB_HTTP_BUFF_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

#ifndef STRNCASECMP

#ifdef WIN32
#define STRNCASECMP _strnicmp
#else
#define STRNCASECMP strncasecmp
#endif
#endif

enum ENUM_HTTP_TYPE {
    ENUM_HTTP_HTML = 0x01,
    ENUM_HTTP_XML  = 0x02,
};


class CHttpBuff {
#define MAX_FOOTER_BUFF_LENGTH 10240
public:
    CHttpBuff();
    ~CHttpBuff();

    // �������
    void ResetHttpContent();

    //
    // ���������� ����ҳ�������ʽ������cgi�������趨��
    // ��������� content_type  ENUM_HTTP_HTML �� ENUM_HTTP_XML
    //
    void SetContentType(int32_t content_type);

    //
    // ���������� ���ص�ǰ��ҳ�������ʽ
    //
    bool IsXML() const { return m_is_output_xml; }

    //
    // ���������� �Ƿ�������jasoncallback �ص�����
    //
    bool HaveJasonCallback() const {return m_is_output_xml && (m_jasoncallback.GetValidLen() > 0);}

    //
    // ���������� �����Ƿ�raw data��־
    //
    bool IsRawData() const {return m_is_raw_data; }

    //
    // ���������� ����ҳ������
    // ��������� title     ҳ�����
    // ��������� module    ������ģ��(Ĭ��ȡ��ִ���ļ���)
    // ��������� is_home_page    ��ҳ���Ƿ���ģ����ҳ
    //
    void SetAttr(const char* title, const char* module = NULL,
                 bool is_home_page = false);

    //
    // ���������� ������ʾ��ӭ�û���ע���ı�־
    // ��������� user_name     ��¼�û�
    //            login_page   �û�ע�����ٵ�½ʱ��ת����ҳ�棬��������ã�����ת��ע��ǰ�ĵ�ǰҳ��
    //
    void SetBanner(const char* user_name, const char* login_page = NULL);

    //
    // ���������� ��ʼ/����һ����(�������)
    // ��������� name
    //
    void BeginGroup(const char* key);
    void EndGroup(const char* key);
    // ���Ը���һ������ֵ
    void BeginGroup(const char* key, uint32_t property_val);
    

    //
    // ���������� ����ҳ������()
    // ��������� key        key��������@�ָ�Ķ༶key
    // ��������� value
    // ��������� desc        ����, ����
    //
    void AddKey(const char* key, const char* value, const char* desc = "", bool new_line = true,
                // ���û���ʹ��href��Ϊkey��ʱ��,
                // �����xml��key������,������ʱ���û�����ָ��һ��key��Ϊxml��key
                const char* xml_key = NULL);
    void AddKey(const char* key, const uint16_t value, const char* desc = "");
    void AddKey(const char* key, const uint32_t value, const char* desc = "");
    void AddKey(const char* key, const uint64_t value, const char* desc = "");
    void AddKey(const char* key, const float value, const char* desc = "");
    // ����һ����
    void AddLineKey(bool new_line = true);

    //
    // key-value-desc ��ʽ 
    //
    // title,href���key
    // value
    // desc ע���ֶ�
    //
    // ���ڲ���href������Ϊkey����
    // ��Ҫ�����������url�����Ϊkey����
    //
    // use title as value
    // use href as key
    //
    void AddInternalHrefKey(const char* title, const char* href,
                            const char* value, const char* desc);


    //
    // ���������� ���ӳ�����(BodyB, �����Ӳ���)
    // ��������� title    �����ӵı���
    // ��������� href        ����ָ���URL
    //            new_line, �Ƿ���
    //
    void AddHref(const char* title, const char* href, const char* desc = "", bool is_xml = false,
                 bool new_line = true);

    bool MakeRealURL(const char* in_url,uint32_t in_url_len,
                     char* out_url, uint32_t out_url_len,
                     bool is_xml = false, bool is_homepage = false);

    //
    // ����raw data,����ͨ��GetRawData ��ȡ��raw data
    // set raw data,����m_is_raw_data = true, GetRawData����m_is_raw_data = false
    // ����������Ҫ�ɶ�ʹ��
    //
    void SetRawData(const char* raw_data, uint32_t len);
    bool GetRawData(const char** out_content, uint32_t* out_content_len);
    void SetUseRawData(){ m_is_raw_data = true;}

    // ��ȡ�������ֵ�����
    bool GetHead(const char** out_content, uint32_t* out_content_len);
    bool GetBody(const char** out_content, uint32_t* out_content_len);
    bool GetTail(const char** out_content, uint32_t* out_content_len);

    // ���ô��������
    void SetProxy(const char* proxy_url);

    // ������ҳ��ַ
    void SetHome(const char* home_url);

    // ����xsl�ļ�
    void SetXslFile(const char* xsl_filename);

    //
    void SetHostPort(const char* host_port);

    void FormatXmlString(const char* in_buff, uint32_t in_buff_len,
                         CStrBuff& out_strbuff);

    void FormatXmlTag(const char* in_buff,
                      uint32_t    in_buff_len,
                      CStrBuff&   out_strbuff);

    CStrBuff* GetRawDataObj(){ return &m_http_raw_data;}

    // &xml=true&jasoncallback ����Ч
    void SetJasonCallback(const char* jasoncallback);

private:
    void FormatToJavascriptString(CStrBuff& buff);

    //
    // �Ƿ�����ҳ
    //
    bool     m_is_homepage;
    bool     m_is_output_xml;
    bool     m_is_set_login_name;
    bool     m_is_set_login_page;

    char     m_title[256];
    char     m_module[256];
    char     m_login_name[32];
    char     m_login_page[1024];

    uint32_t m_div_idx;

    char     m_url_proxy[1024];
    char     m_url_homepage[256];
    char     m_host_port[32];
    char     m_xsl_filename[256];
    char*    m_footer_buffer;

    CStrBuff m_http_head;
    CStrBuff m_http_tail;
    CStrBuff m_http_body;

    // ֧��raw data,���ڴ������������
    // raw data��ʱ�������������Ч
    CStrBuff m_http_raw_data;
    bool m_is_raw_data;

    CStrBuff m_strbuff_html;
    CStrBuff m_strbuff_xml;

    // &xml=true&jasoncallback=xxx
    CStrBuff m_jasoncallback;

    //
    // ����CGI����
    //
    CParserCGIParameter m_cgi_parser;

    // BeginGroup called
    bool m_is_group_model;
};

void SetModuleName(const char* module);

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_HTTP_BUFF_H_
