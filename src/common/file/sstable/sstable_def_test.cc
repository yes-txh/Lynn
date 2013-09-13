#include <stdlib.h>
#include <string>

#include "common/config/cflags.hpp"
#include "thirdparty/gtest/gtest.h"
#include "thirdparty/glog/logging.h"

#include "common/file/sstable/sstable_def.h"

namespace sstable
{
TEST(SSTableDef, ComparePrefixCompressedKey1)
{
    // 1: test with (key1.prefix_len = 0 && key2.prefix_len != 0)
    //             || (key2.prefix_len = 0 && key1.prefix_len != 0);
    // 1.1: key2.remainer_len != 0;
    sstable::PrefixCompressedKey key1;
    sstable::PrefixCompressedKey key2;
    std::string key1_prefix = "";
    std::string key1_remainder = "abcd1234";
    std::string key2_prefix = "abcd";
    std::string key2_remainder = "1234567";
    key1.prefix_key_len = 0;
    key1.remainder_key = const_cast<char*>(key1_remainder.c_str());
    key1.remainder_key_len = key1_remainder.length();

    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len =  key2_prefix.length();
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_GT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_LT(0, ComparePrefixCompressedKey(key2, key1));

    key2_remainder = "1234";
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_EQ(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_EQ(0, ComparePrefixCompressedKey(key2, key1));

    key2_remainder = "123";
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_LT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_GT(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abce";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len =  key2_prefix.length();
    ASSERT_GT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_LT(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcc";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len =  key2_prefix.length();
    ASSERT_LT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_GT(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcd";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len =  key2_prefix.length();
    key2_remainder = "1244";
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_GT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_LT(0, ComparePrefixCompressedKey(key2, key1));

    key2_remainder = "1144";
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_LT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_GT(0, ComparePrefixCompressedKey(key2, key1));

    key2_remainder = "123";
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_LT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_GT(0, ComparePrefixCompressedKey(key2, key1));

    // 1.2: key2.remainer_len == 0;
    key2_prefix = "abcd1234";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len =  key2_prefix.length();
    key2.remainder_key = NULL;
    key2.remainder_key_len = 0;
    ASSERT_EQ(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_EQ(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcd1233";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len =  key2_prefix.length();
    ASSERT_LT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_GT(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcd123";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len =  key2_prefix.length();
    ASSERT_LT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_GT(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcd123456";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len =  key2_prefix.length();
    ASSERT_GT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_LT(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcd1244";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len =  key2_prefix.length();
    ASSERT_GT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_LT(0, ComparePrefixCompressedKey(key2, key1));

    // 2: test with key1.prefix_len != 0 && key2.prefix_len != 0;
    // 2.1: key1.remainder_len == 0 && key2.remainder_len == 0
    key1_prefix = "abcd1234";
    key1_remainder = "";
    key2_prefix = "abcd1234";
    key2_remainder = "";
    key1.prefix_key = const_cast<char*>(key1_prefix.c_str());
    key1.prefix_key_len = key1_prefix.length();
    key1.remainder_key = NULL;
    key1.remainder_key_len = 0;
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len = key2_prefix.length();
    key2.remainder_key = NULL;
    key2.remainder_key_len = 0;
    ASSERT_EQ(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_EQ(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abce1234";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len = key2_prefix.length();
    ASSERT_GT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_LT(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcd1234567";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len = key2_prefix.length();
    ASSERT_GT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_LT(0, ComparePrefixCompressedKey(key2, key1));

    // 2.2: key1.remainder_len !=0 && key2.remainder_len == 0
    key1_prefix = "abcd";
    key1_remainder = "1234";
    key2_prefix = "abcd1234";
    key2_remainder = "";
    key1.prefix_key = const_cast<char*>(key1_prefix.c_str());
    key1.prefix_key_len = key1_prefix.length();
    key1.remainder_key = const_cast<char*>(key1_remainder.c_str());
    key1.remainder_key_len = key1_remainder.length();
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len = key2_prefix.length();
    key2.remainder_key = NULL;
    key2.remainder_key_len = 0;
    ASSERT_EQ(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_EQ(0, ComparePrefixCompressedKey(key2, key1));

    key1_prefix = "abce";
    key1_remainder = "1234";
    key1.prefix_key = const_cast<char*>(key1_prefix.c_str());
    key1.prefix_key_len = key1_prefix.length();
    key1.remainder_key = const_cast<char*>(key1_remainder.c_str());
    key1.remainder_key_len = key1_remainder.length();
    ASSERT_LT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_GT(0, ComparePrefixCompressedKey(key2, key1));

    key1_prefix = "abcc";
    key1_remainder = "1234";
    key1.prefix_key = const_cast<char*>(key1_prefix.c_str());
    key1.prefix_key_len = key1_prefix.length();
    key1.remainder_key = const_cast<char*>(key1_remainder.c_str());
    key1.remainder_key_len = key1_remainder.length();
    ASSERT_GT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_LT(0, ComparePrefixCompressedKey(key2, key1));

    key1_prefix = "abcd";
    key1_remainder = "1234567";
    key1.prefix_key = const_cast<char*>(key1_prefix.c_str());
    key1.prefix_key_len = key1_prefix.length();
    key1.remainder_key = const_cast<char*>(key1_remainder.c_str());
    key1.remainder_key_len = key1_remainder.length();
    ASSERT_LT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_GT(0, ComparePrefixCompressedKey(key2, key1));

    key1_prefix = "abcd";
    key1_remainder = "1235";
    key1.prefix_key = const_cast<char*>(key1_prefix.c_str());
    key1.prefix_key_len = key1_prefix.length();
    key1.remainder_key = const_cast<char*>(key1_remainder.c_str());
    key1.remainder_key_len = key1_remainder.length();
    ASSERT_LT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_GT(0, ComparePrefixCompressedKey(key2, key1));

    key1_prefix = "abcd";
    key1_remainder = "1233";
    key1.prefix_key = const_cast<char*>(key1_prefix.c_str());
    key1.prefix_key_len = key1_prefix.length();
    key1.remainder_key = const_cast<char*>(key1_remainder.c_str());
    key1.remainder_key_len = key1_remainder.length();
    ASSERT_GT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_LT(0, ComparePrefixCompressedKey(key2, key1));

    key1_prefix = "abcd";
    key1_remainder = "123";
    key1.prefix_key = const_cast<char*>(key1_prefix.c_str());
    key1.prefix_key_len = key1_prefix.length();
    key1.remainder_key = const_cast<char*>(key1_remainder.c_str());
    key1.remainder_key_len = key1_remainder.length();
    ASSERT_GT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_LT(0, ComparePrefixCompressedKey(key2, key1));

    // 2.3: key1.remainder_len !=0 && key2.remainder_len != 0
    key1_prefix = "abcd";
    key1_remainder = "1234";
    key2_prefix = "abcd";
    key2_remainder = "1234";
    key1.prefix_key = const_cast<char*>(key1_prefix.c_str());
    key1.prefix_key_len = key1_prefix.length();
    key1.remainder_key = const_cast<char*>(key1_remainder.c_str());
    key1.remainder_key_len = key1_remainder.length();
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len = key2_prefix.length();
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_EQ(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_EQ(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcc";
    key2_remainder = "1234";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len = key2_prefix.length();
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_LT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_GT(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcde";
    key2_remainder = "1234";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len = key2_prefix.length();
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_GT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_LT(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcd1";
    key2_remainder = "234";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len = key2_prefix.length();
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_EQ(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_EQ(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcd1";
    key2_remainder = "1234";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len = key2_prefix.length();
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_LT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_GT(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcd";
    key2_remainder = "1233";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len = key2_prefix.length();
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_LT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_GT(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcd";
    key2_remainder = "123";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len = key2_prefix.length();
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_LT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_GT(0, ComparePrefixCompressedKey(key2, key1));

    key2_prefix = "abcd";
    key2_remainder = "1234567";
    key2.prefix_key = const_cast<char*>(key2_prefix.c_str());
    key2.prefix_key_len = key2_prefix.length();
    key2.remainder_key = const_cast<char*>(key2_remainder.c_str());
    key2.remainder_key_len = key2_remainder.length();
    ASSERT_GT(0, ComparePrefixCompressedKey(key1, key2));
    ASSERT_LT(0, ComparePrefixCompressedKey(key2, key1));
}

TEST(SSTableDef, CompareKVType)
{
    // 1. key1:fkfv
    RecordKVType type1 =
        static_cast<RecordKVType>(kTypeFixedLen | kTypeFixedLen << 4);
    RecordKVType type2 =
        static_cast<RecordKVType>(kTypeFixedLen | kTypeFixedLen << 4);
    ASSERT_EQ(type1 , CompareKVType(type1, type2));
    ASSERT_EQ(type2 , CompareKVType(type1, type2));

    type2 = static_cast<RecordKVType>(kTypeVariableLen | kTypeFixedLen << 4);
    ASSERT_EQ(type2 , CompareKVType(type1, type2));

    type2 = static_cast<RecordKVType>(kTypeFixedLen | kTypeVariableLen << 4);
    ASSERT_EQ(type2 , CompareKVType(type1, type2));

    type2 = static_cast<RecordKVType>(kTypeVariableLen | kTypeVariableLen << 4);
    ASSERT_EQ(type2 , CompareKVType(type1, type2));

    // 2. key1:vkfv
    type1 = static_cast<RecordKVType>(kTypeVariableLen | kTypeFixedLen << 4);
    type2 = static_cast<RecordKVType>(kTypeFixedLen | kTypeFixedLen << 4);
    ASSERT_EQ(type1 , CompareKVType(type1, type2));

    type2 = static_cast<RecordKVType>(kTypeVariableLen | kTypeFixedLen << 4);
    ASSERT_EQ(type1 , CompareKVType(type1, type2));
    ASSERT_EQ(type2 , CompareKVType(type1, type2));

    type2 = static_cast<RecordKVType>(kTypeFixedLen | kTypeVariableLen << 4);
    ASSERT_EQ(kTypeVariableLen | kTypeVariableLen << 4 , CompareKVType(type1, type2));

    type2 = static_cast<RecordKVType>(kTypeVariableLen | kTypeVariableLen << 4);
    ASSERT_EQ(type2 , CompareKVType(type1, type2));

    // 3. key1:fkvv
    type1 = static_cast<RecordKVType>(kTypeFixedLen | kTypeVariableLen << 4);
    type2 = static_cast<RecordKVType>(kTypeFixedLen | kTypeFixedLen << 4);
    ASSERT_EQ(type1 , CompareKVType(type1, type2));

    type2 = static_cast<RecordKVType>(kTypeVariableLen | kTypeFixedLen << 4);
    ASSERT_EQ(kTypeVariableLen | kTypeVariableLen << 4 , CompareKVType(type1, type2));

    type2 = static_cast<RecordKVType>(kTypeFixedLen | kTypeVariableLen << 4);
    ASSERT_EQ(type1 , CompareKVType(type1, type2));
    ASSERT_EQ(type2 , CompareKVType(type1, type2));

    type2 = static_cast<RecordKVType>(kTypeVariableLen | kTypeVariableLen << 4);
    ASSERT_EQ(type2 , CompareKVType(type1, type2));

    // 4. key1:vkvv
    type1 = static_cast<RecordKVType>(kTypeVariableLen | kTypeVariableLen << 4);
    type2 = static_cast<RecordKVType>(kTypeFixedLen | kTypeFixedLen << 4);
    ASSERT_EQ(type1 , CompareKVType(type1, type2));

    type2 = static_cast<RecordKVType>(kTypeVariableLen | kTypeFixedLen << 4);
    ASSERT_EQ(type1 , CompareKVType(type1, type2));

    type2 = static_cast<RecordKVType>(kTypeFixedLen | kTypeVariableLen << 4);
    ASSERT_EQ(type1 , CompareKVType(type1, type2));

    type2 = static_cast<RecordKVType>(kTypeVariableLen | kTypeVariableLen << 4);
    ASSERT_EQ(type1 , CompareKVType(type1, type2));
    ASSERT_EQ(type2 , CompareKVType(type1, type2));
}

TEST(SSTableDef, CompareCompressType)
{
    CompressType LZO = static_cast<CompressType>(intern::BlockCompressionCodec::LZO);
    CompressType BMZ = static_cast<CompressType>(intern::BlockCompressionCodec::BMZ);
    CompressType QUICKLZ = static_cast<CompressType>(intern::BlockCompressionCodec::QUICKLZ);
    CompressType NONE = static_cast<CompressType>(intern::BlockCompressionCodec::NONE);

    ASSERT_EQ(LZO, CompareCompressType(LZO, LZO));
    ASSERT_EQ(LZO, CompareCompressType(LZO, BMZ));
    ASSERT_EQ(LZO, CompareCompressType(LZO, QUICKLZ));
    ASSERT_EQ(LZO, CompareCompressType(LZO, NONE));

    ASSERT_EQ(LZO, CompareCompressType(BMZ, LZO));
    ASSERT_EQ(BMZ, CompareCompressType(BMZ, BMZ));
    ASSERT_EQ(BMZ, CompareCompressType(BMZ, QUICKLZ));
    ASSERT_EQ(BMZ, CompareCompressType(BMZ, NONE));

    ASSERT_EQ(LZO, CompareCompressType(QUICKLZ, LZO));
    ASSERT_EQ(BMZ, CompareCompressType(QUICKLZ, BMZ));
    ASSERT_EQ(QUICKLZ, CompareCompressType(QUICKLZ, QUICKLZ));
    ASSERT_EQ(QUICKLZ, CompareCompressType(QUICKLZ, NONE));

    ASSERT_EQ(LZO, CompareCompressType(NONE, LZO));
    ASSERT_EQ(BMZ, CompareCompressType(NONE, BMZ));
    ASSERT_EQ(QUICKLZ, CompareCompressType(NONE, QUICKLZ));
    ASSERT_EQ(NONE, CompareCompressType(NONE, NONE));
}
}

int main(int argc, char** argv)
{
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);

    if (!cflags::ParseCommandLine(argc, argv))
    {
        return EXIT_FAILURE;
    }

    return RUN_ALL_TESTS();
}
