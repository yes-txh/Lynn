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

STATIC_ASSERT(sizeof("����") == 5, "���ļ�Ϊ��GB���룬�����ΪGBK����");

/// ���� GB2312/GBK �����һЩ����
struct GB
{
    /**
     * �Ƿ��������ֽ�
     **/
    static inline bool IsLeadingByte(unsigned char c)
    {
        return c >= 0x81 && c <= 0xFE;
    }

    /**
     * ��������: �ж��Ƿ�ΪGBK�ַ�
     * �������: word ����Ĵ�
     * ���� :   true���ǣ�false�����ǡ�
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
     * �ж��ǲ��� GBK ���ַ�(2 ���ֽ�)
     * ���Ĳ���:
     * [A1A0 - A1FF]   ȫ�Ǳ�����          ���˵�
     * [A2A0 - A2FF]   ���ֱ�����          ���˵�
     * [A3A0 - A3FF]   �����ASCII�ַ���Ӧ   ת������Ӧ��ASCII�ַ�,Ӣ�ķִʴ���
     * [A4-A5,A0-FF]   ����Ƭ����            ���ķִʴ���
     * [A6-A9,A0-FF]   ���ַ���              ���˵�
     * [A8-A9,40-9F]   ���ַ���              ���˵�
     * [B0-F7,A0-FF]   GB2312����            ���ķִʴ���
     * [81-A0,40-FF]   ��չ���ֵ�һ����      ���ķִʴ������˵�
     * [AA-FE,40-AF]   ��չ���ֵڶ�����      ���ķִʴ������˵�
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
     * ��������: �ж��Ƿ�ΪGB2312������
     * �������: word ����Ĵ�
     * ���� :   true����GB2312��㣻false�����ǡ�
     **/
    static inline bool IsPunct(const char* word)
    {
        unsigned char c1, c2;

        if (2 == strlen(word))
        {
            c1 = static_cast<unsigned char>(*word);
            c2 = static_cast<unsigned char>(*(word + 1));

            // �����ձ�Ƭ������һЩ���ı�ʾ������,
            // �Լ�GBK��չ�ַ�[81-A0,40-FF],[AA-FE,40-AF]
            // ������˵� ���������š���
            if ((0xA1 == c1) && (0xB6 == c2 || 0xB7 == c2))
            {
                return false;
            }

            if (((0xA3 == c1 || 0xA9 == c1) && (0xA0 <= c2)) ||
                // ��0xA1F0~0xF5��6���ַ���Ϊ�����ַ�,���ִ��Ż�ʹ��
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
     * �ж��Ƿ�����Ч�� GB ������ַ���
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
     ��������: �ַ������ˣ����ַ�����β�Ŀհ��ַ����հ��ַ���ȫ�ǿո�ȥ��
     �������: source �����˵��ַ���
     �������: dwLen �ַ�������
     ���� :   �ɹ������ع��˺���ַ�����ʧ�ܣ�����NULL
    **/
    static char* Trim(char* source, size_t length, size_t* result_length)
    {
        char* begin = source;
        char* end = &begin[length-1];

        // ȥ���ַ���ǰ��İ�Ǻ�ȫ�ǿո�
        while (begin < end)
        {
            if (Ascii::IsSpace(*begin))
                begin++;
            else if (GetUnaligned<uint16_t>(begin) == 0xA1A1)
                begin += 2;
            else
                break;
        }

        // ȥ���ַ�������İ�Ǻ�ȫ�ǿո�
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
     ��������: �ַ������ˣ����ַ�����β�Ŀհ��ַ����հ��ַ���ȫ�ǿո�ȥ��
     �������: source �����˵��ַ���
     ���� :   �ɹ������ع��˺���ַ�����ʧ�ܣ�����NULL
    **/
    static char* Trim(char* source)
    {
        size_t length = strlen(source);
        return Trim(source, length, &length);
    }
};

#endif // COMMON_ENCODING_GB_HPP
