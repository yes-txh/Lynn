// simple_http_proxy.cc
//
// ////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string>
#include "thirdparty/gflags/gflags.h"
#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/simple_http_proxy.h"

using namespace std;
DECLARE_USING_LOG_LEVEL_NAMESPACE;

// 默认的首页，出错时跳转到该页面
const char* DEFAULT_REAL_SERVER = "http://xfs.soso.oa.com/index.html";

_START_XFS_BASE_NAMESPACE_

// 解析cgi链接中的 domain cgi_name
bool ParseUrl(const char* url, std::string* domain, std::string* cgi_name) {
    if(NULL == url || NULL == domain || NULL == cgi_name) {
        return false;
    }

    // parse host port
    const char* p = strstr(url, "://");

    // 跳过 ://
    if (p) {
        p += 3;
    } else {
        p = url;
    }

    if(*p =='/') {
        // invalid url
        return false;
    }

    // 查找Host:Port的分隔符
    const char* q = strchr(p, '/');

    if(NULL == q) {
        *domain = std::string(p);
        *cgi_name = std::string("");
        return true;
    }

    *domain = std::string(p,q-p);

    const char* k =strchr(q,'?');

    if(k) {
        *cgi_name = std::string(q,k-q);
    } else {
        *cgi_name = std::string(q);
    }

    return true;
}


//
// output buff to socket
//
void CallBackProxyOutput(CStrBuff* buff_obj, const char* buff, int32_t len) {
    buff_obj->AppendStr(buff, static_cast<uint32_t>(len));
}

//
// 如果callback为空则默认输出到屏幕
//
void OutputPrint(Closure<void, const char* , int32_t>* callback,
                 const char* fmt, ...) {
    char buff[2048];
    va_list args;
    va_start(args, fmt);
    int32_t bytes = _vsnprintf(static_cast<char*>(buff), sizeof(buff) - 1, fmt, args);
    buff[bytes] = 0;
    va_end(args);

    if (bytes != -1) {
        OutputWrite(callback, buff, bytes);

    } else {
        // vsnprintf error
    }
}

void OutputWrite(Closure<void, const char* , int32_t>* callback,
                 const char* buff, int32_t len) {
    if(callback) {
        // debug only
        LOG(INFO) << buff;
        callback->Run(buff, len);

    } else {
        fwrite(buff, 1, len, stdout);
        fflush(stdout);
    }
}

//
// proxy 实现
// cgi:         cgi参数解析对象
// is_post:     用户是否post方式提交请求到cgi
// output_cb:   proxy过程的输出回调,如果为空则输出到屏幕
// refer_proxy: 本次用户调用的CGI URI 
//
int32_t RealProxy(CParserCGIParameter* cgi, bool is_post,
                  Closure<void, const char* , int32_t>* output_cb,
                  const char* refer_proxy) {
    const char* real_server = NULL;
    const char* home_page = NULL;
    const char* proxy = NULL;
    const char* xml = NULL;
    const char* jason_callback = NULL;

    cgi->GetParameter("realserver", &real_server);
    cgi->GetParameter("home", &home_page);
    cgi->GetParameter("proxy", &proxy);
    cgi->GetParameter("xml", &xml);
    cgi->GetParameter("jasoncallback", &jason_callback);
    
    // 优先使用用户设定的proxy,如果用户没有设定proxy,则使用运行时候的proxy
    CHECK_NOTNULL(refer_proxy);
    if(!proxy) proxy = refer_proxy;    

    if (real_server == NULL) {
        // TODO: ...would delete after MasterV2
        cgi->GetParameter("?realserver", &real_server);

        // 办公网络不能直接访问IDC，需要通过该CGI访问，
        // real_server指向用户真正要访问的地址；
        // real_server不存在时跳转到xfs的主页面
        if (real_server == NULL) {
            // ca登录时，如果输入密码错误，会出现不能跳回登录界面的情况
            // 由OA的Error.aspx对url中含有两个?的情况解析失败导致
            // 强制跳回到xfs_entry主页面http://xfs.soso.oa.com/
            real_server = const_cast<char*>(DEFAULT_REAL_SERVER);
        }
    }

    std::string domain;
    std::string cgi_name;

    if ( false == ParseUrl(real_server, &domain, &cgi_name) ) {
        return -1;
    }

    // 所有的参数都放到param里面，后面在http_response中的GenerateRequest会进行处理
    // 如果是GET方式，会生成cgi_name+?+param的形式
    // 如果是POST方式，会将param放到http包的body中
    std::string str_param;

    // 向后面传递proxy
    str_param = string("proxy=") + string(proxy);    

    if (home_page) {
        str_param += string("&home=") + string(home_page);        

        // TODO home中不应该包含问号，暂时去掉，后面看看是否有问题
        //      while (param[len-1] == '?')    len--;
    }

    if (xml) {
        str_param += string("&xml=") + string(xml);
    }

    // 取剩余的其他keys
    int32_t keys = cgi->GetParameterCount();

    for (int32_t i = 0; i < keys; i++) {
        if (strcmp(cgi->GetParameterName(i), "randkey") != 0 &&
                strcmp(cgi->GetParameterName(i), "home") != 0 &&
                strcmp(cgi->GetParameterName(i), "realserver") != 0 &&
                strcmp(cgi->GetParameterName(i), "xml") != 0 &&
                strcmp(cgi->GetParameterName(i), "proxy") != 0) {
            str_param += string("&") + string(cgi->GetParameterName(i)) +
                         string("=") + string(cgi->GetParameterVal(i));
        }
    }

    // 如果是post,将url中?之后的其他参数放到Params中
    if (is_post) {
        const char* p = strstr(cgi_name.c_str(), "?");

        if (p) {
            str_param += string("&") + string(p + 1);
        }
    }

    // 将RealServer中?后面的参数追加到param中
    const char* p = strchr(real_server, '?');

    if(p) {
        str_param += string("&") + string(p + 1);
    }

    //超时时间，单位:秒
    const uint32_t timeout = 5;
    CGetHttpResponse http;
    HTTP_CGI_ERROR ret = http.GetResponse(domain.c_str(), cgi_name.c_str(),
                                          str_param.c_str(), is_post, timeout);

    //
    // TODO: (wookin)
    // 如果output_cb为空是在普通CGI里面printf输出,这时候是apache自动加上的HTTP/1.1 200 OK
    // 如果是自己程序输出cgi内容,则必须输出HTTP/1.1 200 OK
    //
    if(output_cb) {
        OutputPrint(output_cb, "HTTP/1.1 200 OK\r\n");
    }

    int32_t err_code = -1;

    // 输出其他头部内容
    if (ret != ERROR_HTTP_OK) { // 错误
        OutputPrint(output_cb, "content-type: text/html;charset=utf-8\r\n");
        char err_info[4096] = {0};
        int32_t valid_len = 0;

        if ((ret == ERROR_HTTP_CONNECT) || (ret == ERROR_HTTP_CONNECT_TIMEOUT)) {
            valid_len = safe_snprintf(err_info, sizeof(err_info),
                          "<html>"
                          "<link href='http://xfs.soso.oa.com/xfs/css/xfs_style.css'"
                          " type='text/css' rel='stylesheet'/>"                          
                          "<body>connect %s error\nreturn err:%s<br>\n"
                          "<br>Try get http response fail,you can visit the URL(without proxy):"
                          "<a href=\"%s\">%s</a></body></html>",
                          domain.c_str(), GetHttpErrString(ret),
                          real_server, real_server);
        } else {
            valid_len = safe_snprintf(err_info, sizeof(err_info),
                          "<html>"
                          "<link href='http://xfs.soso.oa.com/xfs/css/xfs_style.css'"
                          " type='text/css' rel='stylesheet'/>"                          
                          "<body>http.GetResponse(%s) %s return err:%s<br>\n"
                          "<br>realserver=%s<br>proxy=%s<br></body></html>",
                          cgi_name.c_str(), domain.c_str(), GetHttpErrString(ret),
                          real_server, proxy);        }

        // update content-length
        OutputPrint(output_cb, "content-length: %u\r\n\r\n", valid_len);
        OutputWrite(output_cb, err_info, valid_len);

        err_code = -1;

    } else { // 正确结果
        const char* filename = strrchr(real_server, '/');

        if (filename && strstr(filename, ".dat")
                && !strstr((const char*)http.GetHttpContent(), "as moved or not support")) {
            OutputPrint(output_cb, "content-type: application/force-download\r\n");
            OutputPrint(output_cb, "Content-Disposition:attachment; filename=%s\r\n",
                        filename + 1);

        } else if(filename && strstr(filename, ".xsl")) {
            OutputPrint(output_cb, "content-type: text/xsl; charset=utf-8\r\n");
        } else {
            if (xml && strcmp(xml, "true") == 0) {
                if (jason_callback)
                    OutputPrint(output_cb,
                                "content-type: application/javascript; charset=utf-8\r\n");
                else
                    OutputPrint(output_cb, "content-type: application/xml; charset=utf-8\r\n");

            } else {
                OutputPrint(output_cb, "content-type: text/html;charset=utf-8\r\n");
            }
        }

        // update content-length

        OutputPrint(output_cb, "content-length: %u\r\n\r\n", http.GetHttpContentLen());
        OutputWrite(output_cb, http.GetHttpContent(), http.GetHttpContentLen());

        err_code = 0;
    }

    LOG(INFO) << "RealProxy resulr: " << (err_code == 0 ? "OK" : "FAIL");
    return err_code;
}

_END_XFS_BASE_NAMESPACE_
