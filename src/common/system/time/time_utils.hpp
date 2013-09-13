#ifndef COMMON_SYSTEM_TIME_TIME_UTILS_HPP
#define COMMON_SYSTEM_TIME_TIME_UTILS_HPP

#include <string.h>
#include <string>
#include "common/base/stdint.h"

struct TimeUtils
{
    // Milliseconds always returns milliseconds(1/1000s) since Jan 1, 1970 GMT.
    static int64_t Milliseconds();

    // Milliseconds always returns microseconds(1/1000000s) since Jan 1, 1970 GMT.
    static int64_t Microseconds();

    // Returns the offset in hours between local time and GMT (or UTC) time.
    static int GetGMTOffset();

    static std::string GetCurTime();

    static std::string GetCurMilliTime();
};


#endif // COMMON_SYSTEM_TIME_TIME_UTILS_HPP

