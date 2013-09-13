#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include "cpu_usage.h"
//include from thirdparty
#include "gflags/gflags.h"

using namespace std;

DEFINE_int32(pid, 0, "process id");

int main(int argc, char** argv)
{
    google::ParseCommandLineFlags(&argc, &argv, true);

    double cpu = 0;
    if (GetCpuUsage(FLAGS_pid, &cpu))
    {
        cout << cpu << endl;
    }
    return EXIT_SUCCESS;
}
