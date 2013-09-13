// Copyright 2010, Tencent Inc.
// Authors: typherque(typherque@tencent.com)
//          Yongqiang Zou (aaronzou@tencent.com)

#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/distribute_lock/zookeeper_wrapper.h"
#include "glog/logging.h"
#include "gtest/gtest.h"
#include "gflags/gflags.h"

class ZookeeperWrapperTest : public testing::Test {
protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    void TestGetValue(const bool is_use_zookeeper) {
        FLAGS_zookeeper = is_use_zookeeper;

        // SetValue first, so that to run without configuration
        std::string cluster_name("xaec-unittest");
        const std::string key("/master_config/config");
        std::string old_value("127.0.0.1");

        EXPECT_TRUE(m_wrapper.SetValue(cluster_name, key, old_value));

        std::string value;
        // Test get now
		LOG(INFO) << "get value 1";
        EXPECT_TRUE(m_wrapper.GetValue(cluster_name, key, &value));

		LOG(INFO) << "get value 2";
		EXPECT_TRUE(m_wrapper.GetValue(cluster_name, key, &value));

		LOG(INFO) << "get value 3";
        EXPECT_TRUE(m_wrapper.GetValue(cluster_name, key, &value));

		LOG(INFO) << "change cluster name:";
        EXPECT_TRUE(value == old_value);
        EXPECT_FALSE(m_wrapper.GetValue(cluster_name + "-not-exist", key, &value));
        EXPECT_TRUE(m_wrapper.GetValue(cluster_name, key, &value));
        EXPECT_TRUE(value == old_value);

        std::string new_value;
        int version = -1;
        EXPECT_TRUE(m_wrapper.GetValue(cluster_name, key, &new_value, &version));
        EXPECT_TRUE(new_value == value);
        LOG(INFO) << "value " << new_value << " version " << version;
        if (FLAGS_zookeeper) {
            EXPECT_TRUE(version >= 0);
        }
        int new_version = -2;
        EXPECT_TRUE(m_wrapper.GetValue(cluster_name,
            key, &new_value, &new_version));
        EXPECT_TRUE(new_value == value);
        LOG(INFO) << "value " << new_value << " version " << version;
        if (FLAGS_zookeeper) {
            EXPECT_TRUE(version >= 0);
            EXPECT_TRUE(version == new_version);
        }
    }

    void TestSetValue(const bool is_use_zookeeper) {
        FLAGS_zookeeper = is_use_zookeeper;

        std::string cluster_name("xaec-unittest");
        const std::string key("/master_config/replica_view_test");
        std::string value("P:127.0.0.1:12000,S:127.0.0.1:12001,S:127.0.0.1:12002");

        EXPECT_TRUE(m_wrapper.SetValue(cluster_name, key, value));

        std::string got_value;
        int version = -1;
        EXPECT_TRUE(m_wrapper.GetValue(cluster_name, key, &got_value, &version));
        EXPECT_TRUE(value == got_value);
        if (FLAGS_zookeeper) {
            EXPECT_TRUE(version >= 0);
        }

        std::string new_value = value + ",N:127.0.0.1:12004";
        int new_version = -5;
        EXPECT_TRUE(m_wrapper.SetValue(cluster_name, key, new_value, version));
        EXPECT_TRUE(m_wrapper.GetValue(cluster_name, key, &got_value, &new_version));
        EXPECT_TRUE(got_value == new_value);
        if (FLAGS_zookeeper) {
            EXPECT_TRUE(new_version == version + 1);
            // wrong version
            EXPECT_FALSE(m_wrapper.SetValue(cluster_name, key, value, new_version + 10));

            // wrong node.
            EXPECT_FALSE(m_wrapper.SetValue(cluster_name, key + "wrongnode-not-exist", value));
            EXPECT_FALSE(m_wrapper.SetValue(cluster_name, key + "wrongnode-not-exist",
                value, new_version));
        }
    }


private:
    ZookeeperWrapper m_wrapper;
};

TEST_F(ZookeeperWrapperTest, TestGetValue) {
    TestGetValue(true);
    TestGetValue(false);
}

TEST_F(ZookeeperWrapperTest, TestSetValue) {
    TestSetValue(false);
    TestSetValue(true);
}

TEST_F(ZookeeperWrapperTest, TestReadNode) {
	std::string val;
    for(int32_t i = 0; i < 100; ++i) {
        ReadZooKeeperNodeVal("/xaec.zk.oa.com/zk/xaec/xfs/cluster_xaec-test/machine_list",
                             3000, &val);
    }
}

