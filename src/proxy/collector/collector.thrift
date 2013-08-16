service Collector {
    void SendHeartbeat(1: string heartbeat_ad),
    string MatchMachine(1: string job_ad),
    string QueryMachine(1: string machine),
    list<string> ListAllMachines(),
}
