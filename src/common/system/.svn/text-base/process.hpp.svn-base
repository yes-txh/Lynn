#ifndef COMMON_SYSTEM_PROCESS_HPP
#define COMMON_SYSTEM_PROCESS_HPP

#ifdef __unix__
#include <unistd.h>
#endif

#ifdef _WIN32
#define _CRT_NONSTDC_NO_WARNINGS 1
#define _POSIX
#include <process.h>
typedef int pid_t;
#endif

// common APIs:
//
// pid_t getpid();
// int system(const char *command);

class Process
{
public:
    bool Create(const char** );
    bool WaitForExit(int* exit_code);
    bool SendSignal(int signal);
};

#endif // COMMON_SYSTEM_PROCESS_HPP

