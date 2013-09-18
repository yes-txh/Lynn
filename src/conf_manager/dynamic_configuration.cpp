#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "conf_manager/dynamic_configuration.h"

using log4cplus::Logger;

static Logger logger = Logger::getInstance("collector");

bool DynamicConfiguration::Init(const std::string& module) {
    if (module != "collector" && module != "scheduler") {
        LOG4CPLUS_ERROR(logger, "you give a wrong  module when call init");
        return false;
    }

    if (m_zk.Init(FLAGS_cluster_name) < 0) {
        LOG4CPLUS_ERROR(logger, "Can't connect to zk");
        return false;
    }
    
    return true;
}
