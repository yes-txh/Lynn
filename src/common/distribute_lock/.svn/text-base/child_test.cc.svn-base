
#include <string>
#include "thirdparty/gtest/gtest.h"
#include "common/distribute_lock/distribute_lock.h"

int main(int argc, char *argv[])
{
    std::string node;
    if ( argc < 2 ) {
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
    int ret = dist_lock.Init(ip_port_group.c_str(), &listener, 4000, "./log");
    CHECK_EQ(ret, 0);

    ret = dist_lock.Exists(node);
    if (ret != 0) {
        ret = dist_lock.CreateNode(node, EVENT_MASK_NONE);
        CHECK_EQ(ret, 0);
    }
//    while ( true ) { ///< 不停的增加&删除孩子结点
//        std::string child_node = node + "/" + std::string("child");
//        ret = dist_lock.CreateNode(child_node);
//        sleep(2);
//        ret = dist_lock.Unlink(child_node);
//        sleep(2);
//    }

    while (true)
    {
        usleep(1000);
    }
    return 0;
}
