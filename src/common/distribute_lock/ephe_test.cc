#include <iostream>
#include "zookeeper/zookeeper.h"
#include "common/distribute_lock/distribute_lock.h"
#include "common/distribute_lock/event_listener.h"


int main(int argc, char *argv[])
{
    using namespace distribute_lock;
    std::string ip_port_group = "xaec.zk.oa.com:2181/zk/xaec/xcube/master_test";
    DistributeLock dist_lock;
    EventListener listener;
    dist_lock.Init(ip_port_group.c_str(), &listener, 4000, "log");
    while(true)
    {
        sleep(5);
    }
    return 0;
}
