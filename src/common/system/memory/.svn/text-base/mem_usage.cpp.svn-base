#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include "common/base/stdint.h"

#ifdef _WIN32   // windows platform
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")

bool GetMemUsage(int32_t pid, uint64_t* vm_size, uint64_t* mem_size)
{
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    PROCESS_MEMORY_COUNTERS pmc;
    bool ret = false;
    if(GetProcessMemoryInfo(handle, &pmc, sizeof(pmc)))
    {
        *mem_size = pmc.WorkingSetSize;
        *vm_size = pmc.PagefileUsage;
        ret = true;
    }
    CloseHandle(handle);
    return ret;
}

#else           // linux platform
bool GetMemUsage(int32_t pid, uint64_t* vm_size, uint64_t* mem_size)
{
    char path[PATH_MAX];
    sprintf(path, "/proc/%d", pid);
    struct stat statbuf;
    if (stat(path, &statbuf))
        return false;

    char filename[PATH_MAX];
    sprintf(filename, "%s/%s", path, "statm");
    int fd = open(filename, O_RDONLY, 0);
    if (fd == -1)
        return false;
    char buffer[1024];
    int bytes = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);
    if(bytes <= 0)
        return false;
    buffer[bytes] = '\0';

    long size, resident, share, trs, lrs, drs, dt;
    sscanf(buffer, "%ld %ld %ld %ld %ld %ld %ld",
            &size, &resident, &share, &trs, &lrs, &drs, &dt);

    *vm_size = size * 4;
    *mem_size = resident * 4;

    return true;
}

#endif
