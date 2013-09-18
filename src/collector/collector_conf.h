#ifndef COLLECTOR_COLLECTOR_CONF_H
#define COLLECTOR_COLLECTOR_CONF_H

#include <gflags/gflags.h>

///cluster_name
DECLARE_string(cluster_name);

/// 是否以daemon启动collector
DECLARE_bool(is_daemon);

/// collector的lockfile，默认为/var/run/lynn/collector.pid,
///// 保存了collector进程的pid
DECLARE_string(lockfile);

/// 本机ip
DECLARE_string(IP);

/// 向scheduler汇报宕机信息的超时时间
DECLARE_int32(report_schedd_timeout);

/// collector的endpoint
DECLARE_string(collector_endpoint);

/// 上报周期，该配置项与report_timeout相关
DECLARE_int32(heartbeat_interval);

/// scheduler的endpoint，用于collector上报宕机信息
DECLARE_string(scheduler_endpoint);

/// ZooKeeper的server信息
DECLARE_string(zk_server);

#endif
