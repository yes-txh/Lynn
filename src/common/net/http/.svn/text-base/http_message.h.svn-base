// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)
// Xiaokang Liu (hsiaokangliu@tencent.com)

#ifndef COMMON_NET_HTTP_HTTP_MESSAGE_H
#define COMMON_NET_HTTP_HTTP_MESSAGE_H

#include <string>
#include <vector>

// Describes an http message, which is the base class for http request and
// response. It includes the start line, headers and body.
class HttpMessage {
public:
    enum {
        VERSION_UNKNOWN = 0,
        VERSION_0_9 = 9,
        VERSION_1_0 = 10,
        VERSION_1_1 = 11,
    };

    enum ErrorType {
        ERROR_NORMAL = 0,
        ERROR_NO_START_LINE,
        ERROR_START_LINE_NOT_COMPLETE,
        ERROR_VERSION_UNSUPPORTED,
        ERROR_RESPONSE_STATUS_NOT_FOUND,
        ERROR_FIELD_NOT_COMPLETE,
        ERROR_METHOD_NOT_FOUND,
    };

    HttpMessage() {
        m_http_version = VERSION_1_1;
    }
    virtual ~HttpMessage() {}
    virtual void Reset();

public:
    // Parse http headers (including the start line) from data.
    // return: error code which is defined as ErrorType.
    virtual int ParseHeaders(const std::string& data);

    int http_version() const { return m_http_version; }
    void set_http_version(int version) {
        m_http_version = version;
    }

    const std::string& http_body() const { return m_http_body; }
    std::string* mutable_http_body() { return &m_http_body; }
    void set_body(const std::string& body) {
        m_http_body = body;
    }

    int GetContentLength();
    bool IsKeepAlive() const;

    // Get all the header values.
    const std::vector<std::pair<std::string, std::string> >& headers() const {
        return m_headers;
    }
    // Return false if it doesn't exist.
    bool GetHeader(const std::string& header_name,
                   std::string* header_value) const;
    // Used when a http header appears multiple times.
    // return false if it doesn't exist.
    bool GetHeaders(const std::string& header_name,
                    std::vector<std::string>* header_values) const;
    // Set a header field. if it exists, overwrite the header value.
    void SetHeader(const std::string& header_name,
                   const std::string& header_value);
    // Add a header field, just append, no overwrite.
    void AddHeader(const std::string& header_name,
                   const std::string& header_value);
    // Remove an http header field.
    void RemoveHeader(const std::string& header_name);

    // Const function. headers to string.
    std::string HeadersToString() const;

protected:
    static const char* GetVersionString(int version);
    static int  GetVersionNumber(const std::string& http_version);

    virtual std::string GenerateStartLine() const = 0;
    virtual int ParseStartLine(const std::string& data) = 0;

    int  m_http_version;
    std::vector<std::pair<std::string, std::string> > m_headers;
    std::string m_http_body;
};

// Describes a http request.
class HttpRequest : public HttpMessage {
public:
    enum {
        METHOD_UNKNOWN = -1,
        METHOD_HEAD,
        METHOD_GET,
        METHOD_POST,
        METHOD_PUT,
        METHOD_DELETE,
        METHOD_OPTIONS,
        METHOD_TRACE,
        METHOD_CONNECT,
        METHOD_UPPER_BOUND,  // no use, just label the bound.
    };

    HttpRequest() : m_method(METHOD_UNKNOWN) {
        m_path = "/";
    }
    ~HttpRequest() {}
    virtual void Reset();

public:
    static int GetMethodByName(const char* method_name);
    static const char* GetMethodName(int method);

    int method() const { return m_method; }
    void set_method(int method) {
        m_method = method;
    }

    const std::string& path() const { return m_path; }
    void set_path(const std::string& path) {
        m_path = path;
    }

private:
    virtual std::string GenerateStartLine() const;
    virtual int ParseStartLine(const std::string& data);

    int m_method;
    std::string m_path;
};

// Describes a http response.
class HttpResponse: public HttpMessage {
public:
    HttpResponse() : m_status(-1) {}
    ~HttpResponse() {}
    virtual void Reset();

    int status() const { return m_status; }
    void set_status(int status) {
        m_status = status;
    }

private:
    static const char* GetStatusMessage(int status_code);
    virtual std::string GenerateStartLine() const;
    virtual int ParseStartLine(const std::string& data);

    int m_status;
};

#endif // COMMON_NET_HTTP_HTTP_MESSAGE_H
