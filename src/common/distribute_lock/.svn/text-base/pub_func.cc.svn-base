#include "common/distribute_lock/pub_func.h"
#include "glog/logging.h"

namespace distribute_lock
{
void Watcher(zhandle_t *handle, int type, int state, const char *path, void *context)
{
    DistributeLock* dist_lock = (DistributeLock*) context;
    ///
    std::string node_path = std::string(path);
    LOG(INFO) <<" watcher " << std::string(TypeString(type)) << " event state = " <<
        std::string(StateString(state)) << ", path = " << node_path <<
        ", handle = " << dist_lock->GetZKHandle();
    if( type == ZOO_SESSION_EVENT )
    {
        if( state == ZOO_EXPIRED_SESSION_STATE )
        {
            VLOG(1) << "ZOO_EXPIRED_SESSION_STATE, path = " << node_path;
            if (dist_lock->IsWaitInit()) {
                VLOG(1) << "In init mode, encounter session expire. Init with invalid session?";
                dist_lock->GetSyncEvent().Set();
            } else {
                dist_lock->OnSessionExpired();
            }
        }
        else if (state == ZOO_CONNECTING_STATE)
        {
            VLOG(1) << "ZOO_CONNECTING_STATE";
        }
        else if (state == ZOO_CONNECTED_STATE)
        {
            VLOG(1) << "ZOO_CONNECTED_STATE";
            if (dist_lock->IsWaitInit()) {
                VLOG(1) << "In init mode, watcher send notify event.";
                dist_lock->GetSyncEvent().Set();
            }
        }
    }
    else if (type == ZOO_CHANGED_EVENT) ///<当节点发生改变时
    {
        VLOG(3) << "ZOO_CHANGED_EVENT, path = " << node_path;
        dist_lock->OnNodeChanged(path, state);
    }
    else if (type == ZOO_DELETED_EVENT) ///<当node被删除的时候
    {
        VLOG(3) <<  "ZOO_DELETED_EVENT. node = " << node_path;
        dist_lock->OnNodeDeleted(path, state);
    }
    else if (type == ZOO_CHILD_EVENT) ///<当一个node的子node发生变化的时候
    {
        VLOG(3) << "CHILD_EVENT, node = " << node_path << ", state = " << StateString(state);
        dist_lock->OnChildNode(path, state);
    }
    else if (type == ZOO_CREATED_EVENT) ///<当不存在的节点被创建的时候
    {
        VLOG(3) << "ZOO_CREATED_EVENT. node = "  << node_path;
        dist_lock->OnNodeCreate(path, state);
    }
}


void LockWatcher(zhandle_t *zh, int type, int state, const char *path, void * wathcherCtx)
{
    DistributeLock* lock = (DistributeLock*) wathcherCtx;
    VLOG(1) << "lock watcher call back. node = " << std::string(path);
    if( type == ZOO_DELETED_EVENT ) ///<当node被删除的时候
    {
        VLOG(1) << "ZOO_DELETED_EVENT, node = " << std::string(path);
        lock->GetWaitLockSyncEvent().Set();
    }
    else
    {
        VLOG(1) <<  "node = " << std::string(path) << ", event type = " <<
            std::string(TypeString(type)) <<
             ", state = " << std::string(StateString(state));
    }
}

int Vstrcmp(const void* str1, const void* str2)
{
    const char **a = (const char**) str1;
    const char **b = (const char**) str2;
    return strcmp(strrchr(*a, '-')+1, strrchr(*b, '-')+1);
}

void SortChildren(struct String_vector *vectorst)
{
    qsort(vectorst->data, vectorst->count, sizeof(char*), &Vstrcmp);
}

} // namespace distribute_lock
