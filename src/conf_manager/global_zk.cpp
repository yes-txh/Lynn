#include <string>
#include <map>
#include <list>
#include <zookeeper/zookeeper.h>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "conf_manager/global_zk.h"
#include "conf_manager/cluster_info.h"

using log4cplus::Logger;

static Logger logger = Logger::getInstance("collector");

// get idc_name from cluster_name
int GlobalZKInfo::GetIDCFromCluster(const std::string& cluster_name, ClusterInfo& cluster) {
    std::string::size_type pos = cluster_name.find("-");

    if (pos != std::string::npos && pos != 0 && pos != cluster_name.size()-1) {
        std::string idc_name = cluster_name.substr(0, pos);
        cluster.cluster_name = cluster_name;
        cluster.zk_servers = idc_name + ".zk.oa.com:2181";
        cluster.zk_prefix = "/zk/" + idc_name + "/tborg/" + cluster_name;
        return 0;
    } else {
        LOG4CPLUS_ERROR(logger, "The cluster_name has no '-' or begin with '-' or has no cluster self info");
        return -1;
    }

    return 0;
}
