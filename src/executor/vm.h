/********************************
 FileName: executor/vm.h
 Author:   WangMin
 Date:     2013-08-27
 Version:  0.1
 Description: virtual machine, include kvm and lxc
*********************************/

#ifndef SRC_EXECUTOR_VM_H
#define SRC_EXECUTOR_VM_H

#include <string>
#include <boost/shared_ptr.hpp>

#include "include/proxy.h"
#include "common/clynn/rwlock.h"
#include "executor/type.h"
#include "executor/task_entity_pool.h"

using std::string;
using boost::shared_ptr;
using clynn::RWLock;

class VM {
public:
    // TODO
    explicit VM(const TaskInfo& info) {
        m_id = info.id;
        m_info = info;
        m_type = info.type;
        m_state = VMState::VM_OFFLINE;
    }

    ~VM() {}

    int64_t GetId();

    string GetName();

    TaskInfo GetTaskInfo();

    VMState::type GetState();

    TaskPtr GetTaskPtr();    

    void SetState(VMState::type state);

    void SetNameByString(string name);

    // key function
    // virtual void SetName();

    virtual int32_t CreateEnv() = 0; // create enviroment, kvm or lxc

    virtual bool Execute() = 0;  // execute the task, run the app

    virtual bool Stop() = 0;

    virtual bool Kill() = 0;

    virtual HbVMInfo GetHbVMInfo() = 0; // get heartbeart

    // general

private:
    virtual void SetName() = 0;

private:
    int64_t m_id;
    string m_name;     // vm name, maybe kvm name or lxc name
    VMType::type m_type; // VM_KVM or VM_LXC
    TaskInfo m_info;   // TODO
    VMState::type m_state;
    RWLock m_lock;
    
    time_t m_start_time;
    int32_t m_timestamp;
    int32_t m_time_to_death;
};

typedef shared_ptr<VM> VMPtr;

#endif
