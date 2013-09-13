// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_ENCODING_GB_HPP
#define COMMON_ENCODING_GB_HPP

#include <stddef.h>
#include "common/base/static_assert.hpp"
#include "common/base/stdint.h"
#include "common/encoding/ascii.hpp"
#include "common/system/memory/unaligned.hpp"

STATIC_ASSERT(sizeof("中文") == 5, "本文件为非GB编码，请另存为GBK编码");

/// 处理 GB2312/GBK 编码的一些函数
struct GB
{
    /**
     * 是否是引导字节
     **/
    static inline bool IsLeadingByte(unsigned char c)
    {
        return c >= 0x81 && c <= 0xFE;
    }

    /**
     * 功能描述: 判断是否为GBK字符
     * 输入参数: word 输入的词
     * 返回 :   true，是；false，不是。
     **/
    static inline bool IsChar(const char* word)
    {
        unsigned char c1, c2;

        c1 = static_cast<unsigned char>(*word);
        c2 = static_cast<unsigned char>(*(word + 1));

        if ((c1 >= 0x81) && (c1 <= 0xFE) &&
            (c2 >= 0x40) && (c2 <= 0xFE))
        {
            return true;
        }

        return false;
    };

    /**
     * 判断是不是 GBK 宽字符(2 个字节)
     * 中文部分:
     * [A1A0 - A1FF]   全角标点符号          过滤掉
     * [A2A0 - A2FF]   各种标题标号          过滤掉
     * [A3A0 - A3FF]   与基本ASCII字符对应   转换成相应的ASCII字符,英文分词处理
     * [A4-A5,A0-FF]   日文片假名            中文分词处理
     * [A6-A9,A0-FF]   各种符号              过滤掉
     * [A8-A9,40-9F]   各种符号              过滤掉
     * [B0-F7,A0-FF]   GB2312汉字            中文分词处理
     * [81-A0,40-FF]   扩展汉字第一部分      中文分词处理或过滤掉
     * [AA-FE,40-AF]   扩展汉字第二部分      中文分词处理或过滤掉
     **/
    static inline bool IsWideChar(const char* word)
    {
        unsigned char c1, c2;

        c1 = static_cast<unsigned char>(*word);
        c2 = static_cast<unsigned char>(*(word + 1));

        if (((c1 >= 0xA1 && c1 <= 0xA9) && (c2 >= 0xA0)) ||
            ((c1 >= 0xA8 && c1 <= 0xA9) && (c2 >= 0x40)) ||
            ((c1 >= 0xB0 && c1 <= 0xF7) && (c2 >= 0xA0)) ||
            ((c1 >= 0x81 && c1 <= 0xA0) && (c2 >= 0x40)) ||
            ((c1 >= 0xAA && c1 <= 0xFE) && (c2 >= 0x40 && c2 <= 0xAF)))
        {
            return true;
        }

        return false;
    }

    /**
     * 功能描述: 判断是否为GB2312标点符号
     * 输入参数: word 输入的词
     * 返回 :   true，是GB2312标点；false，不是。
     **/
    static inline bool IsPunct(const char* word)
    {
        unsigned char c1, c2;

        if (2 == strlen(word))
        {
            c1 = static_cast<unsigned char>(*word);
            c2 = static_cast<unsigned char>(*(word + 1));

            // 保留日本片假名和一些中文表示的数字,
            // 以及GBK扩展字符[81-A0,40-FF],[AA-FE,40-AF]
            // 其余过滤掉 保留书名号《》
            if ((0xA1 == c1) && (0xB6 == c2 || 0xB7 == c2))
            {
                return false;
            }

            if (((0xA3 == c1 || 0xA9 == c1) && (0xA0 <= c2)) ||
                // 把0xA1F0~0xF5这6个字符作为保留字符,给分词优化使用
                ((0xA1 == c1) && ((c2 >= 0xA0 && c2 <= 0xEF) ||
                (c2 >= 0xF6 && c2 <= 0xFE))) ||
                ((0xA8 == c1) && (0xC1 <= c2 && 0xE9 >= c2)))
            {
                return true;
            }
        }

        return false;
    }

    /**
     * 判断是否是有效的 GB 编码的字符串
     * */
    static inline bool IsValidString(const char* text, size_t text_len)
    {
        const char* cur = text;

        while (cur < text + text_len)
        {
            if (*cur & 0x80)
            {
                if (cur + 1 < text + text_len)
                {
                    if (IsWideChar(cur))
                    {
                        cur += 2;
                        continue;
                    }
                    else
                    {
                        return false;
                    }
                }
            }

            if (!Ascii::IsPrint(*cur))
            {
                return false;
            }

            cur++;
        }

        return true;
    }

    /**
     功能描述: 字符串过滤，把字符串首尾的空白字符（空白字符和全角空格）去掉
     输入参数: source 待过滤的字符串
     结果参数: dwLen 字符串长度
     返回 :   成功，返回过滤后的字符串；失败，返回NULL
    **/
    static char* Trim(char* source, size_t length, size_t* result_length)
    {
        char* begin = source;
        char* end = &begin[length-1];

        // 去掉字符串前面的半角和全角空格
        while (begin < end)
        {
            if (Ascii::IsSpace(*begin))
                begin++;
            else if (GetUnaligned<uint16_t>(begin) == 0xA1A1)
                begin += 2;
            else
                break;
        }

        // 去掉字符串后面的半角和全角空格
        while (begin <= end)
        {
            if ((begin + 2) <= end && GetUnaligned<uint16_t>(begin) == 0xA1A1)
                end -= 2;
            else if (Ascii::IsSpace(*end))
                end--;
        }
        *(end+1) = 0;

        if (*result_length)
            *result_length = end - begin + 1;

        return begin;
    }

    /**
     功能描述: 字符串过滤，把字符串首尾的空白字符（空白字符和全角空格）去掉
     输入参数: source 待过滤的字符串
     返回 :   成功，返回过滤后的字符串；失败，返回NULL
    **/
    static char* Trim(char* source)
    {
        size_t length = strlen(source);
        return Trim(source, length, &length);
    }
};

#endif // COMMON_ENCODING_GB_HPP
