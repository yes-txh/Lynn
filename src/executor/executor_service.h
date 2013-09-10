/********************************
 FileName: executor/executor_service.h
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

    void SendVMHeartbeat(const string& heartbeat_ad);

    // void GetMachineInfo(string& info);

    bool StartTask(const string& info);

    bool StopTask(const int32_t task_id);

    bool KillVM(const int32_t task_id);
};

#endif
