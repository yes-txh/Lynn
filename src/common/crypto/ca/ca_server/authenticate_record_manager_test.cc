// Copyright 2010, Tencent Inc.
// Author: GuiYou Tian (joeytian@tencent.com)
// 测试authenticate_record_manager.cc

#include "gtest/gtest.h"

#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/crypto/ca/ca_server/authenticate_record_manager.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE

using namespace ca;

#ifndef WIN32

int main(int argc, char** argv) {
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, false);

    InitGoogleDefaultLogParam(argv[0]);

    AutoBaseLib auto_baselib;

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif

TEST(AuthRecord, AppendAndRead) {
    // 增加一个认证记录
    const char* user_name = "test";
    const char* client_ip = "xxx.xxx.xxx.xxx";
    AuthRecord::GetInstance()->AppendRecord(user_name, client_ip);

    time_t current_epoch_time = time(NULL);
    struct tm current_local_time;
    safe_localtime(&current_epoch_time, &current_local_time);

    // 格式化记录, 格式为[time]--identity ip
    char current_gmt_time[128];
    strftime(current_gmt_time, sizeof(current_gmt_time), "%Y-%m-%d %H:%M:%S", &current_local_time);
    char auth_record[128];
    safe_snprintf(auth_record, sizeof(auth_record),
        "[%s]--%s %s\n", current_gmt_time, user_name, client_ip);



    // 读出并校验该记录
    time_t req_chart_epoch_time = time(NULL);
    time_t req_text_epoch_time = time(NULL);
    int32_t req_day_num = 1;
    int32_t req_page_num = 0;
    int32_t total_page_num = 0;
    std::string auth_record_file;
    std::string count_statistics_file;

    // ---------------------------------------------------------------------------------
    // 如果只读出统计记录
    AuthRecord::GetInstance()->ReadRecord(req_chart_epoch_time, req_text_epoch_time, req_day_num,
        req_page_num, &total_page_num, &auth_record_file, &count_statistics_file);
    // 校验返回值
    EXPECT_EQ(total_page_num, 0);
    EXPECT_EQ(auth_record_file.length(), (uint32_t)0);

    struct tm req_local_time = {0};
    char str_daily_count[128] = {0};
    safe_localtime(&req_chart_epoch_time, &req_local_time);
    safe_snprintf(str_daily_count, sizeof(str_daily_count), "%d=1\n", req_local_time.tm_hour);
    // 校验读出的记录
    EXPECT_STREQ(count_statistics_file.c_str(), str_daily_count);

    // ---------------------------------------------------------------------------------
    // 读出统计记录和认证记录
    auth_record_file.clear();
    count_statistics_file.clear();
    req_page_num = 1;
    AuthRecord::GetInstance()->ReadRecord(req_chart_epoch_time, req_text_epoch_time, req_day_num,
        req_page_num, &total_page_num, &auth_record_file, &count_statistics_file);
    // 校验返回值
    EXPECT_EQ(total_page_num, 1);
    EXPECT_STREQ(count_statistics_file.c_str(), str_daily_count);
    EXPECT_STREQ(auth_record_file.c_str(), auth_record);

    // ---------------------------------------------------------------------
    // 增加多条记录
    int32_t record_num = 1050;
    for (int32_t i = 0; i < record_num; ++i) {
        AuthRecord::GetInstance()->AppendRecord(user_name, client_ip);
    }
    // 读出校验
    auth_record_file.clear();
    count_statistics_file.clear();
    req_page_num = 1;
    AuthRecord::GetInstance()->ReadRecord(req_chart_epoch_time, req_text_epoch_time, req_day_num,
        req_page_num, &total_page_num, &auth_record_file, &count_statistics_file);
    // 校验返回值，默认一页为1000条，则总页数该是2页
    EXPECT_EQ(total_page_num, 2);
    // 加上之前的一条记录，共有record_num + 1条记录
    safe_snprintf(str_daily_count, sizeof(str_daily_count), "%d=%d\n",
        req_local_time.tm_hour, record_num + 1);
    EXPECT_STREQ(count_statistics_file.c_str(), str_daily_count);
    return;
}
