#include <string.h>
#include <stdio.h>

#include <string>
#include <gtest/gtest.h>

#include "common/base/stdint.h"
#include "common/base/byte_order.hpp"
#include "common/crypto/hash/sha1.hpp"

// uncomment the following line to test compile time overflow check
// const size_t BUFFER_SIZE = SHA1::kDigestLength - 1;
const size_t BUFFER_SIZE = SHA1::kDigestLength;

TEST(SHA1, OverflowCheckChar)
{
    char buffer[BUFFER_SIZE];
    SHA1 sha1;
    sha1.Final(buffer);
    sha1.Final(&buffer);
}

TEST(SHA1, OverflowCheckSChar)
{
    signed char buffer[BUFFER_SIZE];
    SHA1 sha1;
    sha1.Final(buffer);
    sha1.Final(&buffer);
}

TEST(SHA1, OverflowCheckUChar)
{
    unsigned char buffer[BUFFER_SIZE];
    SHA1 sha1;
    sha1.Final(buffer);
    sha1.Final(&buffer);
}

TEST(SHA1, HashBuffer)
{
    char digest[SHA1::kDigestLength];
    SHA1::Digest("", 0, digest);
    ASSERT_EQ('\xda', digest[0]);
    ASSERT_EQ('\x39', digest[1]);
    ASSERT_EQ('\x09', digest[19]);
}

TEST(SHA1, HashToHexString)
{
    ASSERT_EQ(
        "da39a3ee5e6b4b0d3255bfef95601890afd80709",
        SHA1::HashToHexString("")
    );

    ASSERT_EQ(
        "c2050e43c87a7ff7180e316c3c4a9da2ff042e37",
        SHA1::HashToHexString("phongchen")
    );

    ASSERT_EQ(
        "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12",
        SHA1::HashToHexString("The quick brown fox jumps over the lazy dog")
    );

    ASSERT_EQ(
        "de9f2c7fd25e1b3afad3e85a0bd17d9b100db4b3",
        SHA1::HashToHexString("The quick brown fox jumps over the lazy cog")
    );
}


