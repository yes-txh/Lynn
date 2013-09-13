#ifndef __COMMON_WEB_URI_HPP__
#define __COMMON_WEB_URI_HPP__

/**
 * @file uri.hpp
 * @brief 
 * @author welkin
 * @date 2011-04-20
 */

#include <string>

namespace common {
namespace web {

class Uri
{
public:
    Uri();
    Uri(const std::string & uri_string);
    virtual ~Uri();

    virtual void Clear();
    virtual bool Load(const std::string & uri_string);
    virtual bool Load(const std::string & uri_string, const std::string & base_uri_string);
    virtual bool Merge(Uri & base_uri);
    virtual bool IsAbsolute();
    virtual bool IsValid();

protected:
    virtual bool Parse(const std::string & uri_string);
    virtual void Normalize();
    virtual void Recompose();

    virtual void SeparateComponents(const std::string & uri_string);
    virtual bool ParseScheme();
    virtual bool ParseAuthority();
    virtual bool ParsePath();
    virtual bool ParseQuery();
    virtual bool ParseFragment();
    virtual void AssignPort(const std::string & port);
    virtual void RemoveDotSegments(std::string & path);

public:
    std::string m_scheme;
    std::string m_authority;
    std::string m_user;
    std::string m_password;
    std::string m_host;
    int m_port;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;

    std::string m_absolute_uri;
    std::string m_relative_uri;

    enum
    {
        kDefinedScheme = 0x01,
        kDefinedAuthority = 0x02,
        kDefinedUser = 0x04,
        kDefinedPassword = 0x08,
        kDefinedHost = 0x10,
        kDefinedPort = 0x20,
        kDefinedPath = 0x40,
        kDefinedQuery = 0x80,
        kDefinedFragment = 0x100
    };

    int m_defined_flag;
    bool m_parse_succ_flag;
    bool m_display_port_flag;
};


class HttpUri : public Uri
{
public:
    HttpUri();
    HttpUri(const std::string & uri_string);
    virtual ~HttpUri();

    virtual bool IsAbsolute();
    virtual bool IsValid();

protected:
    virtual void Normalize();

    virtual bool ParseScheme();
    virtual void AssignPort(const std::string & port);
};


class HttpsUri : public Uri
{
public:
    HttpsUri();
    HttpsUri(const std::string & uri_string);
    virtual ~HttpsUri();

    virtual bool IsAbsolute();
    virtual bool IsValid();

protected:
    virtual void Normalize();

    virtual bool ParseScheme();
    virtual void AssignPort(const std::string & port);
};


class FtpUri : public Uri
{
public:
    FtpUri();
    FtpUri(const std::string & uri_string);
    virtual ~FtpUri();

    virtual bool IsAbsolute();
    virtual bool IsValid();

protected:
    virtual void Normalize();

    virtual bool ParseScheme();
    virtual void AssignPort(const std::string & port);
};


class UriFactory
{
public:
    UriFactory();
    ~UriFactory();

    Uri * GetUri(const std::string & uri_string);
};

} // namespace web
} // namespace common


#endif