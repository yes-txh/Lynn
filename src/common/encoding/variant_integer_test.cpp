// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/encoding/variant_integer.hpp"
#include "common/base/array_size.h"
#include "common/base/stdint.h"
#include "gtest/gtest.h"

// GLOBAL_NOLINT(runtime/int)

const int32_t kTestStep = 512;

TEST(VariantInteger, Types)
{
    unsigned char buffer[VariantInteger::MAX_ENCODED_SIZE];
    VariantInteger::Encode<uint8_t>(uint8_t(1), buffer, sizeof(buffer));
    VariantInteger::Encode<uint16_t>(uint16_t(1), buffer, sizeof(buffer));
    VariantInteger::Encode<uint32_t>(uint32_t(1), buffer, sizeof(buffer));
    VariantInteger::Encode<uint64_t>(uint64_t(1), buffer, sizeof(buffer));

    VariantInteger::Encode<int8_t>(int8_t(1), buffer, sizeof(buffer));
    VariantInteger::Encode<int16_t>(int16_t(1), buffer, sizeof(buffer));
    VariantInteger::Encode<int32_t>(int32_t(1), buffer, sizeof(buffer));
    VariantInteger::Encode<int64_t>(int64_t(1), buffer, sizeof(buffer));
}

TEST(VariantInteger, EncodedSize)
{
    // unsigned
    // unsigned
    EXPECT_EQ(1U, VariantInteger::EncodedSize<int16_t>(1U));
    EXPECT_EQ(2U, VariantInteger::EncodedSize<int16_t>(64U));
    EXPECT_EQ(2U, VariantInteger::EncodedSize<int16_t>(8191U));
    EXPECT_EQ(3U, VariantInteger::EncodedSize<int16_t>(8192U));

    EXPECT_EQ(1U, VariantInteger::EncodedSize<uint16_t>(1U));
    EXPECT_EQ(2U, VariantInteger::EncodedSize<uint16_t>(128U));
    EXPECT_EQ(2U, VariantInteger::EncodedSize<uint16_t>(16383U));
    EXPECT_EQ(3U, VariantInteger::EncodedSize<uint16_t>(16384U));

    EXPECT_EQ(1U, VariantInteger::EncodedSize<uint32_t>(1U));
    EXPECT_EQ(2U, VariantInteger::EncodedSize<uint32_t>(128U));
    EXPECT_EQ(2U, VariantInteger::EncodedSize<uint32_t>(16383U));
    EXPECT_EQ(3U, VariantInteger::EncodedSize<uint32_t>(16384U));

    EXPECT_EQ(3U, VariantInteger::EncodedSize<long>(16384U));
    EXPECT_EQ(3U, VariantInteger::EncodedSize<unsigned long>(16384U));

    // signed
    EXPECT_EQ(1U, VariantInteger::EncodedSize<int32_t>(1));
    EXPECT_EQ(1U, VariantInteger::EncodedSize<int32_t>(-1));
    EXPECT_EQ(1U, VariantInteger::EncodedSize<int32_t>(63));
    EXPECT_EQ(2U, VariantInteger::EncodedSize<int32_t>(64));

    size_t length = 0;
    for (uint32_t i = 1; i < UINT_MAX - 128; i += 127)
    {
        length += VariantInteger::EncodedSize<uint32_t>(i);
    }
    volatile size_t n = length;
    (void) n;
}

TEST(VariantInteger, EncodeUnsigned)
{
    unsigned char buffer[VariantInteger::MAX_ENCODED_SIZE];
    EXPECT_EQ(1, VariantInteger::Encode<int32_t>(1, buffer, sizeof(buffer)));
    EXPECT_EQ(1, VariantInteger::Encode<int16_t>(1, buffer, sizeof(buffer)));
    EXPECT_EQ(1, VariantInteger::UncheckedEncode<uint16_t>(uint16_t(1), buffer));
    EXPECT_EQ(1, buffer[0]);

    EXPECT_EQ(2, VariantInteger::Encode<uint32_t>(128U, buffer, sizeof(buffer)));
    EXPECT_EQ(0x80, buffer[0]);
    EXPECT_EQ(1, buffer[1]);

    for (uint32_t i = 1; i < UINT_MAX - 128; i += 127)
    {
        volatile int length = VariantInteger::EncodedSize<uint32_t>(i);
        (void) length;
#if 0
        ASSERT_EQ(length, static_cast<int>(VariantInteger::EncodedSize(i)));

        // decode and verify
        uint32_t n;
        int decoded_length = VariantInteger::Decode(buffer, sizeof(buffer), &n);
        ASSERT_EQ(decoded_length, length);
        ASSERT_EQ(n, i);
#endif
    }
}

TEST(VariantInteger, EncodedSpeed)
{
    unsigned char buffer[VariantInteger::MAX_ENCODED_SIZE];
    for (uint32_t i = 1; i < 10000000; ++i)
    {
        ASSERT_GT(VariantInteger::Encode<int32_t>(i, buffer, sizeof(buffer)), 0);
    }
}

class DecodedSpeedTest : public testing::Test
{
private:
    virtual void SetUp()
    {
        for (size_t i = 0; i < ARRAY_SIZE(buffer); ++i)
            ASSERT_GT(VariantInteger::Encode<int32_t>(i, buffer[i], sizeof(buffer[i])), 0);
    }
protected:
    unsigned char buffer[4096][VariantInteger::MAX_ENCODED_SIZE];
};

TEST_F(DecodedSpeedTest, DecodedSpeed)
{
    for (uint32_t i = 1; i < 10000000; ++i)
    {
        uint32_t n;
        ASSERT_GT(VariantInteger::Decode(buffer[i % ARRAY_SIZE(buffer)], sizeof(buffer[i]), &n), 0);
    }
}

TEST_F(DecodedSpeedTest, UncheckedDecodedSpeed)
{
    for (uint32_t i = 1; i < 10000000; ++i)
    {
        uint32_t n;
        ASSERT_GT(VariantInteger::UncheckedDecode(buffer[i % ARRAY_SIZE(buffer)], &n), 0);
    }
}

TEST(VariantInteger, EncodeSigned)
{
    unsigned char buffer[VariantInteger::MAX_ENCODED_SIZE];
    EXPECT_EQ(1, VariantInteger::Encode<uint32_t>(1U, buffer, sizeof(buffer)));
    EXPECT_EQ(1, buffer[0]);

    EXPECT_EQ(2, VariantInteger::Encode<uint32_t>(128U, buffer, sizeof(buffer)));
    EXPECT_EQ(0x80, buffer[0]);
    EXPECT_EQ(1, buffer[1]);

    int length = VariantInteger::Encode<int32_t>(-2147483521, buffer, sizeof(buffer));
    int n;
    int length2 = VariantInteger::Decode(buffer, length, &n);
    EXPECT_EQ(length, length2);
    EXPECT_EQ(-2147483521, n);
}

TEST(VariantInteger, MaxEncodedSizeOf)
{
    EXPECT_EQ(2U, VariantInteger::MaxEncodedSizeOf<char>::Value);
    EXPECT_EQ(2U, VariantInteger::MaxEncodedSizeOf<signed char>::Value);
    EXPECT_EQ(2U, VariantInteger::MaxEncodedSizeOf<unsigned char>::Value);
    EXPECT_EQ(3U, VariantInteger::MaxEncodedSizeOf<int16_t>::Value);
    EXPECT_EQ(3U, VariantInteger::MaxEncodedSizeOf<uint16_t>::Value);
    EXPECT_EQ(5U, VariantInteger::MaxEncodedSizeOf<int32_t>::Value);
    EXPECT_EQ(5U, VariantInteger::MaxEncodedSizeOf<uint32_t>::Value);
    EXPECT_EQ(10U, VariantInteger::MaxEncodedSizeOf<int64_t>::Value);
    EXPECT_EQ(10U, VariantInteger::MaxEncodedSizeOf<uint64_t>::Value);
}

TEST(VariantInteger, VerifyUnsigned)
{
    unsigned char buffer[VariantInteger::MAX_ENCODED_SIZE];
    for (uint32_t i = 1; i < UINT_MAX - kTestStep; i += kTestStep)
    {
        int length = VariantInteger::Encode<uint32_t>(i, buffer, sizeof(buffer));
        ASSERT_EQ(length, static_cast<int>(VariantInteger::EncodedSize<uint32_t>(i)));

        // decode and verify
        uint32_t n;
        int decoded_length = VariantInteger::Decode(buffer, sizeof(buffer), &n);
        ASSERT_EQ(decoded_length, length);
        ASSERT_EQ(n, i);
    }
}

TEST(VariantInteger, VerifySigned)
{
    unsigned char buffer[VariantInteger::MAX_ENCODED_SIZE];
    for (int i = INT_MIN; i < INT_MAX - kTestStep; i += kTestStep)
    {
        int length = VariantInteger::Encode<int>(i, buffer, sizeof(buffer));
        ASSERT_GT(length, 0) << i;
        ASSERT_EQ(length, static_cast<int>(VariantInteger::EncodedSize<int>(i)));

        // decode and verify
        int n;
        int decoded_length = VariantInteger::Decode(buffer, sizeof(buffer), &n);
        ASSERT_EQ(decoded_length, length);
        ASSERT_EQ(n, i);
    }
}

TEST(VariantInteger, VerifyUInt64)
{
    unsigned char buffer[VariantInteger::MAX_ENCODED_SIZE];
    uint64_t step = USHRT_MAX * UINT_MAX * 1024ULL;
    for (uint64_t i = 1; i < ULLONG_MAX - step; i += step - 1)
    {
        int length = VariantInteger::Encode<uint64_t>(i, buffer, sizeof(buffer));
        ASSERT_GT(length, 0) << i;
        ASSERT_EQ(length, static_cast<int>(VariantInteger::EncodedSize<uint64_t>(i)));

        // decode and verify
        uint64_t n;
        int decoded_length = VariantInteger::Decode(buffer, sizeof(buffer), &n);
        ASSERT_EQ(decoded_length, length);
        ASSERT_EQ(n, i);
    }
}

TEST(VariantInteger, Unckecked)
{
    unsigned char buffer[VariantInteger::MAX_ENCODED_SIZE];
    for (int i = INT_MIN; i < INT_MAX - kTestStep; i += kTestStep)
    {
        size_t encoded_size =  VariantInteger::EncodedSize<int>(i);
        int length = VariantInteger::UncheckedEncode<int>(i, buffer);
        ASSERT_GT(length, 0) << i;
        ASSERT_EQ(static_cast<size_t>(length), encoded_size);

        // decode and verify
        int n;
        int decoded_length = VariantInteger::UncheckedDecode(buffer, &n);
        ASSERT_EQ(decoded_length, length);
        ASSERT_EQ(n, i);
    }
}

TEST(VariantInteger, BufferCheck)
{
    unsigned char buffer[VariantInteger::MAX_ENCODED_SIZE];
    for (int i = INT_MIN; i < INT_MAX - kTestStep; i += kTestStep)
    {
        size_t encoded_size =  VariantInteger::EncodedSize<int>(i);
        int length = VariantInteger::Encode<int>(i, buffer, encoded_size);
        ASSERT_GT(length, 0) << i;
        ASSERT_EQ((size_t)length, encoded_size);

        // decode and verify
        int n;
        int decoded_length = VariantInteger::Decode(buffer, length, &n);
        ASSERT_EQ(decoded_length, length);
        ASSERT_EQ(n, i);

        length = VariantInteger::Encode<int>(n, buffer, length - 1);
        ASSERT_LT(length, 0);
    }
}

TEST(VariantInteger, DecodedSize)
{
    unsigned char buffer[VariantInteger::MAX_ENCODED_SIZE];
    for (int i = INT_MIN; i < INT_MAX - kTestStep; i += kTestStep)
    {
        size_t encoded_size =  VariantInteger::EncodedSize<int>(i);
        int length = VariantInteger::UncheckedEncode<int>(i, buffer);
        EXPECT_GT(length, 0) << i;
        EXPECT_EQ((size_t)length, encoded_size);
        EXPECT_EQ(length, VariantInteger::DecodedSize(buffer, length));
        EXPECT_EQ(length, VariantInteger::UncheckedDecodedSize(buffer));
    }
}

