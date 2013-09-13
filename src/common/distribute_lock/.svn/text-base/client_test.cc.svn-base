#include <iostream>
#include <vector>
#include <string>
#include "thirdparty/gtest/gtest.h"
#include "common/distribute_lock/distribute_lock.h"
#include "common/distribute_lock/event_listener.h"
#include "common/distribute_lock/event_mask.h"

class DistributeLockTest : public testing::Test {
public:
    virtual void SetUp()
    {

    }
    virtual void TearDown()
    {
    }
private:
};

TEST_F(DistributeLockTest, ClientTest)
{
    using namespace distribute_lock;
    std::string ip_port_group = "xaec.zk.oa.com:2181/zk/xaec/xcube/master_test";
    DistributeLock dist_lock;
    EventListener listener;
    int ret = dist_lock.Init(ip_port_group.c_str(), &listener, 2000, "log");
    assert(ret == 0);
    EXPECT_EQ(ret, 0);

    std::vector<std::string> vec_str;
    dist_lock.ListNode("/undo", vec_str);
    for ( size_t i = 0 ; i < vec_str.size(); i++ ) {
        std::cout << "console delete files : " << vec_str[i] << std::endl;
    }

    ret = dist_lock.Exists("/undo");
    if ( ret == 0 ) {
        ret = dist_lock.Unlink("/undo");
        EXPECT_EQ(ret, 0);
    }

    ret = dist_lock.MkDir("/undo", EVENT_MASK_CHILD_CHANGED);
    ASSERT_EQ(ret, 0);

    ret = dist_lock.CreateNode("/undo/ts", EVENT_MASK_NONE);
    ASSERT_EQ(ret, 0);

    ret = dist_lock.Lock("/undo/ts");
    ASSERT_EQ(ret, 0);

    ret = dist_lock.SetAttr("/undo/ts", "serverid");
    ASSERT_EQ(ret, 0);

    ret = dist_lock.SetAttr("/undo/ts", "serverid", "128.0.0.1");
    ASSERT_EQ(ret, 0);

    std::string val;
    ret = dist_lock.GetAttr("/undo/ts", "serverid", val);
    std::cout << "serverid " << val << std::endl;

    ret = dist_lock.SetAttr("/undo/ts", "serverid", "129.0.0.1");
    ASSERT_EQ(ret, 0);

    ret = dist_lock.GetAttr("/undo/ts", "serverid", val);
    std::cout << "serverid " << val << std::endl;
    sleep(1);

    ret = dist_lock.CreateNode("/undo/master", EVENT_MASK_ATTR_CHANGED);
    ASSERT_EQ(ret, 0);
}

