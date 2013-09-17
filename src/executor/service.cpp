/********************************
 FileName: executor/service.cpp
 Author:   WangMin
 Date:     2013-08-14
 Version:  0.1
 Description: executor service
*********************************/

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "executor/task_entity_pool.h"
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

    if (TaskPoolI::Instance()->Find(ptr))
    {
        LOG4CPLUS_ERROR(logger, "The task had exist in the executor, id:" << ptr->GetId());
        TaskPoolI::Instance()->PrintAll();
        return false;
    }
    else
    {
        TaskPoolI::Instance()->Insert(ptr);
        TaskPoolI::Instance()->PrintAll();
        LOG4CPLUS_INFO(logger, "Insert task into TaskPool, id:" << ptr->GetId());
        return true;
    }
}

bool ExecutorService::StopTask(const int32_t task_id) {
    // return TaskPoolI::Instance()->StopTask(task_id);
}

bool ExecutorService::KillTask(const int32_t task_id) {
    return TaskPoolI::Instance()->KillTaskById(task_id);
}

void ExecutorService::SendVMHeartbeat(const string& heartbeat_ad) {
}

//void ExecutorService::GetMachineInfo(string& info) {
//}
