
#include <iostream>
#include <string>
#include "glog/logging.h"
#include "glog/raw_logging.h"
#include "gtest/gtest.h"
#include "common/distribute_lock/distribute_lock.h"

TEST(DistributeLock, GetNodeDataTest)
{
    using namespace distribute_lock;
    std::string ip_port_group = "tjd1.zk.oa.com:2181/zk/tjd1/xcube/indexing";
    EventListener listener;
    DistributeLock dist_lock;
    int ret = dist_lock.Init(ip_port_group.c_str(), &listener, 4000, "log");
    EXPECT_EQ(ret, 0);

    std::string val;
    std::string node = "/ts/10.168.1.29_31000_20110516161949";
    ret = dist_lock.GetAttr(node, "tablets", val);
    std::cout<<"val: " << val << std::endl;
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    FLAGS_log_dir = "./log";
    FLAGS_per_log_size = 1000;
    FLAGS_max_log_size = 20000;
    FLAGS_stderrthreshold = 3;
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    return RUN_ALL_TESTS();
}
