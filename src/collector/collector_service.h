/********************************
 FileName: collector/collector_service.h
 Author:   ZhangZhang
 Description: collector service
*********************************/

#ifndef SRC_COLLECTOR_SERVICE_H
#define SRC_COLLECTOR_SERVICE_H

#include "include/proxy.h"

using std::string;
using std::vector;

class CollectorService : public CollectorIf
{
public:
    void SendHeartbeat(const string& heartbeat_ad);
    bool RegistMachine(const string& machine_ad);
    void MatchMachine(string& result, const string& job_ad);
    void QueryMachine(string& result, const string& machine);
    void ListAllMachines(vector<string>& result);
};

#endif
