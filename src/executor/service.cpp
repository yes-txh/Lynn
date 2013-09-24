/********************************
 FileName: executor/service.cpp
 Author:   WangMin
 Date:     2013-08-14
 Version:  0.1
 Description: executor service
*********************************/

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "executor/type.h"
#include "executor/task_entity_pool.h"
#include "executor/vm_pool.h"
#include "executor/dispatcher.h"
#include "executor/service.h"

using log4cplus::Logger;

static Logger logger = Logger::getInstance("executor");

// test, hello world
int32_t ExecutorService::Helloworld() {
    printf("Hello world\n");
    return 0;
}

bool ExecutorService::StartTask(const string& info) {
    bool ret = false; 
    TaskPtr ptr(new TaskEntity(info, ret));
    if (false == ret) {
        LOG4CPLUS_ERROR(logger, "Fails to init TaskEntity");
        return false;
    }

    if (TaskPoolI::Instance()->Find(ptr)) {
        LOG4CPLUS_ERROR(logger, "The task had exist in the executor, job_id:" << ptr->GetID().job_id << ", task_id:" << ptr->GetID().task_id);
        TaskPoolI::Instance()->PrintAll();
        return false;
    } else {
        TaskPoolI::Instance()->Insert(ptr);
        TaskPoolI::Instance()->PrintAll();
        LOG4CPLUS_INFO(logger, "Insert task into TaskPool, job_id:" << ptr->GetID().job_id << ", task_id:" << ptr->GetID().task_id);
        return true;
    }
}

bool ExecutorService::StopTask(const int32_t job_id, const int32_t task_id) {
    // TaskID id;
    // id.job_id = job_id;
    // id.task_id = task_id;
    // return TaskPoolI::Instance()->StopTask(id);
}

bool ExecutorService::KillTask(const int32_t job_id, const int32_t task_id) {
    TaskID id;
    id.job_id = job_id;
    id.task_id = task_id;
    // new KillActionEvent
    EventPtr event(new KillActionEvent(id));
    // Push event into Queue
    EventDispatcherI::Instance()->Dispatch(event->GetType())->PushBack(event);
    return true;
    // return TaskPoolI::Instance()->KillTaskByID(id);
}

bool ExecutorService::SendVMHeartbeat(const VM_HbVMInfo& hb_vm_info) {
    return VMPoolI::Instance()->ProcessHbVMInfo(hb_vm_info);
}

//void ExecutorService::GetMachineInfo(string& info) {
//}
