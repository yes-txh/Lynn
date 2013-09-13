//////////////////////////////////////////////////////////////////////////
// base_protocol_test.cc
// @brief:     Test class BaseProtocolPack & BaseProtocolUnPack
// @author:  fatliu@tencent
// @time:     2010-10-14
// @version: 1.0
//////////////////////////////////////////////////////////////////////////
#include <string.h>
#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

#ifdef WIN32
int32_t TestBaseProtocol(int32_t argc, char** argv)
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

// @brief:     send and receive data type of unsigned char*
TEST(TestSetKeyAndGetVal, UpcharType) {
    CBaseProtocolPack pack;
    CBaseProtocolUnpack unpack;
    pack.Init();
    unpack.Init();

    // prepare package
    uint16_t key = 15;
    unsigned char* val = (unsigned char*)"unsigned char*";
    bool b = pack.SetKey(key, val);
    CHECK(b);

    // get package
    unsigned char* package;
    uint32_t len = 0;
    pack.GetPackage(&package, &len);

    // unpack package
    unpack.AttachPackage(package, len);
    unpack.Unpack();

    // get val and compare val and get_val
    unsigned char* get_val;
    uint32_t len_val = 0;
    b = unpack.GetVal(key, &get_val, &len_val);
    CHECK(b);
    CHECK_EQ(0, strcmp((const char*)val, (const char*)get_val));

    pack.Uninit();
    unpack.Uninit();
}

// @brief:     send and receive data type of char*
TEST(TestSetKeyAndGetVal, PcharType) {
    CBaseProtocolPack pack;
    CBaseProtocolUnpack unpack;
    pack.Init();
    unpack.Init();

    // prepare package
    uint16_t key = 15;
    unsigned char* val = (unsigned char*)"char*";
    bool b = pack.SetKey(key, val);
    CHECK(b);

    // get package
    unsigned char* package;
    uint32_t len = 0;
    pack.GetPackage(&package, &len);

    // unpack package
    unpack.AttachPackage(package, len);
    unpack.Unpack();

    // get val and compare val and get_val
    unsigned char* get_val;
    uint32_t len_val = 0;
    b = unpack.GetVal(key, &get_val, &len_val);
    CHECK(b);
    CHECK_EQ(0, strcmp((const char*)val, (const char*)get_val));

    pack.Uninit();
    unpack.Uninit();
}

// @brief:     send and receive data type of object std::string
TEST(TestSetKeyAndGetVal, StringType) {
    CBaseProtocolPack pack;
    CBaseProtocolUnpack unpack;
    pack.Init();
    unpack.Init();

    // prepare package
    uint16_t key = 15;
    std::string val = "string";
    bool b = pack.SetKey(key, &val);
    CHECK(b);

    // get package
    unsigned char* package;
    uint32_t len = 0;
    pack.GetPackage(&package, &len);

    // unpack package
    unpack.AttachPackage(package, len);
    unpack.Unpack();

    // get val and compare val and get_val
    std::string* get_val;
    b = unpack.GetVal(key, &get_val);
    CHECK(b);
    CHECK_EQ(0, strcmp(val.c_str(), get_val->c_str()));

    pack.Uninit();
    unpack.Uninit();
}

// @brief:     send and receive data of different type:
//                 int16_t, uint16_t, int32_t, uint32_t, int64_t,
//                 uint64_t, unsigned char, char, float, double
template <typename T>
class TestSetKeyAndGetVal:public::testing::Test {
public:
    T value_;
};

typedef testing::Types<int16_t, uint16_t,
        int32_t, uint32_t,
        int64_t, uint64_t,
        unsigned char, char,
        float, double> MyTypes;

TYPED_TEST_CASE(TestSetKeyAndGetVal, MyTypes);

TYPED_TEST(TestSetKeyAndGetVal, SetKeyAndGetVal) {
    TypeParam val = this->value_;

    bool b = false;
    CBaseProtocolPack pack;
    CBaseProtocolUnpack unpack;
    pack.Init();
    unpack.Init();

    // prepare package
    uint16_t key = safe_rand() % 10000;
    b = pack.SetKey(key, val);
    CHECK(b);

    // get package
    unsigned char* package;
    uint32_t len = 0;
    pack.GetPackage(&package, &len);

    // unpack package
    unpack.AttachPackage(package, len);
    unpack.Unpack();

    // get val and compare val and get_val
    TypeParam get_val;
    b = unpack.GetVal(key, &get_val);
    CHECK(b);
    CHECK_EQ(get_val, val);

    pack.Uninit();
    unpack.Uninit();
}

// @brief:     test function AppendKeyData
TEST(CBaseProtocolPack, AppendKeyData) {
    bool b = false;

    CBaseProtocolPack pack;
    CBaseProtocolUnpack unpack;
    pack.Init();
    unpack.Init();

    // prepare package
    uint16_t key_cpbyte = 0;
    const unsigned char* val_cpbyte=
        (const unsigned char*)"const unsigned char*";

    uint32_t len = (uint32_t)strlen((const char*)val_cpbyte);
    b = pack.SetKey(key_cpbyte, val_cpbyte, len);
    CHECK(b);

    // append key data
    const unsigned char* data = (const unsigned char*)"append";
    len = (uint32_t)strlen((const char*)data);
    b = pack.AppendKeyData(key_cpbyte, data, len);
    CHECK(b);

    // get package
    unsigned char* package;
    len = 0;
    pack.GetPackage(&package, &len);

    // unpack package
    unpack.AttachPackage(package, len);
    unpack.Unpack();

    // get val and compare val and get_val
    unsigned char* val_upchar;
    uint32_t len1 = 0;
    b = unpack.GetVal(key_cpbyte,
                      (unsigned char**)&val_upchar,
                      &len1);
    CHECK(b);
    CHECK_EQ(0, strncmp("const unsigned char*append",
                        (const char*)val_upchar, len1));
    unpack.Uninit();
    pack.Uninit();
}

// @brief:     test function SetOption & GetOption
TEST(TestOption, Option) {
    CBaseProtocolPack pack;
    CBaseProtocolUnpack unpack;
    pack.Init();
    unpack.Init();

    // set option
    pack.SetOption(BASEPROTOCOL_OPT_NETORDER, true);
    pack.SetOption(BASEPROTOCOL_OPT_KEEPALIVE, true);
    pack.SetOption(BASEPROTOCOL_OPT_NETORDER, false);

    unsigned char* package;
    uint32_t len = 0;
    pack.GetPackage(&package, &len);

    unpack.AttachPackage(package, len);
    unpack.Unpack();

    // get option
    uint32_t option = unpack.GetOption();
    CHECK(option & BASEPROTOCOL_OPT_KEEPALIVE);
    CHECK_EQ(0, option & BASEPROTOCOL_OPT_NETORDER);

    pack.Uninit();
    unpack.Uninit();
}

// @brief:     test function SetTTL & GetTTL
TEST(TestTTL, TTL) {
    CBaseProtocolPack pack;
    CBaseProtocolUnpack unpack;
    pack.Init();
    unpack.Init();

    // set TTL
    pack.SetTTL('a');

    unsigned char* package;
    uint32_t len = 0;
    pack.GetPackage(&package, &len);

    unpack.AttachPackage(package, len);
    unpack.Unpack();

    // get TTL
    uint32_t ttl = unpack.GetTTL();
    CHECK_EQ(ntohl('a'), ttl);

    pack.Uninit();
    unpack.Uninit();
}

// @brief:     test function SetSeq & GetSeq
TEST(TestSeq, Seq) {
    CBaseProtocolPack pack;
    CBaseProtocolUnpack unpack;
    pack.Init();
    unpack.Init();

    // set Seq
    pack.SetSeq(3);

    unsigned char* package;
    uint32_t len = 0;
    pack.GetPackage(&package, &len);

    unpack.AttachPackage(package, len);
    unpack.Unpack();

    // get Seq
    uint32_t seq = unpack.GetSeq();
    CHECK_EQ(3, seq);

    pack.Uninit();
    unpack.Uninit();
}

// @brief:     test function SetServiceType & GetServiceType
TEST(TestServiceType, ServiceType) {
    CBaseProtocolPack pack;
    CBaseProtocolUnpack unpack;
    pack.Init();
    unpack.Init();

    // set service type
    pack.SetServiceType(5);

    unsigned char* package;
    uint32_t len = 0;
    pack.GetPackage(&package, &len);

    unpack.AttachPackage(package, len);
    unpack.Unpack();

    // get service type
    uint16_t type = unpack.GetServiceType();
    CHECK_EQ(uint32_t(5), type);

    pack.Uninit();
    unpack.Uninit();
}
