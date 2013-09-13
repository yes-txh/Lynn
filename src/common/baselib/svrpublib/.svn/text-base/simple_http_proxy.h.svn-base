// simple_http_proxy.h:
// wookin@tencent.com
// ////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_SIMPLE_HTTP_PROXY_H_
#define COMMON_BASELIB_SVRPUBLIB_SIMPLE_HTTP_PROXY_H_

#include "common/base/closure.h"

_START_XFS_BASE_NAMESPACE_

//
// output buff to socket
//
void CallBackProxyOutput(CStrBuff* buff_obj, const char* buff, int32_t len);

//
// 如果callback为空则默认输出到屏幕
//
void OutputPrint(Closure<void, const char* ,int32_t>* callback, const char* fmt, ...);
void OutputWrite(Closure<void, const char* ,int32_t>* callback, const char* buff, int32_t len);


//
// proxy 实现
// cgi:         cgi参数解析对象
// is_post:     用户是否post方式提交请求到cgi
// output_cb:   proxy过程的输出回调,如果为空则输出到屏幕
// refer_proxy: 本次用户调用的CGI URI 
//
int32_t RealProxy(CParserCGIParameter* cgi, bool is_post,
                  Closure<void, const char* ,int32_t>* output_cb,
                  const char* refer_proxy);


_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_SIMPLE_HTTP_PROXY_H_

