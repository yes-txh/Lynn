// Copyright 2011, Tencent Inc.
// Author: Xiaokang Liu (hsiaokangliu@tencent.com)

#ifndef COMMON_NET_HTTP_HTTP_CONNECTION_MANAGER_H
#define COMMON_NET_HTTP_HTTP_CONNECTION_MANAGER_H

#include <set>
#include "common/netframe/netframe.hpp"
#include "common/system/concurrency/mutex.hpp"
#include "common/system/concurrency/condition_variable.hpp"

namespace poppy {

class ConnectionManager {
public:
    explicit ConnectionManager(netframe::NetFrame* net_frame);
    ~ConnectionManager();

    // Add a connection.
    bool AddConnection(int64_t connection_id);

    // Close a connection by id.
    void CloseConnection(int64_t connection_id, bool immediate);

    // Close all connections.
    void CloseAllConnections();

    // Remove a connection.
    void RemoveConnection(int64_t connection_id);

private:
    netframe::NetFrame* m_net_frame;
    std::set<int64_t> m_connections;

    SimpleMutex m_mutex;
    ConditionVariable m_cond;
};

} // namespace poppy

#endif // COMMON_NET_HTTP_HTTP_CONNECTION_MANAGER_H
