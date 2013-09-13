// Copyright (c) 2011, Tencent Inc. All rights reserved.

/// @brief string algorithms
/// @author hsiaokangliu
/// @date 2010-11-26

#ifndef COMMON_BASE_STRING_ALGORITHM_HPP
#define COMMON_BASE_STRING_ALGORITHM_HPP

#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <vector>
#include <string>

#include "common/base/compatible/string.h"
#include "common/base/platform_features.hpp"
#include "common/base/static_assert.hpp"
#include "common/base/stdint.h"
#include "common/encoding/ascii.hpp"
#include "common/system/memory/unaligned.hpp"

// StringFormat has been moved to format.hpp, include here to keep compatible
#include "common/base/string/format.hpp"

// An optimized fast memory compare function, should be inlined
inline bool MemoryEqual(const void* a1, const void* a2, size_t size)
{
    return memeql(a1, a2, size);
}

inline int CompareMemory(const void *b1, const void *b2, size_t len, size_t* prefix_length)
{
    STATIC_ASSERT(sizeof(size_t) == 8 || sizeof(size_t) == 4 || sizeof(size_t) == 2);

    const unsigned char * const a = (const unsigned char *)b1;
    const unsigned char * const b = (const unsigned char *)b2;

    // pos must bu signed type
    ptrdiff_t pos = 0;
    ptrdiff_t end_pos = len - sizeof(size_t);

    int result = 0;

#define COMPARE_MEMORY_ONE_BYTE() \
    result = a[pos] - b[pos]; \
    if (result) { \
        *prefix_length = pos;\
        return result;\
    } \
    ++pos

    while (pos <= end_pos) // compare by word size
    {
        if (GetUnaligned<size_t>(a + pos) != GetUnaligned<size_t>(b + pos))
        {
            switch (sizeof(size_t))
            {
            case 8:
                COMPARE_MEMORY_ONE_BYTE();
                COMPARE_MEMORY_ONE_BYTE();
                COMPARE_MEMORY_ONE_BYTE();
                COMPARE_MEMORY_ONE_BYTE();
                // fall through
            case 4:
                COMPARE_MEMORY_ONE_BYTE();
                COMPARE_MEMORY_ONE_BYTE();
                // fall through
            case 2:
                COMPARE_MEMORY_ONE_BYTE();
                COMPARE_MEMORY_ONE_BYTE();
            }
            assert(!"unreachable");
        }
        pos += sizeof(size_t);
    }

    switch (len - pos) // handle tail
    {
    case 7: COMPARE_MEMORY_ONE_BYTE();
    case 6: COMPARE_MEMORY_ONE_BYTE();
    case 5: COMPARE_MEMORY_ONE_BYTE();
    case 4: COMPARE_MEMORY_ONE_BYTE();
    case 3: COMPARE_MEMORY_ONE_BYTE();
    case 2: COMPARE_MEMORY_ONE_BYTE();
    case 1: COMPARE_MEMORY_ONE_BYTE();
    }

#undef COMPARE_MEMORY_ONE_BYTE

    *prefix_length = len;
    return result; // match
}

inline int CompareMemory(const void *b1, const void *b2, size_t len)
{
    size_t prefix_length;
    return CompareMemory(b1, b2, len, &prefix_length);
}

/// @brief  求两个串的最大公共前缀串
/// @param  lhs     lhs的buffer
/// @param  lhs_len lhs的长度
/// @param  rhs     rhs的buffer
/// @param  rhs_len rhs的长度
/// @return 最大公共前缀串的长度
inline size_t GetCommonPrefixLength(
    const void* lhs, size_t lhs_len,
    const void* rhs, size_t rhs_len
)
{
    size_t prefix_length;
    size_t common_length = lhs_len < rhs_len ? lhs_len : rhs_len;
    CompareMemory(lhs, rhs, common_length, &prefix_length);
    return prefix_length;
}

size_t OldGetCommonPrefixLength(const void* lhs, size_t lhs_len,
        const void* rhs, size_t rhs_len);

inline size_t GetCommonPrefixLength(const std::string& lhs, const std::string& rhs)
{
    return GetCommonPrefixLength(lhs.c_str(), lhs.length(),
            rhs.c_str(), rhs.length());
}

/// @brief  按字节大小比较字符串lhs 和 rhs
/// @param  lhs     lhs的buffer
/// @param  lhs_len lhs的长度
/// @param  rhs     rhs的buffer
/// @param  rhs_len rhs的长度
/// @param  inclusive 返回两个字符串是否存在包含关系
/// @retval <0 lhs < rhs
/// @retval 0  lhs = rhs;
/// @retval >0 lhs > rhs
/// @note 需要 inline
inline int CompareByteString(const void* lhs, size_t lhs_len,
        const void* rhs, size_t rhs_len, bool* inclusive)
{
    const unsigned char* p1 = reinterpret_cast<const unsigned char*>(lhs);
    const unsigned char* p2 = reinterpret_cast<const unsigned char*>(rhs);
    ptrdiff_t min_len = (lhs_len <= rhs_len) ? lhs_len : rhs_len;
    ptrdiff_t pos = 0;
    ptrdiff_t end_pos = min_len - sizeof(size_t) + 1;

    while (pos < end_pos)
    {
        if (GetUnaligned<size_t>(p1 + pos) == GetUnaligned<size_t>(p2 + pos))
            pos += sizeof(size_t); // 按机器字长剔除公共前缀串
        else
            break;
    }

    while ((pos < min_len) && (p1[pos] == p2[pos]))
        pos++;

    *inclusive = (pos == min_len);
    if (*inclusive)
    {
        if (lhs_len > rhs_len)
            return 1;
        else if (lhs_len == rhs_len)
            return 0;
        else
            return -1;
    }
    else
    {
        return p1[pos] - p2[pos];
    }
}

/// @brief  按字节大小比较字符串lhs 和 rhs
/// @param  lhs     lhs的buffer
/// @param  lhs_len lhs的长度
/// @param  rhs     rhs的buffer
/// @param  rhs_len rhs的长度
/// @retval <0 lhs < rhs
/// @retval 0  lhs = rhs;
/// @retval >0 lhs > rhs
inline int CompareByteString(
    const void* lhs, size_t lhs_len,
    const void* rhs, size_t rhs_len
)
{
    bool inclusive;
    return CompareByteString(lhs, lhs_len, rhs, rhs_len, &inclusive);
}

inline int CompareByteString(const std::string& lhs, const std::string& rhs)
{
    return CompareByteString(lhs.c_str(), lhs.length(), rhs.c_str(), rhs.length());
}

inline bool IsOctDigit(char c)
{
    return c >= '0' && c <= '7';
}

inline bool IsWhiteString(char const *str)
{
    if (!str) return false;

    size_t i = 0;
    size_t len = strlen(str);
    while (Ascii::IsSpace(str[i]) && i < len)
    {
        i++;
    }
    return (i == len);
}

inline bool IsCharInString(char c, const std::string& str)
{
    size_t len = str.size();
    for (size_t i = 0; i < len; i++)
    {
        if (str[i] == c) return true;
    }
    return false;
}

inline bool HasPrefixString(const std::string& str, const std::string& prefix)
{
    return str.size() >= prefix.size() &&
        str.compare(0, prefix.size(), prefix) == 0;
}

inline std::string RemovePrefixString(const std::string& str, const std::string& prefix)
{
    if (HasPrefixString(str, prefix))
    {
        return str.substr(prefix.size());
    }
    return str;
}

inline bool HasSuffixString(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() &&
        str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

inline std::string RemoveSuffixString(const std::string& str, const std::string& suffix)
{
    if (HasSuffixString(str, suffix))
    {
        return str.substr(0, str.size() - suffix.size());
    }
    return str;
}

std::string ReplaceString(const std::string& s, const std::string& from, const std::string& to);
std::string ReplaceAll(const std::string& s, const std::string& from, const std::string& to);
size_t ReplaceAllChars(std::string* s, const std::string& from, char to);
std::string ReplaceAllChars(const std::string& s, const std::string& from, char to);
std::string StripString(const std::string&s, const std::string& remove, char replacewith);
std::string JoinStrings(const std::vector<std::string>& components, const char* delim);
std::string RemoveContinuousBlank(const std::string& str);

void StripString(std::string* s, const char* remove, char replacewith);
void RemoveContinuousBlank(std::string* str);

//    These methods concatenate a vector of strings into a C++ string, using
//    the C-string "delim" as a separator between components. There are two
//    flavors of the function, one flavor returns the concatenated string,
//    another takes a pointer to the target string. In the latter case the
//    target string is cleared and overwritten.
void JoinStrings(const std::vector<std::string>& components, const char* delim, std::string* res);

// Split a string using a character delimiter. Append the components to
// 'result'.  If there are consecutive delimiters, this function skips over all
// of them.
// Note: For multi-character delimiters, this routine will split on *ANY* of
// the characters in the string, not the entire string as a single delimiter.
// So it's NOT the reverse function of JoinStrings.
void SplitString(const std::string& full, const char* delim, std::vector<std::string>* res);

// The 'delim' is a delimiter string, it's the reverse function of JoinStrings.
void SplitStringByDelimiter(const std::string& full,
                            const char* delim,
                            std::vector<std::string>* result);

std::string StringTrimLeft(const std::string& str, const std::string& trim_value);
std::string StringTrimLeft(const std::string& str);
std::string StringTrimRight(const std::string& str, const std::string& trim_value);
std::string StringTrimRight(const std::string& str);
std::string StringTrim(const std::string& str, const std::string& trim_value);
std::string StringTrim(const std::string& str);

void StringTrimLeft(std::string* str, const std::string& trim_value);
void StringTrimLeft(std::string* str);
void StringTrimRight(std::string* str, const std::string& trim_value);
void StringTrimRight(std::string* str);
void StringTrim(std::string* str, const std::string& trim_value);
void StringTrim(std::string* str);

std::string RemoveSubString(
    const std::string& str,
    const std::string& sub,
    bool fill_blank = false
);

std::string RemoveAllSubStrings(
    const std::string& str,
    const std::string& sub,
    bool fill_blank = false
);

std::string CUnescapeString(const std::string& src);
std::string CEscapeString(const std::string& src);
int CEscapeString(const char* src, int src_len, char* dest, int dest_len);
/// Make sure the dest buffer is big enough
int CUnescapeString(const char* src, char* dest);

inline void UpperString(std::string* s)
{
    std::string::iterator end = s->end();
    for (std::string::iterator i = s->begin(); i != end; ++i)
    {
        if (*i < 0) ++i;
        else *i = toupper(*i);
    }
}

inline void LowerString(std::string* s)
{
    std::string::iterator end = s->end();
    for (std::string::iterator i = s->begin(); i != end; ++i)
    {
        if (*i < 0) ++i;
        else *i = tolower(*i);
    }
}

inline std::string UpperString(const std::string& s)
{
    std::string res = s;
    UpperString(&res);
    return res;
}

inline std::string LowerString(const std::string& s)
{
    std::string res = s;
    LowerString(&res);
    return res;
}

#endif // COMMON_BASE_STRING_ALGORITHM_HPP
