//include "../scheduler/scheduler.thrift"
//include "../collector/collector.thrift"

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

enum AppState {
    APP_NOTFOUND,
    APP_ONLINE,
    APP_FINISHED,
    APP_FAILED,
}

enum EventType {
    STATE_EVENT,
    ACTION_EVENT,
}

struct VM_AppInfo {
    1: i32 id,
    2: string name,
    3: string app_source,
    4: string install_dir,
    5: string exe,
    6: string argument,

    // reserve
    7: string out_dir,     // inside vm
    8: string app_out_dir, // outside vm
    9: string run_type,
    10: i32 interval,
}

struct VM_HbAppInfo {
    1: i32 id,
    2: string name,
    3: AppState state,
    4: i32 error_id,
}

struct VM_HbVMInfo {
    1: i32 job_id,
    2: i32 task_id,
    3: double cpu_usage,
    4: double memory_usage,
    5: i32 bytes_in,
    6: i32 bytes_out,
    7: VMState state,
    8: bool app_running,
    9: VM_HbAppInfo hb_app_info,
}

service Executor {
    i32 Helloworld(),
    // string GetMachineInfo(), // register static info of Machine to Collector
    bool StartTask(1: string task_ad),
    bool StopTask(1: i32 job_id, 2: i32 task_id),
    bool KillTask(1: i32 job_id, 2: i32 task_id),
    bool SendVMHeartbeat(1: VM_HbVMInfo hb_vm_info),
}
