/********************************
 FileName: executor/config.cpp
 Author:   WangMin
 Date:     2013-08-20
 Version:  0.1
 Description: config for executor
*********************************/

#include "gflags/gflags.h"

DEFINE_int32(port, 9997, "executor port");
DEFINE_string(collector_endpoint, "127.0.0.1:9998", "collector endpoint");
DEFINE_string(scheduler_endpoint, "127.0.0.1:9999", "scheduler endpoint");
DEFINE_int32(heartbeat_interval, 15, "heartbeat interval");
DEFINE_string(interface, "br0", "network interface");
DEFINE_string(img_dir, "/var/lib/libvirt/images/", "img template dir");
DEFINE_string(log_path, "../log/", "executor log path");

// DEFINE_string(work_directory, "/tmp/cello", "cellet work directory");
// DEFINE_string(dfs_ip, "", "distributed file system server ip");
// DEFINE_int32(dfs_port, 0, "distributed file system server port");
// DEFINE_string(policy_file, "", "policy configuration file");
