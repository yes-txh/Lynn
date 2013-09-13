#include "common/base/string/string_algorithm.hpp"
#include "common/baselib/svrpublib/generate_cgi_parameter.h"
#include "common/baselib/svrpublib/url_codec.h"

GenerateCGIParameter::GenerateCGIParameter() {}

GenerateCGIParameter::~GenerateCGIParameter() {}

void GenerateCGIParameter::AppendParameter(std::string& key, std::string& value) {
    value = UrlCodec::Encode(value);
    std::string parameter = key + std::string("=") + value;
    m_parameter_list.push_back(parameter);
}

std::string GenerateCGIParameter::GetGeneratedUrl() {
    std::string generate_url = m_http_server_domain;
    if(!m_http_server_cgi.empty()) {
        generate_url += std::string("/") + m_http_server_cgi;
    }

    for (uint32_t u=0; u<m_parameter_list.size(); ++u) {
        std::string str_parameter;
        if(u == 0) {
            str_parameter = std::string("?") + m_parameter_list[u];
        } else {
            str_parameter = std::string("&") + m_parameter_list[u];
        }
        generate_url += str_parameter;
    }

    return generate_url;
}