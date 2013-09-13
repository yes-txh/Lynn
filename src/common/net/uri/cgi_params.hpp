// Copyright (c) 2011, Tencent.com
// All rights reserved.

/// @file cgi_params.h
/// @brief CGI paramters handling
/// @date  03/30/2011 05:23:27 PM
/// @author CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_NET_URI_CGI_PARAMS_HPP
#define COMMON_NET_URI_CGI_PARAMS_HPP

#include <string>
#include <vector>
#include "common/base/string/string_piece.hpp"

struct CgiParam
{
public:
    CgiParam() {}
    CgiParam(const std::string& aname, const std::string& avalue)
        : name(aname), value(avalue)
    {
    }
public:
    std::string name;
    std::string value;
};

class CgiParams
{
public:
    bool Parse(const std::string& params);
    bool ParseFromUrl(const std::string& url);
    void AppendToString(std::string* target) const;
    void WriteToString(std::string* target) const;
    std::string ToString() const;

    const CgiParam* Find(const std::string& name) const;
    CgiParam* Find(const std::string& name);

    CgiParam& Get(size_t index);
    const CgiParam& Get(size_t index) const;

    bool GetValue(const std::string& name, std::string* value) const;
    bool GetValue(const std::string& name, int32_t* value) const;

    const std::string& GetOrDefaultValue(
        const std::string& name,
        const std::string& default_value) const;

    size_t Count() const;
    void Clear();
    bool IsEmpty() const;
    void Add(const CgiParam& param);
    void Add(const std::string& name, const std::string& value);
    void Set(const std::string& name, const std::string& value);
    bool Remove(const std::string& name);
private:
    std::vector<CgiParam> m_params;
};

#endif // COMMON_NET_URI_CGI_PARAMS_HPP

