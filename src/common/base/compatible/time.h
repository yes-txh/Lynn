#ifndef COMMON_BASE_COMPATIBLE_TIME_H
#define COMMON_BASE_COMPATIBLE_TIME_H

#include <time.h>
#include "common/base/compatible/internal.h"

#ifdef _MSC_VER

DEFINE_INTEGER_CONST(size_t, ASCTIME_LENGTH, sizeof("Day Mon dd hh:mm:ss yyyy\n"));
DEFINE_INTEGER_CONST(size_t, ASCTIME_BUFFER_SIZE, ASCTIME_LENGTH - 1);

COMPATIBLE_INLINE char *asctime_r(const struct tm *tm, char *buf)
{
    if (asctime_s(buf, ASCTIME_BUFFER_SIZE, tm) == 0)
        return buf;
    else
        return NULL;
}

COMPATIBLE_INLINE char *ctime_r(const time_t *timep, char *buf)
{
    if (ctime_s(buf, ASCTIME_BUFFER_SIZE, timep) == 0)
        return buf;
    else
        return NULL;
}

COMPATIBLE_INLINE struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
    if (gmtime_s(result, timep) == 0)
        return result;
    return NULL;
}

COMPATIBLE_INLINE struct tm *localtime_r(const time_t *timep, struct tm *result)
{
    if (localtime_s(result, timep) == 0)
        return result;
    return NULL;
}

EXTERN_C char *strptime(const char *s, const char *format, struct tm *tm);

#endif

#endif // COMMON_BASE_COMPATIBLE_TIME_H
