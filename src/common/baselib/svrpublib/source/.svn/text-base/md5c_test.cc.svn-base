//////////////////////////////////////////////////////////////////////////
// md5c_test.cc
// @brief:      Test md5
// @author:     fatliu@tencent
// @time:       2010-10-14
// @version:    1.0
//////////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

#ifdef WIN32
int32_t TestMD5(int32_t argc, char** argv)
#else
int32_t main(int32_t argc, char** argv)
#endif
{
#ifndef WIN32
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, true);

    AutoBaseLib auto_baselib;
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
#else
    return 0;
#endif
}

// @brief:  test md5 init
TEST(MD5, Init) {
    MD5_CTX md5Info;
    Hash_MD5Init(&md5Info);

    EXPECT_EQ(md5Info.buf[0], static_cast<long int>(1732584193UL));
    EXPECT_EQ(md5Info.buf[1], static_cast<long int>(4023233417UL));
    EXPECT_EQ(md5Info.buf[2], static_cast<long int>(2562383102UL));
    EXPECT_EQ(md5Info.buf[3], static_cast<long int>(271733878UL));

    EXPECT_EQ(md5Info.bits[0], 0);
    EXPECT_EQ(md5Info.bits[1], 0);
}

// @brief:  test "12345678" md5 update
TEST(MD5, Update) {
    const char *inBuffer = "12345678";
    int32_t len = static_cast<int32_t>(strlen(inBuffer));

    MD5_CTX md5Info;
    Hash_MD5Init(&md5Info);
    Hash_MD5Update(&md5Info,
                   reinterpret_cast<const unsigned char*>(inBuffer),
                   len);
    md5Info.in[8] = '\0';

    EXPECT_STRCASEEQ("12345678", (const char*)md5Info.in);
    EXPECT_EQ(64, md5Info.bits[0]);
    EXPECT_EQ(0, md5Info.bits[1]);
}

// @brief:  test "12345678" md5 final
TEST(MD5, Final) {
    char unsigned outBuffer[17] = {0};
    const char *inBuffer = "12345678";
    int32_t len = static_cast<int32_t>(strlen(inBuffer));

    MD5_CTX md5Info;

    Hash_MD5Init(&md5Info);
    Hash_MD5Update(&md5Info,
                   reinterpret_cast<const unsigned char*>(inBuffer),
                   len);
    Hash_MD5Final(&md5Info, outBuffer);

    char buff[33] = {0};
    for (int32_t i = 0; i < 16; ++i)
        safe_snprintf(&buff[i * 2], sizeof(buff), "%02x", outBuffer[i]);
#ifdef WIN32
    EXPECT_STRCASEEQ("48c125ab99f9b48e033e1a47a6e091", buff);
#endif
}

// @brief:  test "auewngaagrvjytrefdcvbf" md5 Md5HashBuffer
TEST(MD5, Md5HashBuffer) {
    char unsigned outBuffer[17] = {0};
    const char *inBuffer = "12345678";
    int32_t len = static_cast<int32_t>(strlen(inBuffer));
    Md5HashBuffer(outBuffer,
                  reinterpret_cast<const unsigned char*>(inBuffer),
                  len);

    char buff[33] = {0};
    for (int32_t i = 0; i < 16; ++i)
        safe_snprintf(&buff[i * 2], sizeof(buff), "%02x", outBuffer[i]);
#ifdef WIN32
    EXPECT_STRCASEEQ("48c125ab99f9b48e033e1a47a6e091", buff);
#endif
}
