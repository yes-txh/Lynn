/********************************
 FileName: executor/vm_pool.cpp
 Author:   WangMin
 Date:     2013-09-04
 Version:  0.1
 Description: the pool of vm, include kvm and lxc
*********************************/

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "executor/vm_pool.h"

using log4cplus::Logger;
using lynn::ReadLocker;
using lynn::WriteLocker;
  
static Logger logger = Logger::getInstance("executor");

// TODO just for test
void VMPool::PrintAll() {
    ReadLocker locker(m_lock);
    printf("vm_pool.cpp\n");
    printf("************ VMs ************\n");
    for (map<int64_t, VMPtr>::iterator it = m_vm_map.begin();
         it != m_vm_map.end(); ++it) {
        printf("VM: %ld\n", it->first);
    }
    printf("*****************************\n");
}

// insert VMPtr into map and queue
void VMPool::Insert(const VMPtr& ptr) {
    InsertIntoPool(ptr);
    InsertIntoQueue(ptr);
}

// insert VMPtr into map
void VMPool::InsertIntoPool(const VMPtr& ptr) {
    WriteLocker locker(m_lock);
    m_vm_map[ptr->GetId()] = ptr;
}

// insert VMPtr into queue
void VMPool::InsertIntoQueue(const VMPtr& ptr) {
    WriteLocker locker(m_lock);
    m_queue.push(ptr);
}

// delete VMPtr from map
void VMPool::DeleteFromPool(const int64_t id) {
    WriteLocker locker(m_lock);
    // erase() will invoke destructor(xi gou) func
    m_vm_map.erase(id);
}

// @brief: find a waiting VM, and start it
int32_t VMPool::StartVM() {
    WriteLocker locker(m_lock);
    // queue is empty
    if (m_queue.empty())
    {
        // printf("empty!\n");
        return false;
    }
    
    // get first vm_ptr
    VMPtr ptr = m_queue.front();
    m_queue.pop();
    printf("get vm %ld\n", ptr->GetId());
    
    if (0 == ptr->CreateEnv()) {
        // success start
        printf("1\n");
        // printf("start vm %s successfully\n", ptr->GetName());
    } else {
        printf("2\n");
        // printf("fail to start vm %ld\n", ptr->GetId());
        return -1;
    }

    printf("3\n");
    // no this task
    // TaskPtr task_ptr = GetTaskPtr();
    // if (!task_ptr)
    //    return false;
    
    return 0;
}

// get VMPtr from 
VMPtr VMPool::GetVMPtr(int64_t id) {
    ReadLocker locker(m_lock);
    map<int64_t, VMPtr>::iterator it = m_vm_map.find(id);
    if (it != m_vm_map.end()) {
        return it->second;
    }
    // not find then return NULL
    return VMPtr();
}
