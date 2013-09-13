// parser_proc.h

#ifndef COMMON_BASELIB_SVRPUBLIB_PARSE_PROC_H_
#define COMMON_BASELIB_SVRPUBLIB_PARSE_PROC_H_

#include "common/baselib/svrpublib/base_config.h"
#include <vector>
_START_XFS_BASE_NAMESPACE_


struct CpuRate {
    float user;
    float nice;
    float system;
    float idle;
    float iowait;
    float irq;
    float softirq;
};

struct CpuStatus {
    int32_t  id;      // cpu id
    uint64_t user;    // 用户态cpu占用
    uint64_t nice;    //
    uint64_t system;  // 系统cpu占用
    uint64_t idle;    // 空闲cpu
    uint64_t iowait;  // io等待时间
    uint64_t irq;     // 硬件中断
    uint64_t softirq; // 软件中断

    uint64_t total() const {
        return user + nice + system + idle + iowait + irq + softirq;
    }
    CpuStatus() {
        memset(this, 0, sizeof(CpuStatus));
    }
};

struct SystemMemoryStatus {
    uint32_t mem_total;     // 系统最大内存,单位 KB
    uint32_t mem_free;      // 系统空闲内存,单位 KB
    uint32_t mem_buffers;   //

    uint32_t swap_total;    //
    uint32_t swap_free;
    uint32_t swap_cached;   // cached
};

struct ProcessMemStatus {
    uint32_t pid;
    uint32_t fd_size;       // 最大文件描述符
    uint32_t vm_size;       // 总占用内存大小(包括虚拟内存),单位 KB
    uint32_t vm_rss;        // 物理内存占用大小,单位 KB
    uint32_t vm_data;       // 数据段占用内存大小,单位 KB
    uint32_t vm_lib;        // 加载的lib占用的内存大小,单位 KB
};

struct DiskStatus {
    char* df;
};

struct ProcessInfo {
    uint32_t inode;
    uint32_t pid;
    char     name[256];
};

struct NetSocketPairInfo {
    uint32_t inode;
    uint32_t local_addr, remote_addr;
    uint32_t tx_queue, rx_queue;
    uint16_t local_port, remote_port;
    unsigned char  status, protocol;
    uint32_t uid;
};

struct IOStat {
    std::string dev;// 设备编号 sda sdb...
    float rio;   // 每秒完成的读设备次数
    float wio;   // 每秒完成的写设备次数
    float rkb;   // 每秒完成的读设备流量(KB)
    float wkb;   // 每秒完成的写设备流量(KB)

    float argqu_sz; // 平均I/O队列长度
    float await; // 平均每次设备I/O操作的等待时间 (毫秒)
    float svctm; // 平均每次设备I/O操作的服务时间 (毫秒)
    float util;  // 一秒中有百分之多少的时间用于 I/O 操作
                 // 或者说一秒中有多少时间 I/O 队列是非空的。
};

struct NetStat {
    std::string dev; // 设备编号 eth0 ...

    uint64_t rbytes; // 网卡读出的字节数
    uint64_t wbytes; // 网卡写入的字节数

    uint64_t rpackets; // 网卡读出的包量
    uint64_t wpackets; // 网卡写入的包量
};

enum SocketProtocol {
    ENUM_PROTOCOL_TCP = 0x01,
    ENUM_PROTOCOL_UDP = 0x02,
    ENUM_PROTOCOL_TCP_LISTEN = 0x04,
};

#define CPU_LABLE "cpu"
#define MEM_TOTAL_LABLE "MemTotal"
#define MEM_FREE_LABLE "MemFree"
#define MEM_BUFF_LABLE "Buffers"
#define SWAP_TOTAL_LABLE "SwapTotal"
#define SWAP_FREE_LABLE "SwapFree"
#define SWAP_CACHE_LABLE "Cached"

#define FD_SIZE_LABLE "FDSize"
#define VM_SIZE_LABLE "VmSize"
#define VM_RSS_LABLE "VmRSS"
#define VM_DATA_LABLE "VmData"
#define VM_LIB_LABLE "VmLib"


class CParseProc {
#define MAX_CPU_NUM 128
#define DF_CMD_LINES "df -h"
public:

    CParseProc();
    ~CParseProc();

    //
    // 功能描述： 获取CPU状态信息
    // 输出参数： status     CPU信息
    // 返回值  ： true       成功
    //            false      失败
    //
    bool ParseCpu(std::vector<CpuStatus>* status);

    //
    // 功能描述： 计算CPU利用率（利用两次获取两次CPU状态之间的差值计算）
    // 输出参数： cpu_rate        存储CPU利用率的结构体指针
    // 输入参数： old_cpu_status  较旧时间 CPU状态
    // 输入参数： new_cpu_status  较新时间 CPU状态
    // 返回值  ： true            成功
    //            false           失败
    //
    bool CalcCpu(CpuRate* cpu_rate, CpuStatus* old_cpu_status,
                 CpuStatus* new_cpu_status);

    //
    // 功能描述： 获取系统内存状态信息
    // 输出参数： mem_status    存储系统内存信息的结构体指针
    // 返回值  ： true          成功
    //           false         失败
    //
    bool ParseSystemMemoryStatus(SystemMemoryStatus* mem_status);

    //
    // 功能描述： 获取进程内存状态信息()
    // 输出参数： status           内存信息
    // 输出参数： parse_threads    是否解析各个线程
    // 返回值  ： true                成功
    //            false               失败
    //
    bool ParseProcessMemoryStatus(std::vector<ProcessMemStatus>* status, bool parse_threads = true);

    //
    // 功能描述： 获取磁盘状态信息（df -h）
    // 返回值  ： df返回信息，失败为NULL
    //
    const char* ParseDiskStatus();

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
    bool ParseSocketPair(int32_t protocol, std::vector<NetSocketPairInfo>* socket_pair);

    //
    // 功能描述： 获取进程socket列表
    // 返回值  ：
    //
    bool ParseProcessInfo(std::vector<ProcessInfo>* process_info);

    const char* GetProtocol(uint32_t protocol_id);
    const char* GetNetDesc(uint32_t net_state_id);


    //
    // 功能描述： 获取所有磁盘的io信息
    // 返回值  ：
    //
    bool ParseDiskStat(std::vector<IOStat>* io_stat);

    //
    // 功能描述： 获取所有网卡的流量信息
    // 返回值  ：
    //
    bool ParseNetStat(std::vector<NetStat>* net_stat);

private:

    //
    // 功能描述： 获取进程内存状态
    //           分析/proc/pid/status 或 /proc/pid/task/status
    // 返回值  ：
    //
    bool ParseProcessStatus(const char* proc_file,
                            ProcessMemStatus* process_mem_status);

    //
    // 功能描述： 解析一行信息中的数字值
    // 返回值  ：
    //
    bool ParseMemInfoLine(char* buf_line, uint32_t* out_value);

    //
    // 功能描述： 通过进程id获取进程名
    // 返回值  ：
    //
    bool Pid2Name(uint32_t pid, char* name, uint32_t name_size);

    //
    // 功能描述: 获取系统socket对列表
    //           (与ProcessInfo 通过inode连接在一起)
    // 返回值  :
    //
    bool _ParseSocketPair(SocketProtocol protocol, std::vector<NetSocketPairInfo>* socket_pair);

private:

    char* m_disk_status_buffer;
    uint32_t m_disk_status_buffer_length;
};

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_PARSE_PROC_H_

