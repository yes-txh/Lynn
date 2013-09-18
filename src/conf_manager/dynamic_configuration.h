#ifndef CONF_MANAGER_DYNAMIC_CONFIGURATION_H
#define CONF_MANAGER_DYNAMIC_CONFIGURATION_H

#include <string>

#include "conf_manager/zk_common.h"

class DynamicConfiguration {
public:
    bool Init(const std::string& module, const std::string& zk_server);
private:
    ZookeeperCommon m_zk;
};

#endif 
