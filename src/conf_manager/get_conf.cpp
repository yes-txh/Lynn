#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "conf_manager/get_conf.h"

using log4cplus::Logger;

static Logger logger = Logger::getInstance("collector");

ZookeeperForModule::ZookeeperForModule() {
}

ZookeeperForModule::~ZookeeperForModule() {
}

int ZookeeperForModule::Init(std::string cluster_name, std::string module, std::string zk_server) {
    if (module != "collector" && module != "scheduler" ) {
        LOG4CPLUS_ERROR(logger, "get an error module when call init");
        return -1;
    }

    m_module = module;
    if (m_zk.Init(cluster_name, zk_server) < 0) {
        return -1;
    }

    m_zk_prefix = m_zk.GetZKPrefix();
    m_cluster_name = cluster_name;
    m_zk_server = m_zk.m_hostports;

    return 0;
}

//
int ZookeeperForModule::GetAllNodeOfModule(std::map<std::string, std::string> *results) {
    int rc = 0;
    results->clear();
    std::string module_path = m_zk_prefix + "/config/" + m_module;

    rc = m_zk.GetChildren(module_path, results);
    if(rc < 0) {
        LOG4CPLUS_ERROR(logger, "error happens when GetChildren." << "the path is .. " << module_path);
        return -1;
    }
    return rc;
}
