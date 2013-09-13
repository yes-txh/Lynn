#include <string.h>
#include <stdio.h>

#include <string>

#include <common/base/stdint.h>
#include <common/base/byte_order.hpp>
#include <common/crypto/hash/md5.hpp>
#include <gtest/gtest.h>

TEST(MD5, Correction)
{
    ASSERT_EQ(htonll(0xd41d8cd98f00b204ULL), MD5::GetHash64(""));
    ASSERT_EQ(htonll(0x0cc175b9c0f1b6a8ULL), MD5::GetHash64("a"));
    ASSERT_EQ(htonll(0x900150983cd24fb0ULL), MD5::GetHash64("abc"));
    ASSERT_EQ(htonll(0xf96b697d7cb7938dULL), MD5::GetHash64("message digest"));
    ASSERT_EQ(htonll(0xc3fcd3d76192e400ULL), MD5::GetHash64("abcdefghijklmnopqrstuvwxyz"));
    ASSERT_EQ(htonll(0x9e107d9d372bb682ULL), MD5::GetHash64("The quick brown fox jumps over the lazy dog"));

    static const char message[] = "message digest";
    static const size_t message_length = strlen(message);
    unsigned long long ull;
    unsigned long long result = htonll(0xf96b697d7cb7938dULL);

    ull = 0;
    MD5::Hash64(message, ull);
    ASSERT_EQ(ull, result);

    ull = 0;
    MD5::Hash64(message, message_length, ull);
    ASSERT_EQ(ull, result);
}

TEST(MD5, Hash)
{
    char digest[MD5::DigestLength];
    MD5::Digest("The quick brown fox jumps over the lazy cog", digest);
    ASSERT_EQ(0x10, digest[0]);
    ASSERT_EQ(0x4b, digest[15]);
}

TEST(MD5, Hash64String)
{
    char digest_string[MD5::DigestLength * 2 + 1];
    MD5::Hash64String("The quick brown fox jumps over the lazy cog", digest_string);
    ASSERT_EQ(std::string("17476731383137391888"), digest_string);
}

TEST(MD5, GetHash64String)
{
    std::string digest_string = MD5::GetHash64String("The quick brown fox jumps over the lazy cog");
    ASSERT_EQ(std::string("17476731383137391888"), digest_string);
}
