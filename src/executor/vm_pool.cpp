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
using clynn::ReadLocker;
using clynn::WriteLocker;
  
static Logger logger = Logger::getInstance("executor");

// TODO just for test
void VMPool::PrintAll() {
    ReadLocker locker(m_lock);
    printf("vm_pool.cpp\n");
    printf("************ VMs ************\n");
    for (map<TaskID, VMPtr>::iterator it = m_vm_map.begin();
         it != m_vm_map.end(); ++it) {
        printf("VM job_id:%d, task_id:%d\n", (it->first).job_id, (it->first).task_id);
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
    m_vm_map[ptr->GetID()] = ptr;
}

// insert VMPtr into queue
void VMPool::InsertIntoQueue(const VMPtr& ptr) {
    WriteLocker locker(m_lock);
    m_queue.push(ptr);
}

// delete VMPtr from map
void VMPool::DeleteFromPool(const TaskID id) {
    WriteLocker locker(m_lock);
    // erase() will invoke destructor(xi gou) func
    m_vm_map.erase(id);
}

bool VMPool::FindByTaskID(const TaskID id) {
    ReadLocker locker(m_lock);
    // ptr->GetID() is TaskID(job_id, task_id)
    map<TaskID, VMPtr>::iterator it = m_vm_map.find(id);
    return it != m_vm_map.end();
}

// @brief: find a waiting VM, and start it
int32_t VMPool::StartVM() {
    WriteLocker locker(m_lock);
    // queue is empty
    if (m_queue.empty()) {
        return -1;
    }
    
    // get first vm_ptr
    VMPtr ptr = m_queue.front();
    m_queue.pop();
    
    // vm is exist in pool(map)?
    TaskID id = ptr->GetID();
    map<TaskID, VMPtr>::iterator it = m_vm_map.find(id);
    if (m_vm_map.end() == it) {
        LOG4CPLUS_ERROR(logger, "Can't find the VM in the Pool, job_id:" << id.job_id << ", task_id:" << id.task_id);
        LOG4CPLUS_ERROR(logger, "Fails to start the VM, job_id:" << id.job_id << ", task_id:" << id.task_id); 
        return -1;
    }

    LOG4CPLUS_INFO(logger, "Begin to start the VM, job_id:" << id.job_id << ", task_id:" << id.task_id);
   
    // create enviroment 
    if (0 == ptr->CreateEnv()) {
        // success start
        LOG4CPLUS_INFO(logger, "Create environment successfully for VM, name:" << ptr->GetName() << ", job_id:" << id.job_id << ", task_id:" << id.task_id);
    } else {
        LOG4CPLUS_INFO(logger, "Fail to create environment for VM, name:" << ptr->GetName() << ", job_id:" << id.job_id << ", task_id:" << id.task_id);
        return -1;
    }

    // execute 
    if (ptr->Execute()) {
        // success execute
        LOG4CPLUS_INFO(logger, "Execute successfully for VM, name:" << ptr->GetName() << ", job_id:" << id.job_id << ", task_id:" << id.task_id);
    } else {
        LOG4CPLUS_INFO(logger, "Fail to execute for VM, name:" << ptr->GetName() << ", job_id:" << id.job_id << ", task_id:" << id.task_id);
        return -1;
    }

    return 0;
}

// kill vm by task id
bool VMPool::KillVMByTaskID(const TaskID id) {
    WriteLocker locker(m_lock);

    // find the vm
    map<TaskID, VMPtr>::iterator it = m_vm_map.find(id);
    if (m_vm_map.end() == it) {
        LOG4CPLUS_ERROR(logger, "Can't find the VM, job_id:" << id.job_id << ", task_id:" << id.task_id);
        LOG4CPLUS_INFO(logger, "Kill task directly, job_id:" << id.job_id << ", task_id:" << id.task_id);
        return true;
    }

    if (!(it->second)->Kill()) {
        LOG4CPLUS_ERROR(logger, "Fails to kill VM, job_id:" << id.job_id << ", task_id:" << id.task_id);
        return false;
    }

    // Delete task from Pool(map)
    m_vm_map.erase(id);
    LOG4CPLUS_INFO(logger, "Kill VM successfully, job_id:" << id.job_id << ", task_id:" << id.task_id);
    
    return true;
}

// get VMPtr from 
VMPtr VMPool::GetVMPtr(const TaskID id) {
    ReadLocker locker(m_lock);
    map<TaskID, VMPtr>::iterator it = m_vm_map.find(id);
    if (it != m_vm_map.end()) {
        return it->second;
    }
    // not find then return NULL
    return VMPtr();
}

vector<HbVMInfo> VMPool::GetAllHbVMInfo() {
    ReadLocker locker(m_lock);
    vector<HbVMInfo> vm_list;
    for (map<TaskID, VMPtr>::iterator it = m_vm_map.begin();
        it != m_vm_map.end(); ++it) {
        // TODO
        // if((it->second)->GetState() != VMState::VM_OFFLINE){
            vm_list.push_back((it->second)->GetHbVMInfo());
        // }
    }
    return vm_list;
}
