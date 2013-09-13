// Copy right 2011, Tencent Inc.
// Author : Xiaokang Liu (hsiaokangliu@tencent.com)
// Feng Chen (phongchen@tencent.com)

#include "common/system/debug/print_trace.hpp"

/* A dummy function to make the backtrace more interesting. */
void dummy_function ()
{
    PrintStackTrace ();
}

int main ()
{
    dummy_function ();
    return 0;
}
