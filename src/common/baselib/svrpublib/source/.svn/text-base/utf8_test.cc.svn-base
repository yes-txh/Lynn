//////////////////////////////////////////////////////////////////////////
// utf8_test.cc
// @brief:   测试utf-8字符安全copy、Ansi转化为utf-8
// @author:  fatliu@tencent
// @time:    2010-09-28
// @version: 1.0
//////////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/general_head.h"
#include "common/baselib/svrpublib/general_type_def.h"
#include "common/baselib/svrpublib/thread_mutex.h"
#include "common/baselib/svrpublib/general_util.h"
#include "common/baselib/svrpublib/log.h"
#include "common/baselib/svrpublib/utf8.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

#ifdef WIN32
int32_t TestUTF8(int32_t argc, char** argv)
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

TEST(TestGetUTF8ByteType, GetUTF8ByteType) {
    // 分别测试ENUM_UTF8_BYTE_TYPE的8种可能情况
    ENUM_UTF8_BYTE_TYPE type = GetUTF8ByteType('A');
    CHECK_EQ(UTF8_BYTE_TYPE_1st_OF_1, type);

    type = GetUTF8ByteType(0xB0);
    CHECK_EQ(UTF8_BYTE_TYPE_CONTINUING, type);

    type = GetUTF8ByteType(0xC0);
    CHECK_EQ(UTF8_BYTE_TYPE_1st_OF_2, type);

    type = GetUTF8ByteType(0xE0);
    CHECK_EQ(UTF8_BYTE_TYPE_1st_OF_3, type);

    type = GetUTF8ByteType(0xF0);
    CHECK_EQ(UTF8_BYTE_TYPE_1st_OF_4, type);

    type = GetUTF8ByteType(0xF8);
    CHECK_EQ(UTF8_BYTE_TYPE_1st_OF_5, type);

    type = GetUTF8ByteType(0xFC);
    CHECK_EQ(UTF8_BYTE_TYPE_1st_OF_6, type);

    type = GetUTF8ByteType(0xFE);
    CHECK_EQ(UTF8_BYTE_TYPE_UNKNOWN, type);
}

TEST(TestStrncpyUtf8, Normal) {
    // 正常情况
    unsigned char src_buf[16]= {0};
    // src_buf内容为:"utf-8测试FAT"
    src_buf[0]='u';
    src_buf[1]='t';
    src_buf[2]='f';
    src_buf[3]='-';
    src_buf[4]='8';
    src_buf[5]=0xE6;
    src_buf[6]=0xB5;
    src_buf[7]=0x8B;
    src_buf[8]=0xE8;
    src_buf[9]=0xAF;
    src_buf[10]=0x95;
    src_buf[11]='F';
    src_buf[12]='A';
    src_buf[13]='T';
    src_buf[14]='\0';

    printf("%s", src_buf);
    unsigned char dst_buf[16] = {0};
    int32_t copy = 0;
    for (uint32_t i = 0; i != 16; i++) {
        copy = strncpy_utf8(reinterpret_cast<char*>(dst_buf),
                            (const char*)src_buf,
                            i);
        // 复制到汉字"测"时,需要copy3个Bytes
        if (i >= 5 && i <= 7) {
            CHECK_EQ(5, copy);
        } else if (i >= 8 && i <= 10) { // 复制到汉字"试"时,需要copy3个Bytes
            CHECK_EQ(8, copy);
        } else if (i <= 13) {
            CHECK_EQ((int32_t)i, copy);
        } else { // 复制到'\0'时,停止复制
            CHECK_EQ(14, copy);
        }
    }
}

TEST(TestStrncpyUtf8, Abnormal) {
    // 非正常情况
    unsigned char src_buf[16]= {0};
    unsigned char dst_buf[16]= {0};
    memset(src_buf, 0, sizeof(src_buf));
    memset(dst_buf, 0, sizeof(dst_buf));
    // src_buf开头信息不为utf-8格式:
    // 开头为UTF8_BYTE_TYPE_CONTINUING
    src_buf[0]=0xB0;
    src_buf[1]='F';
    src_buf[2]='A';
    src_buf[3]='T';
    src_buf[4]='\0';
    int32_t copy = strncpy_utf8(reinterpret_cast<char*>(dst_buf),
                                (const char*)src_buf,
                                16);
    CHECK_EQ(0, copy);

    memset(src_buf, 0, sizeof(src_buf));
    memset(dst_buf, 0, sizeof(dst_buf));
    // src_buf中间信息不为utf-8格式:
    // UTF8_BYTE_TYPE_1st_OF_2(11xxxxxx)后面跟随格式不为10xxxxxx
    // 只copy出错前的字符
    src_buf[0] = 'u';
    src_buf[1] = 't';
    src_buf[2] = 'f';
    src_buf[3] = '-';
    src_buf[4] = '8';
    src_buf[5] = 0xC0;
    src_buf[6] = 0xC0;
    src_buf[7] = 'F';
    src_buf[8] = 'A';
    src_buf[9] = 'T';
    src_buf[10] = '\0';
    copy = strncpy_utf8(reinterpret_cast<char*>(dst_buf),
                        (const char*)src_buf,
                        16);
    CHECK_EQ(5, copy);
}

TEST(TestConvertAnsiToUTF8, Normal) {
    // 正常情况
    const char* ansi_sz = "ANSI测试FAT";
    int32_t ansi_len = static_cast<int32_t>(strlen(ansi_sz));
    int32_t utf8_buff_len = 3 * ansi_len + 1;
    char* utf8_buff = new char[utf8_buff_len];
    int32_t valid_utf8_len = 0;

    bool b = ConvertAnsiToUTF8(ansi_sz,
                               ansi_len,
                               utf8_buff,
                               utf8_buff_len,
                               &valid_utf8_len);
    CHECK_EQ(true, b);
    CHECK_EQ(13, valid_utf8_len);
    delete []utf8_buff;
}

TEST(TestConvertAnsiToUTF8, Abnormal) {
    // utf8_buff_len过小的情况
    const char* ansi_sz = "ANSI测试FAT";
    int32_t ansi_len = static_cast<int32_t>(strlen(ansi_sz));
    int32_t utf8_buff_len = ansi_len;
    char* utf8_buff = new char[utf8_buff_len];
    int32_t valid_utf8_len = 0;

    bool b = ConvertAnsiToUTF8(ansi_sz,
                               ansi_len,
                               utf8_buff,
                               utf8_buff_len,
                               &valid_utf8_len);
    CHECK_EQ(false, b);
    CHECK_EQ(0, valid_utf8_len);
    delete []utf8_buff;
}
