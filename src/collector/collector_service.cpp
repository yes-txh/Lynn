/********************************
 FileName: collector/collector_service.cpp
 Author:   ZhangZhang
 Description: collector service
*********************************/
#include "collector/collector_service.h"
#include "collector/machine_pool.h"


void CollectorService::SendHeartbeat(const string& heartbeat_ad){
    printf("SendHeartbeat\n");
}

int32_t CollectorService::RegistMachine(const string& machine_ad){
    MachinePtr machine_ptr(new Machine());
    if(machine_ptr->ParseAttr(machine_ad) != 0){
         return 1;
    }

    //TODO 
    //write into zookeeper

    MachinePoolI::Instance()->Insert(machine_ptr);
    return 0;
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
