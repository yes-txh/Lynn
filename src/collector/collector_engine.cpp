#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "common/classad/classad_complemention.h"
#include "collector/collector_engine.h"

using log4cplus::Logger;

static Logger logger = Logger::getInstance("collector");

CollectorEngine::CollectorEngine() {
    m_collector_requirements = NULL;
    for (int id = 0; id < MACHINE_NUM; ++id) {
        m_machine_ads[id] = NULL;
    }

    m_cur_id = 0;
   
    for (int i = 0;i < MACHINE_NUM; ++i) {
        pthread_rwlock_init(&(m_machine_locks[i]), NULL);
    }
    
    pthread_rwlock_init(&m_id_lock, NULL);
    /// 默认集群已经冻结，防止collector重启后忘记
    m_cluster_frozen = true; 
}

CollectorEngine::~CollectorEngine() {
     delete m_collector_requirements;
     m_collector_requirements = NULL;
     
     for (int i = 0;i < MACHINE_NUM; ++i) {
        pthread_rwlock_destroy(&(m_machine_locks[i]));
    }
    pthread_rwlock_destroy(&m_id_lock);
} 


int CollectorEngine::Init() {
    LOG4CPLUS_INFO(logger, "Init collector engine");
    string err;
    SetCollectorRequirements(FLAGS_collector_requirements, err);
    return 0;
}

bool CollectorEngine::SetCollectorRequirements(string str, string& error_desc) {
    delete m_collector_requirements;
    m_collector_requirements = NULL;

    if (str.empty()) {
        return true;
    }

    string ad_str = "["
    "Requirements = " + str + ";"
    "MyType = \"Job\";"
    "TargetType = \"Machine\";"
    "]";
    m_collector_requirements = stringToAd(ad_str);
    if (!m_collector_requirements) {
        LOG4CPLUS_ERROR(logger, "NEW ERROR when SetCollectorRequirements.");
        return false;
    }

    return true; 
}
