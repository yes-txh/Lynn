#ifndef CONF_MANAGER_GET_CONF_H
#define CONF_MANAGER_GET_CONF_H

#include <string>
#include "conf_manager/zk_common.h"

class ZookeeperForModule {
    public:
        ZookeeperForModule();
        ~ZookeeperForModule();
        int Init(std::string cluster_name, std::string module);   

        int GetAllNodeOfModule(std::map<std::string, std::string> *result); 
        std::string m_zk_server;
        std::string m_zk_prefix;
    private:
        std::string m_cluster_name;
        std::string m_module;
        ZookeeperCommon m_zk;
};

#endif
