//
//  general_util.cpp
//  wookin
// /////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/base_config.h"

#ifndef WIN32
#include <sys/syscall.h>
#endif // WIN32

_START_XFS_BASE_NAMESPACE_

#ifdef WIN32
#pragma   warning(disable:4127)
#endif // WIN32

void XSleep(uint32_t millisecs) {
    if (millisecs == 0)
        return;

#ifdef WIN32
    Sleep(millisecs);
#else
    XUSleep(millisecs * 1000);
#endif // WIN32
}

void    XUSleep(uint32_t microseconds) {
#ifdef WIN32
    Sleep(microseconds/1000);
#else // linux

    // 默认不使用sem_timedwait代替usleep，其中涉及要使用clock_gettime
#if 0
    sem_t h_sem;
    if (sem_init(&h_sem, 0, 0) != 0){
        LOG(ERROR) << "sem_init fail.";
        return;
    }

    struct timespec ts = {0};
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1){
        LOG(ERROR) << "clock_gettime fail: " << errno << ":" << strerror(errno);
        sem_destroy(&h_sem);
        return;
    }

    uint32_t sec = microseconds/1000000;
    ts.tv_sec += sec;

    uint32_t nano_sec = (microseconds%1000000) * 1000;

    // total nano secs
    nano_sec += ts.tv_nsec;
    ts.tv_nsec = nano_sec % 1000000000;
    ts.tv_sec += nano_sec / 1000000000;

    //
    // 不需要判断超时
    // int32_t ret = sem_timedwait(...);
    // if(ret == -1 && errno == ETIMEDOUT) ... timeout;
    //

    sem_timedwait(&h_sem, &ts);
    sem_destroy(&h_sem);

#else
    usleep(microseconds);
#endif

#endif // linux
}

#ifdef WIN32
void init_daemon(void) {
}
#else
void init_daemon(void) {
    daemon(1, 0);
}

// --- rdtsc ---
#if defined __i386__
uint64_t rdtsc() {
    uint32_t val;
    __asm__ __volatile__("rdtsc" : "=A" (val));
    return val;
}
#elif defined __x86_64__

uint64_t rdtsc() {
    uint32_t a, d;
    __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));
    return ((uint64_t)a) | (((uint64_t)d) << 32);
}
#endif
// --- rdtsc ---

void    XSigFunDefault(int32_t num_sig) {
    VLOG(3) << "in XSigFunDefault(), sig = " << num_sig;
}

void    XSetSigAction(int32_t sig, Fun_SigAction* fun) {
    struct sigaction action;
    bzero(&action, sizeof(action));
    sigemptyset(&action.sa_mask);

    if (fun)
        action.sa_handler = fun;
    else
        action.sa_handler = XSigFunDefault;

    action.sa_flags = 0;
    sigaction(sig, &action, 0);
}

uint32_t GetModuleFileName(void* module, char* filename, uint32_t buff_size) {
    const char* exe_name = "/proc/self/exe";
    int32_t len = buff_size;
    int32_t read_len = readlink(exe_name, filename, len);

    if (read_len == -1)
        return 0;
    else
        return read_len;
}
#endif // linux

void    IgnoreSig(void) {
#ifndef WIN32
    // signal(SIGPIPE, SIG_IGN);
    XSetSigAction(SIGPIPE, NULL);
#endif //
}

bool    SetFDLimit(uint32_t max_fds) {
#ifndef WIN32
    struct rlimit rlim = {0};
    struct rlimit rlim_new = {0};

    if (getrlimit(RLIMIT_NOFILE, &rlim) != 0)
        return false;

    if (rlim.rlim_cur >= max_fds)
        return true;

    if (rlim.rlim_max == RLIM_INFINITY || rlim.rlim_max >= max_fds) {
        rlim_new.rlim_max = rlim.rlim_max;
        rlim_new.rlim_cur = max_fds;
    } else {
        if (geteuid() != 0) {
            return false;
        }

        rlim_new.rlim_max = rlim_new.rlim_cur = max_fds;
    }

    if (setrlimit(RLIMIT_NOFILE, &rlim_new) != 0) {
        // failed. try raising just to the old max
        setrlimit(RLIMIT_NOFILE, &rlim);
        return false;
    }

#else // !WIN32
    max_fds = max_fds;
#endif
    return true;
}

bool    SetCoreLimit() {
#ifndef WIN32
    struct rlimit rlim = {0};
    rlim.rlim_cur = RLIM_INFINITY;
    rlim.rlim_max = RLIM_INFINITY;
    return setrlimit(RLIMIT_CORE, &rlim)  == 0;
#endif
    return true;
}

//
// 返回成功copy到buff里面的长度
// (自动在字符串末尾添加\0, 不会超过buff)
//
int32_t safe_snprintf_impl(const char* file, int32_t line,
                           char* buff, int32_t len, const char* fmt, ...){
    if (!buff)
        return 0;

    buff[0] = 0;

#ifdef WIN32

    va_list va;
    va_start(va, fmt);
    int32_t n = _vsnprintf(buff, len - 1, fmt, va);
    va_end(va);

    if (n < 0) {
        // 获取短文件名
        const char* name = strrchr(file, '/');
        if (!name) name = strrchr(file, '\\');
        if (name)
            name++;
        else
            name = file;

        LOG(FATAL) << "call at:[ " << name << ":" << line
                   << "], dest buff is too small, buffer len:" << len;
        return 0;
    } else {
        buff[n] = 0;
        return n;
    }

#else
    va_list va;
    va_start(va, fmt);
    int32_t n = _vsnprintf(buff, len, fmt, va);
    va_end(va);

    if (n >= len) {
        // 获取短文件名
        const char* name = strrchr(file, '/');
        if (!name) name = strrchr(file, '\\');
        if (name)
            name++;
        else
            name = file;

        LOG(FATAL) << "call at:[" << name << ":" << line
                   << "], dest buff is too small, buffer len:" << len
                   << ", required len:" << n + 1;
        // 多一位保存\0

        return len - 1;
    } else {
        buff[n] = 0; // 保险起见, 可能有些厂家_vsnprintf不补\0
        return n;
    }

#endif //
}

//
//  START:shmXXX() functions
//
#ifdef WIN32
key_t ftok(const char* pathname, int32_t proj_id) {
    uint32_t hash = (uint32_t)proj_id;
    uint32_t len = STRLEN(pathname);

    for (uint32_t u = 0; u < len; u++) {
        hash = 31 * hash + pathname[u];
    }

    return hash;
}

int32_t shmget(key_t key, int32_t size, int32_t shmflg) {
    char name[32] = {0};
    safe_snprintf(name, sizeof(name), "%08x", key);

    DWORD flag = 0;

    if (shmflg == IPC_CREAT)
        flag = PAGE_READWRITE;
    else
        flag = PAGE_READONLY;

    return (int32_t)CreateFileMapping((HANDLE(-1)), 0,
                                      flag, 0,
                                      size, name);
}

void*    shmat(int32_t shmid, const void* shmaddr, int32_t shmflg) {
    if (shmid == 0 || !shmaddr) {
        VLOG(3) << "shmat, invalid input parameter.";
    }

    uint32_t access_mode = (uint32_t)shmflg;

    if (shmflg == 0)
        access_mode = FILE_MAP_ALL_ACCESS;

    return MapViewOfFile((HANDLE)(shmid), access_mode, 0, 0, 0);
}

int32_t shmdt(const void* shmaddr) {
    if (!shmaddr) {
        VLOG(3) << "shmdt, invalid parameter.";
    }

    return 0;
}


int32_t gettimeofday(struct timeval* tv, void* zone) {
    if (zone) {
        LOG(WARNING) << "not support set time zone in this wrapper.";
    }

#define EPOCHFILETIME (116444736000000000ULL)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    uint64_t ft64  = (static_cast<uint64_t>(ft.dwHighDateTime) << 32)
                     + ft.dwLowDateTime;
    ft64 = ft64 - EPOCHFILETIME;
    ft64 = ft64 / 10;                      //  返回的是0.1 microsecond,
    //  这里得到转化为microseconds的结果

    tv->tv_sec = uint32_t(ft64 / 1000000); // 10microseconds
    tv->tv_usec = uint32_t(ft64 % 1000000);

    return 0;
}
#endif // WIN32
// END:shmXXX() functions


int32_t lite_gettimeofday(struct timeval* tv, void* zone) {
    // 使用rdtsc计算出来的时间和绝对时间之间存在误差，不能用来做绝对时间
    // 所以现在先直接调用gettimeofday的方法
    return gettimeofday(tv, static_cast<struct timezone*>(zone));    
}

int32_t fast_getrelativetimeofday(struct timeval* tv){
#ifdef WIN32
    gettimeofday(tv, NULL);
#else
    uint64_t now_tsc   = rdtsc();
    uint64_t duration_count = now_tsc - g_lib_vars.m_lite_time.m_start_tsc;
    uint32_t duration_sec   = duration_count / g_lib_vars.m_lite_time.m_cpu_hz;

    // less than a second, 0.xxxx s
    double in_a_sec =
        static_cast<double>(duration_count % g_lib_vars.m_lite_time.m_cpu_hz)
        / static_cast<double>(g_lib_vars.m_lite_time.m_cpu_hz);

    uint32_t duration_usec = static_cast<uint32_t>(in_a_sec * 1000000.0);

    tv->tv_sec  = g_lib_vars.m_lite_time.m_start_time.tv_sec  + duration_sec;
    tv->tv_usec = g_lib_vars.m_lite_time.m_start_time.tv_usec + duration_usec;

    if (tv->tv_usec >= 1000000) {
        tv->tv_sec += tv->tv_usec / 1000000;
        tv->tv_usec = tv->tv_usec % 1000000;
    }
#endif
    return 0;
}

time_t relativetime(){
    timeval t = {0};
    fast_getrelativetimeofday(&t);
    return static_cast<time_t>(t.tv_sec);
}

void SetAbsTimeout(struct timeval& curent_tm, uint64_t ms_time_wait) {
    static const int32_t kMicrosInOneMileSecond = 1000;
    static const int32_t kMicrosecInOneSecond = 1000000;

    curent_tm.tv_usec += static_cast<long>(ms_time_wait) * kMicrosInOneMileSecond;
    curent_tm.tv_sec += curent_tm.tv_usec / kMicrosecInOneSecond;
    curent_tm.tv_usec = curent_tm.tv_usec % kMicrosecInOneSecond;
}

uint64_t CalcTimeCost_MS(const struct timeval& start, const struct timeval& end) {
    int64_t time_cost =
        (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000;
    if (time_cost < 0) time_cost = 0;
    return time_cost;
}
uint64_t CalcTimeCost_US(const struct timeval& start, const struct timeval& end) {
    int64_t time_cost =
        (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
    if (time_cost < 0) time_cost = 0;
    return time_cost;
}

bool IsReachedCheckTimePoint(const struct timeval* t_now, const struct timeval* t_check_point) {
    return (t_now->tv_sec>t_check_point->tv_sec) ||
           (t_now->tv_sec == t_check_point->tv_sec && t_now->tv_usec >= t_check_point->tv_usec);
}


time_t lite_time(time_t* t) {
    struct timeval tv;

    lite_gettimeofday(&tv, NULL);

    if (t) {
        *t = tv.tv_sec;
    }

    return tv.tv_sec;
}

// gen uuid by random
void GetGUID(XGUID* guid) {
    static uint32_t volatile get_guid_count = 0;
    // 每个client尽量使用一个不同的种子
    if(get_guid_count == 0)
        get_guid_count = static_cast<uint32_t>(rdtsc() % 0xffffffff);    

    //
    // * We do this all the time, but this is the only source of
    // * randomness if /dev/random/urandom is out to lunch.
    //
    if (guid) {
        ++get_guid_count;

        // 前4 bytes强制使用唯一计数
        guid->data1 = get_guid_count;

        unsigned char* sz = reinterpret_cast<unsigned char*>(guid);
        uint32_t guid_len = sizeof(XGUID);        

        for (uint32_t u = 4; u < guid_len; ++u)
            sz[u] = ((safe_rand() >> 7) & 0xFF);
    }
}

// output guid info
void ShowGUIDInfo(const XGUID* guid, const char* desc) {
    char sz[128] = {0};
    safe_snprintf(sz, sizeof(sz), "%x-%x-%x-%02x%02x%02x%02x%02x%02x%02x%02x",
             guid->data1,guid->data2,guid->data3,
             guid->data4[0], guid->data4[1], guid->data4[2], guid->data4[3],
             guid->data4[4], guid->data4[5], guid->data4[6], guid->data4[7]);

    VLOG(3) << (desc ? desc : "") << " guid:" << sz;              
}

DECLARE_USING_LOG_LEVEL_NAMESPACE;


BinaryLogConfig::BinaryLogConfig() {
    m_binary_log_max_file_count = 10;
    m_binary_log_max_file_size  = 20 * MEGA_VALUE;
}

CAutoLibGlobalVars::CAutoLibGlobalVars() {
    // 系统中有些函数会用到随机数
    srand(static_cast<uint32_t>(time(0)));

    m_num_init_base_lib_count = 0;    

#if defined(_DEBUG) || defined(_DEBUG_COUNT)
    m_num_close_udp_fd_count = 0;         // 统计close UDP socket次数
    m_num_new_udp_count = 0;              // 统计new UDP socket次数

    m_num_new_tcp_count = 0;              // 统计new TCP socket次数
    m_num_accepted_tcp_count = 0;         // 统计new accept成功的socket handle

    m_num_close_accepted_tcp_count = 0;   // 统计close(accept) TCP socket次数
    m_num_close_new_tcp_count = 0;        // 统计close(new fd) TCP socket次数
    m_num_close_unknown_tcp_count = 0;    // 统计未指明的类型(非new, accepted,
    // 或调用者未指定)

    m_num_close_tcp_socket_count = 0;     // 统计close TCP socket总次数
#endif // _DEBUG || _DEBUG_COUNT

#if (defined(AVG_ACCEPT_TCP))
    m_num_total_accept = 0;               // 当前接收fd socket的最小数目
    m_num_long_conn_object_count = 0;     // 当前长连接个数
#endif // AVG_ACCEPT_TCP

    // Set rlimit
    SetFDLimit(10000000);

    // SetCoreLimit
    SetCoreLimit();

#ifdef WIN32
    m_tls_thread_continue = TlsAlloc();
#else
    pthread_key_create(&m_tls_thread_continue, 0);
#endif //

    m_http_module_name[0] = 0;
    safe_snprintf(m_http_module_name, sizeof(m_http_module_name),
                 "Unknown module, please set it");
}

CAutoLibGlobalVars::~CAutoLibGlobalVars() {
    // 如果ShuntDown()没有调用则需要关闭
    // 这里不能再输出LOG相关信息, 因为glog可能已经 shut down
    // XCloseLogFile();

#ifdef WIN32
    TlsFree(m_tls_thread_continue);
#else
    pthread_key_delete(m_tls_thread_continue);
#endif //
}

void CAutoLibGlobalVars::AddRefAcceptedSockFDCount() {
#if defined(_DEBUG) || defined(_DEBUG_COUNT)
    m_update_data_mutex.Lock();
    m_num_accepted_tcp_count++;
    m_update_data_mutex.UnLock();
#else
    // nothing
    (void)m_num_accepted_tcp_count;
#endif // _DEBUG || _DEBUG_COUNT
}

void CAutoLibGlobalVars::AddRefNewTCPFDCount() {
#if defined(_DEBUG) || defined(_DEBUG_COUNT)
    m_update_data_mutex.Lock();
    m_num_new_tcp_count++;
    m_update_data_mutex.UnLock();
#else
    // nothing
    (void)m_num_new_tcp_count;

#endif // _DEBUG || _DEBUG_COUNT
}

void CAutoLibGlobalVars::AddRefNewUDPFDCount() {
#if defined(_DEBUG) || defined(_DEBUG_COUNT)
    m_update_data_mutex.Lock();
    m_num_new_udp_count++;
    VLOG(3) << "***new udp fd socket count:" << m_num_new_udp_count;
    m_update_data_mutex.UnLock();
#else
    (void)m_num_new_udp_count;
#endif // _DEBUG || _DEBUG_COUNT
}


void CAutoLibGlobalVars::AddRefCloseFDCount(bool is_tcp,
        TCP_FD_TYPE type,
        SOCKET hsock) {
#if defined(_DEBUG) || defined(_DEBUG_COUNT)
    m_update_data_mutex.Lock();

    if (is_tcp) {
        m_num_close_tcp_socket_count++;

        if (type == TCP_FD_UNKNOWN)
            m_num_close_unknown_tcp_count++;

        if (type == TCP_FD_NEW)
            m_num_close_new_tcp_count++;

        if (type == TCP_FD_ACCEPTED)
            m_num_close_accepted_tcp_count++;

        VLOG(1) << "Close TCP sock:" << hsock << ";new:" <<
                  m_num_new_tcp_count << "+accept:" << m_num_accepted_tcp_count <<" = " <<
                  m_num_new_tcp_count + m_num_accepted_tcp_count << ";" <<
                  "closed:" << m_num_close_tcp_socket_count <<
                  "[new:" << m_num_close_new_tcp_count << "+accept:" <<
                  m_num_close_accepted_tcp_count << "+unknown:" <<
                  m_num_close_unknown_tcp_count << " = " <<
                  m_num_close_new_tcp_count + m_num_close_accepted_tcp_count +
                  m_num_close_unknown_tcp_count;
    } else {
        m_num_close_udp_fd_count++;
        VLOG(1) << "General close UDP FD count:" << m_num_close_udp_fd_count <<
                  ", new UDP fd:" << m_num_new_udp_count;
    }

    m_update_data_mutex.UnLock();
#else

    // nothing
    if (is_tcp || hsock || type) {}

#endif // _DEBUG || _DEBUG_COUNT
}

void CSpeedDbg::Routine() {
#ifdef _DEBUG

    if (m_start_time == 0)
        m_start_time = time(0);

    m_count++;

    if (m_count % m_trigger_val == 0) {
        uint32_t duration =
            static_cast<uint32_t>(time(0)) -
            static_cast<uint32_t>(m_start_time);

        float avg = static_cast<float>(m_count) /
                    static_cast<float>(duration);

        (void)avg; // warning: unused variable 'avg'
        VLOG(3) << m_sz << "avg:" << avg << ", total:" << m_count <<
                  ", in " << duration << " seconds";
    }

#else
    // nothing
    (void)m_count;
#endif // _DEBUG
}

void CProcessRequestSpeed::ProcessFinished() const {
#ifdef _DEBUG
    struct timeval end_time;
    lite_gettimeofday(&end_time, NULL);
    double t = (end_time.tv_sec - m_start_time.tv_sec) +
               (end_time.tv_usec - m_start_time.tv_usec) / 1000000;
    double  avg_speed = static_cast<double>(m_num_accept) / t;
    VLOG(3) << "total receive request " << m_num_accept;
    VLOG(3) << "process speed is " << avg_speed << "/ms.";
#endif // _DEBUG
}

// for safe_rand(),线程安全rand
TLS_VAR uint32_t  g_tls_safe_rand_seed = 0;

int32_t safe_rand() {
    if (g_tls_safe_rand_seed == 0) {        
        g_tls_safe_rand_seed = static_cast<uint32_t>(rdtsc() % 0xFFFFFFFF);
        srand(g_tls_safe_rand_seed);
    }

#ifdef WIN32
    return rand();
#else
    return rand_r(&g_tls_safe_rand_seed);
#endif // linux
}


// 线程安全localtime函数, 替换localtime()
void safe_localtime(const time_t* timep, struct tm* tm_result) {
    if (!tm_result || !timep)
        return;

#ifdef WIN32
    localtime_s(tm_result, timep);
#else
    localtime_r(timep, tm_result);
#endif
}

// 线程安全的ctime函数，替换ctime()
void safe_ctime(const time_t *time, size_t buffer_size, char* buffer) {
    if (!buffer || !time) {
        VLOG(3) << "invalid input parameter";
        return;
    }
#ifdef WIN32
    ctime_s(buffer, buffer_size, time);
#else
    ctime_r(time, buffer);
#endif
}

CXThreadMutex g_inter_lock_mutex;

ATOM_INT InterlockedIncrement(ATOM_INT* addend) {
    CXThreadAutoLock auto_lock(&g_inter_lock_mutex);
    ++(*addend);
    return (*addend);
}

ATOM_INT InterlockedDecrement(ATOM_INT* decend) {
    CXThreadAutoLock auto_lock(&g_inter_lock_mutex);
    --(*decend);
    return (*decend);
}


#ifndef WIN32
// tls, current thread id
__thread pid_t g_current_tid = 0;
#endif // linux


int32_t GetTID() {
#ifdef WIN32
    return GetCurrentThreadId();
#else
    // On Linux and FreeBSD, we try to use gettid().
    if(g_current_tid == 0)
        g_current_tid = syscall(SYS_gettid);
    return g_current_tid;
#endif // linux
}

int32_t GetPID(){
#ifdef WIN32
    return _getpid();
#else
    return getpid();
#endif
}

_END_XFS_BASE_NAMESPACE_
