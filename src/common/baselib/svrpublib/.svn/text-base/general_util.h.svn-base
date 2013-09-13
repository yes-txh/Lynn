//  general_util.h
//  wookin
// ///////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_GENERAL_UTIL_H_
#define COMMON_BASELIB_SVRPUBLIB_GENERAL_UTIL_H_

#include "common/baselib/svrpublib/general_type_def.h"
#include "common/baselib/svrpublib/thread_mutex.h"

#include "common/baselib/svrpublib/auto_baselib.h"
#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

// //////////////////////////
// Public function
// //////////////////////////

#ifdef WIN32
#define bzero(x, y) memset((unsigned char*)x, 0, y)
#define lseeki64    _lseeki64
#define snprintf    _snprintf
int32_t gettimeofday(struct timeval* tv, void* zone);

#define strerror_r  strerror_s

//
// for lite_gettimeofday
//
inline uint64_t rdtsc() {
    __asm rdtsc
}

#else // linux
#define _vsnprintf      vsnprintf
#define stricmp         strcasecmp
#define lseeki64        lseek


// --- rdtsc ---
uint64_t rdtsc();

//
//  signal
//
void    XSigFunDefault(int32_t num_sig);
typedef void(Fun_SigAction)(int32_t);
void    XSetSigAction(int32_t sig, Fun_SigAction* fun);


inline uint64_t get_cpu_hz() {
    FILE *fp = NULL;
    char str[50] = {0};
    fp = popen("cat /proc/cpuinfo "
               "| grep 'cpu MHz' | sed -e 's/.*:[^0-9]//'", "r");
    if (fp == NULL) {
        printf("error\r\n");
        return 0;
    }

    fgets(str, 49, fp);
    pclose(fp);
    fp = NULL;

    // 得到的是Mhz
    double hz = atof(str);

    // 转换成hz
    hz *= 1000 * 1000;
    return (uint64_t)hz;
}
#endif // linux

int32_t safe_rand();

int32_t lite_gettimeofday(struct timeval* tv, void* zone);

// 这个时间是用rdtsc来实现的,只用来做相对时间的判定
// 不能用作绝对时间
int32_t fast_getrelativetimeofday(struct timeval* tv);

// 和time()相同,但是取得的是相对时间,可以用于绝对精度不高的场合,当时相对精度很高
time_t relativetime();

//
// SetAbsTimeout:当前时间 + 需要等待的时间(ms)
//
void SetAbsTimeout(struct timeval& curent_tm, uint64_t ms_time_wait);

uint64_t CalcTimeCost_MS(const struct timeval& start, const struct timeval& end);
uint64_t CalcTimeCost_US(const struct timeval& start, const struct timeval& end);

// 只精确到0.001s, milliseconds
bool IsReachedCheckTimePoint(const struct timeval* t_now, const struct timeval* t_check_point);

time_t lite_time(time_t *t);

void    IgnoreSig(void);
void    init_daemon(void);

// 线程安全localtime函数, 替换localtime()
void safe_localtime(const time_t* timep, struct tm* tm_result);

// 线程安全的ctime函数，替换ctime()
void safe_ctime(const time_t *time,
                size_t buffer_size,
                char* buffer);

//
// 返回成功copy到buff里面的长度(自动在字符串末尾添加\0, 不会超过buff)
// 返回长度不包含\0
//
// 传入长度不用-1, 直接为sizeof(buff)
//
//  char* src = "123";
//  char buf[3];
//  int32_t n = safe_snprintf(buf, sizeof(buf), "%s", src);
//  printf("n = %d, buf = %s\r\n", n, buf);
//
//  WIN32
//  n = 0, buf = 12烫烫烫烫烫01C
//
//  Linux
//  n = 2, buf = 12
//

int32_t safe_snprintf_impl(const char* file, int32_t line,
                           char* buff, int32_t len, const char* fmt, ...);

#define safe_snprintf(buff, len, ...)    safe_snprintf_impl(__FILE__, __LINE__, buff, len, __VA_ARGS__)


//  设置系统允许的进程所能打开的文件描述符的最大值
bool    SetFDLimit(uint32_t max_fds);

bool    SetCoreLimit();


void    XUSleep(uint32_t micro_seconds);
void    XSleep(uint32_t milliseconds);

#ifndef MAX
#define MAX(x, y)           ((x) > (y)) ? (x) : (y)
#define MIN(x, y)           ((x) < (y)) ? (x) : (y)
#endif // end define MAX,MIN

#define EXIST(x)            ((x) ? true : false)
#define MAKEUINT32(low, hi) ((uint32_t)(((uint16_t)(low)) |     \
                            ((uint32_t)((uint16_t)(hi))) << 16))

#define LOWUSHORT(l)        ((uint16_t)(l))
#define HIUSHORT(l)         ((uint16_t)(((uint32_t)(l) >> 16) & 0xFFFF))

#define MEGA_VALUE          (1024*1024)

// for const
#ifdef WIN32
#define LINUX_CONST
#else
#define LINUX_CONST const
#endif //

//  START:safety function define
#ifndef STRLEN
#define STRLEN(x)           (uint32_t)((x != NULL) ? strlen((x)) :0)
#endif // STRLEN

#define ATOI(x)                       ((x != NULL) ? atoi((x)) : 0)
#ifdef WIN32
#define ATOI64(x)                     ((x != NULL) ? _atoi64(x) :0)
#define FTELL64fp(x)                  _telli64(fileno(x))
#define FSEEK64fp(fp, offset, origin) _lseeki64(fileno(fp), offset, origin)
#define FTELL64(x)                    _telli64((x))
#define FSEEK64(fd, offset, origin)   _lseeki64(fd, offset, origin)
#else
#define ATOI64(x)                     ((x != NULL) ? atoll(x) :0)
#define FTELL64fp                     ftello
#define FTELL64(fhn)                  lseek(fhn, (off_t)0, SEEK_CUR)
#define FSEEK64fp                     fseeko
#define FSEEK64                       lseek
//
// 使用大文件的时候在Makefile中定义#define _FILE_OFFSET_BITS 64
//
#endif //

#define SAFE_FD_ISSET(fd, set)   ((fd == INVALID_SOCKET) ? \
                                                   false : \
                                         FD_ISSET(fd, set))

#define SAFE_FD_SET(fd, set)     { if (fd != INVALID_SOCKET) FD_SET(fd, set); }
//  END:safety function define
//

// _QTEXT
#ifdef WIN32
#define _QTEXT(exp)    _T(exp)
#else // linux
#define _QTEXT(exp)    (exp)
#endif // linux

#ifndef ARM
#ifdef WIN32
#ifndef QASSERT
#define QASSERT(exp) if (!(exp))                               \
            {                                                          \
                _asm { int 3 }                                         \
                {LOG(ERROR) << _QTEXT("ASSERT ERROR, 断言错误");}      \
            }
#endif // QASSERT
#else // linux
#ifndef QASSERT
#define QASSERT(exp) if (!(exp))                               \
            {                                                          \
                {LOG(ERROR) << _QTEXT("ASSERT ERROR, 断言错误");}      \
            }
#endif // QASSERT
#endif //
#else
#ifndef QASSERT
#define QASSERT(exp) (0)
#endif // QASSERT
#endif

#ifndef assert
#define assert(x) QASSERT(x)
#endif // assert

#define ABORT   abort()

// ////////////////////////////
// START:shmXXX() functions
// ////////////////////////////
#define IFLAGS (IPC_CREAT|IPC_EXCL)

#ifdef WIN32
typedef uint32_t key_t;

// Resource get request flags
#define IPC_CREAT  00001000   // Create if key is nonexistent
#define IPC_EXCL   00002000   // Fail if key exists
#define IPC_NOWAIT 00004000   // Return error on wait

key_t   ftok(const char *pathname, int32_t proj_id);
int32_t shmget(key_t key, int32_t size, int32_t shmflg);
void*   shmat(int32_t shmid, const void *shmaddr, int32_t shmflg);
int32_t shmdt(const void *shmaddr);

#endif //
// ////////////////////////////
// END:shmXXX() functions
// ////////////////////////////

//
// Thread local storage
//
#ifdef WIN32
#define TLS_GET_VAL  TlsGetValue
#define TLS_SET_VAL  TlsSetValue
#define TLS_KEY      DWORD32

//
// 以后可以不用那么麻烦每次TlsGetValue去取，只用TLS_VAR这个宏就可以了
// wookin 2011-3-25
//
#define TLS_VAR      __declspec(thread)

#else
#define TLS_GET_VAL  pthread_getspecific
#define TLS_SET_VAL  pthread_setspecific
#define TLS_KEY      pthread_key_t
#define TLS_VAR      __thread
#endif //

#ifndef WIN32
uint32_t   GetModuleFileName(void* module_handle, char* filename_buff,
                             uint32_t buff_size);
#endif //


// ----------------------------------------------------------------------------
// GUID
// ----------------------------------------------------------------------------
// gen uuid by random
void GetGUID(XGUID* guid);

// output guid info
void ShowGUIDInfo(const XGUID* guid, const char* desc = NULL);

//
// Integer hash
// Robert Jenkins' 32 bit integer hash function
// http://www.concentric.net/~Ttwang/tech/inthash.htm
//
inline uint32_t IntegerHash(uint32_t a) {
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23c) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09) ^ (a >> 16);
    return a;
}

#ifdef WIN32
typedef LONG         ATOM_INT;
#else // linux
typedef int32_t      ATOM_INT;

ATOM_INT InterlockedIncrement(ATOM_INT* addend);

ATOM_INT InterlockedDecrement(ATOM_INT* decend);

// Atomically add @i to @v
// __inline__ void atomic_add(int i, ATOM_T* v);
#endif //



int32_t GetTID();

int32_t GetPID();


//
// 定义LogParam
//
struct LogParam {
    FILE*          m_fp_logfile;
    uint32_t       m_num_logfile_id_count;
    char           m_log_module_name[MAX_PATH];

    // 是否第一次选择file id,
    // 如果是，且用户设置为append追加日志方
    // 式则需要寻找到第一个符合条件的日志id
    bool           m_select_file_id_first;

    //  输出缓冲区setvbuf
    char*           m_output_vbuff;

    LogParam() {
        m_output_vbuff = NULL;
        m_fp_logfile = 0;
        m_num_logfile_id_count = 0;
        memset(m_log_module_name, 0, sizeof(m_log_module_name));
        m_select_file_id_first = true;

        // 设置当前程序名为默认日志模块名
        // 默认在当前运行目录下生成日志
        GetModuleFileName(NULL, m_log_module_name, sizeof(m_log_module_name));
    }

    ~LogParam() {
        if (m_fp_logfile != NULL)
            fclose(m_fp_logfile);
        m_fp_logfile = 0;

        delete []m_output_vbuff;
        m_output_vbuff = 0;
    }
};

struct BinaryLogConfig {
    uint32_t m_binary_log_max_file_size;
    uint32_t m_binary_log_max_file_count;
    BinaryLogConfig();
};

struct LiteTime {
    //
    // Lite gettimeofday
    //
    struct timeval m_start_time;
    uint64_t volatile m_start_tsc;
    uint64_t m_cpu_hz;

    LiteTime() {
        memset(&m_start_time, 0, sizeof(m_start_time));
        m_start_tsc = 0;
        m_cpu_hz    = 0;

        //
        // Lite gettimeofday
        //
#ifndef WIN32
        gettimeofday(&m_start_time, NULL);
        m_cpu_hz    = get_cpu_hz();
        m_start_tsc = rdtsc();
#endif
    }
};

class CAutoLibGlobalVars {
public:
    CAutoLibGlobalVars();
    ~CAutoLibGlobalVars();

    void AddRefAcceptedSockFDCount();
    void AddRefNewTCPFDCount();
    void AddRefNewUDPFDCount();

    void AddRefCloseFDCount(bool is_tcp, TCP_FD_TYPE type, SOCKET hsock);

    //
    //  Lock for update data
    //

    CXThreadMutex     m_update_data_mutex;
    uint32_t volatile m_num_close_udp_fd_count;  // 统计close UDP socket次数
    uint32_t volatile m_num_new_udp_count;       // 统计new UDP socket次数

    uint32_t volatile m_num_new_tcp_count;       // 统计new TCP socket次数

    uint32_t volatile m_num_accepted_tcp_count;  // 统计new accept成功的
    // socket handle

    // 统计close TCP socket总次数
    uint32_t volatile m_num_close_tcp_socket_count;

    // 统计close(accept)
    // TCP socket次数
    uint32_t volatile m_num_close_accepted_tcp_count;

    uint32_t volatile m_num_close_new_tcp_count; // 统计close(new fd)
    // TCP socket次数

    // 统计未指明的类型(非new,
    // accepted, 或调用者未指定)
    uint32_t volatile m_num_close_unknown_tcp_count;

    //
    // 是否平均每个长连接对象接收相等的socket fd数目
    //
    CXThreadMutex     m_avg_accept_mutex;
    uint32_t volatile m_num_total_accept;        // 当前接收fd socket的最小数目

    // 当前长连接个数
    uint32_t volatile m_num_long_conn_object_count;

    //
    // 线程挂起相关
    //
    TLS_KEY           m_tls_thread_continue;
    
    BinaryLogConfig   m_binary_log_config;
    LiteTime          m_lite_time;

    //
    // Fake epoll
    //
    CXThreadMutex     m_fake_epoll_mutex;

    // 支持多次初始化
    ATOM_INT          m_num_init_base_lib_count;

    // save http module name
    char              m_http_module_name[128];
};

extern CAutoLibGlobalVars g_lib_vars;

//
// for CAutoLibMgr call
//
#define __AddRefAcceptedSockFDCount g_lib_vars.AddRefAcceptedSockFDCount
#define __AddRefCloseFDCount        g_lib_vars.AddRefCloseFDCount
#define __AddRefNewTCPFDCount       g_lib_vars.AddRefNewTCPFDCount
#define __AddRefNewUDPFDCount       g_lib_vars.AddRefNewUDPFDCount
#define __GetAutoLigMgrUpdateMutex  (&g_lib_vars.m_update_data_mutex)
#define __GetAutoLigMgrAcceptMutex  (&g_lib_vars.m_avg_accept_mutex)
#define __GetAutoLigMgrObj()        (g_lib_vars)

//
// Fake epoll
//
#define __GetFakeepollMutex()       g_lib_vars.m_fake_epoll_mutex

class CSpeedDbg {
public:
    void Routine();
    CSpeedDbg() {
        m_start_time = 0;
        m_count = 0;
        m_trigger_val = 100;
        memset(m_sz, 0, sizeof(m_sz));
    }

    void SetTrigegr(uint32_t trigger_val) {
        m_trigger_val = trigger_val;
    }

    void SetString(const char* psz) {
        safe_snprintf(m_sz, sizeof(m_sz), "%s", psz);
    }
private:
    char                m_sz[256];
    time_t volatile     m_start_time;
    uint32_t volatile   m_count;
    uint32_t volatile   m_trigger_val;
};

class CProcessRequestSpeed {
public:
    CProcessRequestSpeed() {
        m_num_accept = 0;
        lite_gettimeofday(&m_start_time, NULL);
    }

    ~CProcessRequestSpeed() {
        ProcessFinished();
    }

    void Reset() {
        m_num_accept = 0;
        lite_gettimeofday(&m_start_time, NULL);
    }

    void Add(int32_t nAccept) {
        if (m_num_accept == 0) {
            Reset();
        }

        m_num_accept += nAccept;

        if (m_num_accept%10000 == 0) {
            ProcessFinished();
        }
    }

    void ProcessFinished() const;
private:
    int32_t         m_num_accept;
    struct timeval  m_start_time;
};

class CStr {
public:
    CStr() {
        m_str = NULL;
    }

    ~CStr() {
        delete []m_str;
        m_str = NULL;
    }

    void SetStr(const char* pstr, uint32_t len) {
        delete []m_str;
        m_str = NULL;

        m_str = new char[len+1];
        memcpy(m_str, pstr, len);
        m_str[len] = 0;
    }

    void SetStr(const char* pstr) {
        SetStr(pstr, (uint32_t)strlen(pstr));
    }

    const char* Value() const {
        return m_str;
    }

    uint32_t GetValidLen() {
        return STRLEN(m_str);
    }

private:
    char*   m_str;
};

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_GENERAL_UTIL_H_
