// Copyright 2011, Tencent Inc.
// Author: XiaoDong Chen (donniechen@tencent.com)
// 使用url encode生成链接

// 例如:
// http://www.soso.com/q?pid=s.idx&w=abc
// m_http_server_domain = "http://www.soso.com/q"
// m_parameter_list[0] = "pid=s.idx";
// m_parameter_list[1] = "w=云"
// m_generated_url= "http://www.soso.com/q?pid=s.idx&w=%D4%C6"
// 云 urlencode = "%D4%C6"

#ifndef COMMON_BASELIB_SVRPUBLIB_GENERATE_CGI_PARAMETER_H_
#define COMMON_BASELIB_SVRPUBLIB_GENERATE_CGI_PARAMETER_H_

#include <string>
#include <vector>
#include "common/baselib/svrpublib/server_publib.h"

class GenerateCGIParameter {
public:
    GenerateCGIParameter();
    virtual ~GenerateCGIParameter();

    void SetDomain(std::string& domain) { m_http_server_domain = domain; }

    void SetCgi(std::string& cgi) { m_http_server_cgi = cgi; }

    // Append a parameter to the url
    void AppendParameter(std::string& key, std::string& value);

    // return the generated url
    std::string GetGeneratedUrl();

private:
    std::string m_http_server_domain;
    std::string m_http_server_cgi;
    std::vector <std::string> m_parameter_list;
};

#endif // COMMON_BASELIB_SVRPUBLIB_GENERATE_CGI_PARAMETER_H_
