
#include <iostream>
#include <vector>
#include <string>

#include <common/distribute_lock/distribute_lock.h>
#include "gtest/gtest.h"
#include <common/distribute_lock/event_listener.h>
#include <common/distribute_lock/event_mask.h>


int main()
{
    using namespace distribute_lock;
    std::string ip_port_group = "172.24.18.209:2181/zk/idc-xaec/soso/xcube/master_test/xcube";
    EventListener listener;
    static int s_num = 1000;
    DistributeLock dist_lock[s_num];
    for( int i = 0 ; i < s_num ; ++ i ){
        int ret = dist_lock[i].Init(ip_port_group.c_str(), &listener, 10000, "./");
        fprintf(stderr, "no = %d\n", i);
        CHECK_EQ(ret, 0);
    }
    return 0;
}
