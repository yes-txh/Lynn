// my_simple_http.cc: interface for the CMySimpleHttpThread class.
// jackyzhao@tencent.com
//////////////////////////////////////////////////////////////////////

#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/baselib/svrpublib/test_tools/test_http_service/my_simple_http.h"

using namespace xfs::base;

// 用户可以继承这个函数实现具体响应
bool CMySimpleHttpThread::OnUserHTTPRequest(const BufferV* received_buff,
        CHttpBuff* ptr_http_response)
{
    if (strcmp((char*)received_buff->buff, "/") == 0 ||
        STRNCASECMP((char*)received_buff->buff, "/index.htm", strlen("/index.htm")) == 0) {
        ptr_http_response->SetAttr("index.html", "simple http test");
        ptr_http_response->AddKey("Welcome ....", "", "");

        //设置页面超链
        char link[1024] = {0};

        safe_snprintf(link, sizeof(link), "http://%s/system_info.html", GetListenHostPort());
        ptr_http_response->AddHref("system_info.html", link, "system info", true);

        safe_snprintf(link, sizeof(link), "http://%s/stat.html", GetListenHostPort());
        ptr_http_response->AddHref("stat.html", link, "stat");


        safe_snprintf(link, sizeof(link), "http://%s/go_to_oa.html", GetListenHostPort());
        ptr_http_response->AddHref("go_to_oa.html", link, "go to oa");

        safe_snprintf(link, sizeof(link), "http://%s/download.html?user=fatliu", 
            GetListenHostPort());
        ptr_http_response->AddHref("download_file", link, "used_node");

        return true;
    }

    if (STRNCASECMP((char*)received_buff->buff, "/stat.htm", strlen("/stat.htm")) == 0) {
        //设置页面标题
        ptr_http_response->SetAttr("About node_server information", "node_server");

        //设置统计项 key - value 及描述信息
        char        info[1024] = {0};

        safe_snprintf(info, sizeof(info), "version=1.4.5, total node=1098, free node=33, "
                      "used node=1098-33, total space=2T, free space=1.5T, used space=0.5T");

        ptr_http_response->AddKey("", info, "");
        ptr_http_response->AddKey("", "a<br>b&nbsp;c", "");
        ptr_http_response->AddKey("<hr class='xfs_line' align='left'>",
                                  "",
                                  "");

        SetBottomLink(ptr_http_response);

        return true;
    }


    if (STRNCASECMP((char*)received_buff->buff, "/go_to_oa.html", 
        strlen("/go_to_oa.html")) == 0) {
        char html_buff[256] = {0};
        const char* proxy = GetProxyRequest();
        safe_snprintf(html_buff, sizeof(html_buff),
                      "<meta http-equiv='Refresh' content='1;"
                      "URL=http://passport.oa.com/modules/passport/signin.ashx?"
                      "url=%s%shttp://%s/back_from_oa.html' />",
                      proxy , STRLEN(proxy)? "?realserver=":"", GetListenHostPort());
        ptr_http_response->AddKey("", html_buff, "");


        return true;
    }

    if (STRNCASECMP((char*)received_buff->buff, "/back_from_oa.html", 
        strlen("/back_from_oa.html")) == 0) {
        ptr_http_response->AddKey("", "aha!", "");

        CParserCGIParameter parser;
        parser.AttachEnvironmentString(strchr((const char*)received_buff->buff, '?') + 1);
        const char* ticket = NULL;
        parser.GetParameter("ticket", &ticket);

        return true;
    }

    if (STRNCASECMP((char*)received_buff->buff, "/download.html", 
        strlen("/download.html")) == 0) {
        CParserCGIParameter parser;
        parser.AttachEnvironmentString(strchr((const char*)received_buff->buff, '?') + 1);
        const char* user = NULL;
        parser.GetParameter("user", &user);
        
        char html_buff[256] = {0};
        const char* proxy = GetProxyRequest();
        safe_snprintf(html_buff, sizeof(html_buff),
                      "<meta http-equiv='Refresh' content='1;URL=%s%shttp://%s/%s.dat' />",
                      proxy , STRLEN(proxy)? "?realserver=":"", 
                      GetListenHostPort(), user ? user : "unknown");
        ptr_http_response->AddKey("", html_buff, "");


        return true;
    }

    return false;
}

void CMySimpleHttpThread::SetBottomLink(CHttpBuff* ptr_http_response)
{
    ptr_http_response->AddHref("Tencent company",
                               "http://www.tencent.com", "test link to tencent.com");
}


void CMySimpleHttpThread::DealGetUsedNodeReq(CHttpBuff* ptr_http_response)
{
    ptr_http_response->AddKey("local_id", "ptr_mmap_node[u].m_local_id", "");
    ptr_http_response->AddKey("used_node", "num_used_node", "");
}

