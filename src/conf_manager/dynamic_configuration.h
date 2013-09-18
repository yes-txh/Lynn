#ifndef CONF_MANAGER_DYNAMIC_CONFIGURATION_H
#define CONF_MANAGER_DYNAMIC_CONFIGURATION_H

#include <string>
#include <gflags/gflags.h>

#include "conf_manager/zk_common.h"

DECLARE_string(cluster_name);

class DynamicConfiguration {
public:
    bool Init(const std::string& module);
private:
    ZookeeperCommon m_zk;
};

#endif 
