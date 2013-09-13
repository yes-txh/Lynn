#include "common/net/http/http_handler_manager.h"
#include "glog/logging.h"

namespace poppy {

HttpHandlerManager::HttpHandlerManager() {
}

HttpHandlerManager::~HttpHandlerManager() {
    Clear();
}

void HttpHandlerManager::Clear() {
    RWLock::WriterLocker locker(m_handler_lock);
    std::set<HttpHandler*>::iterator iter;
    for (iter = m_handlers.begin(); iter != m_handlers.end(); ++iter) {
        delete *iter;
    }
    m_handler_map.clear();
    m_handlers.clear();
}

void HttpHandlerManager::RegisterHandler(const std::string& path,
        HttpHandler* handler) {
    CHECK_NOTNULL(handler);
    RWLock::WriterLocker locker(m_handler_lock);
    std::map<std::string, HttpHandler*>::iterator iter;
    iter = m_handler_map.find(path);
    CHECK(iter == m_handler_map.end())
        << "A handler on the specified path has already been registered: "
        << path;
    m_handler_map[path] = handler;
    if (m_handlers.find(handler) == m_handlers.end()) {
        m_handlers.insert(handler);
    }
}

HttpHandler* HttpHandlerManager::FindHandler(const std::string& path) {
    RWLock::ReaderLocker locker(m_handler_lock);
    std::map<std::string, HttpHandler*>::iterator iter;
    iter = m_handler_map.find(path);
    if (iter == m_handler_map.end()) {
        return NULL;
    }
    return iter->second;
}

} // namespace poppy
