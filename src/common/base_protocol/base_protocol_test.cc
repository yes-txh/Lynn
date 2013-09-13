//////////////////////////////////////////////////////////////////////////
// ivanhuang @ 20101106
// unit test
//
// 注意！！！
// common库并没有包含gtest的相关头文件和库文件,make之前请执行以下步骤:
// 1.下载gtest的tar包并解压
// 2.进入gtest目录,执行./configure、make、make install
//////////////////////////////////////////////////////////////////////////

#include "common/base_protocol/base_protocol.h"
#include <string.h>
#include <stdio.h>
#include <gtest/gtest.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif // _WIN32

// 设置环境
class BaseProtocolEnvironment : public testing::Environment
{
public:
    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }
};

// 设置脚手架
class ProtocolTestF : public testing::Test
{
protected:
    virtual void SetUp()
    {
        // 分配内存并初始化
        packer_ = new BaseProtocolPack;
        unpacker_ = new BaseProtocolUnpack;

        packer_->Init();
        unpacker_->Init();
    }
    virtual void TearDown()
    {
        // 反初始化并释放内存
        packer_->Uninit();
        unpacker_->Uninit();

        delete packer_;
        packer_ = NULL;

        delete unpacker_;
        unpacker_ = NULL;
    }

protected:
    BaseProtocolPack   *packer_;
    BaseProtocolUnpack *unpacker_;
};

// 测试包头数据的打包解包
TEST_F(ProtocolTestF, PacketHeadPackAndUnpack)
{
    // 重置打包类
    packer_->ResetContent();

    // 设置TTL
    unsigned char time_to_live = 1;
    packer_->SetTTL(time_to_live);

    // 设置序号
    unsigned int sequence_number = 2;
    packer_->SetSeq(sequence_number);

    // 设置服务类型
    unsigned short service_type = 3;
    packer_->SetServiceType(service_type);

    // 设置包头的保留数据
    const char *test_string = "haha, this is gtest";
    packer_->SetReservedData((unsigned char *)test_string, strlen(test_string));

    // 打包
    unsigned char *packed_data;
    unsigned int  packed_len;
    packer_->GetPackage(&packed_data, &packed_len);

    // 将打好包的数据放入解包类
    unpacker_->AttachPackage(packed_data, packed_len);
    unpacker_->Unpack();

    // 期望解析的包头数据一致
    EXPECT_EQ(time_to_live, unpacker_->GetTTL());
    EXPECT_EQ(sequence_number, unpacker_->GetSeq());
    EXPECT_EQ(service_type, unpacker_->GetServiceType());

    unsigned char *reserved_data;
    unsigned int  reserved_len;
    EXPECT_TRUE(unpacker_->GetReservedData(&reserved_data, &reserved_len));
    EXPECT_STREQ((const char *)test_string, (const char *)reserved_data);
}

// 测试基本数据类型的打包解包
TEST_F(ProtocolTestF, BaseTypePackAndUnpack)
{
    // 定义测试用的key
    enum TestKey {
        kTestForChar = 0,
        kTestForShort,
        kTestForInt,
        kTestForLong,
        kTestForByte,
    };

    // 重置打包类
    packer_->ResetContent();

    // 打包数据
    unsigned char   packed_char  = 2;
    unsigned short  packed_short = 12;
    unsigned int    packed_int   = 1983;
    unsigned long   packed_long  = 2010;
    packer_->SetKey(kTestForChar, packed_char);
    packer_->SetKey(kTestForShort, packed_short);
    packer_->SetKey(kTestForInt, packed_int);
    packer_->SetKey(kTestForLong, (uint64_t)packed_long);
    packer_->SetKey(kTestForByte, (unsigned char *)"ivan huang", strlen("ivan huang"));

    // 获取包
    unsigned char *packed_data;
    unsigned int  packed_len;
    packer_->GetPackage(&packed_data, &packed_len);

    // 将打好包的数据放入解包类
    unpacker_->AttachPackage(packed_data, packed_len);

    // 解包，必须成功
    ASSERT_TRUE(unpacker_->Unpack());

    unsigned char   unpacked_char   = 0;
    unsigned short  unpacked_short  = 0;
    unsigned int    unpacked_int    = 0;
    unsigned long   unpacked_long   = 0;
    unsigned char   *string         = NULL;
    unsigned int    string_len      = 0;

    // 错乱解包类型，期望解包失败
    EXPECT_FALSE(unpacker_->GetVal(kTestForChar, string));
    EXPECT_FALSE(unpacker_->GetVal(kTestForShort, &unpacked_char));
    EXPECT_FALSE(unpacker_->GetVal(kTestForInt, &unpacked_short));
    EXPECT_FALSE(unpacker_->GetVal(kTestForLong, &unpacked_int));
    EXPECT_FALSE(unpacker_->GetVal(kTestForByte, (uint64_t *)&unpacked_long));

    // 正确解包类型，期望解包成功
    EXPECT_TRUE(unpacker_->GetVal(kTestForChar, &unpacked_char));
    EXPECT_TRUE(unpacker_->GetVal(kTestForShort, &unpacked_short));
    EXPECT_TRUE(unpacker_->GetVal(kTestForInt, &unpacked_int));
    EXPECT_TRUE(unpacker_->GetVal(kTestForLong, (uint64_t *)&unpacked_long));
    EXPECT_TRUE(unpacker_->GetVal(kTestForByte, &string, &string_len));

    // 期望解析出来的数据与打包数据一致
    EXPECT_EQ(packed_char, unpacked_char);
    EXPECT_EQ(packed_short, unpacked_short);
    EXPECT_EQ(packed_int, unpacked_int);
    EXPECT_EQ(packed_long, unpacked_long);
    EXPECT_STREQ("ivan huang", (const char *)string);
}

// 测试追加数据
TEST_F(ProtocolTestF, AppendData)
{
    // 重置打包类
    packer_->ResetContent();

    // 设置数据
    const unsigned short kTestKey       = 1;
    const char           *first_string  = "ivan ";
    const char           *second_string = "huang";
    packer_->SetKey(kTestKey, (unsigned char *)first_string, strlen(first_string));
    packer_->AppendKeyData(kTestKey, (unsigned char *)second_string, strlen(second_string));

    // 获取包
    unsigned char *packed_data;
    unsigned int  packed_len;
    packer_->GetPackage(&packed_data, &packed_len);

    // 将打好包的数据放入解包类
    unpacker_->AttachPackage(packed_data, packed_len);

    // 解包，必须成功
    ASSERT_TRUE(unpacker_->Unpack());

    // 获取数据
    unsigned char    *string          = NULL;
    unsigned int    string_len        = 0;
    EXPECT_TRUE(unpacker_->GetVal(kTestKey, &string, &string_len));

    // 对比数据
    char combo_string[128];
    snprintf(combo_string, sizeof(combo_string), "%s", first_string);
    snprintf(combo_string + strlen(combo_string),
        sizeof(combo_string) - strlen(combo_string), "%s", second_string);
//  strcpy(combo_string, first_string);
//  strcat(combo_string, second_string);
    EXPECT_STREQ((const char *)combo_string, (const char *)string);
}

// 主测试函数
int main(int argc, char **argv)
{
    testing::AddGlobalTestEnvironment(new BaseProtocolEnvironment);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
