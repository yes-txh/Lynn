//////////////////////////////////////////////////////////////////////////
// ivanhuang @ 20101106
// unit test
//
// ע�⣡����
// common�Ⲣû�а���gtest�����ͷ�ļ��Ϳ��ļ�,make֮ǰ��ִ�����²���:
// 1.����gtest��tar������ѹ
// 2.����gtestĿ¼,ִ��./configure��make��make install
//////////////////////////////////////////////////////////////////////////

#include "common/base_protocol/base_protocol.h"
#include <string.h>
#include <stdio.h>
#include <gtest/gtest.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif // _WIN32

// ���û���
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

// ���ý��ּ�
class ProtocolTestF : public testing::Test
{
protected:
    virtual void SetUp()
    {
        // �����ڴ沢��ʼ��
        packer_ = new BaseProtocolPack;
        unpacker_ = new BaseProtocolUnpack;

        packer_->Init();
        unpacker_->Init();
    }
    virtual void TearDown()
    {
        // ����ʼ�����ͷ��ڴ�
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

// ���԰�ͷ���ݵĴ�����
TEST_F(ProtocolTestF, PacketHeadPackAndUnpack)
{
    // ���ô����
    packer_->ResetContent();

    // ����TTL
    unsigned char time_to_live = 1;
    packer_->SetTTL(time_to_live);

    // �������
    unsigned int sequence_number = 2;
    packer_->SetSeq(sequence_number);

    // ���÷�������
    unsigned short service_type = 3;
    packer_->SetServiceType(service_type);

    // ���ð�ͷ�ı�������
    const char *test_string = "haha, this is gtest";
    packer_->SetReservedData((unsigned char *)test_string, strlen(test_string));

    // ���
    unsigned char *packed_data;
    unsigned int  packed_len;
    packer_->GetPackage(&packed_data, &packed_len);

    // ����ð������ݷ�������
    unpacker_->AttachPackage(packed_data, packed_len);
    unpacker_->Unpack();

    // ���������İ�ͷ����һ��
    EXPECT_EQ(time_to_live, unpacker_->GetTTL());
    EXPECT_EQ(sequence_number, unpacker_->GetSeq());
    EXPECT_EQ(service_type, unpacker_->GetServiceType());

    unsigned char *reserved_data;
    unsigned int  reserved_len;
    EXPECT_TRUE(unpacker_->GetReservedData(&reserved_data, &reserved_len));
    EXPECT_STREQ((const char *)test_string, (const char *)reserved_data);
}

// ���Ի����������͵Ĵ�����
TEST_F(ProtocolTestF, BaseTypePackAndUnpack)
{
    // ��������õ�key
    enum TestKey {
        kTestForChar = 0,
        kTestForShort,
        kTestForInt,
        kTestForLong,
        kTestForByte,
    };

    // ���ô����
    packer_->ResetContent();

    // �������
    unsigned char   packed_char  = 2;
    unsigned short  packed_short = 12;
    unsigned int    packed_int   = 1983;
    unsigned long   packed_long  = 2010;
    packer_->SetKey(kTestForChar, packed_char);
    packer_->SetKey(kTestForShort, packed_short);
    packer_->SetKey(kTestForInt, packed_int);
    packer_->SetKey(kTestForLong, (uint64_t)packed_long);
    packer_->SetKey(kTestForByte, (unsigned char *)"ivan huang", strlen("ivan huang"));

    // ��ȡ��
    unsigned char *packed_data;
    unsigned int  packed_len;
    packer_->GetPackage(&packed_data, &packed_len);

    // ����ð������ݷ�������
    unpacker_->AttachPackage(packed_data, packed_len);

    // ���������ɹ�
    ASSERT_TRUE(unpacker_->Unpack());

    unsigned char   unpacked_char   = 0;
    unsigned short  unpacked_short  = 0;
    unsigned int    unpacked_int    = 0;
    unsigned long   unpacked_long   = 0;
    unsigned char   *string         = NULL;
    unsigned int    string_len      = 0;

    // ���ҽ�����ͣ��������ʧ��
    EXPECT_FALSE(unpacker_->GetVal(kTestForChar, string));
    EXPECT_FALSE(unpacker_->GetVal(kTestForShort, &unpacked_char));
    EXPECT_FALSE(unpacker_->GetVal(kTestForInt, &unpacked_short));
    EXPECT_FALSE(unpacker_->GetVal(kTestForLong, &unpacked_int));
    EXPECT_FALSE(unpacker_->GetVal(kTestForByte, (uint64_t *)&unpacked_long));

    // ��ȷ������ͣ���������ɹ�
    EXPECT_TRUE(unpacker_->GetVal(kTestForChar, &unpacked_char));
    EXPECT_TRUE(unpacker_->GetVal(kTestForShort, &unpacked_short));
    EXPECT_TRUE(unpacker_->GetVal(kTestForInt, &unpacked_int));
    EXPECT_TRUE(unpacker_->GetVal(kTestForLong, (uint64_t *)&unpacked_long));
    EXPECT_TRUE(unpacker_->GetVal(kTestForByte, &string, &string_len));

    // ��������������������������һ��
    EXPECT_EQ(packed_char, unpacked_char);
    EXPECT_EQ(packed_short, unpacked_short);
    EXPECT_EQ(packed_int, unpacked_int);
    EXPECT_EQ(packed_long, unpacked_long);
    EXPECT_STREQ("ivan huang", (const char *)string);
}

// ����׷������
TEST_F(ProtocolTestF, AppendData)
{
    // ���ô����
    packer_->ResetContent();

    // ��������
    const unsigned short kTestKey       = 1;
    const char           *first_string  = "ivan ";
    const char           *second_string = "huang";
    packer_->SetKey(kTestKey, (unsigned char *)first_string, strlen(first_string));
    packer_->AppendKeyData(kTestKey, (unsigned char *)second_string, strlen(second_string));

    // ��ȡ��
    unsigned char *packed_data;
    unsigned int  packed_len;
    packer_->GetPackage(&packed_data, &packed_len);

    // ����ð������ݷ�������
    unpacker_->AttachPackage(packed_data, packed_len);

    // ���������ɹ�
    ASSERT_TRUE(unpacker_->Unpack());

    // ��ȡ����
    unsigned char    *string          = NULL;
    unsigned int    string_len        = 0;
    EXPECT_TRUE(unpacker_->GetVal(kTestKey, &string, &string_len));

    // �Ա�����
    char combo_string[128];
    snprintf(combo_string, sizeof(combo_string), "%s", first_string);
    snprintf(combo_string + strlen(combo_string),
        sizeof(combo_string) - strlen(combo_string), "%s", second_string);
//  strcpy(combo_string, first_string);
//  strcat(combo_string, second_string);
    EXPECT_STREQ((const char *)combo_string, (const char *)string);
}

// �����Ժ���
int main(int argc, char **argv)
{
    testing::AddGlobalTestEnvironment(new BaseProtocolEnvironment);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
