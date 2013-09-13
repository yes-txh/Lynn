
#include <string>
#include <common/distribute_lock/distribute_lock.h>
#include <common/distribute_lock/event_mask.h>
#include <common/distribute_lock/listener_test.h>

int main(int argc, char *argv[])
{
    std::string node = "/node";
    using namespace distribute_lock;
    std::string ip_port_group = "172.24.18.209:2181/zk/idc-xaec/soso/xcube/master_test";
    Listener listener;
    DistributeLock dist_lock;
    int ret = dist_lock.Init(ip_port_group.c_str(), &listener, 2000, "log");

    ret = dist_lock.Exists(node);
    if( ret != 0 ){
        ret = dist_lock.CreateNode(node, EVENT_MASK_NONE);
    }
    dist_lock.SetWatcher(node, EVENT_MASK_ATTR_CHANGED );
    while( true ){
        usleep(100);
    }
}
