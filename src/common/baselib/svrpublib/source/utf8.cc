//
//  utf8.cpp
//  wookin@tencent.com
/////////////////////////////////////////////////////////

#ifndef WIN32
#include <iconv.h>
#endif

#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_


ENUM_UTF8_BYTE_TYPE GetUTF8ByteType(unsigned char uchar) {
    unsigned char uc = 0;
    for (uc = 7; uc >= 1; uc--) {
        unsigned char temp_char = (unsigned char)(uchar >> uc);
        temp_char = (unsigned char)(temp_char&0x01);   //  00000001b
        if (temp_char == 0x00) {
            break;
        }
    }
    return ENUM_UTF8_BYTE_TYPE(7-uc);
}

//
// Return n bytes copyed
//
uint32_t strncpy_utf8(char* dst,
                      const char* src_utf8,
                      uint32_t max_dst_buff_len) {
    uint32_t bytes_copyed = 0;
    if (src_utf8 && dst && max_dst_buff_len) {
        uint32_t last_multibyte_char_start = 0;
        bool is_continue = true;
        while (is_continue) {
            //
            //  Get multi bytes char
            //  First byte of multi bytes char
            //
            last_multibyte_char_start = bytes_copyed;
            ENUM_UTF8_BYTE_TYPE type_1st_byte =
                GetUTF8ByteType(src_utf8[bytes_copyed]);
            if (type_1st_byte == UTF8_BYTE_TYPE_1st_OF_1 ||
                    (type_1st_byte >= UTF8_BYTE_TYPE_1st_OF_2 &&
                     type_1st_byte <= UTF8_BYTE_TYPE_1st_OF_6)) {
                //
                // Copy first byte of multi bytes
                //
                if (src_utf8[bytes_copyed] && bytes_copyed < max_dst_buff_len) {
                    dst[bytes_copyed] = src_utf8[bytes_copyed];
                    bytes_copyed++;
                } else {
                    //  Maybe at the end of source or out of dst. buffer
                    break;
                }

                //  Copy other bytes
                unsigned char is_copy_other_bytes_fail = 0;
                if (type_1st_byte >= UTF8_BYTE_TYPE_1st_OF_2) {
                    for (uint32_t u = 0; u < (uint32_t)(type_1st_byte-1); u++) {
                        ENUM_UTF8_BYTE_TYPE byte_type =
                            GetUTF8ByteType(src_utf8[bytes_copyed]);
                        if (byte_type == UTF8_BYTE_TYPE_CONTINUING) {
                            //  Must be xxx continuing byte
                            if (bytes_copyed < max_dst_buff_len) {
                                dst[bytes_copyed] = src_utf8[bytes_copyed];
                                bytes_copyed++;
                            } else {
                                // Out of dst. buffer
                                dst[last_multibyte_char_start] = 0;
                                bytes_copyed = last_multibyte_char_start;
                                is_copy_other_bytes_fail = 1;
                                break;
                            }
                        } else {
                            //  Error
                            dst[last_multibyte_char_start] = 0;
                            bytes_copyed = last_multibyte_char_start;
                            is_copy_other_bytes_fail = 1;
                            break;
                        }
                    }
                }

                //  ? Copy other bytes fail
                if (is_copy_other_bytes_fail) {
                    dst[last_multibyte_char_start] = 0;
                    bytes_copyed = last_multibyte_char_start;
                    break;
                }
            } else if (type_1st_byte == UTF8_BYTE_TYPE_UNKNOWN ||
                       type_1st_byte == UTF8_BYTE_TYPE_CONTINUING
                      ) {
                //  Smothing Error
                break;
            }
        }
    }
    return bytes_copyed;
}

#ifdef WIN32
//
// 功能描述： win32平台下ansi转unicode
// 输入参数： pAnsi      待转换的ansi字符串
// 输入参数： iAnsiLen   待转换的ansi字符串长度
// 返回值：   转换后的unicode字符串
//
char* ANSI_to_UNICODE(const char* ansi_sz, int32_t ansi_len) {
    int32_t unicode_buff_len  = 2 * (ansi_len + 1);
    char* unicode_sz   = new char[unicode_buff_len];
    memset(unicode_sz, 0, unicode_buff_len);
    MultiByteToWideChar(CP_ACP, 0, ansi_sz, ansi_len+1,
                        (LPWSTR)unicode_sz, unicode_buff_len);
    return unicode_sz;
}
#endif


//
// 功能描述： ansi转utf8函数。支持win32与linux平台。
//
// 输入参数： ansi_sz           待转换的ansi字符串
// 输入参数： ansi_len          待转换的ansi字符串长度
// 输出参数： utf8_buff         存放转换后的utf8字符串的buffer
// 输入参数： utf8_buff_len     输出缓冲区长度。建议至少为(3*iAnsiLen + 1)
// 输出参数： valid_utf8_len    转换后的utf8字符串实际占用bytes数目
// 返回值：   转换是否成功。    true：成功； false：失败。
//
bool ConvertAnsiToUTF8(const char* ansi_sz,
                       int32_t ansi_len,
                       char* utf8_buff,
                       int32_t utf8_buff_len,
                       int32_t* valid_utf8_len) {
    bool is_ok = false;

    if ((NULL == ansi_sz) || (NULL == utf8_buff) || (0 == valid_utf8_len)) {
        return is_ok;
    }

#ifdef _DEBUG
    // Test if cpp source file is ansi.
    const char* sample_sz = "测";
    int32_t sample_len = STRLEN(sample_sz);
    if (sample_len != 2
            || (unsigned char)sample_sz[0] != 0xB2
            || (unsigned char)sample_sz[1] != 0xE2) {
        LOG(ERROR) << "The cpp source encode is not ansi.";
        return is_ok;
    }
#endif

#ifdef WIN32
    char* unicode_sz  = ANSI_to_UNICODE(ansi_sz, ansi_len);
    int32_t unicode_len = ::WideCharToMultiByte(CP_UTF8, 0,
                          (LPCWSTR)unicode_sz,
                          -1, NULL, 0,
                          NULL, NULL);

    if (unicode_len + 1 <= utf8_buff_len) {
        ZeroMemory(utf8_buff, unicode_len + 1);
        *valid_utf8_len   = ::WideCharToMultiByte(CP_UTF8, 0,
                            (LPCWSTR)unicode_sz, -1,
                            utf8_buff, unicode_len,
                            NULL, NULL);
        *valid_utf8_len = *valid_utf8_len -1;
        is_ok = true;
    }

    delete []unicode_sz;
#else
    memset(utf8_buff, 0, utf8_buff_len);
    iconv_t conv_handle;
    conv_handle = iconv_open("UTF-8", "GB2312");
    if (conv_handle == (iconv_t)(-1)) {
        return is_ok;
    }


    char * in       = const_cast<char *>(ansi_sz);
    char * out      = utf8_buff;
    size_t in_len    = ansi_len;
    size_t out_len   = utf8_buff_len;
    size_t num_not_succ = iconv(conv_handle, &in, &in_len, &out, &out_len);
    *valid_utf8_len   = utf8_buff_len - out_len;

    if ((size_t)(-1) != num_not_succ) {
        iconv_close(conv_handle);
        is_ok = true;
    } else {
        *valid_utf8_len = 0;
    }
#endif
    return is_ok;
}

_END_XFS_BASE_NAMESPACE_
