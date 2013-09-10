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

// app
struct AppInfo {
    string name;
    // string os;
    // outside vm, hdfs
    string src_path;     // source path
    string job_out_dir; // result out dir

    // inside vm
    string install_dir;  // install directory in VM
    string exe_path;     // execute path 
    string stop_path;    // stop
    string exe_out_dir;

    // string user;
};

// resource vm
struct VMInfo {
    
    // resource
    int32_t memory;
    int32_t vcpu;

    string os;
    string ip;
    int32_t port;

    // only for kvm 
    string img_template;
    string iso_location;
    int32_t size;
    int32_t vnc_port;
};

struct TaskInfo {
    int32_t id;
    int32_t job_id;
    // vm resource
    VMType::type type;
    VMInfo vm_info;
    // app
    bool is_run;
    AppInfo app_info;
};

struct HbVMInfo {
    int32_t id;  // task id, vm id
    double cpu_usage;
    double memroy_usage;
    int64_t bytes_in;
    int64_t bytes_out;
    VMState::type state;
    bool app_running;
};

struct HbMachineInfo {
    string ip;
    int32_t port;
    double cpu_usage;
    double memory_usage;
    int64_t bytes_in;
    int64_t bytes_out;
    vector<HbVMInfo> hb_vminfo_list;
};

#endif