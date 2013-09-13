/**
 * @file url.cpp
 * @brief 
 * @author welkin
 * @date 2011-04-20
 */

#include <stdlib.h>
#include <stdio.h>
#include <common/crypto/hash/md5.hpp>

#include "url.hpp"

using namespace web::url;

/////////////////////////////////////////////////////
//                Consts Defination                //
/////////////////////////////////////////////////////

#ifdef _WIN32
#ifndef STRNCASECMP
#define STRNCASECMP         _strnicmp
#endif
#ifndef STRCASECMP
#define STRCASECMP          _stricmp
#endif
#ifndef snprintf
#define snprintf            _snprintf
#endif
#else /// _WIN32
#include <strings.h>
#ifndef STRNCASECMP
#define STRNCASECMP         strncasecmp
#endif
#ifndef STRCASECMP
#define STRCASECMP          strcasecmp
#endif
#endif /// _WIN32

#define URL_CHR_TEST(c, mask)   (URL_CHR_TABLE[(unsigned char)(c)] & (mask))
#define URL_RESERVED_CHAR(c)    URL_CHR_TEST(c, E_URL_CHR_RESERVED)
#define URL_UNSAFE_CHAR(c)      URL_CHR_TEST(c, E_URL_UNSAFE)

#define XNUM_TO_DIGIT(x)        ("0123456789ABCDEF"[x] + 0)
#define XDIGIT_TO_NUM(h)        ((h) < 'A' ? (h) - '0' : toupper(h) - 'A' + 10)
#define X2DIGITS_TO_NUM(h1, h2) ((XDIGIT_TO_NUM (h1) << 4) + XDIGIT_TO_NUM (h2))

/// Url的字符类别
enum
{
    E_URL_CHR_RESERVED = 1,
    E_URL_UNSAFE = 2
};

/// Shorthands for the table
#define R   E_URL_CHR_RESERVED
#define U   E_URL_UNSAFE
#define RU  R|U

/// Characters defined by RFC 3986
const unsigned char URL_CHR_TABLE[256] =
{
    U,  U,  U,  U,   U,  U,  U,  U,   /* NUL SOH STX ETX  EOT ENQ ACK BEL */
    U,  U,  U,  U,   U,  U,  U,  U,   /* BS  HT  LF  VT   FF  CR  SO  SI  */
    U,  U,  U,  U,   U,  U,  U,  U,   /* DLE DC1 DC2 DC3  DC4 NAK SYN ETB */
    U,  U,  U,  U,   U,  U,  U,  U,   /* CAN EM  SUB ESC  FS  GS  RS  US  */
    U,  R,  U, RU,   R,  U,  R,  R,   /* SP  !   "   #    $   %   &   '   */
    R,  R,  R,  R,   R,  0,  0,  R,   /* (   )   *   +    ,   -   .   /   */
    0,  0,  0,  0,   0,  0,  0,  0,   /* 0   1   2   3    4   5   6   7   */
    0,  0, RU,  R,   U,  R,  U,  R,   /* 8   9   :   ;    <   =   >   ?   */
    RU, 0,  0,  0,   0,  0,  0,  0,   /* @   A   B   C    D   E   F   G   */
    0,  0,  0,  0,   0,  0,  0,  0,   /* H   I   J   K    L   M   N   O   */
    0,  0,  0,  0,   0,  0,  0,  0,   /* P   Q   R   S    T   U   V   W   */
    0,  0,  0, RU,   U, RU,  U,  0,   /* X   Y   Z   [    \   ]   ^   _   */
    U,  0,  0,  0,   0,  0,  0,  0,   /* `   a   b   c    d   e   f   g   */
    0,  0,  0,  0,   0,  0,  0,  0,   /* h   i   j   k    l   m   n   o   */
    0,  0,  0,  0,   0,  0,  0,  0,   /* p   q   r   s    t   u   v   w   */
    0,  0,  0,  U,   U,  U,  0,  U,   /* x   y   z   {    |   }   ~   DEL */

    U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,
    U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,
    U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,
    U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,

    U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,
    U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,
    U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,
    U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,
};

#undef RU
#undef U
#undef R

class CharSet
{
public:
    CharSet(const char * str)
    {
        memset(m_char_set, 0, sizeof(m_char_set));

        while (*str)
        {
            m_char_set[static_cast<unsigned char>(*str)] = 1;
            str++;
        }
    }

    bool operator()(unsigned char ch) const
    {
        return (0 != m_char_set[ch]);
    }

public:
    unsigned char m_char_set[256];
};

//const CharSet GEN_DELIMS_SET(":/?#[]@");
//const CharSet SUB_DELIMS_SET("!$&'()*+,;=");
//const CharSet RESERVED_SET(":/?#[]@!$&'()*+,;=");
//const CharSet UNRESERVED_SET("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-._~");
//const CharSet PCHAR_SET("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-._~!$&'()*+,;=:@");
const CharSet SCHEME_SET("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+-.");
const CharSet DOMAIN_SET("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-._");
//const CharSet PATH_SET("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-._~!$&'()*+,;=:@/");
//const CharSet QUERY_SET("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-._~!$&'()*+,;=:@/?");

// 协议定义，新增协议在此添加
const SSchemeInfo SCHEMES[] =
{
    { E_HTTP_PROTOCOL, "http", 80 },
    { E_HTTPS_PROTOCOL, "https", 443 }
};


////////////////////////////////////////////////////////
//                Class Implementation                //
////////////////////////////////////////////////////////

////////////////// Url //////////////////

Url::Url()
{
    Clear();
}

Url::Url(const char * url, const int url_len,
         EScheme default_scheme,
         bool normalize)
{
    Load(url, url_len, default_scheme, normalize);
}

Url::Url(const char * url, const int relative_url_len,
         const char * base_url, const int base_url_len,
         bool normalize)
{
    Load(url, relative_url_len, base_url, base_url_len, normalize);
}

Url::Url(const char * url, const int relative_url_len,
         const Url & base_url,
         bool normalize)
{
    Load(url, relative_url_len, base_url, normalize);
}

void Url::Clear()
{
    m_scheme = E_NO_PROTOCOL;
    m_domain[0] = '\0';
    m_port = 0;
    m_path[0] = '\0';
    m_query[0] = '\0';
    m_has_query = false;

    m_absolute_path[0] = '\0';
    m_reversed_domain[0] = '\0';

    m_domain_len = 0;
    m_path_len = 0;
    m_query_len = 0;
    m_absolute_path_len = 0;

    m_directory_pos_count = 0;
    m_query_pos_count = 0;
    m_resource_pos = -1;
    m_relative_path_start_pos = -1;
    m_portal_path_end_pos = -1;

    m_is_normalized = false;
    m_normalize_type = 0;
    m_truncate_type = 0;

    m_url_hash = 0;
    m_domain_hash = 0;
    m_domain_with_port_hash = 0;
    m_reversed_segment_path_hash = 0;
}

bool Url::Load(const char * url, const int url_len,
               EScheme default_scheme,
               bool normalize)
{
    Clear();

    m_is_normalized = normalize;

    if (Parse(url, url_len, default_scheme, normalize))
    {
        Recompose(normalize);
    }

    return IsValid();
}

bool Url::Load(const char * url, const int relative_url_len,
               const char * base_url, const int base_url_len,
               bool normalize)
{
    Url base_url_obj(base_url, base_url_len, E_NO_PROTOCOL, normalize);
    return Load(url, relative_url_len, base_url_obj, normalize);
}

bool Url::Load(const char * url, const int relative_url_len,
               const Url & base_url,
               bool normalize)
{
    Clear();

    m_is_normalized = normalize;

    /***********************************************************
    * Step 1:
    * The base URL is established according to the rules of
    * Section 3. If the base URL is the empty string (unknown),
    * the embedded URL is interpreted as an absolute URL and
    * we are done.
    ***********************************************************/

    /***********************************************************
    * Step 2:
    * Both the base and embedded URLs are parsed into their
    * component parts as described in RFC1808 Section 2.4.

    * a)
    * If the embedded URL is entirely empty, it inherits the
    * entire base URL (i.e., is set equal to the base URL)
    * and we are done.
    ***********************************************************/
    if (NULL == url)
    {
        return false;
    }

    /***********************************************************
    * b)
    * If the embedded URL starts with a scheme name, it is
    * interpreted as an absolute URL and we are done.
    ***********************************************************/
    if (!Parse(url, relative_url_len, E_NO_PROTOCOL, normalize))
    {
        return false;
    }

    if (E_NO_PROTOCOL != m_scheme)
    {
        Recompose(normalize);
        return true;
    }

    /**********************************************************
    * c)
    * Otherwise, the embedded URL inherits the scheme of
    * the base URL.
    ***********************************************************/
    if (!base_url.IsValid()) return false;

    m_scheme = base_url.m_scheme;

    /***********************************************************
    * Step 3:
    * If the embedded URL's <net_loc> is non-empty, we skip to
    * Step 7. Otherwise, the embedded URL inherits the <net_loc>
    * (if any) of the base URL.
    ***********************************************************/
    if (0 != m_domain_len)
    {
        if (0 == m_port)
        {
            m_port = base_url.m_port;
        }

        Recompose(normalize);
        return true;     /// Step 7
    }
    else
    {
        CopyDomainInfo(base_url);
    }

    /***********************************************************
    * Step 4:
    * If the embedded URL path is preceded by a slash "/", the
    * path is not relative and we skip to Step 7.
    ***********************************************************/
    if ('/' == m_path[0])
    {
        Recompose(normalize);
        return true;     /// not relative and goto Step 7
    }

    /***********************************************************
    * Step 5:
    * If the embedded URL path is empty (and not preceded by a
    * slash), then the embedded URL inherits the base URL path,
    * and
    ***********************************************************/
    if (0 == m_path_len)
    {
        CopyPathInfo(base_url);

        /*********************************************************
        * a)
        * if the embedded URL's <params> is non-empty, we skip to
        * step 7; otherwise, it inherits the <params> of the base
        * URL (if any) and
        **********************************************************/

        /********************************************************
        * b)
        * if the embedded URL's <query> is non-empty, we skip to
        * step 7; otherwise, it inherits the <query> of the base
        * URL (if any) and we skip to step 7.
        ********************************************************/
        if (0 == m_query_len)
        {
            CopyQueryInfo(base_url);
        }

        /// Step 7
        Recompose(normalize);
        return true;
    }

    /***********************************************************
    * Step 6:
    * The last segment of the base URL's path (anything
    * following the rightmost slash "/", or the entire path if no
    * slash is present) is removed and the embedded URL's path is
    * appended in its place. The following operations are
    * then applied, in order, to the new path:
    ***********************************************************************/
    if (!JoinPath(base_url))
    {
        m_scheme = E_NO_PROTOCOL;
        return false;
    }

    Recompose(normalize);
    return true;

    /***********************************************************************
    * Step 7:
    * The resulting URL components, including any inherited from
    * the base URL, are recombined to give the absolute form of
    * the embedded URL.

    * Parameters, regardless of their purpose, do not form a part of the
    * URL path and thus do not affect the resolving of relative paths. In
    * particular, the presence or absence of the ";type=d" parameter on an
    * ftp URL does not affect the interpretation of paths relative to that
    * URL. Fragment identifiers are only inherited from the base URL when
    * the entire embedded URL is empty.

    * The above algorithm is intended to provide an example by which the
    * output of implementations can be tested -- implementation of the
    * algorithm itself is not required. For example, some systems may find
    * it more efficient to implement Step 6 as a pair of segment stacks
    * being merged, rather than as a series of string pattern matches.
    ***********************************************************************/
}

bool Url::Preprocess(const char * url, const int url_len, bool decode_percent_encoded)
{
    bool is_valid = true;
    char ch, decoded_ch;
    int read_pos, write_pos;

    read_pos = write_pos = 0;
    while (read_pos < url_len)
    {
        ch = url[read_pos];

        if (('%' == ch) && (read_pos + 2 < url_len)
            && isxdigit((unsigned char)(url[read_pos + 1]))
            && isxdigit((unsigned char)(url[read_pos + 2])))
        {
            decoded_ch = X2DIGITS_TO_NUM(url[read_pos + 1], url[read_pos + 2]);

            if ((unsigned int)decoded_ch < 0x20)
            {
                is_valid = false;
            }

            if (decode_percent_encoded)
            {
                ch = decoded_ch;

                if (URL_RESERVED_CHAR(ch) || '%' == (ch))
                {
                    memcpy(m_absolute_path + write_pos, url + read_pos, 3);

                    read_pos += 3;
                    write_pos += 3;

                    continue;
                }
                else
                {
                    read_pos += 2;
                    m_normalize_type |= E_NORMALIZE_HEXADECIMAL_ENCODE;
                }
            }

            m_absolute_path[write_pos] = ch;
        }
        else if ('&' != ch)
        {
            m_absolute_path[write_pos] = ch;
        }
        else if ((read_pos + 4 < url_len)
                 && ('a' == url[read_pos + 1])
                 && ('m' == url[read_pos + 2])
                 && ('p' == url[read_pos + 3])
                 && (';' == url[read_pos + 4]))
        {
            m_absolute_path[write_pos] = '&';

            read_pos += 5;
            write_pos++;
            m_normalize_type |= E_NORMALIZE_HTML_ENCODE;

            continue;
        }
        else if ((read_pos + 3 < url_len)
                 && ('l' == url[read_pos + 1])
                 && ('t' == url[read_pos + 2])
                 && (';' == url[read_pos + 3]))
        {
            m_absolute_path[write_pos] = '<';

            read_pos += 4;
            write_pos++;
            m_normalize_type |= E_NORMALIZE_HTML_ENCODE;

            continue;
        }
        else if ((read_pos + 3 < url_len)
                 && ('g' == url[read_pos + 1])
                 && ('t' == url[read_pos + 2])
                 && (';' == url[read_pos + 3]))
        {
            m_absolute_path[write_pos] = '>';

            read_pos += 4;
            write_pos++;
            m_normalize_type |= E_NORMALIZE_HTML_ENCODE;

            continue;
        }
        else if ((read_pos + 5 < url_len)
                 && ('q' == url[read_pos + 1]) && ('u' == url[read_pos + 2])
                 && ('o' == url[read_pos + 3]) && ('t' == url[read_pos + 4])
                 && (';' == url[read_pos + 5]))
        {
            m_absolute_path[write_pos] = '"';

            read_pos += 6;
            write_pos++;
            m_normalize_type |= E_NORMALIZE_HTML_ENCODE;

            continue;
        }
        else if ((read_pos + 5 < url_len)
                 && ('a' == url[read_pos + 1]) && ('p' == url[read_pos + 2])
                 && ('o' == url[read_pos + 3]) && ('s' == url[read_pos + 4])
                 && (';' == url[read_pos + 5]))
        {
            m_absolute_path[write_pos] = '\'';

            read_pos += 6;
            write_pos++;
            m_normalize_type |= E_NORMALIZE_HTML_ENCODE;

            continue;
        }
        else
        {
            m_absolute_path[write_pos] = ch;
        }

        if ('\\' == ch)
        {
            m_absolute_path[write_pos] = '/';
            m_normalize_type |= E_NORMALIZE_BACKSLASH;
        }
        else if ((unsigned int)ch < 0x20)
        {
            is_valid = false;
        }

        read_pos++;

        if (('/' != m_absolute_path[write_pos])
            || (write_pos < 2)
            || ('/' != m_absolute_path[write_pos - 1])
            || (':' == m_absolute_path[write_pos - 2])
            || ('"' == m_absolute_path[write_pos - 2]))
        {
            write_pos++;
        }
        else
        {
            m_normalize_type |= E_NORMALIZE_DUPLICATE_SLASH;
        }
    }

    m_absolute_path[write_pos] = '\0';

    int start_pos = 0;
    int end_pos = write_pos - 1;

    /// 去掉首尾的空格
    while ((end_pos >= start_pos)
           && isspace((unsigned char)m_absolute_path[end_pos]))
    {
        end_pos--;
    }

    if (end_pos < start_pos) return false;

    while ((start_pos <= end_pos)
           && isspace((unsigned char)m_absolute_path[start_pos]))
    {
        start_pos++;
    }

    /// 去掉首尾的引号
    if (('"' == m_absolute_path[start_pos])
        && (end_pos > start_pos)
        && ('"' == m_absolute_path[end_pos]))
    {
        start_pos++;
        end_pos--;
    }

    /// 去掉引号中的空格
    while ((end_pos >= start_pos)
           && isspace((unsigned char)m_absolute_path[end_pos]))
    {
        end_pos--;
    }

    if (end_pos < start_pos)
        return false;

    while ((start_pos <= end_pos)
           && isspace((unsigned char)m_absolute_path[start_pos]))
    {
        start_pos++;
    }

    m_absolute_path_len = end_pos - start_pos + 1;

    if (0 != start_pos)
    {
        memmove(m_absolute_path, m_absolute_path + start_pos, m_absolute_path_len);
    }

    m_absolute_path[m_absolute_path_len] = '\0';

    return is_valid;
}

/*-----------------------------------------------------------------------------
* Description   :   按RFC 3986 解析URL字符串
*
* @param        :   [in]  url               URL字符串
* @param        :   [in]  url_len           URL字符串长度
* @param        :   [in]  default_scheme    默认的协议类型
* @param        :   [in]  normalize         是否规范化URL
* @return
-----------------------------------------------------------------------------*/
bool Url::Parse(const char * url, const int url_len,
                EScheme default_scheme,
                bool normalize)
{
    if ((NULL == url) || (url_len >= MAX_URL_LENGTH))
    {
        return false;
    }

    if (!Preprocess(url, url_len, normalize))
    {
        return false;
    }

    bool has_scheme = false;
    int read_pos, write_pos;

    read_pos = 0;
    if ((('h' == *m_absolute_path) || ('H' == *m_absolute_path))
        && (0 == STRNCASECMP(m_absolute_path + 1, "ttp", 3))
        && ((0 == STRNCASECMP(m_absolute_path + 4, "://", 3))
            || (0 == STRNCASECMP(m_absolute_path + 4, "s://", 4))))
    {
        if (':' == *(m_absolute_path + 4))
        {
            m_scheme = E_HTTP_PROTOCOL;
            m_port = 80;
            read_pos = 7;
        }
        else
        {
            m_scheme = E_HTTPS_PROTOCOL;
            m_port = 443;
            read_pos = 8;
        }

        if (!m_absolute_path[read_pos])
        {
            m_scheme = E_NO_PROTOCOL;
            m_port = 0;
            return false;
        }
    }
    else if (('/' == m_absolute_path[0]) && ('/' == m_absolute_path[1]))
    {
        m_scheme = E_NO_PROTOCOL;
        read_pos = 2;

        if (!m_absolute_path[read_pos])
        {
            m_path[0] = '/';
            m_path[1] = '\0';
            m_path_len = 1;
            return true;
        }

        has_scheme = true;
    }
    else
    {
        /// 协议名必须以字母开头
        if (isalpha((unsigned char)m_absolute_path[0]))
        {
            /// 只检查7个字符以内的协议名，加上":"1个字符总共只需检查头8个字符
            for (int i = 0; i < 8; i++)
            {
                if (!m_absolute_path[i]) break;

                if (':' == m_absolute_path[i])
                {
                    if (('/' == m_absolute_path[i + 1])
                        && ('/' == m_absolute_path[i + 2]))
                    {
                        m_scheme = E_UNKNOWN_PROTOCOL;
                        read_pos = i + 3;
                        if (!m_absolute_path[read_pos])
                        {
                            m_scheme = E_NO_PROTOCOL;
                            return false;
                        }
                    }

                    break;
                }
                else if (!SCHEME_SET(m_absolute_path[i]))
                {
                    break;
                }
            }
        }
    }

    if ((E_NO_PROTOCOL == m_scheme) && (E_NO_PROTOCOL != default_scheme))
    {
        int scheme_count = sizeof(SCHEMES) / sizeof(SSchemeInfo);
        for (int i = 0; i < scheme_count; i++)
        {
            if (SCHEMES[i].m_scheme == default_scheme)
            {
                m_scheme = SCHEMES[i].m_scheme;
                m_port = SCHEMES[i].m_default_port;
                break;
            }
        }
    }

    if ((E_NO_PROTOCOL != m_scheme) || has_scheme)
    {
        write_pos = 0;
        while (m_absolute_path[read_pos]
               && (':' != m_absolute_path[read_pos])
               && ('@' != m_absolute_path[read_pos])
               && ('/' != m_absolute_path[read_pos])
               && ('?' != m_absolute_path[read_pos])
               && ('#' != m_absolute_path[read_pos]))
        {
            if (write_pos >= MAX_DOMAIN_LENGTH - 1)
            {
                m_scheme = E_NO_PROTOCOL;
                return false;
            }

            m_domain[write_pos++] = m_absolute_path[read_pos++];
        }
        m_domain[write_pos] = '\0';
        m_domain_len = write_pos;

        if (':' == m_absolute_path[read_pos])
        {
            read_pos++;
            int start_pos = read_pos;
            bool is_valid_port = true;

            while (m_absolute_path[read_pos]
                   && ('@' != m_absolute_path[read_pos])
                   && ('/' != m_absolute_path[read_pos])
                   && ('?' != m_absolute_path[read_pos])
                   && ('#' != m_absolute_path[read_pos]))
            {
                if (!isdigit((unsigned char)m_absolute_path[read_pos]))
                {
                    is_valid_port = false;
                }

                read_pos++;
            }

            if (!m_absolute_path[read_pos])
            {
                if (read_pos > start_pos)
                {
                    if (!is_valid_port)
                    {
                        m_scheme = E_NO_PROTOCOL;
                        return false;
                    }

                    m_port = atoi(m_absolute_path + start_pos);
                }
            }
            else if ('@' == m_absolute_path[read_pos])
            {
                read_pos++;
                write_pos = 0;
                while (m_absolute_path[read_pos]
                       && (':' != m_absolute_path[read_pos])
                       && ('/' != m_absolute_path[read_pos])
                       && ('?' != m_absolute_path[read_pos])
                       && ('#' != m_absolute_path[read_pos]))
                {
                    if (write_pos >= MAX_DOMAIN_LENGTH - 1)
                    {
                        m_scheme = E_NO_PROTOCOL;
                        return false;
                    }

                    m_domain[write_pos++] = m_absolute_path[read_pos++];
                }
                m_domain[write_pos] = '\0';
                m_domain_len = write_pos;

                if (':' == m_absolute_path[read_pos])
                {
                    read_pos++;
                    start_pos = read_pos;

                    while (m_absolute_path[read_pos]
                           && ('/' != m_absolute_path[read_pos])
                           && ('?' != m_absolute_path[read_pos])
                           && ('#' != m_absolute_path[read_pos]))
                    {
                        if (!isdigit((unsigned char)m_absolute_path[read_pos]))
                        {
                            m_scheme = E_NO_PROTOCOL;
                            return false;
                        }

                        read_pos++;
                    }

                    if (read_pos > start_pos)
                    {
                        m_port = atoi(m_absolute_path + start_pos);
                    }
                }
            }
            else
            {
                if (read_pos > start_pos)
                {
                    if (!is_valid_port)
                    {
                        m_scheme = E_NO_PROTOCOL;
                        return false;
                    }

                    m_port = atoi(m_absolute_path + start_pos);
                }
            }
        }
        else if ('@' == m_absolute_path[read_pos])
        {
            read_pos++;
            write_pos = 0;
            while (m_absolute_path[read_pos]
                   && (':' != m_absolute_path[read_pos])
                   && ('/' != m_absolute_path[read_pos])
                   && ('?' != m_absolute_path[read_pos])
                   && ('#' != m_absolute_path[read_pos]))
            {
                if (write_pos >= MAX_DOMAIN_LENGTH - 1)
                {
                    m_scheme = E_NO_PROTOCOL;
                    return false;
                }

                m_domain[write_pos++] = m_absolute_path[read_pos++];
            }
            m_domain[write_pos] = '\0';
            m_domain_len = write_pos;

            if (':' == m_absolute_path[read_pos])
            {
                read_pos++;
                int start_pos = read_pos;

                while (m_absolute_path[read_pos]
                       && ('/' != m_absolute_path[read_pos])
                       && ('?' != m_absolute_path[read_pos])
                       && ('#' != m_absolute_path[read_pos]))
                {
                    if (!isdigit((unsigned char)m_absolute_path[read_pos]))
                    {
                        m_scheme = E_NO_PROTOCOL;
                        return false;
                    }

                    read_pos++;
                }

                if (read_pos > start_pos)
                {
                    m_port = atoi(m_absolute_path + start_pos);
                }
            }
        }
    }

    /// 校验域名有效性
    for (int i = 0; i < m_domain_len; i++)
    {
        if (0 == i)
        {
            if (!isalpha((unsigned char)m_domain[i])
                && !isdigit((unsigned char)m_domain[i])
                && !IsPercentEncoded(&m_domain[i]))
            {
                m_scheme = E_NO_PROTOCOL;
                return false;
            }
        }
        else
        {
            if (!DOMAIN_SET(m_domain[i])
                && !IsPercentEncoded(&m_domain[i]))
            {
                m_scheme = E_NO_PROTOCOL;
                return false;
            }
        }
    }

    write_pos = 0;
    while (m_absolute_path[read_pos]
           && ('?' != m_absolute_path[read_pos])
           && ('#' != m_absolute_path[read_pos]))
    {
        if (write_pos >= MAX_PATH_LENGTH - 1)
        {
            m_scheme = E_NO_PROTOCOL;
            return false;
        }

        m_path[write_pos++] = m_absolute_path[read_pos++];
    }

    if ((0 == write_pos) && ((E_NO_PROTOCOL != m_scheme) || has_scheme))
    {
        m_path[write_pos++] = '/';
    }
    m_path[write_pos] = '\0';
    m_path_len = write_pos;

    /// 校验路径有效性
    //for (int i = 0; i < m_path_len; i++)
    //{
    //    if (!PATH_SET(m_path[i])
    //        && !IsPercentEncoded(&m_path[i]))
    //    {
    //        m_scheme = E_NO_PROTOCOL;
    //        return false;
    //    }
    //}

    if ('?' == m_absolute_path[read_pos])
    {
        m_has_query = true;
        read_pos++;
        write_pos = 0;
        while (m_absolute_path[read_pos]
               && ('#' != m_absolute_path[read_pos]))
        {
            if (write_pos >= MAX_QUERY_LENGTH - 1)
            {
                m_scheme = E_NO_PROTOCOL;
                return false;
            }

            m_query[write_pos++] = m_absolute_path[read_pos++];
        }
        m_query[write_pos] = '\0';
        m_query_len = write_pos;

        if (!ParseQuery(m_query, m_query_len))
        {
            m_scheme = E_NO_PROTOCOL;
            return false;
        }

        /// 校验查询有效性
        //for (int i = 0; i < m_query_len; i++)
        //{
        //    if (!QUERY_SET(m_query[i])
        //        && !IsPercentEncoded(&m_query[i]))
        //    {
        //        m_scheme = E_NO_PROTOCOL;
        //        return false;
        //    }
        //}

        if (normalize) SortQuery();
    }

    return true;
}

bool Url::ParseQuery(const char * query, const int query_len)
{
    int read_pos = 0;
    int index = 0;

    m_query_pos_count = 0;

    m_query_pos[index] = read_pos;
    m_query_pos[index + 1] = -1;
    m_query_pos[index + 2] = -1;
    m_query_pos[index + 3] = -1;

    bool searching_key = true;
    while (query[read_pos])
    {
        if (searching_key && ('=' == query[read_pos]))
        {
            m_query_pos[index + 1] = read_pos;
            m_query_pos[index + 2] = read_pos + 1;
            searching_key = false;
        }
        else if ('&' == query[read_pos])
        {
            if (-1 == m_query_pos[index + 1])
            {
                m_query_pos[index + 1] = read_pos;
            }
            else if ((-1 != m_query_pos[index + 2])
                     && (read_pos > m_query_pos[index + 2]))
            {
                m_query_pos[index + 3] = read_pos;

                /// 去掉Value首尾的空格
                while ((m_query_pos[index + 3] > m_query_pos[index + 2])
                       && isspace((unsigned char)m_query[m_query_pos[index + 3] - 1]))
                {
                    m_query_pos[index + 3]--;
                }

                if (m_query_pos[index + 3] <= m_query_pos[index + 2])
                {
                    m_query_pos[index + 3] = -1;
                }
                else
                {
                    while ((m_query_pos[index + 2] < m_query_pos[index + 3])
                           && isspace((unsigned char)m_query[m_query_pos[index + 2]]))
                    {
                        m_query_pos[index + 2]++;
                    }
                }
            }

            /// 去掉Key首尾的空格
            while ((m_query_pos[index + 1] > m_query_pos[index])
                   && isspace((unsigned char)m_query[m_query_pos[index + 1] - 1]))
            {
                m_query_pos[index + 1]--;
            }

            while ((m_query_pos[index] < m_query_pos[index + 1])
                   && isspace((unsigned char)m_query[m_query_pos[index]]))
            {
                m_query_pos[index]++;
            }

            if (m_query_pos[index + 1] > m_query_pos[index])
            {
                m_query_pos_count++;
                if (m_query_pos_count >= MAX_QUERY_COUNT)
                {
                    return false;
                }
                index = m_query_pos_count * 4;
            }
            m_query_pos[index] = read_pos + 1;
            m_query_pos[index + 1] = -1;
            m_query_pos[index + 2] = -1;
            m_query_pos[index + 3] = -1;
            searching_key = true;
        }

        read_pos++;
    }

    if (-1 == m_query_pos[index + 1])
    {
        m_query_pos[index + 1] = read_pos;
    }
    else if ((-1 != m_query_pos[index + 2])
             && (read_pos > m_query_pos[index + 2]))
    {
        m_query_pos[index + 3] = read_pos;

        /// 去掉Value首尾的空格
        while ((m_query_pos[index + 3] > m_query_pos[index + 2])
               && isspace((unsigned char)m_query[m_query_pos[index + 3] - 1]))
        {
            m_query_pos[index + 3]--;
        }

        if (m_query_pos[index + 3] <= m_query_pos[index + 2])
        {
            m_query_pos[index + 3] = -1;
        }
        else
        {
            while ((m_query_pos[index + 2] < m_query_pos[index + 3])
                   && isspace((unsigned char)m_query[m_query_pos[index + 2]]))
            {
                m_query_pos[index + 2]++;
            }
        }
    }

    /// 去掉Key首尾的空格
    while ((m_query_pos[index + 1] > m_query_pos[index])
           && isspace((unsigned char)m_query[m_query_pos[index + 1] - 1]))
    {
        m_query_pos[index + 1]--;
    }

    while ((m_query_pos[index] < m_query_pos[index + 1])
           && isspace((unsigned char)m_query[m_query_pos[index]]))
    {
        m_query_pos[index]++;
    }

    if (m_query_pos[index + 1] > m_query_pos[index])
    {
        m_query_pos_count++;
    }

    return true;
}

void Url::SortQuery()
{
    /// 使用冒泡排序法排序查询参数，排序过程同时去除重复的参数对

    int query_pos[4];
    size_t query_pos_size = sizeof(query_pos);
    int i, j, pos_1, pos_2;
    bool exchange;
    int key_compare;

    for (i = 0; i < m_query_pos_count - 1; i++)
    {
        exchange = false;
        for (j = m_query_pos_count - 2; j >= i; j--)
        {
            key_compare = 0;
            pos_1 = m_query_pos[j * 4];
            pos_2 = m_query_pos[(j + 1) * 4];
            while ((pos_1 < m_query_pos[j * 4 + 1])
                   && (pos_2 < m_query_pos[(j + 1) * 4 + 1])
                   && (tolower(m_query[pos_1]) == tolower(m_query[pos_2])))
            {
                if (0 == key_compare)
                {
                    if (m_query[pos_1] > m_query[pos_2])
                        key_compare = 1;
                    else if (m_query[pos_1] < m_query[pos_2])
                        key_compare = -1;
                    else
                        key_compare = 0;
                }
                pos_1++;
                pos_2++;
            }

            if ((pos_1 >= m_query_pos[j * 4 + 1])
                && (pos_2 >= m_query_pos[(j + 1) * 4 + 1]))
            {
                if (0 == key_compare)
                {
                    /// 删除重复的参数，只保留最后一个
                    memmove(m_query_pos + j * 4, m_query_pos + (j + 1) * 4, query_pos_size * (m_query_pos_count - j - 1));
                    m_query_pos_count--;
                    m_normalize_type |= E_NORMALIZE_DUPLICATE_QUERY;
                    continue;
                }
                else if (key_compare > 0)
                {
                    memcpy(query_pos, m_query_pos + (j + 1) * 4, query_pos_size);
                    memcpy(m_query_pos + (j + 1) * 4, m_query_pos + j * 4, query_pos_size);
                    memcpy(m_query_pos + j * 4, query_pos, query_pos_size);
                    exchange = true;
                    m_normalize_type |= E_NORMALIZE_SORT_QUERY;
                    continue;
                }
            }

            if ((pos_1 < m_query_pos[j * 4 + 1])
                && ((pos_2 >= m_query_pos[(j + 1) * 4 + 1])
                || (tolower(m_query[pos_1]) > tolower(m_query[pos_2]))))
            {
                memcpy(query_pos, m_query_pos + (j + 1) * 4, query_pos_size);
                memcpy(m_query_pos + (j + 1) * 4, m_query_pos + j * 4, query_pos_size);
                memcpy(m_query_pos + j * 4, query_pos, query_pos_size);
                exchange = true;
                m_normalize_type |= E_NORMALIZE_SORT_QUERY;
            }
        }
        if (!exchange) return;
    }
}

void Url::Recompose(bool normalize)
{
    int index, read_pos, scheme_count;
    int path_read_pos;
    bool has_port = false;

    m_absolute_path[0] = '\0';

    scheme_count = sizeof(SCHEMES) / sizeof(SSchemeInfo);
    for (int i = 0; i < scheme_count; i++)
    {
        if (SCHEMES[i].m_scheme == m_scheme)
        {
            strcpy(m_absolute_path, SCHEMES[i].m_scheme_string);
            strcat(m_absolute_path, "://");
            if (m_port != SCHEMES[i].m_default_port) has_port = true;
            break;
        }
    }

    /// 域名转小写
    for (int i = 0; i < m_domain_len; i++)
    {
        m_domain[i] = tolower(m_domain[i]);
    }

    /// 忽略域名末尾的'.'
    if ((m_domain_len > 0) && ('.' == m_domain[m_domain_len - 1]))
    {
        m_domain_len--;
        m_domain[m_domain_len] = '\0';
    }

    /// 反转域名
    if ('\0' == m_reversed_domain[0])
    {
        ReverseDomain(m_domain, m_domain_len, m_reversed_domain);
    }

    size_t write_pos = strlen(m_absolute_path);
    memcpy(m_absolute_path + write_pos, m_domain, m_domain_len);
    write_pos += m_domain_len;
    if (has_port)
    {
        sprintf(m_absolute_path + write_pos, ":%u", m_port);
        write_pos = (int)strlen(m_absolute_path);
    }

    m_relative_path_start_pos = write_pos;

    if (RemoveDotSegments(m_path, m_path_len))
    {
        m_normalize_type |= E_NORMALIZE_RELATIVE_DOT_PATH;
    }

    path_read_pos = 0;
    m_directory_pos_count = 0;
    while (m_path[path_read_pos])
    {
        if ('/' == m_path[path_read_pos])
        {
            if (m_directory_pos_count >= MAX_DIRECTORY_COUNT)
            {
                m_scheme = E_NO_PROTOCOL;
                break;
            }

            m_directory_pos[m_directory_pos_count++] = path_read_pos;
            m_resource_pos = path_read_pos + 1;
        }
        path_read_pos++;
    }
    if ((m_resource_pos >= 0) && !m_path[m_resource_pos])
    {
        m_resource_pos = -1;
    }

    if (normalize && TruncateResource())
    {
        m_normalize_type |= E_NORMALIZE_TRUNCATE_DEFAULT_PATH;
    }

    memcpy(m_absolute_path + write_pos, m_path, m_path_len);
    write_pos += m_path_len;
    m_portal_path_end_pos = write_pos;

    if (m_has_query)
    {
        m_absolute_path[write_pos++] = '?';
        for (int i = 0; i < m_query_pos_count; i++)
        {
            index = i * 4;
            read_pos = m_query_pos[index];
            while (read_pos < m_query_pos[index + 1])
                m_absolute_path[write_pos++] = m_query[read_pos++];

            if (-1 != m_query_pos[index + 2])
            {
                m_absolute_path[write_pos++] = '=';
                read_pos = m_query_pos[index + 2];

                if (-1 != m_query_pos[index + 3])
                {
                    while (read_pos < m_query_pos[index + 3])
                        m_absolute_path[write_pos++] = m_query[read_pos++];
                }
            }
            m_absolute_path[write_pos++] = '&';
        }
        if (m_query_pos_count > 0) write_pos--;
    }

    m_absolute_path[write_pos] = '\0';
    m_absolute_path_len = write_pos;
}

bool Url::JoinPath(const Url & url)
{
    char path[MAX_PATH_LENGTH];
    int path_len, pos;

    memcpy(path, m_path, m_path_len);
    path[m_path_len] = '\0';
    path_len = m_path_len;
    memcpy(m_path, url.m_path, url.m_path_len);
    m_path[url.m_path_len] = '\0';
    pos = url.m_path_len;
    while ((pos >= 0) && ('/' != m_path[pos])) pos--;

    if (pos >= 0)
    {
        if (pos + 1 + path_len > MAX_PATH_LENGTH - 1)
        {
            return false;
        }
        else
        {
            memcpy(m_path + pos + 1, path, path_len);
            m_path_len = pos + 1 + path_len;
            m_path[m_path_len] = '\0';
        }
    }
    else
    {
        memcpy(m_path, path, path_len);
        m_path[path_len] = '\0';
        m_path_len = path_len;
    }

    return true;
}

bool Url::RemoveDotSegments(char * path, int & path_len)
{
    /***********************************************************************
    * a)
    * All occurrences of "./", where "." is a complete path
    * segment, are removed.
    ***********************************************************************/

    /**************************************************************************
    * b)
    * If the path ends with "." as a complete path segment, that "." is removed.
    ***************************************************************************/

    /***********************************************************
    * c)
    * All occurrences of "<segment>/../", where <segment> is a
    * complete path segment not equal to "..", are removed.
    * Removal of these path segments is performed iteratively,
    * removing the leftmost matching pattern on each iteration,
    * until no matching pattern remains.
    ************************************************************/

    /***********************************************************
    * d)
    * If the path ends with "<segment>/..", where <segment> is a
    * complete path segment not equal to "..", that
    * "<segment>/.." is removed.
    ***********************************************************/

    bool dot_segment_removed = false;
    int path_read_pos, path_write_pos;
    path_read_pos = path_write_pos = 0;

    while (path_read_pos < path_len)
    {
        if (('.' == path[path_read_pos]) && ((0 == path_write_pos)
            || ('/' == path[path_write_pos - 1])))
        {
            if (path_read_pos + 1 < path_len)
            {
                if ('.' == path[path_read_pos + 1])
                {
                    if (path_read_pos + 2 < path_len)
                    {
                        if ('/' == path[path_read_pos + 2])
                        {
                            /// <segment>/../
                            dot_segment_removed = true;
                            if (path_write_pos > 0)
                            {
                                path_write_pos--;
                            }
                            while ((path_write_pos >= 1) && ('/' != path[path_write_pos - 1]))
                            {
                                path_write_pos--;
                            }
                            path_read_pos += 3;
                            if (path_write_pos <= 0)
                            {
                                path_write_pos = 0;
                                path[path_write_pos++] = '/';
                            }
                            continue;
                        }
                    }
                    else
                    {
                        /// <segment>/..
                        dot_segment_removed = true;
                        if (path_write_pos > 0)
                        {
                            path_write_pos--;
                        }
                        while ((path_write_pos >= 1) && ('/' != path[path_write_pos - 1]))
                        {
                            path_write_pos--;
                        }
                        break;
                    }
                }
                else if ('/' == path[path_read_pos + 1])
                {
                    /// <segment>/./
                    dot_segment_removed = true;
                    path_read_pos += 2;
                    continue;
                }
            }
            else
            {
                /// <segment>/.
                dot_segment_removed = true;
                break;
            }
        }

        if (path_write_pos < MAX_PATH_LENGTH - 1)
        {
            path[path_write_pos++] = path[path_read_pos];
        }
        path_read_pos++;
    }

    path[path_write_pos] = '\0';
    path_len = path_write_pos;

    return dot_segment_removed;
}

bool Url::TruncateResource()
{
    if (!m_has_query
        && (1 == m_directory_pos_count)
        && (-1 != m_resource_pos))
    {
        bool match_truncate_type = false;
        int truncate_type = 0;
        int path_read_pos;

        if (0 == STRNCASECMP(m_path + m_resource_pos, "index.", 6))
        {
            path_read_pos = m_resource_pos + 6;
            truncate_type |= E_TRUNCATE_INDEX;
            match_truncate_type = true;
        }
        else if (0 == STRNCASECMP(m_path + m_resource_pos, "default.", 8))
        {
            path_read_pos = m_resource_pos + 8;
            truncate_type |= E_TRUNCATE_DEFAULT;
            match_truncate_type = true;
        }

        if (match_truncate_type)
        {
            match_truncate_type = false;

            if (0 == STRCASECMP(m_path + path_read_pos, "html"))
            {
                truncate_type |= E_TRUNCATE_HTML;
                match_truncate_type = true;
            }
            else if (0 == STRCASECMP(m_path + path_read_pos, "htm"))
            {
                truncate_type |= E_TRUNCATE_HTM;
                match_truncate_type = true;
            }
            else if (0 == STRCASECMP(m_path + path_read_pos, "shtml"))
            {
                truncate_type |= E_TRUNCATE_SHTML;
                match_truncate_type = true;
            }

            if (match_truncate_type)
            {
                m_path[m_resource_pos] = '\0';
                m_path_len = m_resource_pos;
                m_resource_pos = -1;
                m_truncate_type = truncate_type;
                return true;
            }
        }
    }

    return false;
}

bool Url::GetReversedUrl(void * reversed_url, int & reversed_url_len) const
{
    if (!IsValid()) return false;

    char port[10] = { 0 };
    int port_len = 0;
    int scheme_index;
    int scheme_count = sizeof(SCHEMES) / sizeof(SSchemeInfo);
    for (scheme_index = 0; scheme_index < scheme_count; scheme_index++)
    {
        if (SCHEMES[scheme_index].m_scheme == m_scheme)
        {
            if (m_port != SCHEMES[scheme_index].m_default_port)
            {
                sprintf(port, "#%u", m_port);
                port_len = strlen(port);
            }
            break;
        }
    }

    char * pointer = (char *)reversed_url;

#define APPEND_SIMPLE(var) \
    memcpy(pointer, &var, sizeof(var)); \
    pointer += sizeof(var)

#define APPEND_STRING(name, length) \
    memcpy(pointer, name, length); \
    pointer += length

    if (m_is_normalized)
    {
        APPEND_STRING(m_reversed_domain, m_domain_len);
        APPEND_STRING(port, port_len);
        APPEND_STRING("\"", 1);
        APPEND_STRING(SCHEMES[scheme_index].m_scheme_string, strlen(SCHEMES[scheme_index].m_scheme_string));
        APPEND_STRING("\"", 1);
        APPEND_STRING(m_absolute_path + m_relative_path_start_pos, strlen(m_absolute_path + m_relative_path_start_pos));
    }
    else
    {
        Url normalized_url(m_absolute_path, m_absolute_path_len, E_NO_PROTOCOL, true);

        if (!normalized_url.IsValid()) return false;

        APPEND_STRING(normalized_url.m_reversed_domain, normalized_url.m_domain_len);
        APPEND_STRING(port, port_len);
        APPEND_STRING("\"", 1);
        APPEND_STRING(SCHEMES[scheme_index].m_scheme_string, strlen(SCHEMES[scheme_index].m_scheme_string));
        APPEND_STRING("\"", 1);
        APPEND_STRING(normalized_url.m_absolute_path + normalized_url.m_relative_path_start_pos, strlen(normalized_url.m_absolute_path + normalized_url.m_relative_path_start_pos));
    }

#undef APPEND_SIMPLE
#undef APPEND_STRING

    reversed_url_len = pointer - (char *)reversed_url;

    return true;
}

bool Url::LoadReversedUrl(const void * reversed_url, const int reversed_url_len)
{
    Clear();

    m_is_normalized = true;

    char * pointer = (char *)reversed_url;

    int start_pos;
    int pos = 0;

    /// domain
    while ((pos < reversed_url_len) && ('#' != pointer[pos]) && ('"' != pointer[pos])) pos++;
    if (pos >= reversed_url_len) return false;
    m_domain_len = pos;
    if (m_domain_len >= MAX_DOMAIN_LENGTH)
    {
        m_scheme = E_NO_PROTOCOL;
        return false;
    }
    strncpy(m_reversed_domain, pointer, m_domain_len);
    m_reversed_domain[m_domain_len] = '\0';
    ReverseDomain(m_reversed_domain, m_domain_len, m_domain);

    /// port
    if ('#' == pointer[pos])
    {
        start_pos = pos + 1;
        while ((pos < reversed_url_len) && ('"' != pointer[pos])) pos++;
        if (pos >= reversed_url_len) return false;
        pointer[pos] = '\0';
        m_port = atoi(pointer + start_pos);
        pointer[pos] = '"';
    }

    /// scheme
    pos++;
    start_pos = pos;
    while ((pos < reversed_url_len) && ('"' != pointer[pos])) pos++;
    if (pos >= reversed_url_len) return false;
    int scheme_index;
    int scheme_count = sizeof(SCHEMES) / sizeof(SSchemeInfo);
    for (scheme_index = 0; scheme_index < scheme_count; scheme_index++)
    {
        if (0 == strncmp(pointer + start_pos, SCHEMES[scheme_index].m_scheme_string, pos - start_pos))
        {
            m_scheme = SCHEMES[scheme_index].m_scheme;
            if (0 == m_port) m_port = SCHEMES[scheme_index].m_default_port;
            break;
        }
    }
    if (scheme_index == scheme_count)
    {
        m_scheme = E_UNKNOWN_PROTOCOL;
    }

    /// path
    start_pos = pos + 1;
    while ((pos < reversed_url_len) && ('?' != pointer[pos])) pos++;
    m_path_len = pos - start_pos;
    if (m_path_len >= MAX_PATH_LENGTH)
    {
        m_scheme = E_NO_PROTOCOL;
        return false;
    }
    strncpy(m_path, pointer + start_pos, m_path_len);
    m_path[m_path_len] = '\0';

    /// query
    if (pos < reversed_url_len)
    {
        m_has_query = true;
        start_pos = pos + 1;
        m_query_len = reversed_url_len - start_pos;
        if (m_query_len >= MAX_QUERY_LENGTH)
        {
            m_scheme = E_NO_PROTOCOL;
            return false;
        }
        strncpy(m_query, pointer + start_pos, m_query_len);
        m_query[m_query_len] = '\0';

        if (!ParseQuery(m_query, m_query_len))
        {
            m_scheme = E_NO_PROTOCOL;
            return false;
        }
    }

    Recompose(false);

    return true;
}

std::string Url::GetSegmentPath() const
{
    if (!IsValid()) return std::string("");

    if (m_is_normalized)
    {
        if (IsDynamicPage())
        {
            if (-1 != m_portal_path_end_pos)
            {
                return std::string(m_absolute_path, m_portal_path_end_pos + 1);
            }
        }
        else
        {
            const char * last_slash_pos = strrchr(m_absolute_path, '/');
            if (NULL != last_slash_pos)
            {
                return std::string(m_absolute_path, last_slash_pos - m_absolute_path + 1);
            }
        }
    }
    else
    {
        Url normalized_url(m_absolute_path, m_absolute_path_len, E_NO_PROTOCOL, true);

        if (!normalized_url.IsValid()) return std::string("");

        if (normalized_url.IsDynamicPage())
        {
            if (-1 != normalized_url.m_portal_path_end_pos)
            {
                return std::string(normalized_url.m_absolute_path, normalized_url.m_portal_path_end_pos + 1);
            }
        }
        else
        {
            const char * last_slash_pos = strrchr(normalized_url.m_absolute_path, '/');
            if (NULL != last_slash_pos)
            {
                return std::string(normalized_url.m_absolute_path, last_slash_pos - normalized_url.m_absolute_path + 1);
            }
        }
    }

    return std::string("");
}

std::string Url::GetReversedSegmentPath() const
{
    std::string reversed_url = GetReversedUrl();
    std::string::size_type pos;

    if (IsDynamicPage())
    {
        pos = reversed_url.find('?');
        if (std::string::npos != pos)
        {
            return reversed_url.substr(0, pos + 1);
        }
    }
    else
    {
        pos = reversed_url.rfind('/');
        if (std::string::npos != pos)
        {
            return reversed_url.substr(0, pos + 1);
        }
    }

    return std::string("");
}

std::string Url::GetDomainWithPort() const
{
    if (0 == m_domain_len) return std::string("");

    std::string domain(m_domain);

    if (0 != m_port)
    {
        char buff[10];

        int scheme_count = sizeof(SCHEMES) / sizeof(SSchemeInfo);
        for (int i = 0; i < scheme_count; i++)
        {
            if (SCHEMES[i].m_scheme == m_scheme)
            {
                if (m_port != SCHEMES[i].m_default_port)
                {
                    sprintf(buff, ":%u", m_port);
                    domain.append(buff);
                }
                break;
            }
        }
    }

    return domain;
}

std::string Url::GetReversedDomainWithPort() const
{
    if (0 == m_domain_len) return std::string("");

    std::string reversed_domain(m_reversed_domain);

    if (0 != m_port)
    {
        char buff[10];

        int scheme_count = sizeof(SCHEMES) / sizeof(SSchemeInfo);
        for (int i = 0; i < scheme_count; i++)
        {
            if (SCHEMES[i].m_scheme == m_scheme)
            {
                if (m_port != SCHEMES[i].m_default_port)
                {
                    sprintf(buff, "#%u", m_port);
                    reversed_domain.append(buff);
                }
                break;
            }
        }
    }

    return reversed_domain;
}

void Url::GetQuerys(std::map<std::string, std::string> & querys) const
{
    std::string key;
    std::string value;

    for (int i = 0; i < m_query_pos_count; i++)
    {
        key.assign(m_query + m_query_pos[i * 4], m_query_pos[i * 4 + 1] - m_query_pos[i * 4]);

        if ((-1 != m_query_pos[i * 4 + 2]) && (-1 != m_query_pos[i * 4 + 3]))
        {
            value.assign(m_query + m_query_pos[i * 4 + 2], m_query_pos[i * 4 + 3] - m_query_pos[i * 4 + 2]);
        }
        else
        {
            value.clear();
        }

        querys[key] = value;
    }
}

int Url::GetQuerys(const char ** querys) const
{
    for (int i = 0; i < m_query_pos_count; i++)
    {
        querys[i * 4] = m_query + m_query_pos[i * 4];
        querys[i * 4 + 1] = m_query + m_query_pos[i * 4 + 1];

        if (-1 != m_query_pos[i * 4 + 2])
        {
            querys[i * 4 + 2] = m_query + m_query_pos[i * 4 + 2];

            if (-1 != m_query_pos[i * 4 + 3])
            {
                querys[i * 4 + 3] = m_query + m_query_pos[i * 4 + 3];
            }
            else
            {
                querys[i * 4 + 3] = querys[i * 4 + 2];
            }
        }
        else
        {
            querys[i * 4 + 2] = querys[i * 4 + 3] = NULL;
        }
    }

    return m_query_pos_count;
}

std::string Url::GetDirectory() const
{
    if (m_directory_pos_count == 1)
    {
        return std::string("/");
    }
    else if (m_directory_pos_count > 1)
    {
        return std::string(m_path + m_directory_pos[0], m_directory_pos[m_directory_pos_count - 1] - m_directory_pos[0] + 1);
    }

    return std::string("");
}

void Url::GetDirectory(const char *& begin, const char *& end) const
{
    if (0 == m_directory_pos_count)
    {
        begin = end = NULL;
    }
    else
    {
        begin = m_path + m_directory_pos[0];
        end = m_path + m_directory_pos[m_directory_pos_count - 1];
    }
}

int Url::GetDirectorys(const char ** directorys) const
{
    if (m_directory_pos_count > 0)
    {
        if (1 == m_directory_pos_count)
        {
            directorys[0] = directorys[1] = m_path + m_directory_pos[0];
        }
        else
        {
            for (int i = 0; i < m_directory_pos_count - 1; i++)
            {
                directorys[i * 2] = m_path + m_directory_pos[i];
                directorys[i * 2 + 1] = m_path + m_directory_pos[i + 1];
            }

            return m_directory_pos_count - 1;
        }
    }

    return m_directory_pos_count;
}

std::string Url::GetResourceSuffix() const
{
    const char * suffix_ptr = GetResourceSuffixPtr();

    if (NULL != suffix_ptr)
    {
        return std::string(suffix_ptr);
    }

    return std::string("");
}

const char * Url::GetResourceSuffixPtr() const
{
    if (-1 != m_resource_pos)
    {
        int pos = m_path_len - 1;
        while (pos >= m_resource_pos)
        {
            if ('.' == m_path[pos])
            {
                if (!m_path[pos + 1])
                {
                    /// exclude resource end with '.'
                    break;
                }
                return (m_path + pos + 1);
            }
            pos--;
        }
    }

    return NULL;
}

std::string Url::EncodeUrl(const std::string & url, int flag)
{
    std::string encoded_url;
    std::string::size_type url_len = url.size();
    std::string::size_type pos;
    unsigned char ch;

    for (pos = 0; pos < url_len; pos++)
    {
        ch = url.at(pos);

        if ('%' == ch)
        {
            if (((pos + 2) < url_len)
                && isxdigit((unsigned char)url.at(pos + 1))
                && isxdigit((unsigned char)url.at(pos + 2)))
            {
                encoded_url += ch;
            }
            else
            {
                encoded_url += '%';
                encoded_url += XNUM_TO_DIGIT(ch >> 4);
                encoded_url += XNUM_TO_DIGIT(ch & 0xf);
            }
        }
        else if (URL_UNSAFE_CHAR (ch))
        {
            if ((!(flag & E_ENCODE_RESERVED_CHAR) && URL_RESERVED_CHAR (ch))
                || ((flag & E_NOT_ENCODE_EXTENDED_CHAR_SET) && (ch > 127)))
            {
                encoded_url += ch;
            }
            else
            {
                encoded_url += '%';
                encoded_url += XNUM_TO_DIGIT(ch >> 4);
                encoded_url += XNUM_TO_DIGIT(ch & 0xf);
            }
        }
        else
        {
            if ((flag & E_ENCODE_RESERVED_CHAR) && URL_RESERVED_CHAR(ch))
            {
                encoded_url += '%';
                encoded_url += XNUM_TO_DIGIT(ch >> 4);
                encoded_url += XNUM_TO_DIGIT(ch & 0xf);
            }
            else
            {
                encoded_url += ch;
            }
        }
    }

    return encoded_url;
}

std::string Url::DecodeUrl(const std::string & url, int flag)
{
    unsigned char ch;
    std::string::size_type pos;
    std::string decoded_url;
    const char * url_string = url.c_str();
    unsigned int url_len = url.size();

    for (pos = 0; pos < url_len; pos++)
    {
        if (('%' == url_string[pos]) && (pos + 2 < url_len)
            && isxdigit((unsigned char)(url_string[pos + 1]))
            && isxdigit((unsigned char)(url_string[pos + 2])))
        {
            ch = X2DIGITS_TO_NUM(url_string[pos + 1], url_string[pos + 2]);

            if ((!(flag & E_DECODE_RESERVED_CHAR) && URL_RESERVED_CHAR(ch))
                || (!(flag & E_DECODE_PERCENT_SIGN_CHAR) && ('%' == ch)))
            {
                decoded_url.push_back(url_string[pos]);
                decoded_url.push_back(url_string[pos + 1]);
                decoded_url.push_back(url_string[pos + 2]);
            }
            else
            {
                decoded_url.push_back(ch);
            }

            pos += 2;
        }
        else
        {
            decoded_url.push_back(url_string[pos]);
        }
    }

    return decoded_url;
}

char * Url::ReverseDomain(const char * src, const int src_len, char * dest)
{
    if ((NULL == src) || (NULL == dest))
    {
        return NULL;
    }

    int write_len;
    int write_pos = 0;
    int port_pos = -1;
    int end_pos = src_len;

    for (int pos = src_len - 1; pos > 0; pos--)
    {
        if ('.' == src[pos])
        {
            write_len = end_pos - pos - 1;
            memcpy(dest + write_pos, src + pos + 1, write_len);
            write_pos += write_len;
            dest[write_pos++] = '.';
            end_pos = pos;
        }
        else if (':' == src[pos])
        {
            write_pos = 0;
            end_pos = pos;
            port_pos = pos;
        }
    }

    if ('.' != src[0])
    {
        write_len = end_pos;
        memcpy(dest + write_pos, src, write_len);
        write_pos += write_len;
    }
    else
    {
        write_len = end_pos - 1;
        memcpy(dest + write_pos, src + 1, write_len);
        write_pos += write_len ;
        dest[write_pos++] = '.';
    }

    if (-1 != port_pos)
    {
        write_len = src_len - port_pos;
        memcpy(dest + write_pos, src + port_pos,  write_len);
        write_pos += write_len ;
    }

    dest[write_pos] = '\0';

    return dest;
}

unsigned long long Url::GetUrlHash() const
{
    if (!IsValid()) return 0;

    if (0 == m_url_hash)
    {
        int pos;
        char url[MAX_URL_LENGTH];

        if (m_is_normalized)
        {
            pos = 0;
            while (m_absolute_path[pos])
            {
                /// 计算UrlHash取转小写之后的URL
                url[pos] = tolower(m_absolute_path[pos]);
                pos++;
            }

            /// 计算UrlHash不包括静态目录页末尾的'/'
            if (!IsDynamicPage()
                && (m_directory_pos_count > 1)
                && (pos > 0)
                && ('/' == url[pos - 1]))
            {
                pos--;
            }

            url[pos] = '\0';
        }
        else
        {
            Url normalized_url(m_absolute_path, m_absolute_path_len, E_NO_PROTOCOL, true);

            if (!normalized_url.IsValid()) return 0;

            pos = 0;
            while (normalized_url.m_absolute_path[pos])
            {
                /// 计算UrlHash取转小写之后的URL
                url[pos] = tolower(normalized_url.m_absolute_path[pos]);
                pos++;
            }

            /// 计算UrlHash不包括静态目录页末尾的'/'
            if (!(normalized_url.IsDynamicPage())
                && (normalized_url.m_directory_pos_count > 1)
                && (pos > 0)
                && ('/' == url[pos - 1]))
            {
                pos--;
            }

            url[pos] = '\0';
        }

        m_url_hash = MD5::GetHash64(url);
    }

    return m_url_hash;
}

unsigned long long Url::GetDomainHash() const
{
    if (!IsValid()) return 0;

    if (0 == m_domain_hash)
    {
        if (m_is_normalized)
        {
            m_domain_hash = MD5::GetHash64(m_domain);
        }
        else
        {
            Url normalized_url(m_absolute_path, m_absolute_path_len, E_NO_PROTOCOL, true);

            if (!normalized_url.IsValid()) return 0;

            m_domain_hash = MD5::GetHash64(normalized_url.m_domain);
        }
    }

    return m_domain_hash;
}

unsigned long long Url::GetDomainWithPortHash() const
{
    if (!IsValid()) return 0;

    if (0 == m_domain_with_port_hash)
    {
        std::string domain_with_port;

        if (m_is_normalized)
        {
            domain_with_port = GetDomainWithPort();
        }
        else
        {
            Url normalized_url(m_absolute_path, m_absolute_path_len, E_NO_PROTOCOL, true);

            if (!normalized_url.IsValid()) return 0;

            domain_with_port = normalized_url.GetDomainWithPort();
        }
        
        m_domain_with_port_hash = MD5::GetHash64(domain_with_port.c_str());
    }

    return m_domain_with_port_hash;
}

unsigned long long Url::GetPortalPathHash() const
{
    if (!IsValid()) return 0;

    unsigned long long hash = 0;
    char portal_path[MAX_URL_LENGTH];
    int read_pos, write_pos;

    if (m_is_normalized)
    {
        if (-1 != m_portal_path_end_pos)
        {
            read_pos = write_pos = 0;
            while (read_pos < m_portal_path_end_pos)
            {
                portal_path[write_pos++] = tolower(m_absolute_path[read_pos++]);
            }
            portal_path[write_pos] = '\0';
        }
    }
    else
    {
        Url normalized_url(m_absolute_path, m_absolute_path_len, E_NO_PROTOCOL, true);

        if (!normalized_url.IsValid()) return 0;

        if (-1 != normalized_url.m_portal_path_end_pos)
        {
            read_pos = write_pos = 0;
            while (read_pos < normalized_url.m_portal_path_end_pos)
            {
                portal_path[write_pos++] = tolower(normalized_url.m_absolute_path[read_pos++]);
            }
            portal_path[write_pos] = '\0';
        }
    }

    hash = MD5::GetHash64(portal_path);

    return hash;
}

unsigned long long Url::GetReversedSegmentPathHash() const
{
    if (!IsValid()) return 0;

    if (0 == m_reversed_segment_path_hash)
    {
        std::string reversed_segment_path = GetReversedSegmentPath();
        m_reversed_segment_path_hash = MD5::GetHash64(reversed_segment_path.c_str());
    }

    return m_reversed_segment_path_hash;
}
