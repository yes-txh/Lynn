// parser_proc.cpp
// ttyyang

#ifndef WIN32
#include <dirent.h>
#include <stdio.h>
#endif // linux

#include "common/baselib/svrpublib/server_publib.h"

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

#ifndef STRNCASECMP
#ifdef WIN32
#define STRNCASECMP _strnicmp
#else
#define STRNCASECMP strncasecmp
#endif
#endif

const char* g_parse_proc_proto[] = {
    "tcp",
    "udp",
};

const char* g_parse_proc_states[] = {
    "ESTBLSH",
    "SYNSENT",
    "SYNRECV",
    "FWAIT1",
    "FWAIT2",
    "TMEWAIT",
    "CLOSED",
    "CLSWAIT",
    "LASTACK",
    "LISTEN",
    "CLOSING",
    "UNKNOWN" ,
    ""
};

const char* CParseProc::GetProtocol(uint32_t protocol_id) {
    if (protocol_id > 1) {
        return "";
    }

    return g_parse_proc_proto[protocol_id];
}
const char* CParseProc::GetNetDesc(uint32_t net_state_id) {
    if (net_state_id > 12) {
        return "";
    }

    return g_parse_proc_states[net_state_id];
}

CParseProc::CParseProc() {
    m_disk_status_buffer_length = 10240;
    m_disk_status_buffer = new char[m_disk_status_buffer_length];
}

CParseProc::~CParseProc() {
    delete [] m_disk_status_buffer;
    m_disk_status_buffer = NULL;
}

//
// 功能描述： 获取CPU状态信息
// 输出参数： status     CPU信息
// 返回值  ： true       成功
//            false      失败
//
bool CParseProc::ParseCpu(std::vector<CpuStatus>* status) {

    FILE* fp = fopen("/proc/stat", "r");

    if (fp == NULL) {
        return false;
    }

    char buf[1024];

    // user nice system idle iowait irq softirq
    // CPU time = user+system+nice+idle+iowait+irq+softirq
    while (fgets(buf, sizeof(buf), fp)) {
        CpuStatus cpu;

        if (STRNCASECMP(CPU_LABLE, buf, strlen(CPU_LABLE))  == 0) {
            char* p = buf + strlen(CPU_LABLE);

            if (!isdigit(*p)) {
                continue;
            }

            cpu.id = atoi(p);

            while ( isdigit(*p)) {
                p++;
            }

            while ( !isdigit(*p)) {
                p++;
            }

            int32_t ret = sscanf(p,
                                 "%"FMTu64" %"FMTu64" %"FMTu64
                                 " %"FMTu64" %"FMTu64" %"FMTu64" %"FMTu64 "%*d\n",
                                 &cpu.user, &cpu.nice, &cpu.system,
                                 &cpu.idle, &cpu.iowait, &cpu.irq, &cpu.softirq);

            if (ret == 7) {
                status->push_back(cpu);
            }
        }
    }

    fclose(fp);
    return true;
}

bool CParseProc::CalcCpu(CpuRate* cpu_rate, CpuStatus* old_cpu_status,
                         CpuStatus* new_cpu_status) {
    if (!cpu_rate || !old_cpu_status || !new_cpu_status) {
        return false;
    }

    uint64_t frme = new_cpu_status->total() - old_cpu_status->total();

    if ( frme == 0) {
        frme = 1;
    }

    float scale = static_cast<float>(100.0) / static_cast<float>(frme);
    cpu_rate->idle = static_cast<float>(new_cpu_status->idle - old_cpu_status->idle) * scale;
    cpu_rate->user = static_cast<float>(new_cpu_status->user - old_cpu_status->user) * scale;
    cpu_rate->nice = static_cast<float>(new_cpu_status->nice - old_cpu_status->nice) * scale;
    cpu_rate->system = static_cast<float>(new_cpu_status->system - old_cpu_status->system) * scale;
    cpu_rate->iowait = static_cast<float>(new_cpu_status->iowait - old_cpu_status->iowait) * scale;
    cpu_rate->irq = static_cast<float>(new_cpu_status->irq - old_cpu_status->irq) * scale;
    cpu_rate->softirq = static_cast<float>(
                            new_cpu_status->softirq - old_cpu_status->softirq) * scale;
    return true;
}

bool CParseProc::ParseSystemMemoryStatus(SystemMemoryStatus* mem_status) {
    if (!mem_status) {
        return false;
    }

    FILE* fp = fopen("/proc/meminfo", "r");

    if (!fp) {
        return false;
    }

    char buf[1024];
    int32_t counter  = 0;

    // user nice system idle iowait irq softirq
    // CPU time = user+system+nice+idle+iowait+irq+softirq
    while (fgets(buf, sizeof(buf), fp)) {
        if ( STRNCASECMP(MEM_TOTAL_LABLE, buf,
                         sizeof(MEM_TOTAL_LABLE) - 1) == 0 ) {
            if ( ParseMemInfoLine(buf, &mem_status->mem_total)) {
                counter++;
            }

        } else if ( STRNCASECMP(MEM_FREE_LABLE, buf,
                                sizeof(MEM_FREE_LABLE) - 1) == 0 ) {
            if ( ParseMemInfoLine(buf, &mem_status->mem_free)) {
                counter++;
            }

        } else if ( STRNCASECMP(MEM_BUFF_LABLE, buf,
                                sizeof(MEM_BUFF_LABLE) - 1) == 0 ) {
            if ( ParseMemInfoLine(buf, &mem_status->mem_buffers)) {
                counter++;
            }

        } else if ( STRNCASECMP(SWAP_TOTAL_LABLE, buf,
                                sizeof(SWAP_TOTAL_LABLE) - 1) == 0 ) {
            if ( ParseMemInfoLine(buf, &mem_status->swap_total)) {
                counter++;
            }

        } else if ( STRNCASECMP(SWAP_FREE_LABLE, buf,
                                sizeof(SWAP_FREE_LABLE) - 1) == 0 ) {
            if ( ParseMemInfoLine(buf, &mem_status->swap_free)) {
                counter++;
            }

        } else if ( STRNCASECMP(SWAP_CACHE_LABLE, buf,
                                sizeof(SWAP_CACHE_LABLE) - 1) == 0 ) {
            if ( ParseMemInfoLine(buf, &mem_status->swap_cached)) {
                counter++;
            }
        }

        if (counter == 6) {
            break;
        }
    }

    fclose(fp);

    return counter == 6;
}

bool CParseProc::ParseMemInfoLine(char* buf_line, uint32_t* out_value) {
    char* p = buf_line;

    while ( *p && !isdigit(*p)) {
        p++;
    }

    if (isdigit(*p)) {
        *out_value = atoi(p);
        return true;
    }

    return false;
}

const char* CParseProc::ParseDiskStatus() {
    memset(m_disk_status_buffer, 0, m_disk_status_buffer_length);
#ifndef WIN32
    FILE* fp = popen(DF_CMD_LINES, "r");

    if (fp == NULL) {
        return NULL;
    }

    fread(m_disk_status_buffer, m_disk_status_buffer_length, 1, fp);
    pclose(fp);

#endif

    return m_disk_status_buffer;
}


bool CParseProc::ParseProcessStatus(const char* proc_file,
                                    ProcessMemStatus* process_mem_status) {
    if (!proc_file || !process_mem_status) {
        return false;
    }

    FILE* fp = fopen(proc_file, "r");

    if (!fp) {
        return false;
    }

    uint32_t counter = 0;
    bool    b = false;

    char buf[1024];

    while (fgets(buf, sizeof(buf), fp)) {
        if ( STRNCASECMP(FD_SIZE_LABLE, buf,
                         sizeof(FD_SIZE_LABLE) - 1) == 0 ) {
            if ( ParseMemInfoLine(buf, &process_mem_status->fd_size)) {
                counter++;
            }

        } else if ( STRNCASECMP(VM_SIZE_LABLE, buf,
                                sizeof(VM_SIZE_LABLE) - 1) == 0 ) {
            if ( ParseMemInfoLine(buf, &process_mem_status->vm_size)) {
                counter++;
            }

        } else if ( STRNCASECMP(VM_DATA_LABLE, buf,
                                sizeof(VM_DATA_LABLE) - 1) == 0 ) {
            if ( ParseMemInfoLine(buf, &process_mem_status->vm_data)) {
                counter++;
            }

        } else if ( STRNCASECMP(VM_LIB_LABLE, buf,
                                sizeof(VM_LIB_LABLE) - 1) == 0 ) {
            if ( ParseMemInfoLine(buf, &process_mem_status->vm_lib)) {
                counter++;
            }

        } else if ( STRNCASECMP(VM_RSS_LABLE, buf,
                                sizeof(VM_RSS_LABLE) - 1) == 0 ) {
            if ( ParseMemInfoLine(buf, &process_mem_status->vm_rss)) {
                counter++;
            }
        }

        if (counter == 5) {
            b = true;
            break;
        }
    }

    fclose(fp);

    return b;
}

//
// 功能描述： 获取进程内存状态信息()
// 输出参数： status           内存信息
// 输出参数： parse_threads    是否解析各个线程
// 返回值  ： true                成功
//            false               失败
//
bool CParseProc::ParseProcessMemoryStatus(std::vector<ProcessMemStatus>* status, bool parse_threads) {
#ifndef WIN32
    char proc_file[256] = {0};
    safe_snprintf(proc_file, sizeof(proc_file), "/proc/%d/status", getpid());

    ProcessMemStatus pst;
    pst.pid = getpid();

    // 解析进程
    if (false == ParseProcessStatus(proc_file, &pst)) {
        return false;
    }

    status->push_back(pst);

    // 解析线程
    if(parse_threads) {
        char buf_dir[256];
        safe_snprintf(buf_dir, sizeof(buf_dir),
                      "/proc/%d/task/", getpid());

        struct dirent entry;
        struct dirent* dirp = NULL;
        DIR* dp;

        if ((dp = opendir(buf_dir)) == NULL) {
            return false;
        }

        readdir_r(dp, &entry, &dirp);

        while (dirp != NULL) {
            // 处理找到的文件
            if (0 == strcmp(dirp->d_name, ".") ||
                    0 == strcmp(dirp->d_name, "..")) {
            } else if (DT_DIR == dirp->d_type) {
                pst.pid = atoi(dirp->d_name);
                safe_snprintf(buf_dir, sizeof(buf_dir),
                              "/proc/%d/task/%s", getpid(), dirp->d_name);


                if (ParseProcessStatus(proc_file, &pst)) {
                    status->push_back(pst);
                }
            }

            readdir_r(dp, &entry, &dirp);
        }

        closedir(dp);
    }

#endif
    return true;
}

bool CParseProc::Pid2Name(uint32_t pid, char* name, uint32_t name_size) {
    char buf[512] = {0};
    safe_snprintf(buf, sizeof(buf), "/proc/%d/cmdline", pid);

    FILE* fp = fopen(buf, "r");

    if (fp) {
        char buf_tmp[256] = {0};

        if ( fgets(buf, sizeof(buf), fp) &&
                sscanf(buf, "%s\n", buf_tmp) == 1) {
            char* p = strrchr(buf_tmp, '/');

            if (!p) {
                p = buf_tmp;

            } else {
                p++;
            }

            safe_snprintf(name, name_size, "%s", p);
        }

        fclose(fp);
        return true;

    } else {
        safe_snprintf(name, name_size, "%s", "---");
        return false;
    }
}

//
// 功能描述： 获取进程socket列表
// 返回值  ：
//
bool CParseProc::ParseProcessInfo(std::vector<ProcessInfo>* process_info) {
#ifndef WIN32
    DIR* fd_dir = opendir("/proc");

    if (fd_dir == NULL) {
        return false;
    }

    //    struct dirent *procent, *fdent;
    char buf_dir[512];
    char proc_file[512];
    struct dirent entry;
    struct dirent* procent = NULL;
    struct dirent* fdent = NULL;
    readdir_r(fd_dir, &entry, &procent);

    while ((procent = readdir(fd_dir)) != NULL) {
        if (!isdigit(*(procent->d_name))) {
            continue;
        }

        safe_snprintf(buf_dir, sizeof(buf_dir),
                      "/proc/%s/fd/", procent->d_name);

        DIR* fd = opendir(buf_dir);

        if (!fd) {
            continue;
        }

        while ((fdent = readdir(fd)) != NULL) {
            struct stat st;
            safe_snprintf(proc_file, sizeof(proc_file),
                          "/proc/%s/fd/%s", procent->d_name, fdent->d_name);

            if (stat(proc_file, &st) < 0) {
                continue;
            }

            if (!S_ISSOCK(st.st_mode)) {
                continue;
            }

            ProcessInfo pst;

            pst.pid = atoi(procent->d_name);
            pst.inode = st.st_ino;
            Pid2Name(pst.pid, pst.name, sizeof(pst.name));
            process_info->push_back(pst);
        }

        closedir(fd);
    }

    closedir(fd_dir);

#endif
    return true;
}

//
// 功能描述： 获取本进程网络连接列表
// 输入参数： protocol         网络协议, ENUM_PROTOCOL_TCP_LISTEN
//                                       ENUM_PROTOCOL_TCP
//                                       ENUM_PROTOCOL_UDP
//
// 输出参数： socket_pair      socketpair信息
// 返回值  ： true             成功
//            false            失败
//
bool CParseProc::_ParseSocketPair(SocketProtocol protocol, std::vector<NetSocketPairInfo>* socket_pair) {
    const char* file_name = NULL;

    if (protocol == ENUM_PROTOCOL_TCP_LISTEN) {
        file_name = "/proc/net/tcp_listen";

    } else if (protocol == ENUM_PROTOCOL_TCP) {
        file_name = "/proc/net/tcp";

    } else if (protocol == ENUM_PROTOCOL_UDP) {
        file_name = "/proc/net/udp";
    }

    FILE* fp = fopen(file_name, "r");

    if (fp == NULL) {
        if( protocol == ENUM_PROTOCOL_TCP_LISTEN) {
            return false;
        }

    }

    char buf[1024];

    while (fgets(buf, 1024, fp)) {

        NetSocketPairInfo net;
        memset(&net, 0, sizeof(net));
        int32_t ret = sscanf(buf,
                             "%*u: %X:%hX %X:%hX %hhx %X:%X %*x:%*X %*x %u %*u %u",
                             &net.local_addr,
                             &net.local_port,
                             &net.remote_addr,
                             &net.remote_port,
                             &net.status,
                             &net.tx_queue,
                             &net.rx_queue,
                             &net.uid,
                             &net.inode);

        if (ret  == 9) {
            net.protocol = (unsigned char)protocol;
            socket_pair->push_back(net);
        }
    }

    fclose(fp);

    return true;
}

bool CParseProc::ParseSocketPair(int32_t protocol, std::vector<NetSocketPairInfo>* socket_pair) {

    bool b_ret = false;

    // TCP包含Listen信息
    if ( ENUM_PROTOCOL_TCP & protocol) {
        b_ret = _ParseSocketPair(ENUM_PROTOCOL_TCP, socket_pair);

    } else if ( ENUM_PROTOCOL_TCP_LISTEN & protocol) {
        b_ret = _ParseSocketPair(ENUM_PROTOCOL_TCP_LISTEN, socket_pair);
    }

    // UDP
    if ( ENUM_PROTOCOL_UDP & protocol) {
        b_ret =  _ParseSocketPair(ENUM_PROTOCOL_UDP, socket_pair);
    }

    return b_ret;
}

bool CParseProc::ParseDiskStat(std::vector<IOStat>* io_stat) {
#ifndef WIN32
    FILE* fp = popen("iostat -x", "r");

    if (fp == NULL) {
        return false;
    }

    char dev;
    float argrq_sz; // 平均每次设备I/O操作的数据大小
    float rmerge; // 每秒进行 merge 的读操作数目
    float wmerge; // 每秒进行 merge 的写操作数目
    // 如果svctm 比较接近await，说明I/O 几乎没有等待时间；
    // 如果await 远大于svctm，说明I/O队列太长，应用得到的响应时间变慢

    char buf[1024];

    while (fgets(buf, 1024, fp)) {
        IOStat io;

        int ret = sscanf(buf, "sd%c%*[' ']%f%*[' ']%f%*[' ']%f%*[' ']%f%*[' ']%*f%*[' ']%*f"
                         "%*[' ']%f%*[' ']%f%*[' ']%f%*[' ']%f%*[' ']%f%*[' ']%f%*[' ']%f",
                         &dev, &rmerge, &wmerge, &io.rio, &io.wio,
                         &io.rkb, &io.wkb, &argrq_sz, &io.argqu_sz, &io.await, &io.svctm, &io.util);

        if( ret == 12) {
            sprintf(buf, "sd%c", dev);
            io.dev = std::string(buf);
            io_stat->push_back(io);
        }
    }

    pclose(fp);
#endif
    return io_stat->size() > 0;
}

bool CParseProc::ParseNetStat(std::vector<NetStat>* net_stat) {

    FILE* fp = fopen("/proc/net/dev", "r");

    if (fp == NULL) {
        return false;
    }

    uint64_t temp;

    char buf[1024];

    while (fgets(buf, 1024, fp)) {
        NetStat net;
        int32_t id;

        int ret = sscanf(buf, "%*[' ']eth%d:%"FMTu64"%*[' ']%"FMTu64"%*[' ']%"FMTu64"%*[' ']%"FMTu64"%*[' ']%"FMTu64
                         "%*[' ']%"FMTu64"%*[' ']%"FMTu64"%*[' ']%"FMTu64"%*[' ']%"FMTu64"%*[' ']%"FMTu64"%*s",
                         &id, &net.rbytes, &net.rpackets, &temp, &temp, &temp, &temp, &temp, &temp,
                         &net.wbytes, &net.wpackets);

        if( ret == 11) {
            sprintf(buf, "eth%d", id);
            net.dev = std::string(buf);
            net_stat->push_back(net);
        }
    }

    fclose(fp);

    return net_stat->size() > 0;
}

_END_XFS_BASE_NAMESPACE_
