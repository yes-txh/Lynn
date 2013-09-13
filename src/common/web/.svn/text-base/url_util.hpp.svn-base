#ifndef COMMON_WEB_URL_UTIL_HPP
#define COMMON_WEB_URL_UTIL_HPP

/**
 * @file url_util.hpp
 * @brief 
 * @author welkin
 * @date 2011-04-20
 */

#include <string>
#include <vector>

#include "url.hpp"

namespace web {
namespace url {

class UrlUtil
{
public:
    static std::string ReverseUrl(const std::string & url);

    static std::string ReverseDomain(const std::string & domain);
    static char * ReverseDomain(const char * src, const int src_len, char * dest);

    static std::string GetParentOfReversedSegmentPath(const std::string & segment_path);

    static bool GetPrefixsOfUrl(const Url & url, std::vector<std::string> & url_prefixs);
    static bool GetPrefixsOfUrl(const std::string & url, std::vector<std::string> & url_prefixs);
    static bool GetPrefixsOfReversedUrl(const std::string & reversed_url, std::vector<std::string> & url_prefixs);
    
};

}; /// namespace url
}; /// namespace web

#endif /// COMMON_WEB_URL_UTIL_HPP
