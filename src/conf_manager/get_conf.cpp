#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "conf_manager/get_conf.h"

using log4cplus::Logger;

static Logger logger = Logger::getInstance("collector");

ZookeeperForModule::ZookeeperForModule() {

}

ZookeeperForModule::~ZookeeperForModule() {

}

int ZookeeperForModule::Init(std::string cluster_name, std::string module) {
    if (module != "collector" && module != "scheduler" ) {
        LOG4CPLUS_ERROR(logger, "get an error module when call init");
        return -1;
    }

    m_module = module;
    if (m_zk.Init(cluster_name) < 0) {
        return -1;
    }

    m_zk_prefix = m_zk.GetZKPrefix();
    m_cluster_name = cluster_name;
    m_zk_server = m_zk.m_hostports;

    return 0;
}


