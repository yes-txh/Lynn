//////////////////////////////////////////////////////////////////////////
// expandable_array_test.cc
// @brief:     test serialize and unserialize expandable array
// @author:  fatliu@tencent
// @time:     2010-09-28
// @version: 1.0
//////////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/general_head.h"
#include "common/baselib/svrpublib/general_type_def.h"
#include "common/baselib/svrpublib/thread_mutex.h"
#include "common/baselib/svrpublib/general_util.h"

#include "common/baselib/svrpublib/log.h"
#include "common/baselib/svrpublib/lite_mempool.h"
#include "common/baselib/svrpublib/general_sock.h"

#include "common/baselib/svrpublib/expandable_array.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

#ifdef WIN32
int32_t TestExpandableArray(int32_t argc, char** argv)
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

// @brief:      test function ResetContent
TEST(VarDataSerialize, ResetContent) {
    VarDataSerialize ser_array;
    bool b = ser_array.ResetContent();
    CHECK(b);
}

// @brief:      test send serialize data and receive unserialize data
//                  in normal case
TEST(TestVarData, NormalCase) {
    VarDataSerialize ser_array;
    VarDataUnSerialize unser_array;

    // normal
    CHECK(ser_array.ResetContent());
    uint32_t extend_step = 1024*1024;
    ser_array.SetExtendStep(extend_step);

    const char* str[3];
    str[0] = "abc≤‚ ‘"
             "fat"
             "TEST1";
    str[1] = "def≤‚ ‘"
             "fat"
             "TEST2";
    str[2] = "ghi≤‚ ‘"
             "fat"
             "TEST3";

    // add data
    CHECK(ser_array.AddVarData(str[0],
                               (uint32_t)strlen(str[0])+1));
    CHECK(ser_array.AddVarData(str[1],
                               (uint32_t)strlen(str[1])+1));
    CHECK(ser_array.AddVarData(str[2],
                               (uint32_t)strlen(str[2])+1));

    // package
    const char* pack_data = NULL;
    uint32_t pack_len = 0;
    uint32_t pack_item_count = 0;
    CHECK(ser_array.GetPackage(&pack_data,
                               &pack_len,
                               &pack_item_count));

    // unpackage
    CHECK(unser_array.AttachPackage(pack_data, pack_len));

    uint32_t unpack_item_count = unser_array.GetValidItemsCount();
    CHECK_EQ(pack_item_count, unpack_item_count);

    // compare data before package and after unpackage
    const char* unpack_str = NULL;
    uint32_t unpack_str_len = 0;
    uint32_t unpack_pos = 0;
    for (uint32_t u = 0; u < pack_item_count; u++) {
        CHECK(unser_array.GetNextVal(&unpack_str,
                                     &unpack_str_len,
                                     &unpack_pos));
        CHECK_EQ(unpack_str_len, (uint32_t)strlen(str[u])+1);
        CHECK_EQ(0, strncmp((const char*)unpack_str,
                            str[u],
                            unpack_str_len));
    }

    // fail, if get next val at the end of a package
    CHECK(!unser_array.GetNextVal(&unpack_str,
                                  &unpack_str_len,
                                  &unpack_pos));
}

// @brief:      test send serialize data and receive unserialize data
//                  in abnormal case
TEST(TestVarData, AbnormalCase) {
    VarDataSerialize ser_array;
    VarDataUnSerialize unser_array;

    // no item in package is ok
    const char* data0 = NULL;
    uint32_t len0 = 0;
    uint32_t count0 = 0;
    CHECK(ser_array.GetPackage(&data0, &len0, &count0));

    // fail, length of package is less than head len(4)
    CHECK(!unser_array.AttachPackage(data0, 3));
    // attach package again,
    // next val will be str[0]
    CHECK(unser_array.AttachPackage(data0, 256));
    uint32_t num = unser_array.GetValidItemsCount();
    CHECK_EQ(num, count0);
    CHECK(!unser_array.GetNextVal(&data0, &len0));
}

