#ifndef CONF_MANAGER_CLUSTER_INFO_H
#define CONF_MANAGER_CLUSTER_INFO_H
#include <string>
#include <map>
#include <list>
#include <zookeeper/zookeeper.h>

struct ClusterInfo
{
    ClusterInfo() {
        cluster_name = "";
        zk_servers = "";
        zk_prefix = "";
    }
    std::string cluster_name;
    std::string zk_servers;
    std::string zk_prefix;
};

#endif
