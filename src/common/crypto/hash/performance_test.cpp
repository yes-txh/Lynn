#include "common/crypto/hash/crc.hpp"
#include "common/crypto/hash/md4.hpp"
#include "common/crypto/hash/md5.hpp"
#include "common/crypto/hash/sha1.hpp"
#include "common/crypto/hash/murmur.hpp"
#include "common/crypto/hash/city.hpp"
#include "common/crypto/hash/fingerprint.hpp"
#include "common/crypto/hash/murmur/MurmurHash3.h"

#include <gtest/gtest.h>

static char g_test_data[1024*1024];
const int test_count = 100;

class Performance : public testing::Test
{
protected:
    static void SetUpTestCase()
    {
        // make sure test date be 'hot'
        memset(g_test_data, 0, sizeof(g_test_data));
    }
};

TEST_F(Performance, MD4)
{
    unsigned char digest[16];
    for (int i = 0; i < test_count; ++i)
        MD4::Digest(g_test_data, sizeof(g_test_data), digest);
}

TEST_F(Performance, MD5)
{
    unsigned char digest[16];
    for (int i = 0; i < test_count; ++i)
        MD5::Digest(g_test_data, sizeof(g_test_data), digest);
}

TEST_F(Performance, SHA1)
{
    unsigned char digest[SHA1::kDigestLength];
    for (int i = 0; i < test_count; ++i)
        SHA1::Digest(g_test_data, sizeof(g_test_data), digest);
}

TEST_F(Performance, Murmur2)
{
    for (int i = 0; i < test_count; ++i)
        MurmurHash2(g_test_data, sizeof(g_test_data), 0);
}

TEST_F(Performance, Murmur3_x86_32)
{
    uint32_t digest;
    for (int i = 0; i < test_count; ++i)
        MurmurHash3_x86_32(g_test_data, sizeof(g_test_data), 0, &digest);
}

TEST_F(Performance, Murmur3_x86_64)
{
    uint64_t digest;
    for (int i = 0; i < test_count; ++i)
        MurmurHash3_x86_64(g_test_data, sizeof(g_test_data), 0, &digest);
}

TEST_F(Performance, Murmur3_x86_128)
{
    uint64_t digest[2];
    for (int i = 0; i < test_count; ++i)
        MurmurHash3_x86_128(g_test_data, sizeof(g_test_data), 0, &digest);
}

TEST_F(Performance, City64)
{
    for (int i = 0; i < test_count; ++i)
        CityHash64(g_test_data, sizeof(g_test_data));
}

TEST_F(Performance, City64WithSeed)
{
    for (int i = 0; i < test_count; ++i)
        CityHash64WithSeed(g_test_data, sizeof(g_test_data), 0x9527);
}

TEST_F(Performance, CityHash128)
{
    for (int i = 0; i < test_count; ++i)
        CityHash128(g_test_data, sizeof(g_test_data));
}

TEST_F(Performance, CityHash128WithSeed)
{
    for (int i = 0; i < test_count; ++i)
    {
        CityHash128WithSeed(
            g_test_data, sizeof(g_test_data),
            UInt128(0x9527, 0x5807));
    }
}

TEST_F(Performance, FingerPrint)
{
    for (int i = 0; i < test_count; ++i)
        FingerPrint(g_test_data, sizeof(g_test_data));
}

TEST_F(Performance, Crc32)
{
    for (int i = 0; i < test_count; ++i)
        CRC32Hash32(g_test_data, sizeof(g_test_data));
}
