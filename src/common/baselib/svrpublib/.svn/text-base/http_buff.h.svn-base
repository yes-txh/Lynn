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

    // 清空内容
    void ResetHttpContent();

    //
    // 功能描述： 设置页面输出格式（根据cgi的请求设定）
    // 输入参数： content_type  ENUM_HTTP_HTML 或 ENUM_HTTP_XML
    //
    void SetContentType(int32_t content_type);

    //
    // 功能描述： 返回当前的页面输出格式
    //
    bool IsXML() const { return m_is_output_xml; }

    //
    // 功能描述： 是否设置有jasoncallback 回调函数
    //
    bool HaveJasonCallback() const {return m_is_output_xml && (m_jasoncallback.GetValidLen() > 0);}

    //
    // 功能描述： 返回是否raw data标志
    //
    bool IsRawData() const {return m_is_raw_data; }

    //
    // 功能描述： 设置页面属性
    // 输入参数： title     页面标题
    // 输入参数： module    所属大模块(默认取可执行文件名)
    // 输入参数： is_home_page    该页面是否是模块首页
    //
    void SetAttr(const char* title, const char* module = NULL,
                 bool is_home_page = false);

    //
    // 功能描述： 设置显示欢迎用户和注销的标志
    // 输入参数： user_name     登录用户
    //            login_page   用户注销后再登陆时跳转到的页面，如果不设置，则跳转到注销前的当前页面
    //
    void SetBanner(const char* user_name, const char* login_page = NULL);

    //
    // 功能描述： 开始/结束一个族(必须配对)
    // 输入参数： name
    //
    void BeginGroup(const char* key);
    void EndGroup(const char* key);
    // 可以附加一个属性值
    void BeginGroup(const char* key, uint32_t property_val);
    

    //
    // 功能描述： 设置页面属性()
    // 输入参数： key        key可以是用@分割的多级key
    // 输入参数： value
    // 输入参数： desc        描述, 属性
    //
    void AddKey(const char* key, const char* value, const char* desc = "", bool new_line = true,
                // 当用户是使用href作为key的时候,
                // 输出的xml中key无意义,所以这时候用户可以指定一个key作为xml的key
                const char* xml_key = NULL);
    void AddKey(const char* key, const uint16_t value, const char* desc = "");
    void AddKey(const char* key, const uint32_t value, const char* desc = "");
    void AddKey(const char* key, const uint64_t value, const char* desc = "");
    void AddKey(const char* key, const float value, const char* desc = "");
    // 增加一条线
    void AddLineKey(bool new_line = true);

    //
    // key-value-desc 形式 
    //
    // title,href组成key
    // value
    // desc 注释字段
    //
    // 将内部的href连接作为key加入
    // 需要计算出真正的url后才作为key加入
    //
    // use title as value
    // use href as key
    //
    void AddInternalHrefKey(const char* title, const char* href,
                            const char* value, const char* desc);


    //
    // 功能描述： 增加超链接(BodyB, 超链接部分)
    // 输入参数： title    超链接的标题
    // 输入参数： href        超链指向的URL
    //            new_line, 是否换行
    //
    void AddHref(const char* title, const char* href, const char* desc = "", bool is_xml = false,
                 bool new_line = true);

    bool MakeRealURL(const char* in_url,uint32_t in_url_len,
                     char* out_url, uint32_t out_url_len,
                     bool is_xml = false, bool is_homepage = false);

    //
    // 设置raw data,可以通过GetRawData 获取到raw data
    // set raw data,设置m_is_raw_data = true, GetRawData设置m_is_raw_data = false
    // 这两个函数要成对使用
    //
    void SetRawData(const char* raw_data, uint32_t len);
    bool GetRawData(const char** out_content, uint32_t* out_content_len);
    void SetUseRawData(){ m_is_raw_data = true;}

    // 获取各个部分的内容
    bool GetHead(const char** out_content, uint32_t* out_content_len);
    bool GetBody(const char** out_content, uint32_t* out_content_len);
    bool GetTail(const char** out_content, uint32_t* out_content_len);

    // 设置代理服务器
    void SetProxy(const char* proxy_url);

    // 设置主页地址
    void SetHome(const char* home_url);

    // 设置xsl文件
    void SetXslFile(const char* xsl_filename);

    //
    void SetHostPort(const char* host_port);

    void FormatXmlString(const char* in_buff, uint32_t in_buff_len,
                         CStrBuff& out_strbuff);

    void FormatXmlTag(const char* in_buff,
                      uint32_t    in_buff_len,
                      CStrBuff&   out_strbuff);

    CStrBuff* GetRawDataObj(){ return &m_http_raw_data;}

    // &xml=true&jasoncallback 才有效
    void SetJasonCallback(const char* jasoncallback);

private:
    void FormatToJavascriptString(CStrBuff& buff);

    //
    // 是否是主页
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

    // 支持raw data,用于代理或其他数据
    // raw data的时候其他数据项都无效
    CStrBuff m_http_raw_data;
    bool m_is_raw_data;

    CStrBuff m_strbuff_html;
    CStrBuff m_strbuff_xml;

    // &xml=true&jasoncallback=xxx
    CStrBuff m_jasoncallback;

    //
    // 解析CGI参数
    //
    CParserCGIParameter m_cgi_parser;

    // BeginGroup called
    bool m_is_group_model;
};

void SetModuleName(const char* module);

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_HTTP_BUFF_H_
