#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "thirdparty/gtest/gtest.h"
#include "common/crypto/ca/ca_server/ca.h"

using namespace ca;

#ifdef WIN32
int32_t TestCa(int32_t argc, char** argv)
#else
int32_t main(int32_t argc, char** argv)
#endif
{
#ifndef WIN32
    InitGoogleDefaultLogParam(0);
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, true);
    AutoBaseLib auto_baselib();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#else
    return 0;
#endif
}

class CaTest: public testing::Test {
public:
    CA ca_obj;
};

TEST(CaTest, TestRegister) {
    CA ca_obj;
	ca_obj.Init();
    // ��CAע��,�������CA�䷢��֤��
    const char* user_name = "joey_test";
    CACertificate ca_certificate;
    CA_ERROR_CODE error_code;
    CHECK(ca_obj.RegistCertificate(user_name, &ca_certificate, &error_code));
    // ��֤֤������
    CHECK_EQ(0, strcmp(ca_certificate.fmt, kCaPubFmt));
    CHECK_EQ(ca_certificate.version, kCurrentVersion);
    CHECK_EQ(ca_certificate.user_name_len, (uint32_t)(strlen(user_name) + 1));
    CHECK_EQ(ca_certificate.public_key_len, sizeof(R_RSA_PUBLIC_KEY));
    CHECK_EQ(0, strcmp(ca_certificate.ca_certificate_data.user_name, user_name));
    CHECK_NE(0, ca_certificate.ca_certificate_data.certificate_time_begin);
    CHECK_NE(0, ca_certificate.ca_certificate_data.certificate_time_end);
    CHECK_NE(0, ca_certificate.sign_len);
    CHECK_EQ(0, strcmp(ca_certificate.memo, CA_MEMO));
    CHECK_EQ(0, strcmp(CaGetErrorCodeStr(error_code), "goes well!"));
}

TEST(CaTest, TestGetPublicInfo) {
    CA ca_obj;
	ca_obj.Init();
    const char* user_name = "joey_test";
    CACertificate ca_certificate;
    CA_ERROR_CODE error_code;
    R_RSA_PUBLIC_KEY user_public_key;
    char user_public_key_path[MAX_PATH] = {0};
    // �û�ͨ�������û�������CA��ѯ������Լ���public_key��֤��
    CHECK(ca_obj.GetUserPublicInfo(user_name, &ca_certificate, &user_public_key,
                            user_public_key_path, MAX_PATH, &error_code));
    // ��֤֤������
    CHECK_EQ(0, strcmp(ca_certificate.fmt, kCaPubFmt));
    CHECK_EQ(ca_certificate.version, kCurrentVersion);
    CHECK_EQ(ca_certificate.user_name_len, (uint32_t)(strlen(user_name) + 1));
    CHECK_EQ(ca_certificate.public_key_len, sizeof(R_RSA_PUBLIC_KEY));
    CHECK_EQ(0, strcmp(ca_certificate.ca_certificate_data.user_name, user_name));
    CHECK_NE(0, ca_certificate.ca_certificate_data.certificate_time_begin);
    CHECK_NE(0, ca_certificate.ca_certificate_data.certificate_time_end);
    CHECK_NE(0, ca_certificate.sign_len);
    CHECK_EQ(0, strcmp(ca_certificate.memo, CA_MEMO));
    CHECK_EQ(0, strcmp(CaGetErrorCodeStr(error_code), "goes well!"));
}

TEST(CaTest, TestGetPrivateInfoCheckUser) {
    CA ca_obj;
	ca_obj.Init();
    const char* user_name = "joey_test";
    CAPrivate user_private_key_file;
    CA_ERROR_CODE error_code;
    char user_private_key_path[MAX_PATH] = {0};
    // �û�ͨ�������û�������CA��ѯ�����������Լ���private_key
    CHECK(ca_obj.GetUserPrivateInfo(user_name, &user_private_key_file,
                             user_private_key_path, MAX_PATH, &error_code));

    // ��֤˽Կ�ļ�����
    CHECK_EQ(0, strcmp(user_private_key_file.fmt, kCaPriFmt));
    CHECK_EQ(user_private_key_file.version, kCurrentVersion);
    CHECK_EQ(user_private_key_file.user_name_len, (uint32_t)(strlen(user_name) + 1));
    CHECK_EQ(user_private_key_file.private_key_len, sizeof(R_RSA_PRIVATE_KEY));
    CHECK_EQ(0, strcmp(user_private_key_file.user_name, user_name));
    CHECK_NE(0, user_private_key_file.sign_len);
    CHECK_EQ(0, strcmp(CaGetErrorCodeStr(error_code), "goes well!"));

    CHECK(ca_obj.IsValidUser(user_name, user_private_key_file.sign,
        user_private_key_file.sign_len, &error_code));
    CHECK_EQ(0, strcmp(CaGetErrorCodeStr(error_code), "goes well!"));
}


TEST(CaTest, TestRenewKey) {
    CA ca_obj;
	ca_obj.Init();
    const char* user_name = "joey_test";
    CACertificate ca_certificate;
    CA_ERROR_CODE error_code;
    R_RSA_PUBLIC_KEY user_public_key;
    char user_public_key_path[MAX_PATH] = {0};
    // �û�ͨ�������û�������CA��ѯ������Լ���public_key��֤��
    CHECK(ca_obj.GetUserPublicInfo(user_name, &ca_certificate, &user_public_key,
                            user_public_key_path, MAX_PATH, &error_code));
    // ����ԭ����֤�鵽��ʱ��
    time_t old_time_end = ca_certificate.ca_certificate_data.certificate_time_end;
    XSleep(1000);

    CHECK(ca_obj.ApplyRenewKey(user_name));
    CHECK(ca_obj.GetUserPublicInfo(user_name, &ca_certificate, &user_public_key,
                            user_public_key_path, MAX_PATH, &error_code));
    time_t new_time_end = ca_certificate.ca_certificate_data.certificate_time_end;
    CHECK_GT(new_time_end, old_time_end);
}

TEST(CaTest, TestRevokeCertificate) {
    CA ca_obj;
	ca_obj.Init();
    const char* user_name = "joey_test";

    // �����û�Ŀ¼�Ƿ����
     char module_dir[MAX_PATH] = {0};
    GetModuleFileName(NULL, module_dir, sizeof(module_dir));

    // linux and windows
    char* p = strrchr(module_dir, '/');
    if ( !p )
        p = strrchr(module_dir, '\\');
    CHECK(p);

    p++;

    // ׷��Ŀ¼htdocs
    int32_t len_remain = module_dir + sizeof(module_dir) - p - 1;
    // �߽���
    int32_t len_add_dir = sizeof("htdocs/") + sizeof("ca_dir/") - 2;
    CHECK_GT(len_remain, len_add_dir);

    safe_snprintf(p, len_remain, "htdocs%sca_dir%s%s", SPLIT_SIGN, SPLIT_SIGN, user_name);
    struct stat buf;

    CHECK_EQ(stat(module_dir, &buf), 0);
    CHECK_EQ((buf.st_mode & S_IFDIR), S_IFDIR);

    // �û�ͨ�������û���,��CA���볷������֤��
    CHECK(ca_obj.RevokeCertificate(user_name));
    CHECK_NE(stat(module_dir, &buf), 0);
}
