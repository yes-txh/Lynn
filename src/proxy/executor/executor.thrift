include "../scheduler/scheduler.thrift"
include "../collector/collector.thrift"

enum TaskEntityState {
    TASKENTITY_NOTFOUND,
    TASKENTITY_WAITING,
    TASKENTITY_STARTING,
    TASKENTITY_STARTED,
    TASKENTITY_STOPED,
    TASKENTITY_FINISHED,
    TASKENTITY_FAILED,
}

enum VMType {
    VM_UNKNOWN,
    VM_KVM,
    VM_LXC,
}

enum VMState {
    VM_NOTFOUND,
    VM_OFFLINE,
    VM_ONLINE,
    VM_SERVICE_ONLINE,
}

service Executor {
    i32 Helloworld(),
    void SendVMHeartbeat(1: string heartbeat_ad),
    // string GetMachineInfo(), // register static info of Machine to Collector
    bool StartTask(1: string task_ad),
    bool StopTask(1: i32 job_id, 2: i32 task_id),
    bool KillTask(1: i32 job_id, 2: i32 task_id),
}
