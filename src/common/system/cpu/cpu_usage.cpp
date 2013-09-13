#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include "common/system/cpu/cpu_usage.h"
#include "common/base/string/string_algorithm.hpp"
#include "common/base/string/string_number.hpp"

#ifdef _WIN32 // windows platform

#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")

uint64_t FileTimeToUtc(const FILETIME* ftime)
{
    LARGE_INTEGER li;
    assert(ftime);
    li.LowPart = ftime->dwLowDateTime;
    li.HighPart = ftime->dwHighDateTime;
    return li.QuadPart;
}

int GetProcessorNumber()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return (int)info.dwNumberOfProcessors;
}

bool GetCpuUsage(int32_t pid, double* cpu)
{
    //上一次的时间
    static int64_t last_time = 0;
    static int64_t last_system_time = 0;
    static int processor_count = -1;

    FILETIME now;
    FILETIME creation_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    int64_t system_time;
    int64_t time;
    int64_t system_time_delta;
    int64_t time_delta;

    processor_count = GetProcessorNumber();
    GetSystemTimeAsFileTime(&now);

    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    if (GetProcessTimes(handle, &creation_time, &exit_time, &kernel_time, &user_time) == 0)
    {
        // We don't assert here because in some cases (such as in the TaskManager)
        // we may call this function on a process that has just exited but we have
        // not yet received the notification.
        CloseHandle(handle);
        return false;
    }
    CloseHandle(handle);

    system_time = (FileTimeToUtc(&kernel_time) + FileTimeToUtc(&user_time)) /
                    processor_count;
    time = FileTimeToUtc(&now);

    if (last_time == 0)
    {
        // First call, just set the last values.
        last_system_time = system_time;
        last_time = time;
        return false;
    }

    system_time_delta = system_time - last_system_time;
    time_delta = time - last_time;

    assert(time_delta != 0);
    if (time_delta == 0)
        return false;

    // We add time_delta / 2 so the result is rounded.
    *cpu = static_cast<double>(system_time_delta * 100 + time_delta / 2) / time_delta;
    last_system_time = system_time;
    last_time = time;
    return true;
}

#else // linux platform

#include <sys/utsname.h>
#define LINUX_VERSION(x, y, z)   (0x10000*(x) + 0x100*(y) + z)

#ifndef AT_CLKTCK
#define AT_CLKTCK       17 // frequency of times()
#endif
#ifndef NOTE_NOT_FOUND
#define NOTE_NOT_FOUND  42
#endif

inline int GetLinuxVersion()
{
    static struct utsname uts;
    int x = 0, y = 0, z = 0;    /* cleared in case sscanf() < 3 */

    if (uname(&uts) == -1)
        return -1;
    sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
    return LINUX_VERSION(x, y, z);
}

// For ELF executables, notes are pushed before environment and args
unsigned long FindElfNote(unsigned long name)
{
    unsigned long *ep = (unsigned long *)environ;
    while (*ep) ep++;
    ep++;
    while (*ep)
    {
        if (ep[0] == name) return ep[1];
        ep += 2;
    }
    return NOTE_NOT_FOUND;
}

unsigned long GetHertz()
{
    // Check the linux kernel version support
    if (GetLinuxVersion() <= LINUX_VERSION(2, 4, 0))
        return 0;
    unsigned long hertz = FindElfNote(AT_CLKTCK);
    return hertz != NOTE_NOT_FOUND ? hertz : 0;
}

bool uptime(double *uptime_secs, double *idle_secs)
{
    int fd = open("/proc/uptime", O_RDONLY);
    if (fd == -1)
        return false;
    char buffer[2048];
    lseek(fd, 0L, SEEK_SET);
    int bytes = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);
    if (bytes < 0)
        return false;
    buffer[bytes] = '\0';

    double up = 0, idle = 0;
    char* savelocale = setlocale(LC_NUMERIC, NULL);
    setlocale(LC_NUMERIC, "C");
    if (sscanf(buffer, "%lf %lf", &up, &idle) < 2)
    {
        setlocale(LC_NUMERIC, savelocale);
        return false;
    }
    setlocale(LC_NUMERIC, savelocale);
    if (uptime_secs) *uptime_secs = up;
    if (idle_secs) *idle_secs = idle;
    return true;
}

bool GetCpuUsage(pid_t pid, double* cpu)
{
    char path[PATH_MAX];
    sprintf(path, "/proc/%d", pid);
    struct stat statbuf;
    if (stat(path, &statbuf))
        return false;

    char filename[PATH_MAX];
    sprintf(filename, "%s/%s", path, "stat");
    int fd = open(filename, O_RDONLY, 0);
    if (fd == -1)
        return false;
    char buffer[1024];
    int bytes = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);
    if(bytes <= 0)
        return false;
    buffer[bytes] = '\0';

    char* s = strchr(buffer, '(') + 1;
    char* d = strrchr(s, ')');
    s = d + 2;  // skip ") "

    std::vector<std::string> fields;
    SplitString(s, " ", &fields);
    uint64_t utime, stime, cutime, cstime, start_time;
    bool ret = StringToNumber(fields[11], &utime);
    ret = ret && StringToNumber(fields[12], &stime);
    ret = ret && StringToNumber(fields[13], &cutime);
    ret = ret && StringToNumber(fields[14], &cstime);
    ret = ret && StringToNumber(fields[19], &start_time);
    if (!ret)
        return false;

    double uptime_secs, idle_secs;
    if (!uptime(&uptime_secs, &idle_secs))
        return false;

    uint64_t seconds_since_boot = static_cast<unsigned long>(uptime_secs);
    // frequency of times()
    unsigned long hertz = GetHertz();
    // TODO (hsiaokangliu) Fix bug: hertz is zero
    if (hertz == 0) hertz = 100;
    // seconds of process life
    uint64_t seconds = seconds_since_boot - start_time / hertz;
    uint64_t total_time = utime + stime + cutime + cstime;

    // scaled %cpu, 999 means 99.9%
    *cpu = 0;
    if(seconds) *cpu = (total_time * 1000ULL / hertz) / seconds;
    *cpu = *cpu / 10;

    return true;
}

#endif
