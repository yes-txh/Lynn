#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "common/clynn/get_ip.h"

#include "conf_manager/get_conf.h"

#include "collector/config_manager.h"
#include "collector/collector_conf.h"

using log4cplus::Logger;

static Logger logger = Logger::getInstance("collector");

bool GetCollectorConf() {
    StringFlag string_flags[] = {
        {"collector_endpoint", FLAGS_collector_endpoint},
        {"collector_port", FLAGS_collector_port},
        {"collector_lockfile", FLAGS_collector_lockfile},
        {"scheduler_endpoint", FLAGS_scheduler_endpoint},
    };
    IntFlag int_flags[] = {
        {"heartbeat_interval", FLAGS_heartbeat_interval}
    };
    std::map<std::string, std::string> m_collector_confs;

    ZookeeperForModule zk_for_collector;

    int rt = zk_for_collector.Init(FLAGS_cluster_name, "collector");
    if(rt < 0) {
        LOG4CPLUS_ERROR(logger, "failed to connect to zk.");
        return false;
    } else {
        if(zk_for_collector.GetAllNodeOfModule(&m_collector_confs) < 0) {
            LOG4CPLUS_ERROR(logger, "failed to get all node module from zk.");
            return false;
        }
    }
    FLAGS_lock_node = zk_for_collector.m_zk_prefix + "/config/global/collecotr_endpoint";
    FLAGS_zk_server = zk_for_collector.m_zk_server;
    std::map<std::string, std::string>::iterator it, end; 
    end = m_collector_confs.end();
    for(size_t i = 0; i < sizeof(string_flags) / sizeof(StringFlag); ++i) {
        it = m_collector_confs.find(string_flags[i].zk_name);
        if(it == end) { 
            LOG4CPLUS_ERROR(logger, "failed to set: " << string_flags[i].zk_name );
            return false;
        } else {
            string_flags[i].flag_name = it->second;
            SafetyFlag::Instance()->Set(string_flags[i].zk_name, string_flags[i].flag_name);
        }
    }
    for(size_t i = 0; i < sizeof(int_flags) / sizeof(IntFlag); ++i) {
        it = m_collector_confs.find(int_flags[i].zk_name);
        if(it == end || (int_flags[i].flag_name = atoi((it->second).c_str())) <= 0) {
            LOG4CPLUS_ERROR(logger, "failed to set: " << int_flags[i].zk_name);
            return false;
        } else {
            SafetyIntFlag::Instance()->Set(int_flags[i].zk_name, int_flags[i].flag_name);
        }
    }
    FLAGS_collector_lockfile = SafetyFlag::Instance()->Get("collector_lockfile");
    SafetyIntFlag::Instance()->Set("report_schedd_timeout", 
                                   (3*(FLAGS_heartbeat_interval)/1000));
    FLAGS_IP = GetIP(FLAGS_interface);
    if(FLAGS_IP.empty()) {
        LOG4CPLUS_ERROR(logger, "failed to get ip: " << FLAGS_interface);
        return false;
    }
    return true;
}

bool CollectorConf::Init() {
    return m_dynamic_config.Init("collecotr");
}
