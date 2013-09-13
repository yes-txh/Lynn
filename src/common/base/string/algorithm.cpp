/// @brief string algorithms
/// @author hsiaokangliu
/// @date 2010-11-25

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <limits>
#include <iterator>
#include "common/base/compatible/stdarg.h"

#include "common/base/string/algorithm.hpp"
#include "common/base/string/string_number.hpp"
#include "common/base/scoped_ptr.h"
#include "common/encoding/ascii.hpp"

using namespace std;

inline std::string CharToHex(unsigned char ch)
{
    std::string str("%");

    static const char hexdigits[] = "0123456789abcdef";
    char high = hexdigits[ch / 16];
    char low = hexdigits[ch % 16];

    str += high;
    str += low;
    return str;
}

// ----------------------------------------------------------------------
//    Replace the "old" pattern with the "new" pattern in a string
// ----------------------------------------------------------------------
string ReplaceString(const string& s, const string& oldsub, const string& newsub)
{
    if (oldsub.empty())
        return s;

    string res;
    string::size_type pos = s.find(oldsub);
    if (pos == string::npos)
        return s;
    else
    {
        res.append(s, 0, pos);
        res.append(newsub);
        res.append(s, pos + oldsub.size(), s.length() - pos - oldsub.size());
    }
    return res;
}

// ----------------------------------------------------------------------
//    Replace all the "old" pattern with the "new" pattern in a string
// ----------------------------------------------------------------------
string ReplaceAll(const string& s, const string& oldsub, const string& newsub)
{
    if (oldsub.empty())
        return s;

    string res;
    string::size_type start_pos = 0;
    string::size_type pos;
    do {
        pos = s.find(oldsub, start_pos);
        if (pos == string::npos)
        {
            break;
        }
        res.append(s, start_pos, pos - start_pos);
        res.append(newsub);
        start_pos = pos + oldsub.size();
    } while (true);
    res.append(s, start_pos, s.length() - start_pos);
    return res;
}

// ----------------------------------------------------------------------
//    Replace all the chars in "from" to "to" in a string
// ----------------------------------------------------------------------
size_t ReplaceAllChars(std::string* s, const std::string& from, char to)
{
    size_t num_replaced = 0;
    size_t length = s->length();
    for (size_t i = 0; i < length; ++i)
    {
        if (from.find((*s)[i]) != std::string::npos)
        {
            (*s)[i] = to;
            ++num_replaced;
        }
    }
    return num_replaced;
}

std::string ReplaceAllChars(const std::string& s, const std::string& from, char to)
{
    std::string result(s);
    ReplaceAllChars(&result, from, to);
    return result;
}

// ----------------------------------------------------------------------
//    Replaces any occurrence of the characters in 'remove' with the character 'replacewith'.
// ----------------------------------------------------------------------
void StripString(string* s, const char* remove, char replacewith)
{
    const char * str_start = s->c_str();
    const char * str = str_start;
    for (str = strpbrk(str, remove); str != NULL; str = strpbrk(str + 1, remove))
    {
        (*s)[str - str_start] = replacewith;
    }
}

std::string StripString(const std::string&s, const string& remove, char replacewith)
{
    string res = s;
    StripString(&res, remove.c_str(), replacewith);
    return res;
}

// ----------------------------------------------------------------------
//  This function merges a vector of string components
// ----------------------------------------------------------------------
void JoinStrings(const vector<string>& components, const char* delim, string* result)
{
    int length = 0;
    int delim_length = strlen(delim);

    vector<string>::const_iterator iter = components.begin();
    for (; iter != components.end(); ++iter)
    {
        if (iter != components.begin())
        {
            length += delim_length;
        }
        length += iter->size();
    }
    result->reserve(length);
    for (iter = components.begin(); iter != components.end(); ++iter)
    {
        if (iter != components.begin())
        {
            result->append(delim, delim_length);
        }
        result->append(iter->data(), iter->size());
    }
}

string JoinStrings(const vector<string>& components, const char* delim)
{
    string result;
    JoinStrings(components, delim, &result);
    return result;
}

void RemoveContinuousBlank(std::string* str)
{
    bool first_blank = true;
    string::size_type end_pos = 0;
    string::size_type start_pos = 0;
    size_t len = str->length();

    for (start_pos = 0; start_pos != len; start_pos++)
    {
        if (str->at(start_pos) != ' ')
        {
            str->at(end_pos) = str->at(start_pos);
            end_pos++;
            first_blank = true;
        }
        else
        {
            if (first_blank)
            {
                str->at(end_pos) = str->at(start_pos);
                end_pos++;
                first_blank = false;
            }
        }
    }
    str->resize(end_pos);
}

std::string RemoveContinuousBlank(const std::string& str)
{
    string res = str;
    RemoveContinuousBlank(&res);
    return res;
}

string RemoveSubString(const string& s, const string& substr, bool fill_blank)
{
    return fill_blank ? ReplaceString(s, substr, " ") :
           ReplaceString(s, substr, "");
}

string RemoveAllSubStrings(const string& s, const string& substr, bool fill_blank)
{
    return fill_blank ? ReplaceAll(s, substr, " ") :
           ReplaceAll(s, substr, "");
}

template <typename ITR>
static inline
void SplitStringToIteratorUsing(const string& full, const char* delim, ITR& result)
{
    // Optimize the common case where delim is a single character.
    if (delim[0] != '\0' && delim[1] == '\0')
    {
        char c = delim[0];
        const char* p = full.data();
        const char* end = p + full.size();
        while (p != end)
        {
            if (*p == c)
                ++p;
            else
            {
                const char* start = p;
                while (++p != end && *p != c) {}
                *result++ = string(start, p - start);
            }
        }
        return;
    }

    string::size_type begin_index, end_index;
    begin_index = full.find_first_not_of(delim);
    while (begin_index != string::npos)
    {
        end_index = full.find_first_of(delim, begin_index);
        if (end_index == string::npos)
        {
            *result++ = full.substr(begin_index);
            return;
        }
        *result++ = full.substr(begin_index, (end_index - begin_index));
        begin_index = full.find_first_not_of(delim, end_index);
    }
}

// ----------------------------------------------------------------------
//    Split a string using a character delimiter.
// ----------------------------------------------------------------------
void SplitString(const string& full, const char* delim, vector<string>* result)
{
    back_insert_iterator< vector<string> > it(*result);
    SplitStringToIteratorUsing(full, delim, it);
}

template <typename ITR>
static inline
void SplitUsingStringDelimiterToIterator(const string& full,
                                         const char* delim,
                                         ITR& result)
{
    if (full.empty())
    {
        return;
    }
    if (delim[0] == '\0')
    {
        *result++ = full;
        return;
    }

    // Optimize the common case where delim is a single character.
    if (delim[1] == '\0')
    {
        SplitStringToIteratorUsing(full, delim, result);
        return;
    }

    size_t delim_length = strlen(delim);
    for (string::size_type begin_index = 0; begin_index < full.size(); )
    {
        string::size_type end_index = full.find(delim, begin_index);
        if (end_index == string::npos)
        {
            *result++ = full.substr(begin_index);
            return;
        }
        if (end_index > begin_index)
        {
            *result++ = full.substr(begin_index, (end_index - begin_index));
        }
        begin_index = end_index + delim_length;
    }
}

void SplitStringByDelimiter(const std::string& full,
                            const char* delim,
                            std::vector<std::string>* result) {
    back_insert_iterator< vector<string> > it(*result);
    SplitUsingStringDelimiterToIterator(full, delim, it);
}

void StringTrimLeft(std::string* str)
{
    size_t start_pos = 0;
    size_t end_pos = str->length();
    while (start_pos != end_pos && Ascii::IsSpace(str->at(start_pos)))
        start_pos++;
    *str = str->substr(start_pos);
}

std::string StringTrimLeft(const std::string& str)
{
    string res = str;
    StringTrimLeft(&res);
    return res;
}

void StringTrimRight(std::string* str)
{
    int end_pos = static_cast<int>(str->length()) - 1;
    while (end_pos >= 0 && Ascii::IsSpace(str->at(end_pos)))
        end_pos--;
    *str = str->substr(0, end_pos + 1);
}

std::string StringTrimRight(const std::string& str)
{
    string res = str;
    StringTrimRight(&res);
    return res;
}

void StringTrim(std::string* str)
{
    size_t start_pos = 0;
    size_t end_pos = str->length();
    while (start_pos != end_pos && Ascii::IsSpace(str->at(start_pos)))
        start_pos++;
    if (start_pos == end_pos)
    {
        *str = "";
        return;
    }
    end_pos--;
    while (Ascii::IsSpace(str->at(end_pos))) // end_pos always >= 0
        end_pos--;
    *str = str->substr(start_pos, end_pos - start_pos + 1);
}

std::string StringTrim(const std::string& str)
{
    string res = str;
    StringTrim(&res);
    return res;
}

void StringTrimLeft(std::string* str, const std::string& trim_value)
{
    size_t start_pos = 0;
    size_t end_pos = str->length();
    while (start_pos != end_pos && IsCharInString(str->at(start_pos), trim_value))
        start_pos++;
    *str = str->substr(start_pos);
}

std::string StringTrimLeft(const std::string& str, const std::string& trim_value)
{
    string res = str;
    StringTrimLeft(&res, trim_value);
    return res;
}

void StringTrimRight(std::string* str, const std::string& trim_value)
{
    int end_pos = static_cast<int>(str->length()) - 1;
    while (end_pos >= 0 && IsCharInString(str->at(end_pos), trim_value))
        end_pos--;
    *str = str->substr(0, end_pos + 1);
}

std::string StringTrimRight(const std::string& str, const std::string& trim_value)
{
    string res = str;
    StringTrimRight(&res, trim_value);
    return res;
}

void StringTrim(std::string* str, const std::string& trim_value)
{
    size_t start_pos = 0;
    size_t end_pos = str->length();
    while (start_pos != end_pos && IsCharInString(str->at(start_pos), trim_value))
        start_pos++;
    if (start_pos == end_pos)
    {
        *str = "";
        return;
    }
    end_pos--;
    while (IsCharInString(str->at(end_pos), trim_value))
        end_pos--;
    *str = str->substr(start_pos, end_pos - start_pos + 1);
}

std::string StringTrim(const std::string& str, const std::string& trim_value)
{
    string res = str;
    StringTrim(&res, trim_value);
    return res;
}

int CUnescapeString(const char* source, char* dest)
{
    char* d = dest;
    const char* p = source;

    // Small optimization for case where source = dest and there's no escaping
    while (p == d && *p != '\0' && *p != '\\')
        p++, d++;

    while (*p != '\0')
    {
        if (*p != '\\')
        {
            *d++ = *p++;
        }
        else
        {
            switch (*++p) // skip '\\'
            {
                case '\0':
                {
                    *d = '\0';
                    return d - dest;   // game is over
                }
                case 'a':  *d++ = '\a';  break;
                case 'b':  *d++ = '\b';  break;
                case 'f':  *d++ = '\f';  break;
                case 'n':  *d++ = '\n';  break;
                case 'r':  *d++ = '\r';  break;
                case 't':  *d++ = '\t';  break;
                case 'v':  *d++ = '\v';  break;
                case '\\': *d++ = '\\';  break;
                case '?':  *d++ = '\?';  break;
                case '\'': *d++ = '\'';  break;
                case '"':  *d++ = '\"';  break;
                case '0': case '1': case '2': case '3':  // octal digit: 1 to 3 digits
                case '4': case '5': case '6': case '7':
                {
                    char ch = *p - '0';
                    if (IsOctDigit(p[1]))
                        ch = ch * 8 + *++p - '0';
                    if (IsOctDigit(p[1]))
                        ch = ch * 8 + *++p - '0';
                    *d++ = ch;
                    break;
                }
                case 'x':
                case 'X':
                {
                    if (!Ascii::IsHexDigit(p[1])) // ignore '\x' of \xCC while CC is not xdigit
                    {
                        break;
                    }
                    unsigned int ch = 0;
                    while (Ascii::IsHexDigit(p[1]))
                        ch = (ch << 4) + HexDigitToInt(*++p);
                    // there should be a warning here if ch > 0xFF, the max value of 8bits.
                    *d++ = static_cast<char>(ch);
                    break;
                }
                default: // ignore unknown character
                    // FIXME there should be a warning here because the character is unknown
                    break;
            }
            p++;    // read next character
        }
    }
    *d = '\0';
    return d - dest;
}

string CUnescapeString(const string& src)
{
    scoped_array<char> unescaped(new char[src.size() + 1]);
    int len = CUnescapeString(src.c_str(), unescaped.get());
    return string(unescaped.get(), len);
}

int CEscapeInternal(const char* src, int src_len, char* dest, int dest_len)
{
    int used = 0;
    const char* src_end = src + src_len;
    for (; src < src_end; src++)
    {
        if (dest_len - used < 2)   // at least two chars needed.
        {
            return -1;
        }
        switch (*src)
        {
            case '\a': dest[used++] = '\\'; dest[used++] = 'a';  break;
            case '\b': dest[used++] = '\\'; dest[used++] = 'b';  break;
            case '\f': dest[used++] = '\\'; dest[used++] = 'f';  break;
            case '\n': dest[used++] = '\\'; dest[used++] = 'n';  break;
            case '\r': dest[used++] = '\\'; dest[used++] = 'r';  break;
            case '\t': dest[used++] = '\\'; dest[used++] = 't';  break;
            case '\v': dest[used++] = '\\'; dest[used++] = 'v';  break;
            case '\"': dest[used++] = '\\'; dest[used++] = '\"'; break;
            case '\'': dest[used++] = '\\'; dest[used++] = '\''; break;
            case '\\': dest[used++] = '\\'; dest[used++] = '\\'; break;
            default:
                if (Ascii::IsPrint(*src))
                {
                    dest[used++] = *src;
                }
                else
                {
                    if (dest_len - used < 4)
                        return -1;
                    sprintf(dest + used, "\\x%02x", static_cast<uint8_t>(*src));
                    used += 4;
                }
        }
    }
    if (dest_len - used < 1)
        return -1;
    dest[used] = '\0';
    return used;
}

int CEscapeString(const char* src, int src_len, char* dest, int dest_len)
{
    return CEscapeInternal(src, src_len, dest, dest_len);
}

string CEscapeString(const string& src)
{
    const int dest_length = src.size() * 4 + 1; // Maximum space needed
    scoped_array<char> dest(new char[dest_length]);
    const int len = CEscapeInternal(src.data(), src.size(), dest.get(), dest_length);
    if (len >= 0)
        return string(dest.get(), len);
    return "";
}

size_t OldGetCommonPrefixLength(const char* lhs, size_t lhs_len,
        const char* rhs, size_t rhs_len)
{
    char* p1 = const_cast<char*>(lhs);
    char* p2 = const_cast<char*>(rhs);
    size_t min_len = (lhs_len <= rhs_len) ? lhs_len : rhs_len;
    size_t prefix_len = 0;

    while (((prefix_len + sizeof(size_t) - 1) < min_len)
            && GetUnaligned<size_t>(p1 + prefix_len)
            == GetUnaligned<size_t>(p2 + prefix_len))
    {
        prefix_len += sizeof(size_t); // 按机器字长剔除公共前缀串
    }
    while ((prefix_len < min_len) && (p1[prefix_len] == p2[prefix_len]))
        prefix_len++;

    return prefix_len;
}
