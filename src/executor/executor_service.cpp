/********************************
 FileName: executor/executor_service.cpp
 Author:   WangMin
 Date:     2013-08-14
 Version:  0.1
 Description: executor service
*********************************/

#include "executor/executor_service.h"

int32_t ExecutorService::Helloworld()
{
    printf("Hello world\n");
    return 0;
}

void ExecutorService::SendVMHeartbeat(const string& heartbeat_ad) {
}

void ExecutorService::GetMachineInfo(string& info) {
}

bool ExecutorService::StartTask(const string& info) {
}

bool ExecutorService::StopTask(const int32_t task_id) {
}

bool ExecutorService::KillVM(const int32_t task_id) {
}

