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
// 显示debug信息

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

    // 保存临时自动解压后的key
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
    // bool auto_decode = true : 检测到有编码，则自动解码
    int32_t     ParserEnvironmentString(REQUEST_Type type,
                                        bool* is_post = NULL,
                                        bool auto_decode = true);
    int32_t     ParserCookie();
    int32_t     AttachCookie(const char * cookie_string);
    int32_t     AttachEnvironmentString(const char* environment_sz, bool auto_decode = true);

    //
    //  Get environment key-value
    //
    // auto_decode : true 检测到有编码，则自动解码; false 不自动解码
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
    //  CONTENT_TYPE        数据类型格式
    //  DOCUMENT_ROOT       WWW Server的文件目录
    //  GATEWAY_INTERFACE   服务器使用CGI的版本
    //  HTTP_ACCEPT         HTTP所接受的MIME格式
    //  HTTP_CONNECTION     代表此服务器的连线功能
    //  HTTP_HOST           此服务器的地址
    //  HTTP_REFERER        指向该CGI程序所代表文件的URL
    //  HTTP_USER_AGENT     客户端浏览器的数据(版本 操作系统等)
    //  PATH                系统所设定的路径
    //  REMOTE_ADDR         远端机器的IP ADDRESS
    //  REMOTE_HOST         远端机器的HostName
    //  REMOTE_METHOD       客户端向服务器端提出的要求
    //  SCRIPT_FILENAME     此CGI程序的绝对目录名称文件名
    //  SCRIPT_NAME         此CGI程序的名称文件名
    //  SERVER_ADMIN        此服务器管理者的E-mail地址
    //  SERVER_NAME         主机在网络上的名称
    //  SERVER_PORT         此服务器所使用的通讯口
    //  SERVER_PROTOCOL     此服务器所使用的传输协议
    //  SERVER_SOFTWARE     该主机所使用WWW Server软件的名称
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
