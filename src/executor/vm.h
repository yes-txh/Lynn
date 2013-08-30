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
#include "common/rwlock.h"
#include "executor/type.h"

using std::string;
using boost::shared_ptr;
using lynn::RWLock;

class VM {
public:
    // TODO
    explicit VM(const TaskInfo& info) {
        m_id = info.id;
        m_info = info;
        m_type = info.type;
        m_first = true;
        m_prev_cpu = 0.0;
        m_prev_total = 0.0;
    }

    ~VM() {}

    int64_t GetId() {
        return m_id;
    }

    TaskInfo GetTaskInfo() {
        return m_info;
    }

    void Hello();

    // key function
    virtual int32_t CreateEnv() = 0; // create enviroment, kvm or lxc

    virtual bool Execute() = 0;  // execute the task, run the app

    virtual bool Stop() = 0;

    virtual bool Kill() = 0;

    virtual HbVMInfo GetHbVMInfo() = 0; // get heartbeart

    // general
    // VMState::type GetVMState();

private:
    virtual void SetName();

private:
    int64_t m_id;
    string m_name;     // vm name, maybe kvm name or lxc name
    VMType::type m_type; // VM_KVM or VM_LXC
    TaskInfo m_info;   // TODO
    RWLock m_lock;

    // m_state 
    time_t m_start_time;
    // report resource, is first?
    bool m_first;
    double m_prev_cpu;
    double m_prev_total;
};

typedef shared_ptr<VM> VMPtr;

#endif
