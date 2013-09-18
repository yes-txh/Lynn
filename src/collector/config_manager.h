#ifndef COLLECTOR_CONFIG_MANAGER_H
#define COLLECTOR_CONFIG_MANAGER_H

#include <map>
#include <string>

#include "common/clynn/singleton.h"
#include "conf_manager/dynamic_configuration.h"

using std::string;

struct StringFlag {
    const string zk_name;
    string flag_name;
};

struct IntFlag {
    const string zk_name;
    int flag_name;
};

class CollectorConf {
public:
    CollectorConf() {}
    ~CollectorConf();
    bool Init();
    DynamicConfiguration m_dynamic_config;
private:
    std::map<string, string> m_collector_confs;
};

typedef Singleton<CollectorConf> COLLECTORCONFIG;
#endif
