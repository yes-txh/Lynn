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
#include <tr1/functional>
#include <sys/types.h>

#include "common/singleton.h"
#include "common/rwlock.h"
#include "executor/vm.h"
#include "executor/kvm.h"

using std::map;
using std::queue;
using std::tr1::function;
using std::tr1::placeholders::_1;
using std::tr1::placeholders::_2;
using lynn::RWLock;

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

    // delete VMPtr from map and queue
    // void Delete(const int64_t id);
 
    // delete VMPtr from map
    void DeleteFromPool(const int64_t id);

    // delete VMPtr from queue
    // void DeleteFromQueue(const int64_t id);

    // @brief: find a waiting VM, and start it
    int32_t StartVM();

    // get VMPtr from 
    VMPtr GetVMPtr(int64_t id);

private:
    RWLock m_lock;
    // task_id, VMPtr
    map<int64_t, VMPtr> m_vm_map;
    // to be processed queue
    queue<VMPtr> m_queue;
};

typedef Singleton<VMPool> VMPoolI;

#endif 
