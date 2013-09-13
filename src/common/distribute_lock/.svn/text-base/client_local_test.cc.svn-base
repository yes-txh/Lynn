#include "gtest/gtest.h"
#include <common/distribute_lock/distribute_lock.h>

TEST(ClientLocalTest, DISABLED_ClientTest)
{
    std::string zk_ip = "xaec.zk.oa.com:2181/zk/xaec";
    distribute_lock::DistributeLock dist_lock;
    int ret = dist_lock.Init(zk_ip, NULL, 2000, "./log");
    EXPECT_EQ(ret, 0);

    ret = dist_lock.CreateNode("/xcube");
    EXPECT_EQ(ret, 0);

    ret = dist_lock.CreateNode("/xcube/client_test");
    EXPECT_EQ(ret, 0);

    ret = dist_lock.CreateNode("/xcube/master_test");
    EXPECT_EQ(ret, 0);

    ret = dist_lock.CreateNode("/xcube/ts_test");
    EXPECT_EQ(ret, 0);
}
