service Collector {
    void SendHeartbeat(1: string heartbeat_ad),
    string MatchMachine(const string& job_ad),
    string QueryMachine(const string& machine),
    list<string> ListAllMachines(),
}
