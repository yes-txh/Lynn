// Copyright 2011, Tencent Inc.
// Author: Xiaodong Chen(donniechen@tencent.com)
//
// Unittest for HttpGetResponse

#include "glog/logging.h" 
#include "gtest/gtest.h"
#include "common/baselib/svrpublib/server_publib_namespace.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE

TEST(CGetHttpResponse, Domain) {
    /*CGetHttpResponse http_resp;
    char        ptr_param[512] = {0};
    safe_snprintf(ptr_param, sizeof(ptr_param),
        "RegisterSchedulerMasterIP=http://127.0.0.1:1111/&Name=unit_test&Remark=linux");

    HTTP_CGI_ERROR ret = http_resp.GetResponse(
        "xfs.soso.oa.com", "/cgi-bin/XFSEntry.cgi", ptr_param, false);

    EXPECT_EQ( ERROR_HTTP_OK, ret);

    ret = http_resp.GetResponse(
        "xfs.soso.oa.com:80", "/cgi-bin/XFSEntry.cgi", ptr_param, false);

    EXPECT_EQ(ERROR_HTTP_OK, ret);

    ret = http_resp.GetResponse(
        "xfs.soso.oa.com:81", "/cgi-bin/XFSEntry.cgi", ptr_param, false);

    EXPECT_NE(ERROR_HTTP_OK, ret);

    ret = http_resp.GetResponse(
        "172.24.28.193:80", "/cgi-bin/XFSEntry.cgi", ptr_param, false);

    EXPECT_EQ(ERROR_HTTP_OK, ret);

    ret = http_resp.GetResponse(
        "172.24.28.193", "/cgi-bin/XFSEntry.cgi", ptr_param, false);

    EXPECT_EQ(ERROR_HTTP_OK, ret);

    ret = http_resp.GetResponse(
        "172.24.28.193:8080", "/cgi-bin/XFSEntry.cgi", ptr_param, false);

    EXPECT_NE(ERROR_HTTP_OK, ret);*/
}