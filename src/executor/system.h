/********************************
 FileName: executor/system.h
 Author:   WangMin
 Date:     2013-08-21
 Version:  0.1
 Description: get the physical resource info of the machine
*********************************/

#ifndef SRC_EXECUTOR_SYSTEM_H
#define SRC_EXECUTOR_SYSTEM_H

#include <iostream>
#include <stdint.h> 

using std::string;

class System {
public:
    /// @brief: Load 
    // get cpu average load(1 min)
    static double GetLoadAvginOne();

    // get cpu average load(5 min)
    static double GetLoadAvginFive();

    // get cpu average load(15 min)
    static double GetLoadAvginFifteen();

    /// @brief: CPU
    // get cpu logic core number
    static int GetCpuNum();

    // get total cpu time in USER_HZ unit
    static uint64_t GetCpuTime();

    // get cpu usage
    static double GetCpuUsage();


    /// @brief: Memory
    // get total physical memroy size(MB)
    static int GetTotalMemory();

    // get free physical memory size(MB)
    static int GetPhysicalMemory();

    // get used memory size(MB)
    static int GetUsedMemory();

    // get memory usage
    static double GetMemoryUsage();

    // get swaptotal memory size(MB)
    static int GetSwapTotalMemory();

    // get swapfree memory size(MB)
    static int GetSwapFreeMemory();


    /// @brief: network flow
    // get net flow(size: Bytes)
    static void GetNetFlow(const char* interface, int64_t& bytes_in, int64_t& bytes_out);

    /// @brief: Disk
    // get total disk size(GB)
    static int GetTotalDisk();

    // get used disk(GB)
    static int GetUsedDisk();

    // get disk usage
    static double GetDiskUsage();

    // for img
    static int GetFakeSpace();

    /// @brief: others
    // get os version
    static string GetOSVersion();

    // remove the directory
    static void RemoveDir(const char* path);

    // get current time
    static void GetCurrentTime(char* str, int len);

};

#endif
