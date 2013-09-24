/********************************
 FileName: executor/service.h
 Author:   WangMin
 Date:     2013-08-14
 Version:  0.1
 Description: executor service
*********************************/

#ifndef SRC_EXECUTOR_SERVICE_H
#define SRC_EXECUTOR_SERVICE_H

#include "include/proxy.h"

using std::string;

class ExecutorService : public ExecutorIf {
public:
    int32_t  Helloworld();

    bool StartTask(const string& info);

    bool StopTask(const int32_t job_id, const int32_t task_id);

    bool KillTask(const int32_t job_id, const int32_t task_id);

    bool SendVMHeartbeat(const VM_HbVMInfo& hb_vm_info);

    // void GetMachineInfo(string& info);
};

#endif
