// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)
// Xiaokang Liu (hsiaokangliu@tencent.com)

#include "common/base/string/string_algorithm.hpp"
#include "common/base/string/string_number.hpp"
#include "common/net/http/http_message.h"

static const struct {
    int method;
    const char* method_name;
} kValidMethodNames[] = {
    { HttpRequest::METHOD_HEAD, "HEAD" },
    { HttpRequest::METHOD_GET, "GET" },
    { HttpRequest::METHOD_POST, "POST" },
    { HttpRequest::METHOD_PUT, "PUT" },
    { HttpRequest::METHOD_CONNECT, "CONNECT" },
    { HttpRequest::METHOD_DELETE, "DELETE" },
    { HttpRequest::METHOD_OPTIONS, "OPTIONS" },
    { HttpRequest::METHOD_TRACE, "TRACE" },
    { HttpRequest::METHOD_UNKNOWN, NULL },
};

static const struct {
    int status_code;
    const char* status_message;
    const char* status_description;
} kResponseStatus [] = {
    { 100, "Continue", "Request received, please continue" },
    { 101, "Switching Protocols", "Switching to new protocol; obey Upgrade header" },
    { 200, "OK", "Request fulfilled, document follows" },
    { 201, "Created", "Document created, URL follows" },
    { 202, "Accepted", "Request accepted, processing continues off-line" },
    { 203, "Non-Authoritative Information", "Request fulfilled from cache" },
    { 204, "No Content", "Request fulfilled, nothing follows" },
    { 205, "Reset Content", "Clear input form for further input." },
    { 206, "Partial Content", "Partial content follows." },
    { 300, "Multiple Choices", "Object has several resources -- see URI list" },
    { 301, "Moved Permanently", "Object moved permanently -- see URI list" },
    { 302, "Found", "Object moved temporarily -- see URI list" },
    { 303, "See Other", "Object moved -- see Method and URL list" },
    { 304, "Not Modified", "Document has not changed since given time" },
    { 305, "Use Proxy", "You must use proxy specified in Location to access this resource." },
    { 307, "Temporary Redirect", "Object moved temporarily -- see URI list" },
    { 400, "Bad Request", "Bad request syntax or unsupported method" },
    { 401, "Unauthorized", "No permission -- see authorization schemes" },
    { 402, "Payment Required", "No payment -- see charging schemes" },
    { 403, "Forbidden", "Request forbidden -- authorization will not help" },
    { 404, "Not Found", "Nothing matches the given URI" },
    { 405, "Method Not Allowed", "Specified method is invalid for this resource." },
    { 406, "Not Acceptable", "URI not available in preferred format." },
    { 407, "Proxy Authentication Required", "You must authenticate with this proxy before proceeding." },
    { 408, "Request Timeout", "Request timed out; try again later." },
    { 409, "Conflict", "Request conflict." },
    { 410, "Gone", "URI no longer exists and has been permanently removed." },
    { 411, "Length Required", "Client must specify Content-Length." },
    { 412, "Precondition Failed", "Precondition in headers is false." },
    { 413, "Request Entity Too Large", "Entity is too large." },
    { 414, "Request-URI Too Long", "URI is too long." },
    { 415, "Unsupported Media Type", "Entity body in unsupported format." },
    { 416, "Requested Range Not Satisfiable", "Cannot satisfy request range." },
    { 417, "Expectation Failed", "Expect condition could not be satisfied." },
    { 500, "Internal Server Error", "Server got itself in trouble" },
    { 501, "Not Implemented", "Server does not support this operation" },
    { 502, "Bad Gateway", "Invalid responses from another server/proxy." },
    { 503, "Service Unavailable", "The server cannot process the request due to a high load" },
    { 504, "Gateway Timeout", "The gateway server did not receive a timely response" },
    { 505, "HTTP Version Not Supported", "Cannot fulfill request." },
    { -1, NULL, NULL },
};

static const struct {
    int version_number;
    const char* version_string;
} kHttpVersions [] = {
    { HttpMessage::VERSION_0_9, "HTTP/0.9" },
    { HttpMessage::VERSION_1_0, "HTTP/1.0" },
    { HttpMessage::VERSION_1_1, "HTTP/1.1" },
    { HttpMessage::VERSION_UNKNOWN, NULL },
};

void HttpMessage::Reset() {
    m_http_version = VERSION_1_1;
    m_headers.clear();
    m_http_body.clear();
}

std::string HttpMessage::HeadersToString() const {
    std::string result;
    result.append(GenerateStartLine());
    result.append("\r\n");

    size_t header_number = m_headers.size();
    for (size_t i = 0; i < header_number; ++i) {
        result.append(m_headers[i].first);
        result.append(": ");
        result.append(m_headers[i].second);
        result.append("\r\n");
    }
    result.append("\r\n");
    return result;
}

// Get a header value. return false if it does not exist.
// the header name is not case sensitive.
bool HttpMessage::GetHeader(const std::string& header_name,
                            std::string* header_value) const {
    std::vector<std::pair<std::string, std::string> >::const_iterator iter;
    for (iter = m_headers.begin(); iter != m_headers.end(); ++iter) {
        if (strcasecmp(iter->first.c_str(), header_name.c_str()) == 0) {
            *header_value = iter->second;
            return true;
        }
    }
    return false;
}

// Used when a http header appears multiple times.
// return false if it doesn't exist.
bool HttpMessage::GetHeaders(const std::string& header_name,
                             std::vector<std::string>* header_values) const {
    header_values->clear();
    std::vector<std::pair<std::string, std::string> >::const_iterator iter;
    for (iter = m_headers.begin(); iter != m_headers.end(); ++iter) {
        if (strcasecmp(iter->first.c_str(), header_name.c_str()) == 0) {
            header_values->push_back(iter->second);
        }
    }
    return header_values->size() > 0;
}

// Set a header field. if it exists, overwrite the header value.
void HttpMessage::SetHeader(const std::string& header_name,
                            const std::string& header_value) {
    bool field_exists = false;
    std::vector<std::pair<std::string, std::string> >::iterator tmp;
    std::vector<std::pair<std::string, std::string> >::iterator iter;
    for (iter = m_headers.begin(); iter != m_headers.end(); ) {
        if (strcasecmp(iter->first.c_str(), header_name.c_str()) == 0) {
            if (field_exists) { // duplicated item, remove it
                tmp = iter;
                ++iter;
                m_headers.erase(tmp);
            } else {
                iter->second = header_value;
                field_exists = true;
                ++iter;
            }
        } else {
            ++iter;
        }
    }
    if (!field_exists) {
        // the specific field doesn't exist.
        AddHeader(header_name, header_value);
    }
}

// Add a header field, just append, no overwrite.
void HttpMessage::AddHeader(const std::string& header_name,
                            const std::string& header_value) {
    m_headers.push_back(make_pair(header_name, header_value));
}

void HttpMessage::RemoveHeader(const std::string& header_name) {
    std::vector<std::pair<std::string, std::string> >::iterator tmp;
    std::vector<std::pair<std::string, std::string> >::iterator iter;
    for (iter = m_headers.begin(); iter != m_headers.end(); ) {
        if (strcasecmp(iter->first.c_str(), header_name.c_str()) == 0) {
            tmp = iter;
            ++iter;
            m_headers.erase(tmp);
        } else {
            ++iter;
        }
    }
}

const char* HttpMessage::GetVersionString(int version) {
    for (int i = 0; ; ++i) {
        if (kHttpVersions[i].version_number == VERSION_UNKNOWN) {
            return NULL;
        }
        if (version == kHttpVersions[i].version_number) {
            return kHttpVersions[i].version_string;
        }
    }
}

int HttpMessage::GetVersionNumber(const std::string& http_version) {
    for (int i = 0; ; ++i) {
        if (kHttpVersions[i].version_string == NULL) {
            return HttpMessage::VERSION_UNKNOWN;
        }
        if (strcasecmp(kHttpVersions[i].version_string,
                        http_version.c_str()) == 0) {
            return kHttpVersions[i].version_number;
        }
    }
}

// class HttpMessage
int HttpMessage::ParseHeaders(const std::string& data) {
    m_headers.clear();

    std::vector<std::string> lines;
    // using '\n' as delimiter
    SplitStringByDelimiter(data, "\n", &lines);
    if (lines.empty()) {
        return ERROR_NO_START_LINE;
    }

    // maybe '\r' left on the right side
    for (size_t i = 0; i < lines.size(); ++i) {
        StringTrimRight(&lines[i], "\r");
    }

    int retval = ParseStartLine(lines[0]);
    if (retval != ERROR_NORMAL) {
        return retval;
    }
    // Skip the head line and the last line(empty but '\n')
    for (size_t i = 1u; i < lines.size() - 1; ++i) {
        std::string::size_type pos = lines[i].find(":");
        if (pos == std::string::npos) {
            return ERROR_FIELD_NOT_COMPLETE;
        }
        m_headers.push_back(std::pair<std::string, std::string>(
            StringTrim(lines[i].substr(0, pos)),
            StringTrim(lines[i].substr(pos + 1))));
    }
    return ERROR_NORMAL;
}

int HttpMessage::GetContentLength() {
    std::string content_length;
    if (!GetHeader("Content-Length", &content_length)) {
        return -1;
    }
    int length = 0;
    bool ret = StringToNumber(content_length, &length);
    return (ret && length >= 0) ? length : -1;
};

bool HttpMessage::IsKeepAlive() const {
    if (m_http_version < VERSION_1_1) {
        return false;
    }
    std::string alive;
    if (!GetHeader("Connection", &alive)) {
        return true;
    }
    return (strcasecmp(alive.c_str(), "keep-alive") == 0) ? true : false;
}

void HttpRequest::Reset() {
    HttpMessage::Reset();
    m_method = METHOD_UNKNOWN;
    m_path = "/";
}

// static
int HttpRequest::GetMethodByName(const char* method_name) {
    for (int i = 0; ; ++i) {
        if (kValidMethodNames[i].method_name == NULL) {
            return HttpRequest::METHOD_UNKNOWN;
        }
        // Method is case sensitive.
        if (strcmp(method_name, kValidMethodNames[i].method_name) == 0) {
            return kValidMethodNames[i].method;
        }
    }
}

// static
const char* HttpRequest::GetMethodName(const int method) {
    if (method <= METHOD_UNKNOWN || method >= METHOD_UPPER_BOUND) {
        return NULL;
    }
    return kValidMethodNames[method].method_name;
}

int HttpRequest::ParseStartLine(const std::string& data) {
    std::vector<std::string> fields;
    SplitString(data, " ", &fields);
    if (fields.size() != 2 && fields.size() != 3) {
        return ERROR_START_LINE_NOT_COMPLETE;
    }

    m_method = GetMethodByName(fields[0].c_str());
    if (m_method == METHOD_UNKNOWN) {
        return ERROR_METHOD_NOT_FOUND;
    }
    m_path = fields[1];

    if (fields.size() == 3) {
        m_http_version = GetVersionNumber(fields[2]);
        if (m_http_version == HttpMessage::VERSION_UNKNOWN) {
            return ERROR_VERSION_UNSUPPORTED;
        }
    }
    return ERROR_NORMAL;
}

std::string HttpRequest::GenerateStartLine() const {
    // request line
    std::string start_line = GetMethodName(m_method);
    start_line.append(" ");
    start_line.append(m_path);
    start_line.append(" ");
    start_line.append(GetVersionString(m_http_version));
    return start_line;
}

void HttpResponse::Reset() {
    HttpMessage::Reset();
    m_status = -1;
}

// static
const char* HttpResponse::GetStatusMessage(int status_code) {
    for (int i = 0; ; ++i) {
        if (kResponseStatus[i].status_code == -1) {
            return NULL;
        }
        if (kResponseStatus[i].status_code == status_code) {
            return kResponseStatus[i].status_message;
        }
    }
}

int HttpResponse::ParseStartLine(const std::string& data) {
    std::vector<std::string> fields;
    SplitString(data, " ", &fields);
    if (fields.size() < 2) {
        return ERROR_START_LINE_NOT_COMPLETE;
    }

    m_http_version = GetVersionNumber(fields[0]);
    if (m_http_version == HttpMessage::VERSION_UNKNOWN) {
        return ERROR_VERSION_UNSUPPORTED;
    }

    m_status = atoi(fields[1].c_str());
    if (GetStatusMessage(m_status) == NULL) {
        return ERROR_RESPONSE_STATUS_NOT_FOUND;
    }

    return ERROR_NORMAL;
}

std::string HttpResponse::GenerateStartLine() const {
    // status line
    std::string start_line = GetVersionString(m_http_version);
    start_line.append(" ");
    start_line.append(IntegerToString(m_status));
    start_line.append(" ");
    start_line.append(GetStatusMessage(m_status));
    return start_line;
}
