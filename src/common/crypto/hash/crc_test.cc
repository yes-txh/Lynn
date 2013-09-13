//////////////////////////////////////////////////////////////////////////
// ivanhuang @ 20101106
//////////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>
#include "common/crypto/hash/crc.hpp"

// ≤‚ ‘º”√‹Ω‚√‹∫Ø ˝
TEST(CRC32, BaseOperation)
{
    const char *string = "/home/ivanhuang/data/test";
    size_t size = strlen(string);

    unsigned int now_crc = 0;
    unsigned int old_crc = 12345;

    for (int i = 0; i < 50; ++i)
    {
        now_crc = UpdateCRC32(string, size, old_crc);

        ASSERT_NE(now_crc, old_crc);

        old_crc = now_crc;
    }
}
