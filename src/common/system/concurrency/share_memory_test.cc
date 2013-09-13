// Copyright (c) 2011, Tencent Inc. All rights reserved.

//////////////////////////////////////////////////////////////////////////
// ivanhuang @ 20101106
// unit test
//
// 注意！！！
// common库并没有包含gtest的相关头文件和库文件,make之前请执行一下步骤:
// 1.下载gtest的tar包，解压后将gtest-xx.xx(比如gtest-15.0)放入common目录
// 2.将gtest-xx.xx改名为gtest,进入相关目录./configure、make、make install
// 3.默认情况下,gtest的生成库会放入/usr/local/lib/libgtest.so
// 4.如果libgtest.so不在默认路径,请mv至默认路径
//////////////////////////////////////////////////////////////////////////

#include <fstream>
#include "common/system/concurrency/share_memory.h"
#include "gtest/gtest.h"

// 设置环境
class MyTestEnvironment : public testing::Environment
{
public:
    virtual void SetUp()
    {
        // 确保文件存在
        file.open("ivan huang");

        std::cout << "Open file[ivan huang]" << std::endl;
    }
    virtual void TearDown()
    {
        // 关闭文件
        file.close();

        std::cout << "Close file[ivan huang]" << std::endl;
    }

private:
    std::ofstream file;
};

#ifndef _WIN32

// 测试外部ID
TEST(TestShm, TestOuterKeyCreate)
{
    // 构造共享内存对象以及文件路径名
    ShareMemory shm;
    const char *string = "ivan huang";

    // 循环测试不同project_id的key构造
    for (int index = 0; index < 20; ++index)
    {
        key_t key   = ftok(string, index);
        key_t key_1 = ftok(string, index);

        // key值合法
        EXPECT_NE(-1, key);
        EXPECT_NE(-1, key_1);

        // key值相等
        EXPECT_EQ(key, key_1);
    }
}

// 测试基本操作
TEST(TestShm, TestPV)
{
    // 构造共享内存对象,初始内存为空
    ShareMemory shm;
    ASSERT_TRUE(NULL == shm.GetShmAddress());

    const char   *string  = "ivan huang";
    const int    kProjId  = 64;
    const size_t kShmSize = 200<<20;

    bool init          = 0;
    void *virtual_addr = NULL;

    // 构造外部key
    key_t key = ftok(string, kProjId); // NOLINT

    // 打开共享内存
    bool ret = shm.ForceOpen(key, kShmSize, NULL, 0, &init);
    ASSERT_TRUE(ret);
    ASSERT_TRUE(init);

    // 获取共享内存大小
    int ret_size = shm.GetShmSize();
    EXPECT_EQ(ret_size, static_cast<int>(kShmSize));

    // 获取共享内存指针
    virtual_addr = shm.GetShmAddress();
    ASSERT_TRUE(virtual_addr);

    // 释放共享内存关联
    ASSERT_TRUE((bool)shm.Detach());

    // 删除共享内存
    ASSERT_TRUE((bool)shm.Remove());
}

#endif

// 主测试函数
int main(int argc, char **argv)
{
    testing::AddGlobalTestEnvironment(new MyTestEnvironment);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
