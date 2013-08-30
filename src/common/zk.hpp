#ifndef SRC_COMMON_ZK_H
#define SRC_COMMON_ZK_H

#include <zookeeper/zookeeper.h>

class ZK {
public:

    ZK() {
        m_zk = NULL;
    }

    ~ZK() {
        zookeeper_close(m_zh);
    }

private:
    static void DummyWatcher(zhandle_t *zzk, int32_t type, int32_t state, const char *path, void *watcherCtx) {}
    zhandle_t *m_zh;
};

#endif
