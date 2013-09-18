#include "conf_manager/get_conf.h"

#include "collector/config_manager.h"
#include "collector/collector_conf.h"

bool GetCollectorConf() {
    StringFlag string_flags[] = {
        {"collector_endpoint", FLAGS_collector_endpoint},
        {"collector_lockfile", FLAGS_lockfile},
        {"scheduler_endpoint", FLAGS_scheduler_endpoint},
    };
    IntFlag int_flags[] = {
        {"heartbeat_interval", FLAGS_heartbeat_interval}
    };
    std::map<std::string, std::string> m_collector_confs;

    ZookeeperForModule zk_for_collector;

    int rt = zk_for_collector.Init(FLAGS_cluster_name, "collector");
    if(rt < 0) {
    }

    return true;
}

bool CollectorConf::Init() {
    return m_dynamic_config.Init("collecotr");
}
