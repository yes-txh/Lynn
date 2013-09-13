//    key_value_parser.h: interface for the CKeyValueParser class.
//    专门处理Key = value类型配置文件
//
//    key1 = value1
//    key2 = value2
//    key3 = value3
//    key1 = value4
//
//    相同的key可以按key索引获取
//
//    wookin@tencent.com
// ////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_KEY_VALUE_PARSER_H_
#define COMMON_BASELIB_SVRPUBLIB_KEY_VALUE_PARSER_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

class CKeyValueParser {
public:
    typedef struct _tagKeyValDesc {
        bool        valid;
        uint32_t    key_start;
        uint32_t    key_end;
        uint32_t    val_start;
        uint32_t    val_end;
    } KeyValDesc;
public:
    CKeyValueParser(uint32_t init_buff_size = 1024,
                    uint32_t init_key_val_size = 64,
                    unsigned char debug = 0);
    bool    ParserFromFile(const char* file_name);
    bool    ParserFromBuffer(const unsigned char* buff, uint32_t len);
    bool    GetValue(const char* key, unsigned char* val, int32_t val_buff_len);

    bool    GetValue(const char* key, unsigned char* val, int32_t val_buff_len,
                     uint32_t key_index);

    int32_t  GetSameKeyCount(const char* key);
    virtual ~CKeyValueParser();

#ifdef _DEBUG
    void    ListKeyVal(FILE* fp_log);
#endif // _DEBUG

private:
    bool    Parser();
    bool    GetValue(const char* key, unsigned char* val_buf,
                     int32_t val_buff_len,
                     uint32_t key_index,
                     bool use_index);

    unsigned char*  m_cache_buff;
    uint32_t        m_cache_buff_len;
    uint32_t        m_valid_data_len;

    KeyValDesc*     m_keyval_descs;
    uint32_t        m_max_keyval_descs;
    uint32_t        m_keyval_desc_count;

    // Query cursor of valid keys
    uint32_t        m_key_query_cursor;
    unsigned char   m_is_debug_output;
};

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_KEY_VALUE_PARSER_H_
