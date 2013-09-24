include "../executor/executor.thrift"

service VMWorker {
    bool test(1:i32 id, 2:string str),
    bool InstallApp(1:VMAppInfo app_info),
    bool StartApp(1:VMAppInfo app_info),
    bool StopApp(1:i32 id, 2:string stop),
}
