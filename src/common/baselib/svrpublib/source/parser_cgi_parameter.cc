//
// parser_cgi_parameter.cpp: implementation of the CParserCGIParameter class.
// wookin@tencent.com
//
// ////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/base_config.h"
#include "common/baselib/svrpublib/url_codec.h"

_START_XFS_BASE_NAMESPACE_

// /////////////////////////
// Construction/Destruction
// /////////////////////////

CParserCGIParameter::CParserCGIParameter() {
}

CParserCGIParameter::~CParserCGIParameter() {
}

//
// 1:ok
// 0:fail
//
int32_t    CParserCGIParameter::AttachEnvironmentString(const char* env_string, bool auto_decode) {
    int32_t ok = 0;
    if (env_string) {
        uint32_t len = STRLEN(env_string);
        unsigned char decode_url = auto_decode ? 1:0;
        ok = m_env_keyval_parser.ParserString(env_string, len, decode_url, '&', '=');
    }
    return ok;
}

uint32_t    CParserCGIParameter::GetParameterCount() {
    return m_env_keyval_parser.GetValidItemCount();
}

const char* CParserCGIParameter::GetParameterName(uint32_t key_index) {
    return m_env_keyval_parser.GetKeyName(key_index);
}
const char* CParserCGIParameter::GetParameterVal(uint32_t key_index) {
    return m_env_keyval_parser.GetValString(key_index);
}

//
// 1:ok
// 0:fail
//
int32_t CParserCGIParameter::GetParameter(const char* parameter,
                                          const char** val, bool auto_decode) {
    return m_env_keyval_parser.GetValue(parameter, val, auto_decode);
}

int32_t CParserCGIParameter::GetParameter(const char* parameter, uint32_t* val,
                                         bool auto_decode) {
    return m_env_keyval_parser.GetValue(parameter, val, auto_decode);
}

int32_t CParserCGIParameter::GetParameter(const char* parameter, int32_t* val,
                                          bool auto_decode) {
    return m_env_keyval_parser.GetValue(parameter, val, auto_decode);
}

uint32_t       CParserCGIParameter::GetCookieParameterCount() {
    return m_cookie_parser.GetValidItemCount();
}

int32_t CParserCGIParameter::GetCookieParameter(const char* parameter,
                                                const char** val, bool auto_decode) {
    return m_cookie_parser.GetValue(parameter, val, auto_decode);
}

int32_t CParserCGIParameter::GetCookieParameter(const char* parameter,
                                                uint32_t* val, bool auto_decode) {
    return m_cookie_parser.GetValue(parameter, val, auto_decode);
}

int32_t  CParserCGIParameter::GetCookieParameter(const char* parameter,
                                                 int32_t* val, bool auto_decode) {
    return m_cookie_parser.GetValue(parameter, val, auto_decode);
}

void    CParserCGIParameter::DebugPrintEnvironmentToScreen() const {
#ifdef _DEBUG_ENV
    // debug info
    time_t now = time(0);
    printf("<DEBUG-INFO-Environment>\r\n");
    printf("<DebugStart>");
    printf("**[you can hide this message by disable _DEBUG macro "
           "in your SvrPubLib project Makefile]**");
    printf("</DebugStart>\r\n");
    printf("<![CDATA[\r\n");
    printf("-------------------------------\r\n");
    char tm_buf[128]= {0};
    safe_ctime(&now, sizeof(tm_buf), tm_buf);
    printf("Time:%s", tm_buf);

    printf("-------------------------------\r\n");
    printf("All environment string:\r\n");
    printf("-------------------------------\r\n");
    printf("AUTH_TYPE:%s\r\n", UtilGetEnv("AUTH_TYPE"));
    printf("REQUEST_METHOD:%s\r\n", UtilGetEnv("REQUEST_METHOD"));
    printf("CONTENT_TYPE:%s\r\n", UtilGetEnv("CONTENT_TYPE"));
    printf("DOCUMENT_ROOT:%s\r\n", UtilGetEnv("DOCUMENT_ROOT"));
    printf("GATEWAY_INTERFACE:%s\r\n", UtilGetEnv("GATEWAY_INTERFACE"));
    printf("HTTP_ACCEPT:%s\r\n", UtilGetEnv("HTTP_ACCEPT"));
    printf("HTTP_ACCEPT_CHARSET:%s\r\n",
           UtilGetEnv("HTTP_ACCEPT_CHARSET"));
    printf("HTTP_ACCEPT_ENCODING:%s\r\n",
           UtilGetEnv("HTTP_ACCEPT_ENCODING"));
    printf("HTTP_ACCEPT_LANGUAGE:%s\r\n",
           UtilGetEnv("HTTP_ACCEPT_LANGUAGE"));

    printf("HTTP_COOKIE:%s\r\n", UtilGetEnv("HTTP_COOKIE"));
    printf("HTTP_CONNECTION:%s\r\n", UtilGetEnv("HTTP_CONNECTION"));
    printf("HTTP_FROM:%s\r\n", UtilGetEnv("HTTP_FROM"));
    printf("HTTP_HOST:%s\r\n", UtilGetEnv("HTTP_HOST"));
    printf("HTTP_REFERER:%s\r\n", UtilGetEnv("HTTP_REFERER"));
    printf("HTTP_USER_AGENT:%s\r\n", UtilGetEnv("HTTP_USER_AGENT"));
    printf("PATH:%s\r\n", UtilGetEnv("PATH"));
    printf("PATH_INFO:%s\r\n", UtilGetEnv("PATH_INFO"));
    printf("PATH_TRANSLATED:%s\r\n", UtilGetEnv("PATH_TRANSLATED"));
    printf("REMOTE_ADDR:%s\r\n", UtilGetEnv("REMOTE_ADDR"));
    printf("REMOTE_HOST:%s\r\n", UtilGetEnv("REMOTE_HOST"));
    printf("REMOTE_IDENT:%s\r\n", UtilGetEnv("REMOTE_IDENT"));
    printf("REMOTE_USER:%s\r\n", UtilGetEnv("REMOTE_USER"));
    printf("REMOTE_METHOD:%s\r\n", UtilGetEnv("REMOTE_METHOD"));
    printf("SCRIPT_FILENAME:%s\r\n", UtilGetEnv("SCRIPT_FILENAME"));
    printf("SCRIPT_NAME:%s\r\n", UtilGetEnv("SCRIPT_NAME"));
    printf("SERVER_ADMIN:%s\r\n", UtilGetEnv("SERVER_ADMIN"));
    printf("SERVER_NAME:%s\r\n", UtilGetEnv("SERVER_NAME"));
    printf("SERVER_PORT:%s\r\n", UtilGetEnv("SERVER_PORT"));
    printf("SERVER_PORT_SECURE:%s\r\n", UtilGetEnv("SERVER_PORT_SECURE"));
    printf("SERVER_PROTOCOL:%s\r\n", UtilGetEnv("SERVER_PROTOCOL"));
    printf("SERVER_SOFTWARE:%s\r\n", UtilGetEnv("SERVER_SOFTWARE"));
    printf("-------------------------------\r\n");
    printf("]]>\r\n");
    printf("<DebugEnd>");
    printf("**[you can hide this message by disable _DEBUG macro "
           "in your SvrPubLib project Makefile]**");
    printf("</DebugEnd>");
    printf("</DEBUG-INFO-Environment>");
#endif // _DEBUG_ENV
}

//
// 1:ok
// 0:fail
//
// is_post: 返回本次获取到的参数是否为POST(GET, POST二者之一)
//
int32_t    CParserCGIParameter::ParserEnvironmentString(REQUEST_Type type,
                                                        bool* is_post,
                                                        bool auto_decode) {
    int32_t ok = 0;
    uint32_t len = 0;
    unsigned char decode_url = 0;
    if (auto_decode)
        decode_url = 1;
        
    switch (type) {
    case REQ_POST: {
        if (VerifyRequestMethod("POST")) {
            if(is_post) *is_post = true;
            char* num_content = getenv("CONTENT_LENGTH");
            len = ATOI(num_content);
            if (num_content && len > 0) {
                VLOG(3) << "try parser string[POST,len = " << len << "]...";
                ok = m_env_keyval_parser.ParserString(0, len, decode_url, '&', '=');
            }
        }
    }
    break;
    case REQ_GET: {
        if (VerifyRequestMethod("GET")) {
            if(is_post) *is_post = false;
            char* query = getenv("QUERY_STRING");
            len = STRLEN(query);
            if (query && len > 0) {
                VLOG(3) << "try parser string[GET]...";
                ok = m_env_keyval_parser.ParserString(query,
                                                      len,
                                                      decode_url,
                                                      '&',
                                                      '=');
            }
        }
    }
    break;
    case REQ_AUTO: {
        // AUTO:support GET or POST only
        if (VerifyRequestMethod("GET") || VerifyRequestMethod("POST")) {
            char* query_sz = getenv("QUERY_STRING"); // GET
            len = STRLEN(query_sz);
            if (query_sz && len > 0) {
                if(is_post) *is_post = false;
                VLOG(3) << "try auto parser string[GET]...";
                ok = m_env_keyval_parser.ParserString(query_sz,
                                                      len,
                                                      decode_url,
                                                      '&',
                                                      '=');
            } else {
                if(is_post) *is_post = true;
                char* content_len = getenv("CONTENT_LENGTH"); // POST
                len = ATOI(content_len);
                if (content_len && len > 0) {
                    VLOG(3) << "try auto parser string[POST]...";
                    ok = m_env_keyval_parser.ParserString(0,
                                                          len,
                                                          decode_url,
                                                          '&',
                                                          '=');
                }
            }
        }
    }
    break;
    default:
        break;
    }
    DebugPrintEnvironmentToScreen();
    return ok;
}

//
// 1:ok
// 0:fail
//
int32_t CParserCGIParameter::VerifyRequestMethod(const char* type) {
    int32_t ok = 0;
    const char* request_method = UtilGetEnv("REQUEST_METHOD");
    if (request_method && type) {
#ifdef WIN32
        if (_stricmp(request_method, type) == 0)
#else
        if (stricmp(request_method, type) == 0)
#endif //
            ok = 1;
    }

    VLOG(3) << "VerifyRequestMethod:" << (ok ? "OK":"Fail.") << ".    "
              "REQUEST_METHOD=" << request_method << "[the CGI SUPPORT:" << type << "]";
    return ok;
}

int32_t        CParserCGIParameter::ParserCookie() {
    int32_t ok = 0;
    const char* cookie = UtilGetEnv("HTTP_COOKIE");
    if (cookie) {
        uint32_t len = STRLEN(cookie);
        ok = m_cookie_parser.ParserString(cookie, len, 0, ';', '=');
    }
    return ok;
}

int32_t CParserCGIParameter::AttachCookie(const char * input_cookie) {
    int32_t ok = 0;
    if (input_cookie) {
        uint32_t len = STRLEN(input_cookie);
        ok = m_cookie_parser.ParserString(input_cookie, len, 0, ';', '=');
    }

    return ok;
}

//
// CONTENT_TYPE         数据类型格式
// DOCUMENT_ROOT        WWW Server的文件目录
// GATEWAY_INTERFACE    服务器使用CGI的版本
// HTTP_ACCEPT          HTTP所接受的MIME格式
// HTTP_CONNECTION      代表此服务器的连线功能
// HTTP_HOST            此服务器的地址
// HTTP_REFERER         指向该CGI程序所代表文件的URL
// HTTP_USER_AGENT      客户端浏览器的数据(版本 操作系统等)
// HTTP_COOKIE
// PATH                 系统所设定的路径
// REMOTE_ADDR          远端机器的IP ADDRESS
// REMOTE_HOST          远端机器的HostName
// REMOTE_METHOD        客户端向服务器端提出的要求
// SCRIPT_FILENAME      此CGI程序的绝对目录名称文件名
// SCRIPT_NAME          此CGI程序的名称文件名
// SERVER_ADMIN         此服务器管理者的E-mail地址
// SERVER_NAME          主机在网络上的名称
// SERVER_PORT          此服务器所使用的通讯口
// SERVER_PROTOCOL      此服务器所使用的传输协议
// SERVER_SOFTWARE      该主机所使用WWW Server软件的名称
//
// REQUEST_METHOD       作为HTTP的一部分请求而传送数据的方法，比如:GET
// QUERY_STRING
// CONTENT_LENGTH
//
const char* CParserCGIParameter::UtilGetEnv(const char* env_name) const {
    return getenv(env_name);
}

std::string CParserCGIParameter::GetParmeterContent(const std::string& url) {
    std::string::size_type pos = url.find("?");
    std::string parameter = "";
    if (pos != std::string::npos) {
        parameter = url.substr(pos+1);
    }
    return parameter;
}

CParserKeyVal::CParserKeyVal() {
    m_key_items = 0;
    m_max_items = 0;
    m_valid_items_count = 0;

    m_key_val_buff = 0;
    m_key_val_buff_len = 0;
    m_output_debug_flag = 0;
}

CParserKeyVal::~CParserKeyVal() {
    ReleaseResource();
}

void CParserKeyVal::ReleaseResource() {
    // Release environment string buffer
    delete []m_key_val_buff;
    m_key_val_buff = 0;

    m_key_val_buff_len = 0;

    if (m_max_items && m_key_items) {
        delete []m_key_items;
        m_key_items = 0;
        m_max_items = 0;
        m_valid_items_count = 0;
    }
}

//
// 1:ok
// 0:fail
//
int32_t CParserKeyVal::PrepareBuffer(uint32_t new_length) {
    int32_t ok = 0;
    if (new_length >= m_key_val_buff_len) {
        delete []m_key_val_buff;
        m_key_val_buff = 0;

        m_key_val_buff_len = 0;

        m_key_val_buff = new unsigned char[new_length+256];
        if (m_key_val_buff) {
            m_key_val_buff_len = new_length+256;
        }
    }
    ok = m_key_val_buff ? 1:0;
    return ok;
}

int32_t CParserKeyVal::GetNextChar(const char* sz, uint32_t pos) {
    int32_t ch = 0;
    if (sz)
        ch = sz[pos];
    else
        ch = getchar();

    return ch;
}

void CParserKeyVal::SetOutputDebugFlag(bool debug_output_flag) {
    m_output_debug_flag = debug_output_flag;
}

//
// from_buff : NULL  : from environment [POST]
//             !NULL : from string      [GET]
// 1:ok
// 0:fail
//
int32_t CParserKeyVal::ParserString(const char* from_buff,
        uint32_t len,
        unsigned char decode_url,
        unsigned char div,
        unsigned char equal) {
    Reset();
    PrepareBuffer(len+32);
#ifdef DEBUG_CGI
    printf("<DebugStart>");
    printf("**[you can hide this message by disable DEBUG_CGI macro "
           "in your SvrPubLib project Makefile]**");
    printf("</DebugStart>\r\n");

    printf("<![CDATA[\r\n");
    if (div == ';')
        printf("maybe COOKIE request...\r\n");
    else
        printf("maybe normal HTTP request...\r\n");
    printf("the input string:\r\n-------------------------------\r\n");
#endif // DEBUG_CGI
    int32_t ok = 0;

    uint32_t    pos = 0;
    uint32_t    count = 0;
    uint32_t    get_next_char_count = 0;
    int32_t     ch_val = GetNextChar(from_buff, pos++);

    get_next_char_count++;

    uint32_t    parser_state = PARSER_TryGetKeyStart;

    while (len &&  ch_val != -1 && get_next_char_count <= len) {
        unsigned char uchar = (unsigned char)ch_val;
        if (!ReAllocParameterItemsResource()) {
            // Out of memory
            ok = 0;
            break;
        }

        // Get the char
        char tempchar = 0;
        if (uchar == '+') {
            if (decode_url)
                tempchar = ' ';
            else
                tempchar = uchar;
        } else if (uchar == '%') {
            if (decode_url) {
                char s[3] = {0};
                s[0] = static_cast<char>(GetNextChar(from_buff, pos++));
                get_next_char_count++;
                s[1] = static_cast<char>(GetNextChar(from_buff, pos++));
                get_next_char_count++;
                tempchar = static_cast<char>(H2I(s));
            } else
                tempchar = uchar;
        } else {
            tempchar = uchar;
        }
#ifdef DEBUG_CGI
        if (tempchar)
            printf("%c", tempchar);
#endif // DEBUG_CGI
        m_key_val_buff[count] = tempchar;

        // Key start
        if (parser_state == PARSER_TryGetKeyStart &&
                !(uchar == div || uchar == equal)) {
            m_key_items[m_valid_items_count].key_start_pos = count;
            parser_state++;
        }

        // Key end
        if (parser_state == PARSER_TryGetKeyEnd && uchar == equal
                && count>0) {
            m_key_items[m_valid_items_count].key_end_pos = count-1;
            m_key_val_buff[count] = 0;
            parser_state++;
        }

        if (uchar == div && parser_state == PARSER_TryGetValueStart) {
            // Maybe empty VALUE
            m_key_items[m_valid_items_count].val_start_pos = count;
            m_key_items[m_valid_items_count].val_end_pos = count-1;
            m_key_val_buff[count] = 0;
            m_key_items[m_valid_items_count].valid_items = 1;
            parser_state = PARSER_TryGetKeyStart;
            m_valid_items_count++;
            ok = 1;
        } else {
            // Normal VALUE
            //  Value start
            if (parser_state == PARSER_TryGetValueStart &&
                    !(uchar == div || uchar == equal)) {
                m_key_items[m_valid_items_count].val_start_pos = count;
                parser_state++;
            }

            // Value end
            if (parser_state == PARSER_TryGetValueEnd && uchar == div) {
                m_key_items[m_valid_items_count].val_end_pos = count-1;
                m_key_val_buff[count] = 0;
                m_key_items[m_valid_items_count].valid_items = 1;
                parser_state = PARSER_TryGetKeyStart;
                m_valid_items_count++;
                ok = 1;
            }
        }

        count++; // Get a valid char
        ch_val = GetNextChar(from_buff, pos++);
        get_next_char_count++;
    }
    m_key_val_buff[count] = 0;

    // Last key' value
    if (parser_state == PARSER_TryGetValueEnd &&
            !m_key_items[m_valid_items_count].valid_items) {
        m_key_items[m_valid_items_count].val_end_pos = count-1;
        m_key_items[m_valid_items_count].valid_items = 1;
        m_valid_items_count++;
        parser_state = PARSER_TryGetKeyStart;
        ok = 1;
    }

    // Last key' empty value 'ID=100&Key=&User='
    if (parser_state == PARSER_TryGetValueStart &&
            !m_key_items[m_valid_items_count].valid_items) {
        m_key_items[m_valid_items_count].val_start_pos = count-1;
        m_key_items[m_valid_items_count].val_end_pos = count-1;
        m_key_items[m_valid_items_count].valid_items = 1;
        m_valid_items_count++;
        parser_state = PARSER_TryGetKeyStart;
        ok = 1;
    }

    // Last key' empty value 'ID=100&key=&User'
    if (parser_state == PARSER_TryGetKeyEnd &&
            !m_key_items[m_valid_items_count].valid_items) {
        m_key_items[m_valid_items_count].key_end_pos = count-1;
        m_key_items[m_valid_items_count].val_start_pos = count;
        m_key_items[m_valid_items_count].val_end_pos = count;
        m_key_items[m_valid_items_count].valid_items = 1;
        m_valid_items_count++;
        parser_state = PARSER_TryGetKeyStart;
        ok = 1;
    }

#ifdef DEBUG_CGI
    printf("\r\n-------------------------------\r\n"
           "valid string length:%d \r\n", count);
    printf("]]>\r\n");
#endif // DEBUG_CGI

    AdjustKeyName();
    DebugKeyValsToScreen();

#ifdef DEBUG_CGI
    printf("<DebugEnd>");
    printf("**[you can hide this message by disable _DEBUG macro "
           "in your SvrPubLib project Makefile]**");
    printf("</DebugEnd>\r\n");
#endif // DEBUG_CGI
    return ok;
}

// auto_decode = false 
int32_t CParserKeyVal::ParserString(const char* from_buff, uint32_t len, bool auto_decode){

    unsigned char decode_url = auto_decode ? 1:0;
    return ParserString(from_buff, len, decode_url, '&', '=');
}

// int32_t CParserKeyVal::ParserStringDecode(const char* from_buff, uint32_t len) {
//    const char* p = strstr(from_buff, "?");
//    if (!p)
//        return 0;
//    uint32_t cgi_len = len - (p + 1 - from_buff);
//    return ParserString(p+1, cgi_len, 0, '&', '=');
// }

//
// 1:ok
// 0:fail
//
int32_t CParserKeyVal::ReAllocParameterItemsResource() {
    int32_t ok = 0;
    if (m_valid_items_count >= m_max_items) {
        KeyValItem* temp_items = new KeyValItem[m_max_items+24];
        if (temp_items) {
            m_max_items += 24;
            if (m_key_items) {
                memcpy((unsigned char*)temp_items,
                       (unsigned char*)m_key_items,
                       sizeof(KeyValItem)*m_valid_items_count);
                delete []m_key_items;
                m_key_items = 0;
            }
            memset((unsigned char*)temp_items +
                   sizeof(KeyValItem) * m_valid_items_count,
                   0,
                   sizeof(KeyValItem)*(m_max_items-m_valid_items_count));
            m_key_items = temp_items;
        }
    }
    ok = m_key_items ? 1:0;
    return ok;
}

int32_t CParserKeyVal::H2I(char *sz) {
    if (sz) {
        const char* digits = "0123456789ABCDEF";
        if (islower(sz[0]))
            sz[0] = static_cast<char>(toupper(sz[0]));
        if (islower(sz[1]))
            sz[1] = static_cast<char>(toupper(sz[1]));
        return 16*(strchr(digits, sz[0])-strchr(digits, '0')) +
               (strchr(digits, sz[1]) -
                strchr(digits, '0'));
    } else {
        return 0;
    }
}

void CParserKeyVal::Reset() {
    if (m_max_items && m_key_items) {
        memset((unsigned char*)m_key_items,
               0,
               sizeof(KeyValItem)*m_max_items);
    }
    m_valid_items_count = 0;
}

void CParserKeyVal::DebugKeyValsToScreen() {
#ifdef DEBUG_CGI
    printf("<DEBUG-KeyVals>\r\n");
    printf("<![CDATA[\r\n");
    printf("Total Key-Vals:%u\r\n", m_valid_items_count);
    for (uint32_t i = 0; i < m_valid_items_count; i++) {
        // key-val info:
        printf("Key-Vals[%u]<%2d>:%s=%s\r\n", i,
               STRLEN(reinterpret_cast<char*>(m_key_val_buff) +
                      m_key_items[i].val_start_pos),
               m_key_val_buff+m_key_items[i].key_start_pos,
               m_key_val_buff+m_key_items[i].val_start_pos);
    }
    printf("]]>\r\n");
    printf("</DEBUG-KeyVals>\r\n");
#endif // DEBUG_CGI
}

const char* CParserKeyVal::GetKeyName(uint32_t key_index) {
    if (key_index < m_valid_items_count) {
        return (const char*)(m_key_val_buff +
                             m_key_items[key_index].key_start_pos);
    } else
        return NULL;
}

const char* CParserKeyVal::GetValString(uint32_t key_index) {
    if (key_index < m_valid_items_count) {
        return (const char*)(m_key_val_buff +
                             m_key_items[key_index].val_start_pos);
    } else
        return NULL;
}

uint32_t CParserKeyVal::GetValidItemCount() {
    return m_valid_items_count;
}

const char* CParserKeyVal::GetValue(const char* key, bool auto_decode) {
    
    m_temp_decoded_val = "";
    const char*  val = NULL;
    bool    found = false;
    if (key && m_valid_items_count && m_key_items) {
        uint32_t param_len = STRLEN(key);

        for (uint32_t i = 0; i < m_valid_items_count; i++) {
            if (m_key_items[i].valid_items) {
                
                uint32_t key_len = m_key_items[i].key_end_pos - m_key_items[i].key_start_pos + 1;

                if (param_len == key_len &&
                        memcmp(m_key_val_buff + m_key_items[i].key_start_pos, key, key_len) == 0) {
                    val = reinterpret_cast<char*>(m_key_val_buff) + m_key_items[i].val_start_pos;
                    // UnFormatString(*pszVal);
                    if (auto_decode) {
                        std::string str_val = val;
                        // std::string str_output = "";
                        m_temp_decoded_val = "";
                        //UrlCodec::Decode(str_val, &str_output);
                        UrlCodec::Decode(str_val, &m_temp_decoded_val);
                        // strcpy(val, str_output.c_str());

                        val = m_temp_decoded_val.c_str();
                    }
                    found = 1;
                    break;
                }
            }
        }
    }

    if (m_output_debug_flag && !found) {
        printf("get item:%s fail.\r\n", key);
    }

    return val;
}

int32_t CParserKeyVal::GetValue(const char* key, const char** val, bool auto_decode) {
    int32_t    ok = 0;
    if (val) {
        *val = GetValue(key, auto_decode);
        ok = *val ? 1:0;
    }
    return ok;
}

int32_t CParserKeyVal::GetValue(const char* key, int32_t* val, bool auto_decode){
    int32_t    ok = 0;
    const char* sz_val = GetValue(key, auto_decode);
    if (sz_val && val) {
        *val = ATOI(sz_val);
        ok = 1;
    }
    return ok;
}

int32_t CParserKeyVal::GetValue(const char* key, uint32_t* val, bool auto_decode) {
    return GetValue(key, reinterpret_cast<int32_t*>(val));
}

int32_t CParserKeyVal::GetValue(const char* key, uint16_t* val, bool auto_decode) {
    int32_t    ok = 0;
    const char* sz_val = GetValue(key, auto_decode);
    if (sz_val && val) {
        *val = (uint16_t)(ATOI(sz_val));
        ok = 1;
    }
    return ok;
}

//
// Seek to valid start of key name
//
void CParserKeyVal::AdjustKeyName() {
    if (m_valid_items_count && m_key_items) {
        for (uint32_t i = 0; i < m_valid_items_count; i++) {
            if (m_key_items[i].valid_items) {
                for (uint32_t j = m_key_items[i].key_start_pos;
                        j < m_key_items[i].key_end_pos;
                        j++) {
                    if (m_key_val_buff[m_key_items[i].key_start_pos] == ' ') {
                        m_key_items[i].key_start_pos++;
                    } else
                        break;
                }
            }
        }
    }
}

void CParserKeyVal::UnFormatString(char* sz) {
    if (sz) {
        int32_t i = 0;
        while (sz[i] != 0) {
            if (sz[i] == '+')
                sz[i] = ' ';
            if (sz[i] == '%')
                sz[i] = '&';
            i++;
        }
    }
}

_END_XFS_BASE_NAMESPACE_
