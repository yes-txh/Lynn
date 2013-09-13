/**
 * @file uri.cc
 * @brief 
 * @author welkin
 * @date 2011-04-20
 */

#include <stdio.h>

#include "url_util.hpp"
#include "uri.hpp"

using namespace common::web;
using namespace web::url;

std::string UrlUtil::ReverseUrl(const std::string & url)
{
    Uri uri(url);

    if (uri.IsAbsolute())
    {
        std::string reversed_url;

        /// append reversed domain
        reversed_url = ReverseDomain(uri.m_host);

        /// append port
        if (uri.m_defined_flag & Uri::kDefinedPort)
        {
            reversed_url.append(1, '#');
            char port[8];
            sprintf(port, "%d", uri.m_port);
            reversed_url.append(port);
        }

        /// append scheme
        reversed_url.append(1, '"');
        reversed_url.append(uri.m_scheme);
        reversed_url.append(1, '"');

        /// append relative_path
        reversed_url.append(uri.m_relative_uri);

        return reversed_url;
    }

    return std::string("");
}

std::string UrlUtil::ReverseDomain(const std::string & domain)
{
    if (domain.empty() || (domain.length() >= 1024))
    {
        return std::string("");
    }

    char reversed_domain[1024];

    if (NULL != ReverseDomain(domain.c_str(), domain.length(), reversed_domain))
    {
        return std::string(reversed_domain);
    }

    return std::string("");
}

char * UrlUtil::ReverseDomain(const char * src, const int src_len, char * dest)
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

std::string UrlUtil::GetParentOfReversedSegmentPath(const std::string & segment_path)
{
    if (segment_path.length() > 1)
    {
        std::string::size_type pos = segment_path.rfind('/', segment_path.length() - 2);

        if (std::string::npos != pos)
        {
            return segment_path.substr(0, pos + 1);
        }
    }

    return std::string("");
}

bool UrlUtil::GetPrefixsOfUrl(const Url & url, std::vector<std::string> & url_prefixs)
{
    if (url.IsValid())
    {
        std::string url_prefix(url.m_absolute_path, url.m_relative_path_start_pos);

        for (int i = 0; i < url.m_directory_pos_count; ++i)
        {
            url_prefixs.push_back(url_prefix + std::string(url.m_path, url.m_directory_pos[i] + 1));
        }

        return true;
    }

    return false;
}

bool UrlUtil::GetPrefixsOfUrl(const std::string & url, std::vector<std::string> & url_prefixs)
{
    Url url_obj;

    if (url_obj.Load(url.c_str(), url.length()))
    {
        return UrlUtil::GetPrefixsOfUrl(url_obj, url_prefixs);
    }

    return false;
}

bool UrlUtil::GetPrefixsOfReversedUrl(const std::string & reversed_url, std::vector<std::string> & url_prefixs)
{
    Url url_obj;

    if (url_obj.LoadReversedUrl(reversed_url))
    {
        return UrlUtil::GetPrefixsOfUrl(url_obj, url_prefixs);
    }

    return false;
}
