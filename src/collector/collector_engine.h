#ifndef COLLECTOR_COLLECTOR_ENGINE_H
#define COLLECTOR_COLLECTOR_ENGINE_H

#include <classad/classad.h>
#include "common/clynn/singleton.h"

#define MACHINE_NUM 1000

class CollectorEngine {
    public:
        CollectorEngine();
        ~CollectorEngine();
        int Init();
        ClassAd* m_machine_ads[MACHINE_NUM];

        /// the rwlocks that collector uses
        pthread_rwlock_t m_machine_locks[MACHINE_NUM];
        /// the id rwlocks
        pthread_rwlock_t m_id_lock;

    private:
        /// the collector_requirements configured by conf
        ClassAd* m_collector_requirements;
        int m_cur_id;
        /// if collector has notified scheduler the cluster is froze
        bool m_cluster_frozen;
};

typedef Singleton<CollectorEngine> COLLECTORENGINE;
#endif

