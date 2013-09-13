//  utf8.h
//  wookin@tencent.com
//
//  UTF8安全copy
// ////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_UTF8_H_
#define COMMON_BASELIB_SVRPUBLIB_UTF8_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

enum ENUM_UTF8_BYTE_TYPE {
    UTF8_BYTE_TYPE_1st_OF_1      = 0,
    UTF8_BYTE_TYPE_CONTINUING    = 1,
    UTF8_BYTE_TYPE_1st_OF_2      = 2,
    UTF8_BYTE_TYPE_1st_OF_3      = 3,
    UTF8_BYTE_TYPE_1st_OF_4      = 4,
    UTF8_BYTE_TYPE_1st_OF_5      = 5,
    UTF8_BYTE_TYPE_1st_OF_6      = 6,
    UTF8_BYTE_TYPE_UNKNOWN       = 7,
};

ENUM_UTF8_BYTE_TYPE GetUTF8ByteType(unsigned char uchar);

//
//  Return n bytes that copyed
//
uint32_t strncpy_utf8(char* dst,
                      const char* src_utf8,
                      uint32_t max_dst_buff_len);

//
// 功能描述： ansi转utf8函数。
//            支持win32与linux平台。
//
// 输入参数： pAnsi          待转换的ansi字符串
// 输入参数： iAnsiLen       待转换的ansi字符串长度
// 输出参数： pUTF8Buff      存放转换后的utf8字符串的buffer
// 输入参数： BuffLen        输出缓冲区长度。建议至少为(3*iAnsiLen + 1)
// 输出参数： ValidUTF8Len   转换后的utf8字符串实际占用bytes数目
// 返回值：   转换是否成功。 true：成功； false：失败。
//
bool ConvertAnsiToUTF8(const char* ansi_sz,
                       int32_t ansi_len,
                       char* utf8_buff,
                       int32_t utf8_buff_len,
                       int32_t* valid_utf8_len);

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_UTF8_H_

