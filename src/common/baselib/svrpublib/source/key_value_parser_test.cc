//////////////////////////////////////////////////////////////////////////
// key_value_parser_test.cc
// @brief:     Test class CKeyValueParser
// @author:  fatliu@tencent
// @time:     2010-09-30
// @version: 1.0
//////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/twse_type_def.h"
#include "common/baselib/svrpublib/key_value_parser.h"
#include "common/baselib/svrpublib/general_head.h"
#include "common/baselib/svrpublib/general_type_def.h"
#include "common/baselib/svrpublib/thread_mutex.h"
#include "common/baselib/svrpublib/general_util.h"
#include "common/baselib/svrpublib/log.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

#ifdef WIN32
int32_t TestKeyValueParser(int32_t argc, char** argv)
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

// @brief:     Test method ParserFromBuffer
TEST(CKeyValueParser, ParserFromBuffer) {
    CKeyValueParser parser;

    bool b = false;
    char* str_buff = const_cast<char*>(
                         "TYPE=single\r\nPROJID=380\r\nSERVICEID=1000\r\nSERVICEID=2000"
                     );

    uint32_t len_buff = static_cast<uint32_t>(strlen(str_buff));
    // parse from buffer
    b = parser.ParserFromBuffer((const unsigned char*)str_buff, len_buff);
    CHECK_EQ(b, true);
}

// @brief:     Test GetSameKeyCount in ParserFromBuffer
TEST(TestGetSameKeyCount, ParserFromBuffer) {
    CKeyValueParser parser;

    bool b = false;
    char* str_buff = const_cast<char*>(
                         "TYPE=single\r\nPROJID=380\r\nSERVICEID=1000\r\nSERVICEID=2000"
                     );

    uint32_t len_buff = static_cast<uint32_t>(strlen(str_buff));
    // parse from buffer
    b = parser.ParserFromBuffer((const unsigned char*)str_buff, len_buff);
    CHECK_EQ(b, true);

    // get same key count
    int32_t num = parser.GetSameKeyCount("SERVICEID");
    CHECK_EQ(2, num);
    num = parser.GetSameKeyCount("SER");
    CHECK_EQ(0, num);
}

// @brief:     Test GetVal in ParserFromBuffer
TEST(TestGetValue, ParserFromBuffer) {
    CKeyValueParser parser;

    bool b = false;
    char* str_buff = const_cast<char*>(
                         "TYPE=single\r\nPROJID=380\r\nSERVICEID=1000\r\nSERVICEID=2000"
                     );

    uint32_t len_buff = static_cast<uint32_t>(strlen(str_buff));
    // parse from buffer
    b = parser.ParserFromBuffer((const unsigned char*)str_buff, len_buff);
    CHECK_EQ(b, true);

    // get value
    int32_t val_len = 256;
    unsigned char* val_buff = new unsigned char[val_len];
    b = parser.GetValue("TYPE", val_buff, val_len);
    CHECK_EQ(b, true);
    CHECK_EQ(0, strncmp((const char*)val_buff, "single", strlen("single")));

    memset(val_buff, 0, val_len);
    b = parser.GetValue("TYPE", val_buff, val_len, 1);
    CHECK_EQ(b, false);

    memset(val_buff, 0, val_len);
    b = parser.GetValue("TYPE", val_buff, val_len, 0);
    CHECK_EQ(b, true);
    CHECK_EQ(0, strncmp((const char*)val_buff, "single", strlen("single")));

    delete[] val_buff;
}

// @brief:     Test method ParserFromFile
TEST(CKeyValueParser, ParseFromFile) {
    CKeyValueParser parser;

#ifdef WIN32
    const char* filename = "e:\\parser_test.txt";
#else
    // 当前目录打开
    const char* filename = "parser_test.txt";
#endif

    FILE *h_file = fopen(filename, "w+");
    CHECK(h_file != NULL);

    char* str_buff = const_cast<char*>(
                         "TYPE=single\r\nPROJID=380\r\nSERVICEID=1000\r\nSERVICEID=2000"
                     );

    fprintf(h_file, "%s", str_buff);
    fprintf(h_file, "%s", str_buff);
    fclose(h_file);

    //  parse from file
    bool b = parser.ParserFromFile(filename);
    CHECK_EQ(b, true);
    int32_t num = parser.GetSameKeyCount("SERVICEID");
    CHECK_EQ(4, num);
}

// @brief:     Test GetVal in ParserFromFile
TEST(TestGetVal, ParserFromFile) {
    CKeyValueParser parser;

#ifdef WIN32
    const char* filename = "e:\\parser_test.txt";
#else
    const char* filename = "parser_test.txt";
#endif

    FILE *h_file = fopen(filename, "w+");
    CHECK(h_file != NULL);

    char* str_buff = const_cast<char*>(
                         "TYPE=single\r\nPROJID=380\r\nSERVICEID=1000\r\nSERVICEID=2000"
                     );

    fprintf(h_file, "%s", str_buff);
    fprintf(h_file, "%s", str_buff);
    fclose(h_file);

    //  parse from file
    bool b = parser.ParserFromFile(filename);
    CHECK_EQ(b, true);
    int32_t num = parser.GetSameKeyCount("SERVICEID");
    CHECK_EQ(4, num);

    // get value
    int32_t val_len = 256;
    unsigned char* val_buff = new unsigned char[val_len];
    memset(val_buff, 0, val_len);
    b = parser.GetValue("SERVICEID", val_buff, val_len);
    CHECK_EQ(b, true);
    CHECK_EQ(0, strncmp((const char*)val_buff, "1000", strlen("1000")));

    memset(val_buff, 0, val_len);
    b = parser.GetValue("SERVICEID", val_buff, val_len, 1);
    CHECK_EQ(b, true);
    CHECK_EQ(0, strncmp((const char*)val_buff, "2000", strlen("2000")));

    memset(val_buff, 0, val_len);
    b = parser.GetValue("SERVICEID", val_buff, val_len, 2);
    CHECK_EQ(b, true);
    CHECK_EQ(0, strncmp((const char*)val_buff, "1000", strlen("1000")));

    delete[] val_buff;
}
