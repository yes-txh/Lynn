/********************************
 FileName: executor/vm.cpp
 Author:   WangMin
 Date:     2013-08-27
 Version:  0.1
 Description: virtual machine, include kvm and lxc
*********************************/

#include "executor/vm.h"

using clynn::WriteLocker;
using clynn::ReadLocker;

// public
TaskID VM::GetID() {
    // ReadLocker locker(m_lock);
    return m_id;
}

string VM::GetName() {
    // ReadLocker locker(m_lock);
    return m_name;
}

VMType::type VM::GetVMType() {
    return m_type;
}

TaskInfo VM::GetTaskInfo() {
    // ReadLocker locker(m_lock);
    return m_info;
}

VMState::type VM::GetState() {
    ReadLocker locker(m_lock);
    return m_state;
}

TaskPtr VM::GetTaskPtr() {
    return TaskPoolI::Instance()->GetTaskPtr(m_id);
}

bool VM::IsRun() {
    return m_info.is_run;
}

string VM::GetApp() {
    if (!IsRun())
        return "is not run app";
    string app = m_info.app_info.name;
    if ("" == app)
        return "no app name";
    return app;
}

void VM::SetState(VMState::type state) {
    WriteLocker locker(m_lock);
    m_state = state;
}

void VM::SetNameByString(string name) {
    // WriteLocker locker(m_lock);
    m_name = name;
}

