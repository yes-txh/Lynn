// parser_cgi_parameter.h: interface for the CParserCGIParameter class.
// wookin@tencent.com
// 2006/05/30
// ////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_PARSER_CGI_PARAMETER_H_
#define COMMON_BASELIB_SVRPUBLIB_PARSER_CGI_PARAMETER_H_

#include "common/baselib/svrpublib/base_config.h"

//
// DEBUG_CGI
//
// ��ʾdebug��Ϣ

_START_XFS_BASE_NAMESPACE_

class CParserKeyVal {
public:
    struct KeyValItem {
        uint32_t        key_start_pos;
        uint32_t        key_end_pos;
        uint32_t        val_start_pos;
        uint32_t        val_end_pos;
        unsigned char   valid_items;
    };

    enum PARSER_STATE {
        PARSER_TryGetKeyStart = 0,
        PARSER_TryGetKeyEnd = 1,
        PARSER_TryGetValueStart = 2,
        PARSER_TryGetValueEnd = 3,
    };
public:
    CParserKeyVal();
    virtual ~CParserKeyVal();
    void    SetOutputDebugFlag(bool debug_output);
    int32_t ParserString(const char* from_buff,  //  0:from environment [POST]
                         //  !0:from string     [GET]
                         uint32_t len,
                         unsigned char decode_url,
                         unsigned char div,
                         unsigned char equal);

    int32_t ParserString(const char* from_buff, uint32_t len, bool auto_decode = false);

    const char* GetValue(const char* key, bool auto_decode = false) ;
    //
    // return
    // 1:ok
    // 0:fail
    //
    int32_t  GetValue(const char* key, const char** val, bool auto_decode = false);
    int32_t  GetValue(const char* key, int32_t* val, bool auto_decode = false);
    int32_t  GetValue(const char* key, uint32_t* val, bool auto_decode = false);
    int32_t  GetValue(const char* key, uint16_t* val, bool auto_decode = false);

    void     DebugKeyValsToScreen() ;
    uint32_t GetValidItemCount() ;
    const char* GetKeyName(uint32_t key_index);
    const char* GetValString(uint32_t key_index);

    void ResetContent() {
        m_valid_items_count = 0;
    }
private:
    KeyValItem*     m_key_items;
    uint32_t        m_max_items;
    uint32_t        m_valid_items_count;

    unsigned char*  m_key_val_buff;
    uint32_t        m_key_val_buff_len;

    //
    // 1:ok
    // 0:fail
    //
    int32_t     PrepareBuffer(uint32_t new_length);
    int32_t     ReAllocParameterItemsResource();
    void        ReleaseResource();

    int32_t     GetNextChar(const char* sz, uint32_t pos);
    int32_t     H2I(char *sz);
    void        Reset();
    void        AdjustKeyName();
    void        UnFormatString(char* sz);

    bool        m_output_debug_flag;

    // ������ʱ�Զ���ѹ���key
    std::string m_temp_decoded_val;
};


class CParserCGIParameter {
public:
    typedef enum {
        REQ_UNKNOWN = 0,
        REQ_POST = 1,
        REQ_GET = 2,
        REQ_AUTO = 3, //  Try POST or GET
        REQ_HEAD,
        REQ_DELETE,
        REQ_PUT,
        REQ_TRACE
    } REQUEST_Type;

public:
    CParserCGIParameter();
    virtual ~CParserCGIParameter();

    //
    //  1:ok
    //  0:fail
    //
    // bool auto_decode = true : ��⵽�б��룬���Զ�����
    int32_t     ParserEnvironmentString(REQUEST_Type type,
                                        bool* is_post = NULL,
                                        bool auto_decode = true);
    int32_t     ParserCookie();
    int32_t     AttachCookie(const char * cookie_string);
    int32_t     AttachEnvironmentString(const char* environment_sz, bool auto_decode = true);

    //
    //  Get environment key-value
    //
    // auto_decode : true ��⵽�б��룬���Զ�����; false ���Զ�����
    int32_t     GetParameter(const char* paramter, const char** val, bool auto_decode = false);
    int32_t     GetParameter(const char* paramter, uint32_t* val, bool auto_decode = false);
    int32_t     GetParameter(const char* paramter, int32_t* val, bool auto_decode = false);
    uint32_t    GetParameterCount() ;
    const char* GetParameterName(uint32_t key_index) ;
    const char* GetParameterVal(uint32_t key_index);
    void ResetContent() {
        m_env_keyval_parser.ResetContent();
        m_cookie_parser.ResetContent();
    }

    //
    //  Get cookie key-value
    //
    uint32_t    GetCookieParameterCount();
    int32_t     GetCookieParameter(const char* parameter, const char** val,
                                   bool auto_decode = false);
    int32_t     GetCookieParameter(const char* parameter, uint32_t* val,
                                   bool auto_decode = false);
    int32_t     GetCookieParameter(const char* parameter, int32_t* val,
                                   bool auto_decode = false);

    void        DebugPrintEnvironmentToScreen() const;

    //
    //  CONTENT_TYPE        �������͸�ʽ
    //  DOCUMENT_ROOT       WWW Server���ļ�Ŀ¼
    //  GATEWAY_INTERFACE   ������ʹ��CGI�İ汾
    //  HTTP_ACCEPT         HTTP�����ܵ�MIME��ʽ
    //  HTTP_CONNECTION     ����˷����������߹���
    //  HTTP_HOST           �˷������ĵ�ַ
    //  HTTP_REFERER        ָ���CGI�����������ļ���URL
    //  HTTP_USER_AGENT     �ͻ��������������(�汾 ����ϵͳ��)
    //  PATH                ϵͳ���趨��·��
    //  REMOTE_ADDR         Զ�˻�����IP ADDRESS
    //  REMOTE_HOST         Զ�˻�����HostName
    //  REMOTE_METHOD       �ͻ�����������������Ҫ��
    //  SCRIPT_FILENAME     ��CGI����ľ���Ŀ¼�����ļ���
    //  SCRIPT_NAME         ��CGI����������ļ���
    //  SERVER_ADMIN        �˷����������ߵ�E-mail��ַ
    //  SERVER_NAME         �����������ϵ�����
    //  SERVER_PORT         �˷�������ʹ�õ�ͨѶ��
    //  SERVER_PROTOCOL     �˷�������ʹ�õĴ���Э��
    //  SERVER_SOFTWARE     ��������ʹ��WWW Server���������
    //
    const char*     UtilGetEnv(const char* env_name) const;

    // 
    // url="http://www.xfs.soso.com/proxy.cgi?a=b&c=d"
    // return a=b&c=d
    std::string     GetParmeterContent(const std::string& url);

private:
    //
    //  1:ok
    //  0:fail
    //
    int32_t         VerifyRequestMethod(const char* type);

    CParserKeyVal   m_cookie_parser;
    CParserKeyVal   m_env_keyval_parser;
};

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_PARSER_CGI_PARAMETER_H_
