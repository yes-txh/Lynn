//////////////////////////////////////////////////////////////////////////
// ivanhuang @ 20101106
// unit test
//
// ע�⣡����
// common�Ⲣû�а���gtest�����ͷ�ļ��Ϳ��ļ�,make֮ǰ��ִ�����²���:
// 1.����gtest��tar������ѹ
// 2.����gtestĿ¼,ִ��./configure��make��make install
//////////////////////////////////////////////////////////////////////////

#include "gtest/gtest.h"
#include "common/base_protocol/encryption.h"

// ���Լ��ܽ��ܺ���
TEST(Encryption, BaseOperation)
{
    PseudoProtoHead oHead;

    BaseEncryption::SetPseudoProHead(&oHead);

    unsigned char *data = reinterpret_cast<unsigned char *>(&oHead);
    ASSERT_TRUE(BaseEncryption::IsValidPseudoProHead(data));
}

