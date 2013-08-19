/********************************
 FileName: scheduler/scheduler_service.cpp
 Author:   ZhangZhang
 Date:     2013-08-16
 Version:  0.1
 Description: scheduler service
*********************************/

#include "scheduler/scheduler_service.h"

int64_t SchedulerService::SubmitJob(const string& job_ad)
{
    printf("SendHeartbeat\n");
    return 0;
}

bool SchedulerService::ReportTaskState(int64_t job_id, int64_t task_id, TaskState::type state){
    printf("MatchMachinei\n");
    return true;
}
