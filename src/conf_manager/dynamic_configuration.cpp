#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "conf_manager/dynamic_configuration.h"

using log4cplus::Logger;

static Logger logger = Logger::getInstance("collector");

DynamicConfiguration::DynamicConfiguration() {
}

DynamicConfiguration::~DynamicConfiguration(){
}

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

ThreadSafetyStringFlag::ThreadSafetyStringFlag() {
    pthread_rwlock_init(&m_flag_lock, NULL);
}

ThreadSafetyStringFlag::~ThreadSafetyStringFlag() {
    pthread_rwlock_destroy(&m_flag_lock);
}

string ThreadSafetyStringFlag::Get(const string& config_name) {
    string config_value;
    map<string, string>::iterator it;
    pthread_rwlock_rdlock(&m_flag_lock);
    if ((it = m_flag_map.find(config_name)) != m_flag_map.end()) {
        config_value = it->second;
    }
    pthread_rwlock_unlock(&m_flag_lock);
    return config_value;
}
void ThreadSafetyStringFlag::Set(const string& config_name, const string& config_value) {
    pthread_rwlock_wrlock(&m_flag_lock);
    m_flag_map[config_name] = config_value;
    pthread_rwlock_unlock(&m_flag_lock);
}

ThreadSafetyIntFlag::ThreadSafetyIntFlag() {
    pthread_rwlock_init(&m_flag_lock, NULL);
}

ThreadSafetyIntFlag::~ThreadSafetyIntFlag() {
    pthread_rwlock_destroy(&m_flag_lock);
}

int ThreadSafetyIntFlag::Get(const string& config_name) {
    int config_value = -1;
    map< string, int >::iterator it;
    pthread_rwlock_rdlock(&m_flag_lock);
    if ((it = m_flag_map.find(config_name)) != m_flag_map.end()) {
        config_value = it->second;
    }
    pthread_rwlock_unlock(&m_flag_lock);
    return config_value;
}

void ThreadSafetyIntFlag::Set(const string& config_name, int config_value) {
    pthread_rwlock_wrlock(&m_flag_lock);
    m_flag_map[config_name] = config_value;
    pthread_rwlock_unlock(&m_flag_lock);
}

