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
// ���callbackΪ����Ĭ���������Ļ
//
void OutputPrint(Closure<void, const char* ,int32_t>* callback, const char* fmt, ...);
void OutputWrite(Closure<void, const char* ,int32_t>* callback, const char* buff, int32_t len);


//
// proxy ʵ��
// cgi:         cgi������������
// is_post:     �û��Ƿ�post��ʽ�ύ����cgi
// output_cb:   proxy���̵�����ص�,���Ϊ�����������Ļ
// refer_proxy: �����û����õ�CGI URI 
//
int32_t RealProxy(CParserCGIParameter* cgi, bool is_post,
                  Closure<void, const char* ,int32_t>* output_cb,
                  const char* refer_proxy);


_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_SIMPLE_HTTP_PROXY_H_

