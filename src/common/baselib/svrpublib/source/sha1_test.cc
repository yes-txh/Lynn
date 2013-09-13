//////////////////////////////////////////////////////////////////////////
// sha1_test.cc
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
int32_t TestSha1(int32_t argc, char** argv)
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

// @brief:  test const char* sha1
TEST(TestSHA1, PtrChar) {
    SHA1 sha;
    const char *in_buffer = "12345678";
    int32_t len = static_cast<int32_t>(strlen(in_buffer));
    sha.Input(in_buffer, len);
    uint32_t out_buffer[5] = {0};
    bool b = sha.Result(out_buffer);
    EXPECT_TRUE(b);

    // compare
    EXPECT_EQ(out_buffer[0], 2082615218UL);
    EXPECT_EQ(out_buffer[1], 2457698954UL);
    EXPECT_EQ(out_buffer[2], 4063189281UL);
    EXPECT_EQ(out_buffer[3], 887657252UL);
    EXPECT_EQ(out_buffer[4], 2154003469UL);
}

// @brief:  test const unsigned char* sha1
TEST(TestSHA1, PtrUChar) {
    SHA1 sha;
    const char *in_buffer = "auewngaagrvjytrefdcvbf";
    int32_t len = static_cast<int32_t>(strlen(in_buffer));
    sha.Input(reinterpret_cast<const unsigned char*>(in_buffer), len);
    uint32_t out_buffer[5] = {0};
    bool b = sha.Result(out_buffer);
    EXPECT_TRUE(b);

    // compare
    EXPECT_EQ(out_buffer[0], 2636503810UL);
    EXPECT_EQ(out_buffer[1], 413486021UL);
    EXPECT_EQ(out_buffer[2], 2621628254UL);
    EXPECT_EQ(out_buffer[3], 2509103816UL);
    EXPECT_EQ(out_buffer[4], 2633212044UL);
}

// @brief:  test char sha1
TEST(TestSHA1, Char) {
    SHA1 sha;
    char in_buffer = 'x';
    sha.Input(in_buffer);
    uint32_t out_buffer[5] = {0};
    bool b = sha.Result(out_buffer);
    EXPECT_TRUE(b);

    // compare
    EXPECT_EQ(out_buffer[0], 301378958UL);
    EXPECT_EQ(out_buffer[1], 3307874692UL);
    EXPECT_EQ(out_buffer[2], 2880109948UL);
    EXPECT_EQ(out_buffer[3], 995190019UL);
    EXPECT_EQ(out_buffer[4], 2019303538UL);
}

// @brief:  test unsigned char sha1
TEST(TestSHA1, UChar) {
    SHA1 sha;
    unsigned char in_buffer = 'w';
    sha.Input(in_buffer);
    uint32_t out_buffer[5] = {0};
    bool b = sha.Result(out_buffer);
    EXPECT_TRUE(b);

    // compare
    EXPECT_EQ(out_buffer[0], 2951750910UL);
    EXPECT_EQ(out_buffer[1], 1253113550UL);
    EXPECT_EQ(out_buffer[2], 1083301380UL);
    EXPECT_EQ(out_buffer[3], 1280887214UL);
    EXPECT_EQ(out_buffer[4], 1110652986UL);
}

