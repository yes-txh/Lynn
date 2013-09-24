/********************************
 FileName: executor/task_entity.cpp
 Author:   WangMin
 Date:     2013-08-27
 Version:  0.1
 Description: task entity in executor, corresponds to task, corresponds to kvm/lxc
*********************************/

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include <classad/classad.h>
#include <classad/classad_distribution.h>

#include "include/classad_attr.h"
#include "executor/task_entity.h"
#include "executor/vm_pool.h"

using log4cplus::Logger;
using clynn::WriteLocker;
using clynn::ReadLocker;

static Logger logger = Logger::getInstance("executor");

TaskEntity::TaskEntity(const string& task_info, bool& ret) {
    // classad init, string task_info --> ClassAd *ad_ptr
    ClassAdParser parser;
    ClassAd* ad_ptr = parser.ParseClassAd(task_info);
    ret = true;

    // task overview
    if (!ad_ptr->EvaluateAttrNumber(ATTR_JOB_ID, m_info.id.job_id)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_JOB_ID << " error.");
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrNumber(ATTR_TASK_ID, m_info.id.task_id)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_TASK_ID << " error.");
        ret = false;
        return;
    }

    int32_t type = -1;
    if (!ad_ptr->EvaluateAttrNumber(ATTR_VMTYPE, type)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_VMTYPE << " error.");
        ret = false;
        return;
    }
    m_info.type = VMType::type(type);
    printf("VMType: %d\n", m_info.type);

    if (!ad_ptr->EvaluateAttrBool(ATTR_IS_RUN, m_info.is_run)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_IS_RUN << " error.");
        ret = false;
        return;
    }

    // task vm_info
    if (!ad_ptr->EvaluateAttrNumber(ATTR_MEMORY, m_info.vm_info.memory)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_MEMORY << " error.");
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrNumber(ATTR_VCPU, m_info.vm_info.vcpu)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_VCPU << " error.");
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrString(ATTR_IP, m_info.vm_info.ip)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_IP << " error.");
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrNumber(ATTR_PORT, m_info.vm_info.port)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_PORT << " error.");
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrString(ATTR_OS, m_info.vm_info.os)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_OS << " error.");
        ret = false;
        return;
    }

    // only for kvm 
    if (!ad_ptr->EvaluateAttrString(ATTR_IMG, m_info.vm_info.img_template)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_IMG << " error.");
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrString(ATTR_ISO, m_info.vm_info.iso_location)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_ISO << " error.");
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrNumber(ATTR_SIZE, m_info.vm_info.size)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_SIZE << " error.");
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrNumber(ATTR_VNC_PORT, m_info.vm_info.vnc_port)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_VNC_PORT << " error.");
        ret = false;
        return;
    }

    // is_run = false, no app_info
    if (false == m_info.is_run) {
        // only create vm, not install or run app
        m_id = m_info.id;
        m_state = TaskEntityState::TASKENTITY_WAITING;
        m_percentage = 0;
        return;
    }

    // task, app_info
    if (!ad_ptr->EvaluateAttrNumber(ATTR_APP_ID, m_info.app_info.id)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_APP_ID << " error.");
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrString(ATTR_APP_NAME, m_info.app_info.name)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_APP_NAME << " error.");
        ret = false;
        return;
    }

    // task, app_info, outside vm
    if (!ad_ptr->EvaluateAttrString(ATTR_APP_SRC_PATH, m_info.app_info.app_src_path)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_APP_SRC_PATH << " error.");
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrString(ATTR_APP_OUT_DIR, m_info.app_info.app_out_dir)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_APP_OUT_DIR << " error.");
        ret = false;
        return;
    }

    // task ,app_info, inside vm
    if (!ad_ptr->EvaluateAttrString(ATTR_INSTALL_DIR, m_info.app_info.install_dir)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_INSTALL_DIR << " error.");
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrString(ATTR_EXE_PATH, m_info.app_info.exe_path)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_EXE_PATH << " error.");
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrString(ATTR_STOP_PATH, m_info.app_info.stop_path)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_STOP_PATH << " error.");
        ret = false;
        return;
    }

    if (!ad_ptr->EvaluateAttrString(ATTR_OUT_DIR, m_info.app_info.out_dir)) {
        LOG4CPLUS_ERROR(logger, "Fails to init task entity, because parse " << ATTR_OUT_DIR << " error.");
        ret = false;
        return;
    }

    // init state
    m_id = m_info.id;
    m_state = TaskEntityState::TASKENTITY_WAITING;
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

void TaskEntity::TaskStarted() {
    WriteLocker locker(m_lock);
    m_state = TaskEntityState::TASKENTITY_STARTED;
    m_percentage = 100.0;
    LOG4CPLUS_INFO(logger, "Task has started, job_id:" << m_id.job_id << ", task_id:" << m_id.task_id);
}

void TaskEntity::TaskFinished() {
    WriteLocker locker(m_lock);
    m_state = TaskEntityState::TASKENTITY_FINISHED;
    m_percentage = 100.0;
    LOG4CPLUS_INFO(logger, "Task has finished, job_id:" << m_id.job_id << ", task_id:" << m_id.task_id);
}

void TaskEntity::TaskFailed() {
    WriteLocker locker(m_lock);
    // Kill VM from VMPool
    /*if (!VMPoolI::Instance()->KillVMByTaskID(m_id)) {
        LOG4CPLUS_ERROR(logger, "Fails to kill task, job_id:" << m_id.job_id << ", task_id" << m_id.task_id);
    }*/

    m_state = TaskEntityState::TASKENTITY_FAILED;
    m_percentage = 0.0;
    LOG4CPLUS_INFO(logger, "Task has failed, job_id:" << m_id.job_id << ", task_id:" << m_id.task_id);
}

// TODO
bool TaskEntity::Start() {
    WriteLocker locker(m_lock);
    LOG4CPLUS_INFO(logger, "Begin to start the task, job_id:" << m_id.job_id << ", task_id:" << m_id.task_id);

    if (GetVMType() == VMType::VM_KVM) {
        // init vm
        VMPtr ptr(new KVM(m_info));
        // insert VMPtr into VMPool
        VMPoolI::Instance()->Insert(ptr);
    } else if (GetVMType() == VMType::VM_LXC) {
        // init vm
        VMPtr ptr(new LXC(m_info));
        // insert VMPtr into VMPool
        VMPoolI::Instance()->Insert(ptr);
    } else {
        LOG4CPLUS_ERROR(logger, "Fails to start task, job_id:" << m_id.job_id << ", task_id:" << m_id.task_id 
                        << ", because have no the VMType " << m_info.type);
        m_state = TaskEntityState::TASKENTITY_FAILED;
        return false;
    }

    // change executor state into running
    m_state = TaskEntityState::TASKENTITY_STARTING;

    return true;
}

bool TaskEntity::Stop() {
    return true;
}

bool TaskEntity::Kill() {
    WriteLocker locker(m_lock);
    if (!VMPoolI::Instance()->KillVMByTaskID(m_id)) {
        LOG4CPLUS_ERROR(logger, "Fails to kill task, job_id:" << m_id.job_id << ", task_id" << m_id.task_id);
        return false;
    }
    m_state = TaskEntityState::TASKENTITY_FINISHED;
    return true;
}
