#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "conf_manager/watcher.h"
#include "conf_manager/data_completion.h"

using log4cplus::Logger;

static Logger logger = Logger::getInstance("collector");

/*
static void yield(zhandle_t *zh, int i) {
    usleep(i);
}
*/

watchctx_t::watchctx_t() {
    connected = false;
    zh = 0;
    pthread_rwlock_init(&ev_lock, NULL);
}

watchctx_t::~watchctx_t() {
    if (zh) {
        zookeeper_close(zh);
        zh = 0;
    }
    pthread_rwlock_destroy(&ev_lock);
}

// get the event
evt_t watchctx_t::GetEvent() {
    evt_t evt;
    pthread_rwlock_rdlock(&ev_lock);
    evt = events.front();
    events.pop_front();
    pthread_rwlock_unlock(&ev_lock);
    return evt;
}
// get the number os events
int watchctx_t::CountEvents() {
    int count;
    pthread_rwlock_rdlock(&ev_lock);
    count = events.size();
    pthread_rwlock_unlock(&ev_lock);
    return count;
}

void watchctx_t::PutEvent(evt_t evt) {
    pthread_rwlock_rdlock(&ev_lock);
    events.push_back(evt);
    pthread_rwlock_unlock(&ev_lock);
}

bool watchctx_t::WaitForConnected(zhandle_t *zk) {
    int expires = 10000;
    int pass = 0;
    while (!connected && pass < expires) {
        usleep(1);
        // yield(zk, 1);
        ++pass;
    }
    LOG4CPLUS_INFO(logger,"THE TIMEOUT IS : " << pass);
    return connected;
}
#if 0
/// will be modified later
bool watchctx_t::waitForDisconnected(zhandle_t *zk) {
    time_t expires = time(0) + 15;
    while (connected && time(0) < expires) {
        yield(zk, 1);
    }
    return !connected;
}
#endif

// watcher func
void Watcher(zhandle_t *zh, int type, int state,
             const char *path, void*v) {
    watchctx_t *ctx = reinterpret_cast<watchctx_t*>(v);

    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE) {
            ctx->connected = true;
            const clientid_t *id = zoo_client_id(zh);
            LOG4CPLUS_INFO(logger,"Connect to server success, session id: "
                      << id->client_id);
        } else if (state == ZOO_AUTH_FAILED_STATE) {
            LOG4CPLUS_ERROR(logger,"Authentication failure. Shutting down...");
            zh = NULL;
        } else if (state == ZOO_EXPIRED_SESSION_STATE)   {
            LOG4CPLUS_ERROR(logger,"state == ZOO_EXPIRED_SESSION_STATE..........");
            zh = NULL;
        }
    }
#if 0
    if (type != ZOO_SESSION_EVENT) {
        evt_t evt;
        evt.path = path;
        evt.type = type;
        ctx->putEvent(evt);
    }
#endif
}

// watcher func
void ValueWatcher(zhandle_t *zh, int type, int state,
                  const char *path, void*v) {
    watchctx_t* ctx = reinterpret_cast<watchctx_t*>(v);

    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE) {
            ctx->connected = true;
            const clientid_t *id = zoo_client_id(zh);
            LOG4CPLUS_INFO(logger,"Connect to server success, session id: "
                      << id->client_id);
        } else if (state == ZOO_AUTH_FAILED_STATE) {
            LOG4CPLUS_ERROR(logger,"Authentication failure. Shutting down...");
            zh = NULL;
        } else if (state == ZOO_EXPIRED_SESSION_STATE)   {
            LOG4CPLUS_ERROR(logger,"state == ZOO_EXPIRED_SESSION_STATE..........");
            zh = NULL;
        }
    }
    if (type != ZOO_SESSION_EVENT) {
        evt_t evt;
        evt.path = path;
        evt.type = type;
        ctx->PutEvent(evt);
        if (NULL != zh) {
            int rt = zoo_aget(zh, path, 1, DataCompletion, path);
            if (rt != ZOK) {
                LOG4CPLUS_INFO(logger,"error happens when zoo_aget: " << zerror(rt));
            }
        }
    }
}





