#ifndef CONF_MANAGER_WATCHER_H
#define CONF_MANAGER_WATCHER_H

#include <string>
#include <map>
#include <list>
#include <zookeeper/zookeeper.h>

extern void DataCompletion(int rc, const char *value, int value_len,
                           const Stat *stat, const void *data);

struct evt_t {
    std::string path;
    int type;
};
// events zk watches
class watchctx_t {
    private:
        std::list<evt_t> events;
        watchctx_t(const watchctx_t&);
        watchctx_t& operator=(const watchctx_t&);
    public:
        bool connected;
        zhandle_t* zh;
        // ensure secrity in mul pthread
        pthread_rwlock_t ev_lock;

        watchctx_t();
        ~watchctx_t();
        // get the event
        evt_t GetEvent();
        // get the number os events
        int CountEvents();

        void PutEvent(evt_t evt);

        bool WaitForConnected(zhandle_t *zk);

        bool WaitForDisconnected(zhandle_t *zk);
};

void Watcher(zhandle_t *zh, int type, int state, const char *path, void* v); 
void ValueWatcher(zhandle_t *zh, int type, int state, const char *path, void* v); 
#endif // CONF_MANAGER_WATCHER_H
