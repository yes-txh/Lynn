//////////////////////////////////////////////////////////////////////////
// ivanhuang @ 20101106
// unit test
//
// 注意！！！
// common库并没有包含gtest的相关头文件和库文件,make之前请执行以下步骤:
// 1.下载gtest的tar包并解压
// 2.进入gtest目录,执行./configure、make、make install
//////////////////////////////////////////////////////////////////////////

#include "gtest/gtest.h"
#include "common/base_protocol/encryption.h"

// 测试加密解密函数
TEST(Encryption, BaseOperation)
{
    PseudoProtoHead oHead;

    BaseEncryption::SetPseudoProHead(&oHead);

    unsigned char *data = reinterpret_cast<unsigned char *>(&oHead);
    ASSERT_TRUE(BaseEncryption::IsValidPseudoProHead(data));
}

