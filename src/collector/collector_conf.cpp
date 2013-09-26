#include "collector/collector_conf.h"

/// ip of collector's host
DEFINE_string(IP, "", "host ip");

DEFINE_string(interface, "", "host interface for connecting");

/// name of my cluster
DEFINE_string(cluster_name, "cluster1", "cluster_name");

/// collector lock file
DEFINE_string(collector_lockfile, "/var/run/lynn/collector.pid", "collector lock file");

/// port to use
DEFINE_string(collector_port, "9618", "collector port");

/// endpoint of collector
DEFINE_string(collector_endpoint, "127.0.0.1:9998", "collector endpoint");

/// cycle of reporting to scheduler
DEFINE_int32(report_schedd_timeout, 16, "report_schedd_timeout");

/// maybe modified later......
DEFINE_bool(is_daemon, true, "daemon");

/// scheduler endpoint
DEFINE_string(scheduler_endpoint, "127.0.0.1:9605", "scheduler endpoint");

/// update interval(s)
DEFINE_int32(heartbeat_interval, 5000, "heartbeat_interval");

/// zk server
DEFINE_string(zk_server, "127.0.0.1:2181", "zk server");

// lock_node of collector, not used any more
DEFINE_string(lock_node, "", "node for lead select");
