#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/crypto/ca/ca_server/check_usr_test.h"
#include "common/crypto/ca/ca_server/ca.h"

DEFINE_string(user_name, "onebox_ca_test", "user name");
DEFINE_string(ca_host, "127.0.161.50", "ca server ip");
DEFINE_int32(ca_port, 10080, "ca server port");

int32_t main(int32_t argc, char* argv[]) {
    CXSocketLibAutoManage auto_sock_lib;
    InitGoogleDefaultLogParam(argv[0]);
    google::AllowCommandLineReparsing();
    if ( static_cast<uint32_t>(1) != google::ParseCommandLineFlags(&argc, &argv, true)) {
        LOG(ERROR) << "google::ParseCommandLineFlags return false";
        return -1;
    }

    // 生成测试证书
    CACertificate ca_certifacate;
    CA_ERROR_CODE ca_error_code;
    CA ca;
    ca.Init();
    bool ret = false;
    ret = ca.RegistCertificate(FLAGS_user_name.c_str(), &ca_certifacate, &ca_error_code);
    CHECK(ret) << "register certificate fail";

    // 验证该用户
    CheckUsrTest check_user;
    ret =   check_user.IsValidUser(FLAGS_user_name.c_str());
    if (!ret) {
        LOG(ERROR) << "verify user: " << FLAGS_user_name.c_str() << " fail";
        return -1;
    }
    LOG(INFO) << "verify user: " << FLAGS_user_name.c_str() << " ok";
    return 0;
}
