/********************************
 FileName: executor/task_entity.cpp
 Author:   WangMin
 Date:     2013-08-27
 Version:  0.1
 Description: task entity in executor, corresponds to task, corresponds to kvm/lxc
*********************************/

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>

#include <classad/classad.h>
#include <classad/classad_distribution.h>

#include "task_entity.h"

using lynn::WriteLocker;
using lynn::ReadLocker;

TaskEntity::TaskEntity(const string& task_info, bool& ret) {
    // classad init, string task_info --> ClassAd *ad_ptr
    ClassAdParser parser;
    ClassAd* ad_ptr = parser.ParseClassAd(task_info);
    ret = true;

    if (!ad_ptr->EvaluateAttrNumber("ID", m_info.id)) {
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrNumber("JOB_ID", m_info.job_id)) {
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrBool("IS_RUN", m_info.is_run)) {
        ret = false;
        return;
    }

    int32_t type = -1;
    if (!ad_ptr->EvaluateAttrNumber("VMTYPE", type)) {
        ret = false;
        return;
    }
    m_info.type = VMType::type(type);

    if (!ad_ptr->EvaluateAttrString("OS", m_info.vm_info.os)) {
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrNumber("MEMORY", m_info.vm_info.memory)) {
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrNumber("VCPU", m_info.vm_info.vcpu)) {
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrString("IP", m_info.vm_info.ip)) {
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrNumber("PORT", m_info.vm_info.port)) {
        ret = false;
        return;
    }

    // only for kvm 
    if (!ad_ptr->EvaluateAttrString("IMG", m_info.vm_info.img_template)) {
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrString("ISO", m_info.vm_info.iso_location)) {
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrNumber("SIZE", m_info.vm_info.size)) {
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrNumber("VNC_PORT", m_info.vm_info.vnc_port)) {
        ret = false;
        return;
    }

    m_state = TaskEntityState::TASKENTITY_WAIT;
    m_percentage = 0;
}

TaskEntityState::type TaskEntity::GetState() {
    ReadLocker locker(m_lock);
    return m_state;
}

double TaskEntity::GetPercentage() {
    ReadLocker locker(m_lock);
    return m_percentage;
}

bool TaskEntity::SetStates(const TaskEntityState::type state, const double percentage) {
   WriteLocker locker(m_lock);
   m_state = state;
   m_percentage = percentage;
   return true;
}

bool TaskEntity::SetState(const TaskEntityState::type state) {
   WriteLocker locker(m_lock);
   m_state = state;
   return true;
}

bool TaskEntity::SetPercentage(const double percentage) {
   WriteLocker locker(m_lock);
   m_percentage = percentage;
   return true;
}

// TODO
bool TaskEntity::Start() {
    WriteLocker locker(m_lock);

    // TODO
    // ...

    // change executor state into running
    m_state = TaskEntityState::TASKENTITY_RUN;

    return true;
}

bool TaskEntity::Stop() {
    return true;
}

bool TaskEntity::Kill() {
    return true;
}

