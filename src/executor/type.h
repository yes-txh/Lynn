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

#include "include/proxy.h"

using std::string;

// app
struct AppInfo {
    string name;
    string os;
    // outside vm, hdfs
    string src_path;     // source path
    string job_out_dir; // result out dir

    // inside vm
    string install_dir;  // install directory in VM
    string exe_path;     // execute path 
    string stop_path;    // stop
    string exe_out_dir;

    string user;
};

// resource vm
struct VMInfo {
    VMType::type type;
    string os;
    // resource
    int32_t memory;
    int32_t vcpu;

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
    VMInfo vm_info;
    // app
    bool is_run;
    AppInfo app_info;
};
 
#endif
