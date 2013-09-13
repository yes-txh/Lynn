// Copyright 2011, Tencent Inc.
// Author: Xiaokang Liu (hsiaokangliu@tencent.com)

#ifndef COMMON_NET_HTTP_HTTP_CLIENT_H
#define COMMON_NET_HTTP_HTTP_CLIENT_H

#include "common/net/http/http_server.h"
#include <vector>
#include <map>

namespace poppy {

class HttpClient : public HttpBase {
public:
    friend class HttpClientConnection;
    explicit HttpClient(netframe::NetFrame* net_frame,
            bool own_net_frame = false) :
        HttpBase(net_frame, own_net_frame) {
    }
    explicit HttpClient(int threads = 0) : HttpBase(threads) {}
    virtual ~HttpClient() {}

    // Connect to a server.
    virtual bool ConnectServer(const std::string& address, HttpHandler* handler);

    DECLARE_UNCOPYABLE(HttpClient);
};

} // namespace poppy

#endif // COMMON_NET_HTTP_HTTP_CLIENT_H
