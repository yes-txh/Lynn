/********************************
 FileName: scheduler/scheduler_service.h
 Author:   ZhangZhang
 Date:     2013-08-19
 Version:  0.1
 Description: scheduler service
*********************************/

#ifndef SRC_SCHEDULER_SERVICE_H
#define SRC_SCHEDULER_SERVICE_H

#include "include/proxy.h"

using std::string;
using std::vector;

class SchedulerService : public SchedulerIf
{
public:
    int64_t SubmitJob(const string& job_ad);
    bool ReportTaskState(int64_t job_id, int64_t task_id, TaskState::type task_state);
};

#endif
