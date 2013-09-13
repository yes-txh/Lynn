// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)

#include "common/base/string/string_algorithm.hpp"
#include "common/rpc/proto_rpc/http_message.h"

namespace rpc {

namespace {
static const struct {
    int m_method;
    const char* m_method_name;
} kValidMethodNames[] = {
    { HttpRequest::METHOD_HEAD, "HEAD" },
    { HttpRequest::METHOD_GET, "GET" },
    { HttpRequest::METHOD_POST, "POST" },
    { HttpRequest::METHOD_PUT, "PUT" },
    { HttpRequest::METHOD_CONNECT, "CONNECT" },
    { HttpRequest::METHOD_UNKNOWN, NULL },
};
} // namespace

// class HttpMessage
int HttpMessage::ParseHeaders(const std::string& data) {
    m_start_line.clear();
    m_headers.clear();

    std::vector<std::string> lines;
    SplitStringByDelimiter(data, "\r\n", &lines);
    if (lines.empty()) {
        return -1;
    }

    m_start_line = lines[0];
    // TODO(hansye): handle duplicated header name and space.
    for (size_t i = 1u; i < lines.size(); ++i) {
        std::string::size_type pos = lines[i].find(": ");
        if (pos == std::string::npos) {
            return -1;
        }
        m_headers[lines[i].substr(0, pos)] = lines[i].substr(pos + 2);
    }

    // Assume all passed in bytes belong to http start line and headers.
    return data.size();
}

// class HttpRequest
// static
int HttpRequest::GetMethodByName(const char* method_name) {
    for (int i = 0; ; ++i) {
        if (kValidMethodNames[i].m_method_name == NULL) {
            return HttpRequest::METHOD_UNKNOWN;
        }
        // Method is case sensitive.
        if (strcmp(method_name, kValidMethodNames[i].m_method_name) == 0) {
            return kValidMethodNames[i].m_method;
        }
    }
}

// static
const char* HttpRequest::GetMethodName(const int method) {
    for (int i = 0; ; ++i) {
        if (kValidMethodNames[i].m_method_name == NULL) {
            return NULL;
        }
        if (kValidMethodNames[i].m_method == method) {
            return kValidMethodNames[i].m_method_name;
        }
    }
}

int HttpRequest::ParseHeaders(const std::string& data) {
    int retval = HttpMessage::ParseHeaders(data);
    if (retval <= 0) {
        return retval;
    }

    std::vector<std::string> fields;
    SplitString(m_start_line, " ", &fields);
    if (fields.size() != 2 && fields.size() != 3) {
        return -1;
    }

    m_method = GetMethodByName(fields[0].c_str());
    if (m_method == METHOD_UNKNOWN) {
        return -1;
    }
    m_path = fields[1];

    if (fields.size() == 3) {
        if (!HasPrefixString(fields[2], "HTTP/")) {
            return -1;
        }
    }

    return retval;
}

// class HttpResponse
int HttpResponse::ParseHeaders(const std::string& data) {
    int retval = HttpMessage::ParseHeaders(data);
    if (retval <= 0) {
        return retval;
    }

    std::vector<std::string> fields;
    SplitString(m_start_line, " ", &fields);
    if (fields.size() < 2) {
        return -1;
    }

    if (!HasPrefixString(fields[0], "HTTP/")) {
        return -1;
    }

    m_status = atoi(fields[1].c_str());

    if (m_status < 100 || m_status >= 600) {
        return -1;
    }

    return retval;
}

} // namespace rpc
