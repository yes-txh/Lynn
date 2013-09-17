/********************************
 FileName: executor/vm_pool.h
 Author:   WangMin
 Date:     2013-09-04
 Version:  0.1
 Description: the pool of vm, include kvm and lxc
*********************************/

#ifndef SRC_EXECUTOR_VM_POOL_H
#define SRC_EXECUTOR_VM_POOL_H

#include <map>
#include <queue>
#include <vector>
#include <tr1/functional>
#include <sys/types.h>

#include "common/clynn/singleton.h"
#include "common/clynn/rwlock.h"
#include "executor/vm.h"
#include "executor/kvm.h"

using std::map;
using std::queue;
using std::vector;
using std::tr1::function;
using std::tr1::placeholders::_1;
using std::tr1::placeholders::_2;
using clynn::RWLock;

class VMPool {
public:
    // function pointer, from std::tr1::function
    typedef function<void(VM*)> VMFunc;

    // TODO print all
    void PrintAll();

    // insert VMPtr into map and queue
    void Insert(const VMPtr& ptr);

    // insert VMPtr into map
    void InsertIntoPool(const VMPtr& ptr);

    // insert VMPtr into queue
    void InsertIntoQueue(const VMPtr& ptr);

    // delete VMPtr from map
    void DeleteFromPool(const int64_t id);

    // find vm by task id
    bool FindByTaskId(const int64_t id);

    // @brief: find a waiting VM, and start it
    int32_t StartVM();

    // @brief: stop vm by task id 
    bool StopVMByTaskId(const int64_t task_id);

    // @brief: kill vm by task id
    bool KillVMByTaskId(const int64_t task_id);

    // get VMPtr from 
    VMPtr GetVMPtr(int64_t id);
   
    // get all HbVMInfo
    vector<HbVMInfo> GetAllHbVMInfo();

private:
    RWLock m_lock;
    // task_id, VMPtr
    map<int64_t, VMPtr> m_vm_map;
    // to be processed queue
    queue<VMPtr> m_queue;
};

typedef Singleton<VMPool> VMPoolI;

#endif 
