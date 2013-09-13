#ifndef XCUBE_DISTRIBUTE_LOCK_PUB_FUNC_H_
#define XCUBE_DISTRIBUTE_LOCK_PUB_FUNC_H_

#include "common/distribute_lock/distribute_lock.h"
#include "common/distribute_lock/error_code.h"

namespace distribute_lock
{
    void Watcher(zhandle_t *handle, int type, int state, const char *path, void *context);
    void LockWatcher(zhandle_t *zh, int type, int state, const char *path, void * wathcherCtx);
    int Vstrcmp(const void* str1, const void* str2);
    void SortChildren(struct String_vector *vectorst);
}///namespace
#endif ///XCUBE_DISTRIBUTE_LOCK_PUB_FUNC_H_
