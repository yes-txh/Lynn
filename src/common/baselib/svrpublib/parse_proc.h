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
    uint64_t user;    // �û�̬cpuռ��
    uint64_t nice;    //
    uint64_t system;  // ϵͳcpuռ��
    uint64_t idle;    // ����cpu
    uint64_t iowait;  // io�ȴ�ʱ��
    uint64_t irq;     // Ӳ���ж�
    uint64_t softirq; // ����ж�

    uint64_t total() const {
        return user + nice + system + idle + iowait + irq + softirq;
    }
    CpuStatus() {
        memset(this, 0, sizeof(CpuStatus));
    }
};

struct SystemMemoryStatus {
    uint32_t mem_total;     // ϵͳ����ڴ�,��λ KB
    uint32_t mem_free;      // ϵͳ�����ڴ�,��λ KB
    uint32_t mem_buffers;   //

    uint32_t swap_total;    //
    uint32_t swap_free;
    uint32_t swap_cached;   // cached
};

struct ProcessMemStatus {
    uint32_t pid;
    uint32_t fd_size;       // ����ļ�������
    uint32_t vm_size;       // ��ռ���ڴ��С(���������ڴ�),��λ KB
    uint32_t vm_rss;        // �����ڴ�ռ�ô�С,��λ KB
    uint32_t vm_data;       // ���ݶ�ռ���ڴ��С,��λ KB
    uint32_t vm_lib;        // ���ص�libռ�õ��ڴ��С,��λ KB
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
    std::string dev;// �豸��� sda sdb...
    float rio;   // ÿ����ɵĶ��豸����
    float wio;   // ÿ����ɵ�д�豸����
    float rkb;   // ÿ����ɵĶ��豸����(KB)
    float wkb;   // ÿ����ɵ�д�豸����(KB)

    float argqu_sz; // ƽ��I/O���г���
    float await; // ƽ��ÿ���豸I/O�����ĵȴ�ʱ�� (����)
    float svctm; // ƽ��ÿ���豸I/O�����ķ���ʱ�� (����)
    float util;  // һ�����аٷ�֮���ٵ�ʱ������ I/O ����
                 // ����˵һ�����ж���ʱ�� I/O �����Ƿǿյġ�
};

struct NetStat {
    std::string dev; // �豸��� eth0 ...

    uint64_t rbytes; // �����������ֽ���
    uint64_t wbytes; // ����д����ֽ���

    uint64_t rpackets; // ���������İ���
    uint64_t wpackets; // ����д��İ���
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
    // ���������� ��ȡCPU״̬��Ϣ
    // ��������� status     CPU��Ϣ
    // ����ֵ  �� true       �ɹ�
    //            false      ʧ��
    //
    bool ParseCpu(std::vector<CpuStatus>* status);

    //
    // ���������� ����CPU�����ʣ��������λ�ȡ����CPU״̬֮��Ĳ�ֵ���㣩
    // ��������� cpu_rate        �洢CPU�����ʵĽṹ��ָ��
    // ��������� old_cpu_status  �Ͼ�ʱ�� CPU״̬
    // ��������� new_cpu_status  ����ʱ�� CPU״̬
    // ����ֵ  �� true            �ɹ�
    //            false           ʧ��
    //
    bool CalcCpu(CpuRate* cpu_rate, CpuStatus* old_cpu_status,
                 CpuStatus* new_cpu_status);

    //
    // ���������� ��ȡϵͳ�ڴ�״̬��Ϣ
    // ��������� mem_status    �洢ϵͳ�ڴ���Ϣ�Ľṹ��ָ��
    // ����ֵ  �� true          �ɹ�
    //           false         ʧ��
    //
    bool ParseSystemMemoryStatus(SystemMemoryStatus* mem_status);

    //
    // ���������� ��ȡ�����ڴ�״̬��Ϣ()
    // ��������� status           �ڴ���Ϣ
    // ��������� parse_threads    �Ƿ���������߳�
    // ����ֵ  �� true                �ɹ�
    //            false               ʧ��
    //
    bool ParseProcessMemoryStatus(std::vector<ProcessMemStatus>* status, bool parse_threads = true);

    //
    // ���������� ��ȡ����״̬��Ϣ��df -h��
    // ����ֵ  �� df������Ϣ��ʧ��ΪNULL
    //
    const char* ParseDiskStatus();

    //
    // ���������� ��ȡ���������������б�
    // ��������� protocol         ����Э��, ENUM_PROTOCOL_TCP_LISTEN
    //                                       ENUM_PROTOCOL_TCP
    //                                       ENUM_PROTOCOL_UDP
    //
    // ��������� socket_pair      socketpair��Ϣ
    // ����ֵ  �� true             �ɹ�
    //            false            ʧ��
    //
    bool ParseSocketPair(int32_t protocol, std::vector<NetSocketPairInfo>* socket_pair);

    //
    // ���������� ��ȡ����socket�б�
    // ����ֵ  ��
    //
    bool ParseProcessInfo(std::vector<ProcessInfo>* process_info);

    const char* GetProtocol(uint32_t protocol_id);
    const char* GetNetDesc(uint32_t net_state_id);


    //
    // ���������� ��ȡ���д��̵�io��Ϣ
    // ����ֵ  ��
    //
    bool ParseDiskStat(std::vector<IOStat>* io_stat);

    //
    // ���������� ��ȡ����������������Ϣ
    // ����ֵ  ��
    //
    bool ParseNetStat(std::vector<NetStat>* net_stat);

private:

    //
    // ���������� ��ȡ�����ڴ�״̬
    //           ����/proc/pid/status �� /proc/pid/task/status
    // ����ֵ  ��
    //
    bool ParseProcessStatus(const char* proc_file,
                            ProcessMemStatus* process_mem_status);

    //
    // ���������� ����һ����Ϣ�е�����ֵ
    // ����ֵ  ��
    //
    bool ParseMemInfoLine(char* buf_line, uint32_t* out_value);

    //
    // ���������� ͨ������id��ȡ������
    // ����ֵ  ��
    //
    bool Pid2Name(uint32_t pid, char* name, uint32_t name_size);

    //
    // ��������: ��ȡϵͳsocket���б�
    //           (��ProcessInfo ͨ��inode������һ��)
    // ����ֵ  :
    //
    bool _ParseSocketPair(SocketProtocol protocol, std::vector<NetSocketPairInfo>* socket_pair);

private:

    char* m_disk_status_buffer;
    uint32_t m_disk_status_buffer_length;
};

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_PARSE_PROC_H_

