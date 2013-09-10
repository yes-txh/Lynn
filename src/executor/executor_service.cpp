/********************************
 FileName: executor/executor_service.cpp
 Author:   WangMin
 Date:     2013-08-14
 Version:  0.1
 Description: executor service
*********************************/

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "executor/task_entity_pool.h"
#include "executor/executor_service.h"

using log4cplus::Logger;

static Logger logger = Logger::getInstance("executor");

// test, hello world
int32_t ExecutorService::Helloworld() {
    printf("Hello world\n");
    return 0;
}

void ExecutorService::SendVMHeartbeat(const string& heartbeat_ad) {
}

//void ExecutorService::GetMachineInfo(string& info) {
//}

bool ExecutorService::StartTask(const string& info) {
    bool ret = false; 
    TaskPtr ptr(new TaskEntity(info, ret));
    if (false == ret) {
        LOG4CPLUS_ERROR(logger, "fails to init TaskEntity");
        return false;
    }

    if (TaskPoolI::Instance()->Find(ptr))
    {
        LOG4CPLUS_ERROR(logger, "the task had exist in the executor");
        TaskPoolI::Instance()->PrintAll();
        return false;
    }
    else
    {
        TaskPoolI::Instance()->Insert(ptr);
        TaskPoolI::Instance()->PrintAll();
        return true;
    }
}

bool ExecutorService::StopTask(const int32_t task_id) {
}

bool ExecutorService::KillVM(const int32_t task_id) {
}
