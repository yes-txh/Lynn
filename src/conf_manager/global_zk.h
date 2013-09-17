#ifndef CONF_MANAGER_GLOBAL_ZK_H
#define CONF_MANAGER_GLOBAL_ZK_H
#include <string>
#include <map>
#include <list>
#include <zookeeper/zookeeper.h>
#include "conf_manager/cluster_info.h"

class GlobalZKInfo {
 public:
    GlobalZKInfo() {}
    ~GlobalZKInfo(){}
    int GetIDCFromCluster(const std::string& cluster_name, ClusterInfo& cluster);
};

#endif // CONF_MANAGER_GLOBAL_ZK_H
