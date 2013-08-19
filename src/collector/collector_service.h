/********************************
 FileName: collector/colletor_service.h
 Author:   ZhangZhang
 Date:     2013-08-16
 Version:  0.1
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
    void MatchMachine(string& result, const string& job_ad);
    void QueryMachine(string& result, const string& machine);
    void ListAllMachines(vector<string>& result);
};

#endif