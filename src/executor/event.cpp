/********************************
 FileName: executor/event.cpp
 Author:   WangMin
 Date:     2013-09-24
 Version:  0.1
 Description: event, and its handler
*********************************/

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <gflags/gflags.h>

#include "executor/event.h"
#include "executor/task_entity_pool.h"
#include "executor/vm_pool.h"

using log4cplus::Logger;

static Logger logger = Logger::getInstance("executor");

// task is killed

// kill task
bool KillActionEvent::Handle() {
    TaskID id = GetID();
    LOG4CPLUS_INFO(logger, "KillActionEvent, begin to kill task, job_id:" << id.job_id << ", task_id:" << id.task_id);

    if (!TaskPoolI::Instance()->KillTaskByID(id)) {
        LOG4CPLUS_ERROR(logger, "Fails to kill task, job_id:" << id.job_id << ", task_id:" << id.task_id);
        return false;
    }

    TaskPoolI::Instance()->Delete(id);
    LOG4CPLUS_INFO(logger, "Kill task successfully, job_id:" << id.job_id << ", task_id:" << id.task_id);
    return true;
}

bool StopActionEvent::Handle() {
    TaskID id = GetID();
    LOG4CPLUS_INFO(logger, "StopActionEvent, begin to kill task, job_id:" << id.job_id << ", task_id:" << id.task_id);

    return true;
}

// install app
bool InstallAppEvent::Handle() {
    TaskID id = GetID();
    LOG4CPLUS_INFO(logger, "InstallAppEvent, begin to install app for task, job_id:" << id.job_id << ", task_id:" << id.task_id);
   
    VMPtr ptr;
    ptr = VMPoolI::Instance()->GetVMPtr(id);
    if (!ptr) {
        LOG4CPLUS_ERROR(logger, "No the vm, can't install app, job_id:" << id.job_id << ", task_id:" << id.task_id);
        return false;
    }

    ptr->InstallApp();
    return true;
}

// start app
bool StartAppEvent::Handle() {
    TaskID id = GetID();
    LOG4CPLUS_INFO(logger, "StartAppEvent, begin to start app for task, job_id:" << id.job_id << ", task_id:" << id.task_id);

    VMPtr ptr;
    ptr = VMPoolI::Instance()->GetVMPtr(id);
    if (!ptr) {
        LOG4CPLUS_ERROR(logger, "No the vm, can't start app, job_id:" << id.job_id << ", task_id:" << id.task_id);
        return false;
    }

    // TODO
    ptr->Execute();
    return true;
}

