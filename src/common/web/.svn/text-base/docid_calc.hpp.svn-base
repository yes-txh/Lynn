#ifndef COMMON_DOCID_CALC_HPP
#define COMMON_DOCID_CALC_HPP

/**
 * @file docid.hpp
 * @brief 
 * @author welkin
 * @date 2011-06-02
 */

#include <string>
#include <common/web/url.hpp>
#include <common/web/hashval.hpp>
#include <common/crypto/hash/md5.hpp>

using namespace web::url;

typedef Md5Hash96 Docid;

class DocidCalc
{
public:
    Md5Hash96 CalcDocid96bit(const std::string & url_string) const
    {
        Url url(url_string.c_str(), url_string.length(), E_NO_PROTOCOL, true);
        return CalcDocid96bit(url);
    }

    Md5Hash96 CalcDocid96bit(const Url & url) const
    {
        Md5Hash96 docid;
        std::string normalize_url = GetNormalizedUrl(url);

        if (!normalize_url.empty())
        {
            Md5Hash md5;
            MD5::Digest(normalize_url.c_str(), md5.m_value);
            memcpy(docid.m_value, md5.m_value, sizeof(docid.m_value));
        }

        return docid;
    }

    unsigned long long CalcDocid64bit(const std::string & url_string) const
    {
        Url url(url_string.c_str(), url_string.length(), E_NO_PROTOCOL, true);
        return CalcDocid64bit(url);
    }

    unsigned long long CalcDocid64bit(const Url & url) const
    {
        unsigned long long docid = 0;
        std::string normalize_url = GetNormalizedUrl(url);

        if (!normalize_url.empty())
        {
            MD5::Hash64(normalize_url.c_str(), docid);
        }

        return docid;
    }

    std::string GetNormalizedUrl(const std::string & url_string) const
    {
        Url url(url_string.c_str(), url_string.length(), E_NO_PROTOCOL, true);
        return GetNormalizedUrl(url);
    }

    std::string GetNormalizedUrl(const Url & url) const
    {
        if (!url.IsValid())
        {
            return std::string("");
        }

        int pos;
        char url_string[MAX_URL_LENGTH];
        const char * src_url;
        bool is_directory_page;

        if (url.m_is_normalized)
        {
            src_url = url.m_absolute_path;
            is_directory_page = !url.IsDynamicPage() && (url.m_directory_pos_count > 1)
                && (url.m_absolute_path_len > 0) && ('/' == url.m_absolute_path[url.m_absolute_path_len - 1]);
        }
        else
        {
            Url normalized_url(url.m_absolute_path, url.m_absolute_path_len, E_NO_PROTOCOL, true);
            if (normalized_url.IsValid())
            {
                src_url = normalized_url.m_absolute_path;
                is_directory_page = !normalized_url.IsDynamicPage() && (normalized_url.m_directory_pos_count > 1)
                    && (normalized_url.m_absolute_path_len > 0) && ('/' == normalized_url.m_absolute_path[normalized_url.m_absolute_path_len - 1]);
            }
            else
            {
                return std::string("");
            }
        }

        pos = 0;
        while (src_url[pos])
        {
            /// 计算UrlHash取转小写之后的URL
            url_string[pos] = tolower(src_url[pos]);
            pos++;
        }

        /// 计算UrlHash不包括静态目录页末尾的'/'
        if (is_directory_page)
        {
            pos--;
        }
        url_string[pos] = '\0';

        return std::string(url_string);
    }
};

#endif
