#include <iostream>
#include "zookeeper/zookeeper.h"
#include "common/distribute_lock/distribute_lock.h"
#include "common/distribute_lock/event_mask.h"

int main(int argc, char *argv[])
{
    std::string node;
    if( argc < 2 ){
        node = "/node";
    }
    else
    {
        node = std::string(argv[1]);
    }
    using namespace distribute_lock;
    std::string ip_port_group = "xaec.zk.oa.com:2181/zk/xaec/xcube/master_test";
    EventListener listener;
    DistributeLock dist_lock;
    int ret = dist_lock.Init(ip_port_group.c_str(), &listener, 2000, "log");

    ret = dist_lock.Exists(node);
    if( ret != 0 ){
        ret = dist_lock.CreateNode(node);
    }
    ret = dist_lock.Lock(node);
    printf("%s acquire the lock.", node.c_str());
    while( true ){
        sleep(1);
    }
    return 0;
}
