// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)
//
// Several simple classes for http message, including both request and response.
// TODO(hansye): complete the implemenation of http requst and response.

#ifndef RPC_PROTO_RPC_HTTP_MESSAGE_H_
#define RPC_PROTO_RPC_HTTP_MESSAGE_H_

#include <string>
#include <map>

namespace rpc {

// Describes a http message, which is the base class for http request and
// response. It includes the start line and headers and doesn't include the
// message body.
class HttpMessage {
public:
    HttpMessage() {}
    virtual ~HttpMessage() {}

    // Parse http headers (including the start line) from data. Return -1 if
    // it's not valid, return 0 if it looks incomplete, and return a positive
    // value if successful to indicate how many bytes consumed.
    virtual int ParseHeaders(const std::string& data);

    const std::string& start_line() const { return m_start_line; }

    const std::map<std::string, std::string>& headers() const {
        return m_headers;
    }

protected:
    std::string m_start_line;
    std::map<std::string, std::string> m_headers;
};

// Describes a http request, including request line and headers.
class HttpRequest : public HttpMessage {
public:
    enum {
        METHOD_UNKNOWN = -1,
        METHOD_HEAD,
        METHOD_GET,
        METHOD_POST,
        METHOD_PUT,
        METHOD_CONNECT,
    };

    HttpRequest() : m_method(METHOD_UNKNOWN) {}
    ~HttpRequest() {}

    static int GetMethodByName(const char* method_name);
    static const char* GetMethodName(int method);

    virtual int ParseHeaders(const std::string& data);

    int method() const { return m_method; }

    const std::string& path() const { return m_path; }

private:
    int m_method;
    std::string m_path;
};

// Describes a http response, including status line and headers.
class HttpResponse: public HttpMessage {
public:
    HttpResponse() : m_status(-1) {}
    ~HttpResponse() {}

    virtual int ParseHeaders(const std::string& data);

    int status() const { return m_status; }

private:
    int m_status;
};

} // namespace rpc

#endif // RPC_PROTO_RPC_HTTP_MESSAGE_H_
