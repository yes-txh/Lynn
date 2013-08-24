/********************************
 FileName: collector/collector_service.cpp
 Author:   ZhangZhang
 Description: collector service
*********************************/

#include "collector/collector_service.h"

void CollectorService::SendHeartbeat(const string& heartbeat_ad)
{
    printf("SendHeartbeat\n");
}

bool CollectorService::RegistMachine(const string& machine_ad){
    printf("RegistMachine\n");
    return true;
}

void CollectorService::MatchMachine(string& result, const string& job_ad){
    printf("MatchMachinei\n");
}

void CollectorService::QueryMachine(string& result, const string& machine){
    printf("QueryMachine\n");
}

void CollectorService::ListAllMachines(vector<string>& result){
    printf("ListAllMachines\n");
}
