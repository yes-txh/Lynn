// --------------------------------------------------
// log.h
// wookin@tencent.com
// --------------------------------------------------
//
//   1:printf形式日志, LOG(optional:level, fmt, ...),
//                     Xprintf(fmt, ...),
//                     Xprintf_xxx(fmt, ...)
//   2:支持循环日志(默认NUM_MAX_LOG_FILES(50)个)
//   3:可以设置每个日志大小
//   4:日志文件名格式:<program name>.<hostname>.<user name>.
//                    <severity level:level-xxx>.IDxx<循环日志编号>.log
//                   文件名例:TestSvrpublib.exe.WOOKIN-NB.wookin.
//                            level-ALL.ID00.log
//
//   5:每行日志格式:[IWEFS]mmdd hh:mm:ss.uuuuuu threadid file:line> msg
//                  INFO, WARN, ERR, FATAL, STAT

#ifndef COMMON_BASELIB_SVRPUBLIB_LOG_H_
#define COMMON_BASELIB_SVRPUBLIB_LOG_H_

#include "common/baselib/svrpublib/general_util.h"

// 使用glog
#include "thirdparty/glog/logging.h"
#include "thirdparty/glog/raw_logging.h"
#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

#define DECLARE_USING_LOG_LEVEL_NAMESPACE using namespace google;

//
// 使用GOOGLE glog的时候有效, 设置默认输出路径
//
void InitGoogleDefaultLogParam(const char* program = NULL);

template<class T>
void Xprintf_MemDebug(const char* data, T len);

//
//  char* p = "hello wookin ^_^1234567\r\n"
//            "wahaha----------yes    中国银行 maybe";
//
template<class T>
void Xprintf_MemDebug(const char* data, T len) {
    if (data && len) {
        LOG(INFO) << "debug mem, start addr.:0x" << reinterpret_cast<const void*>(data) <<
                     " len = " << len;
        LOG(INFO) << "xx xx xx xx xx xx xx xx ******** memo:8 bytes/line.";
        LOG(INFO) << "-----------------------+--------";
        char left[256] = {0};
        char right[256] = {0};
        T u = 0;
        uint32_t l_left = 0;
        uint32_t l_right = 0;
        for (u = 0; u < len; u++) {
            char ch1[16] = {0};
            char ch2[16] = {0};
            int32_t n = safe_snprintf(ch1, sizeof(ch1), "%02x ", data[u]);
            ch1[n] = 0;
            // linux平台字符大于128的时候输出格式有问题，所以都输出.符号
            if (data[u] >= '!' && data[u] <= '~')
                n = safe_snprintf(ch2, sizeof(ch2), "%c", data[u]);
            else
                n = safe_snprintf(ch2, sizeof(ch2), ".");
            ch2[n] = 0;
            l_left += safe_snprintf(left + l_left,
                                    sizeof(left)-l_left, "%s", ch1);

            l_right += safe_snprintf(right + l_right,
                                     sizeof(right)-l_right, "%s", ch2);
            if (u && (u+1)%8 == 0) {
                left[l_left] = 0;
                right[l_right] = 0;

                LOG(INFO) << left << right;
                l_left = l_right = 0;
            }
        }

        if ((len)%8) {
            int32_t left_len = STRLEN(left);
            for (int32_t i2 = 0; i2 < 3*8-left_len; i2++)
                l_left += safe_snprintf(left + l_left,
                                        sizeof(left) - l_left, "%s", " ");
            LOG(INFO) << left << right;
        }
        LOG(INFO) << "-----------------------+--------";
    }
}


template<class T>
void   GetSelfThreadID(T id) {
#ifdef WIN32
    DWORD dw = GetCurrentThreadId();
    *id = (uint32_t)dw;
#else
    pthread_t t = pthread_self();
    *id = (uint32_t)t;
#endif //
}

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_LOG_H_
