#ifndef CONF_MANAGER_DYNAMIC_CONFIGURATION_H
#define CONF_MANAGER_DYNAMIC_CONFIGURATION_H

#include <string>
#include <gflags/gflags.h>
#include <map>

#include "conf_manager/zk_common.h"
#include "common/clynn/singleton.h"

using std::map;
DECLARE_string(cluster_name);

class DynamicConfiguration {
public:
    DynamicConfiguration();
    ~DynamicConfiguration();
    bool Init(const std::string& module);
private:
    ZookeeperCommon m_zk;
};

class ThreadSafetyStringFlag {
    public:
        ThreadSafetyStringFlag();
        ~ThreadSafetyStringFlag();
        string Get(const string& config_name);
        void Set(const string& config_name, const string& config_value);
    private:
        std::map<string, string> m_flag_map;
        pthread_rwlock_t m_flag_lock;
};

class ThreadSafetyIntFlag {
    public:
        ThreadSafetyIntFlag();
        ~ThreadSafetyIntFlag();
        int Get(const string& config_name);
        void Set(const string& config_name, int config_value);
    private:
        std::map< string, int > m_flag_map;
        pthread_rwlock_t m_flag_lock;
};

typedef Singleton<ThreadSafetyStringFlag> SafetyFlag;
typedef Singleton<ThreadSafetyIntFlag> SafetyIntFlag;

#endif 
