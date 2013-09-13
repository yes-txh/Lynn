
#include <iostream>
#include <zookeeper/zookeeper.h>
#include <common/distribute_lock/distribute_lock.h>
#include <common/distribute_lock/event_mask.h>


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
    std::string ip_port_group = "172.24.18.209:2181/zk/idc-xaec/soso/xcube/master_test";
    EventListener listener;
    DistributeLock dist_lock;
    int ret = dist_lock.Init(ip_port_group.c_str(), &listener, 20000, "log");
    assert(ret == 0);

    ret = dist_lock.Exists(node);
    if( ret != 0){
        ret = dist_lock.CreateNode(node, EVENT_MASK_NONE);
        assert(ret == 0);
    }

    ret = dist_lock.Lock(node, false);
    if( ret == 0 ){
        printf("get the lock");
        while(true)
        {
            sleep(1);
        }
    }
    else
    {
        while( true){
            if( ret != 0 )
                printf("not get the lock");
            else
                printf("get the lock");
            sleep(6);
            ret = dist_lock.Lock(node, false);
        }

    }
    return 0;
}
