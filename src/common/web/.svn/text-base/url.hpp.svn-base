#ifndef COMMON_WEB_URL_HPP
#define COMMON_WEB_URL_HPP

/**
 * @file url.hpp
 * @brief 
 * @author welkin
 * @date 2011-04-20
 */

#include <string.h>
#include <utility>
#include <string>
#include <vector>
#include <map>
#include <common/base/platform_features.hpp>

namespace web {
namespace url {

/////////////////////////////////////////////////////
//                Consts Defination                //
/////////////////////////////////////////////////////

/// Url长度限制
const int MAX_URL_LENGTH      = 1024;
const int MAX_DOMAIN_LENGTH   = 50;
const int MAX_PATH_LENGTH     = 480;
const int MAX_QUERY_LENGTH    = 480;
const int MAX_DIRECTORY_COUNT = 20;
const int MAX_QUERY_COUNT     = 20;

/// 协议类型枚举
enum EScheme
{
    E_NO_PROTOCOL,
    E_HTTP_PROTOCOL,
    E_HTTPS_PROTOCOL,
    E_UNKNOWN_PROTOCOL
};

/// 协议信息
struct SSchemeInfo
{
    EScheme m_scheme;                   /// 协议类型
    char m_scheme_string[6];            /// 协议名称的字符串
    unsigned short m_default_port;      /// 协议的默认端口
};

/// 规范化类型
enum ENormalizeType
{
    E_NORMALIZE_DECIMAL_ENCODE = 0x01,
    E_NORMALIZE_HEXADECIMAL_ENCODE = 0x02,
    E_NORMALIZE_HTML_ENCODE = 0x04,
    E_NORMALIZE_BACKSLASH = 0x08,
    E_NORMALIZE_DUPLICATE_SLASH = 0x10,
    E_NORMALIZE_RELATIVE_DOT_PATH = 0x20,
    E_NORMALIZE_TRUNCATE_DEFAULT_PATH = 0x40,
    E_NORMALIZE_SORT_QUERY = 0x80,
    E_NORMALIZE_DUPLICATE_QUERY = 0x100
};

/// 默认路径截断类型
enum ETruncateType
{
    E_TRUNCATE_HTM = 0x01,
    E_TRUNCATE_HTML = 0x02,
    E_TRUNCATE_SHTML = 0x03,

    E_TRUNCATE_INDEX = 0x10,
    E_TRUNCATE_DEFAULT = 0x20,

    E_TRUNCATE_INDEX_HTM = 0x11,
    E_TRUNCATE_INDEX_HTML = 0x12,
    E_TRUNCATE_INDEX_SHTML = 0x13,

    E_TRUNCATE_DEFAULT_HTM = 0x21,
    E_TRUNCATE_DEFAULT_HTML = 0x22,
    E_TRUNCATE_DEFAULT_SHTML = 0x23
};

/////////////////////////////////////////////////////
//                Class Declaration                //
/////////////////////////////////////////////////////

////////////////// Url //////////////////

class Url
{
public:
    Url();

    /// 由一个URL字符串构造一个Url对象
    Url(const char * url, const int url_len,
        EScheme default_scheme = E_NO_PROTOCOL,
        bool normalize = false);

    /// 由一个相对URL和基URL构造一个Url对象
    Url(const char * url, const int relative_url_len,
        const char * base_url, const int base_url_len,
        bool normalize = false);

    /// 由一个相对URL和基Url对象构造一个Url对象
    Url(const char * url, const int relative_url_len,
        const Url & base_url,
        bool normalize = false);

    /// 清空对象
    void Clear();

    /// 由一个URL字符串构造一个Url对象
    bool Load(const char * url, const int url_len,
              EScheme default_scheme = E_NO_PROTOCOL,
              bool normalize = false);

    /// 由一个相对URL和基URL构造一个Url对象
    bool Load(const char * url, const int relative_url_len,
              const char * base_url, const int base_url_len,
              bool normalize = false);

    /// 由一个相对URL和基CUrl对象构造一个Url对象
    bool Load(const char * url, const int relative_url_len,
              const Url & base_url,
              bool normalize = false);

    /// 获取反转URL
    bool GetReversedUrl(void * reversed_url, int & reversed_url_len) const;

    inline std::string GetReversedUrl() const
    {
        char reversed_url[MAX_URL_LENGTH];
        int reversed_url_len;

        if (GetReversedUrl(reversed_url, reversed_url_len))
        {
            return std::string(reversed_url, reversed_url_len);
        }

        return std::string("");
    }

    /// 由反转URL构造一个Url对象
    bool LoadReversedUrl(const void * reversed_url, const int reversed_url_len);

    inline bool LoadReversedUrl(const std::string & reversed_url)
    {
        return LoadReversedUrl(reversed_url.c_str(), reversed_url.length());
    }

    /// URL是否有效
    inline bool IsValid() const
    {
        return ((E_NO_PROTOCOL != m_scheme) && (E_UNKNOWN_PROTOCOL != m_scheme) && (m_domain_len > 0));
    }

    /// URL是否为动态页面
    inline bool IsDynamicPage() const
    {
        return m_has_query;
    }

    /// 获取规范化后的URL
    inline std::string GetNormalizedUrl() const
    {
        if (!IsValid())
        {
            return std::string("");
        }
        else if (m_is_normalized)
        {
            return GetAbsolutePath();
        }
        else
        {
            Url normalized_url(m_absolute_path, m_absolute_path_len, E_NO_PROTOCOL, true);
            return normalized_url.GetAbsolutePath();
        }
    }

    /// 获取绝对路径
    inline std::string GetAbsolutePath() const
    {
        return std::string(m_absolute_path);
    }

    inline const char * GetAbsolutePathPtr() const
    {
        return m_absolute_path;
    }

    DEPRECATED_BY(GetAbsolutePathPtr)
    inline const char * getAbsolutePath() const
    {
        return GetAbsolutePathPtr();
    }

    /// 获取相对路径
    inline std::string GetRelativePath() const
    {
        return (-1 == m_relative_path_start_pos) ?
               std::string("") : std::string(m_absolute_path + m_relative_path_start_pos);
    }

    inline const char * GetRelativePathPtr() const
    {
        return (-1 == m_relative_path_start_pos) ?
               NULL : (m_absolute_path + m_relative_path_start_pos);
    }

    DEPRECATED_BY(GetRelativePathPtr)
    inline const char * getRelativePath() const
    {
        return GetRelativePathPtr();
    }

    /// 获取入口路径
    inline std::string GetPortalPath() const
    {
        return (-1 == m_portal_path_end_pos) ?
               std::string("") : std::string(m_absolute_path, m_portal_path_end_pos);
    }

    /// 获取片段路径
    std::string GetSegmentPath() const;

    /// 获取反转片段路径
    std::string GetReversedSegmentPath() const;

    /// 获取首页URL
    inline std::string GetHomepageURL() const
    {
        return (-1 == m_relative_path_start_pos) ?
               std::string("") : std::string(m_absolute_path, m_relative_path_start_pos + 1);
    }

    /// 获取协议
    inline EScheme GetScheme() const
    {
        return m_scheme;
    }

    /// 获取域名
    inline std::string GetDomain() const
    {
        return std::string(m_domain);
    }

    inline const char * GetDomainPtr() const
    {
        return m_domain;
    }

    DEPRECATED_BY(GetDomainPtr)
    inline const char * getDomain() const
    {
        return GetDomainPtr();
    }

    /// 获取带端口的域名
    std::string GetDomainWithPort() const;

    /// 获取反转域名
    inline std::string GetReversedDomain() const
    {
        return std::string(m_reversed_domain);
    }

    inline const char * GetReversedDomainPtr() const
    {
        return m_reversed_domain;
    }

    /// 获取带端口的反转域名
    std::string GetReversedDomainWithPort() const;

    /// 获取端口
    inline unsigned short GetPort() const
    {
        return m_port;
    }

    /// 获取路径
    inline std::string GetPath() const
    {
        return std::string(m_path);
    }

    inline const char * GetPathPtr() const
    {
        return m_path;
    }

    DEPRECATED_BY(GetPathPtr)
    inline const char * getPath() const
    {
        return GetPathPtr();
    }

    /// 获取规范化类型
    inline int GetNormalizeType() const
    {
        return m_normalize_type;
    }

    /// 获取截断类型
    inline int GetTruncateType() const
    {
        return m_truncate_type;
    }

    /// 获取查询
    void GetQuerys(std::map<std::string, std::string> & querys) const;

    int GetQuerys(const char ** querys) const;

    /// 获取目录
    std::string GetDirectory() const;

    void GetDirectory(const char *& begin, const char *& end) const;

    int GetDirectorys(const char ** directorys) const;

    /// 获取资源名
    inline std::string GetResource() const
    {
        return (-1 == m_resource_pos) ?
               std::string("") : std::string(m_path + m_resource_pos);
    }

    inline const char * GetResourcePtr() const
    {
        return (-1 == m_resource_pos) ?
               NULL : (m_path + m_resource_pos);
    }

    DEPRECATED_BY(GetResourcePtr)
    inline const char * getResource() const
    {
        return GetResourcePtr();
    }

    /// 获取资源后缀名
    std::string GetResourceSuffix() const;

    const char * GetResourceSuffixPtr() const;

    DEPRECATED_BY(GetResourceSuffixPtr)
    inline const char * getResourceSuffix() const
    {
        return GetResourceSuffixPtr();
    }

    /// 获取URL的路径深度
    inline unsigned int GetPathDepth() const
    {
        return m_directory_pos_count;
    }

public:
    /// 对URL进行编码，默认不编码保留字
    /// 若需要将URL作为文件名，可选择编码保留字
    /// 若需要保留显示中文字符，可选择不编码扩展字符集
    enum
    {
        E_ENCODE_RESERVED_CHAR = 0x01,          /// 编码保留字
        E_NOT_ENCODE_EXTENDED_CHAR_SET = 0x02   /// 不编码扩展字符集(ASCII值大于127的字符)
    };
    static std::string EncodeUrl(const std::string & url, int flag = 0);

    /// 对URL进行解码，默认不解码保留字和百分号字符
    enum
    {
        E_DECODE_RESERVED_CHAR = 0x10,      /// 解码保留字
        E_DECODE_PERCENT_SIGN_CHAR = 0x20   /// 解码百分号字符
    };
    static std::string DecodeUrl(const std::string & url, int flag = 0);

    /// 反转域名
    static char * ReverseDomain(const char * src, const int src_len, char * dest);

public:
    /// 获取URL哈希
    unsigned long long GetUrlHash() const;

    /// 获取域名哈希
    unsigned long long GetDomainHash() const;
    unsigned long long GetDomainWithPortHash() const;

    /// 获取入口路径哈希
    unsigned long long GetPortalPathHash() const;

    /// 获取反转片段路径哈希
    unsigned long long GetReversedSegmentPathHash() const;

private:
    /// 预处理
    bool Preprocess(const char * url, const int url_len, bool decode_percent_encoded);

    /// 解析URL字符串
    bool Parse(const char * url, const int url_len, EScheme default_scheme, bool normalize);

    /// 解析查询
    bool ParseQuery(const char * query, const int query_len);

    /// 排序查询
    void SortQuery();

    /// 重组URL
    void Recompose(bool normalize);

    /// 拼接路径
    bool JoinPath(const Url & url);

    /// 修整路径
    bool RemoveDotSegments(char * path, int & path_len);

    /// 截断资源名称
    bool TruncateResource();

    /// 复制域名信息
    inline void CopyDomainInfo(const Url & url)
    {
        memcpy(m_domain, url.m_domain, url.m_domain_len);
        m_domain[url.m_domain_len] = '\0';
        m_domain_len = url.m_domain_len;
        m_port = url.m_port;
    }

    /// 复制路径信息
    inline void CopyPathInfo(const Url & url)
    {
        memcpy(m_path, url.m_path, url.m_path_len);
        m_path[url.m_path_len] = '\0';
        m_path_len = url.m_path_len;
        memcpy(m_directory_pos, url.m_directory_pos, sizeof(url.m_directory_pos));
        m_directory_pos_count = url.m_directory_pos_count;
        m_resource_pos = url.m_resource_pos;
    }

    /// 复制查询信息
    inline void CopyQueryInfo(const Url & url)
    {
        memcpy(m_query, url.m_query, url.m_query_len);
        m_query[url.m_query_len] = '\0';
        m_query_len = url.m_query_len;
        memcpy(m_query_pos, url.m_query_pos, sizeof(url.m_query_pos));
        m_query_pos_count = url.m_query_pos_count;
        m_has_query = url.m_has_query;
    }

    /// 判断字符串是否为百分号编码
    inline bool IsPercentEncoded(char * str) const
    {
        if (('%' != str[0]) 
            || !isxdigit((unsigned char)str[1])
            || !isxdigit((unsigned char)str[2]))
        {
            return false;
        }

        return true;
    }

public:
    /// <scheme>://<authority>/<path>;<params>?<query>#<fragment>
    EScheme	m_scheme;                           /// 协议
    char m_domain[MAX_DOMAIN_LENGTH];           /// 域名
    unsigned short m_port;                      /// 端口
    char m_path[MAX_PATH_LENGTH];               /// 路径
    char m_query[MAX_QUERY_LENGTH];             /// 查询
    bool m_has_query;                           /// 是否有查询

    char m_absolute_path[MAX_URL_LENGTH];
    char m_reversed_domain[MAX_DOMAIN_LENGTH];

    int m_domain_len;
    int m_path_len;
    int m_query_len;
    int m_absolute_path_len;

    int m_directory_pos[MAX_DIRECTORY_COUNT];
    int m_directory_pos_count;
    int m_query_pos[MAX_QUERY_COUNT * 4];
    int m_query_pos_count;
    int m_resource_pos;
    int m_relative_path_start_pos;
    int m_portal_path_end_pos;

    bool m_is_normalized;
    int m_normalize_type;
    int m_truncate_type;

    mutable unsigned long long m_url_hash;
    mutable unsigned long long m_domain_hash;
    mutable unsigned long long m_domain_with_port_hash;
    mutable unsigned long long m_reversed_segment_path_hash;
};

DEPRECATED_BY(Url) typedef Url CUrl;

}; /// namespace url
}; /// namespace web

#endif /// COMMON_WEB_URL_HPP
