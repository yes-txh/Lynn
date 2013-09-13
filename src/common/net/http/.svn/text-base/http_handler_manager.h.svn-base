// Copyright 2011, Tencent Inc.
// Author: Xiaokang Liu (hsiaokangliu@tencent.com)

#ifndef COMMON_NET_HTTP_HTTP_HANDLER_MANAGER_H
#define COMMON_NET_HTTP_HTTP_HANDLER_MANAGER_H

#include <set>
#include <map>
#include <string>
#include "common/net/http/http_handler.h"
#include "common/net/http/http_message.h"
#include "common/base/string/string_piece.hpp"
#include "common/system/concurrency/mutex.hpp"
#include "common/system/concurrency/rwlock.hpp"

namespace poppy {

class HttpHandlerManager {
public:
    HttpHandlerManager();
    ~HttpHandlerManager();

    // Clear all handlers.
    void Clear();

    // Register a handler on a specified path.
    // Return false if another handler already has been registered.
    // If a handler has been registered successfully, it will be taken over
    // by the handler manager and CANNOT be unregistered.
    void RegisterHandler(const std::string& path, HttpHandler* handler);
    // Find a handler registered on a specified path. If no handler is
    // registered, NULL will be returned.
    HttpHandler* FindHandler(const std::string& path);

private:
    RWLock m_handler_lock;
    std::set<HttpHandler*> m_handlers;
    std::map<std::string, HttpHandler*>  m_handler_map;
};

} // namespace poppy

#endif // COMMON_NET_HTTP_HTTP_HANDLER_MANAGER_H
