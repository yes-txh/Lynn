#include <stdlib.h>
#include <iostream>
#include "common/system/memory/mem_usage.h"

using namespace std;

int main()
{
#ifdef __unix__
    pid_t pid = 5952;
#else
    DWORD pid = 1234;
#endif
    uint64_t vm_size, mem_size;
    if (GetMemUsage(pid, &vm_size, &mem_size))
    {
        cout << "vm size: " << vm_size << endl;
        cout << "mem size: " << mem_size << endl;
    }
    return EXIT_SUCCESS;
}
