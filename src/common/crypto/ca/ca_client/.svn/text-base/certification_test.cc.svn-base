// Copyright 2010, Tencent Inc.
// Author: fatliu(fatliu@tencent.com)

#include "common/baselib/svrpublib/server_publib.h"
#include "common/base/scoped_ptr.h"
// includes from thirdparty
#include "thirdparty/gtest/gtest.h"
#include "thirdparty/gflags/gflags.h"

#include "common/crypto/ca/ca_public/certification.h"

#include "common/base/module.hpp"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace ca;

#ifndef TEST_MAIN
#ifdef WIN32
#define TEST_MAIN(x)    x##_main
#else
#define TEST_MAIN(x)    main
#endif
#endif

DECLARE_string(ticket);

namespace ca {

TEST(CertificationTest, ModuleInit) {
    std::string role, id;
    EXPECT_TRUE(Certifier::Instance().IsVerified());

    EXPECT_TRUE( Certifier::Instance().GetIdentity(&id) );
    EXPECT_FALSE(id.empty());
    LOG(INFO) << "ModuleInit id: " << id;

    EXPECT_TRUE( Certifier::Instance().GetRole(&role) );
    EXPECT_FALSE(role.empty());
    LOG(INFO) << "ModuleInit role: " << role;
}

TEST(CertificationTest, ValidUser) {
    std::string id;
    struct stat buff;
    if (stat("certification_test.runfiles/", &buff)) {
        system("cp -d *.dat ../");
    }

    // verify from cache
    std::string role = Certifier::Instance().VerifyUser("fatliu");
    EXPECT_FALSE(role.empty());
    LOG(INFO) << "Role: " << role;

    // no username
    // use gflag set --identity=fatliu or pecywang
    // otherwise, fail
    role = Certifier::Instance().VerifyUser(NULL, NULL);
    char* env = getenv("USER");
    if (STRLEN(env) != 0 && (strcmp(env, "fatliu") == 0 || strcmp(env, "pecywang") == 0 || strcmp(env, "lingkunchu") == 0))
        EXPECT_FALSE(role.empty());
    else
        EXPECT_TRUE(role.empty());

    role = Certifier::Instance().VerifyUser("fatliu");
    EXPECT_FALSE(role.empty());
    LOG(INFO) << "Role: " << role;
    EXPECT_TRUE( Certifier::Instance().GetRole(&role) );
    LOG(INFO) << "Role: " << role;

    EXPECT_TRUE( Certifier::Instance().CheckUser("pecywang") );
    EXPECT_TRUE( Certifier::Instance().CheckUser("pecywang", "xfs") );
    EXPECT_FALSE( Certifier::Instance().CheckUser("pecywang_xxx", "xfs") );

    // TODO:Verify Role
    Certifier::Instance().Reset();
    role = Certifier::Instance().VerifyUser("fatliu", "xfs");
    EXPECT_FALSE(role.empty());
    CHECK(role.compare("xfs") == 0);

}

TEST(CertificationTest, CreateTicket) {
    std::string role;

    Certifier::Instance().Reset();
    role = Certifier::Instance().VerifyUser("fatliu", "xfs");
    EXPECT_FALSE(role.empty());
    CHECK(role.compare("xfs") == 0);

    std::string ticket;
    EXPECT_TRUE( Certifier::Instance().CreateTicket(&ticket));
    LOG(INFO) << "Ticket: " << ticket;

    FLAGS_ticket = ticket;
}

TEST(CertificationTest, VerifyFromTicket) {
    std::string id, role;

    LOG(INFO) << "FLAGS_ticket: " << FLAGS_ticket;

    role = Certifier::Instance().VerifyUser();
    EXPECT_FALSE(role.empty());
    CHECK(role.compare("xfs") == 0);

    EXPECT_TRUE(Certifier::Instance().IsVerified());

    EXPECT_TRUE( Certifier::Instance().GetIdentity(&id) );
    CHECK(id.compare("fatliu") == 0);

    EXPECT_TRUE( Certifier::Instance().GetRole(&role) );
    CHECK(role.compare("xfs") == 0);
}

TEST(CertificationTest, CorruptedTicket) {
    std::string id, role;

    FLAGS_ticket = "ZmF0bGl1CXhxcwk3MzkyNzE3NTQ=";

    LOG(INFO) << "FLAGS_ticket: " << FLAGS_ticket;

    Certifier::Instance().Reset();
    role = Certifier::Instance().VerifyUser();
    EXPECT_TRUE(role.empty());

    EXPECT_FALSE(Certifier::Instance().IsVerified());

    EXPECT_FALSE( Certifier::Instance().GetRole(&role) );
    EXPECT_FALSE( Certifier::Instance().GetIdentity(&id) );
}

TEST(CertificationTest, InvalidUser) {
    FLAGS_ticket = "";
    Certifier::Instance().Reset();
    std::string role = Certifier::Instance().VerifyUser("fatliu_xxx");
    EXPECT_TRUE(role.empty());
}

}

////////////////////////////////////////////////////////////////////////
// ²âÊÔµÄmainº¯Êý
//
// int32_t main(argc, argv**)
//
int main(int argc, char** argv) {
#ifndef WIN32

    InitAllModulesAndTest(&argc, &argv);

    AutoBaseLib auto_base_lib;

    return RUN_ALL_TESTS();
#else
    return 0;
#endif
}
