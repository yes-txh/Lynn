include "../scheduler/scheduler.thrift"
include "../collector/collector.thrift"

service Executor {
    i32 Helloworld(),
    void SendVMHeartbeat(1: string heartbeat_ad),
    string GetMachineInfo(), // register static info of Machine to Collector
    bool StartTask(1: string task_ad),
    bool StopTask(1: i32 task_id),
    bool KillVM(1: i32 task_id ),
}
