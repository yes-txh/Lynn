/********************************
 FileName: executor/type.h
 Author:   WangMin
 Date:     2013-08-27
 Version:  0.1
 Description: type, struct
*********************************/

#ifndef SRC_EXECUTOR_TYPE_H
#define SRC_EXECUTOR_TYPE_H

#include <sys/types.h>
// basic data struct in Linux, such as size_t, time_t, pid_t
#include <sys/ipc.h>
// inter process communication
#include <string>
#include <vector>

#include "include/proxy.h"

using std::string;
using std::vector;

// TaskID: job id + task id
struct TaskID {
    int32_t job_id;
    int32_t task_id;
    
    // overwrite comparison operators
    bool operator <(const TaskID& other) const  {
       if (job_id < other.job_id)
           return true;
       else if (job_id == other.job_id)  //如果类型相同，按比例尺升序排序
           return task_id < other.task_id;
       return false;
   }
};

// app
struct AppInfo {
    string name;
    // string os;
    // outside vm, hdfs
    string app_src_path;  // source path
    string app_out_dir;   // result out dir

    // inside vm
    string install_dir;  // install directory in VM
    string exe_path;     // execute path 
    string stop_path;    // stop
    string out_dir;

    // string user;
};

// resource vm
struct VMInfo {
    
    // resource
    int32_t memory;
    int32_t vcpu;
    string ip;
    int32_t port;
    string os;

    // only for kvm 
    string img_template;
    string iso_location;
    int32_t size;
    int32_t vnc_port;
};

struct TaskInfo {
    TaskID id;
    // vm resource
    VMType::type type;
    VMInfo vm_info;
    // app
    bool is_run;
    AppInfo app_info;
};

struct HbVMInfo {
    TaskID id;  // job_id + task_id, vm id
    string name; 
    VMType::type type;
    double cpu_usage;
    double memory_usage;
    int32_t bytes_in;
    int32_t bytes_out;
    VMState::type state;
    bool app_running;
};

struct HbMachineInfo {
    string ip;
    int32_t port;
    double cpu_usage;
    double memory_usage;
    int32_t bytes_in;
    int32_t bytes_out;
    vector<HbVMInfo> hb_vminfo_list;
};

#endif
