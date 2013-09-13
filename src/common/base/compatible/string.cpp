#ifndef COMMON_BASE_COMPATIBLE_STRING_H
#define COMMON_BASE_COMPATIBLE_STRING_H

#include <string.h>

#ifndef __cplusplus
#include "common/base/compatible/stdbool.h"
#endif

#include "common/base/compatible/internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define memeql(a, b, len) (memcmp(a, b, len) == 0)

#ifndef __GNUC__
COMPATIBLE_INLINE const void* internal_memmem(
    const void *haystack,
    size_t haystack_len,
    const void *needle,
    size_t needle_len
    )
{
    const char *begin;
    const char *const last_possible
        = (const char *) haystack + haystack_len - needle_len;

    if (needle_len == 0)
        /* The first occurrence of the empty string is deemed to occur at
           the beginning of the string.  */
        return CONST_CAST(void *, haystack);

    /* Sanity check, otherwise the loop might search through the whole
       memory.  */
    if (haystack_len < needle_len)
        return NULL;

    for (begin = (const char *) haystack; begin <= last_possible; ++begin)
    {
        if (begin[0] == ((const char *) needle)[0] &&
            memeql(&begin[1], ((const char *) needle + 1), needle_len - 1))
            return begin;
    }

    return NULL;
}

#ifdef __cplusplus
COMPATIBLE_INLINE const void* internal_memmem(
    const void *haystack,
    size_t haystack_len,
    const void *needle,
    size_t needle_len
    )
{
    return internal_memmem(haystack, haystack_len, needle, needle_len);
}
#else
COMPATIBLE_INLINE void* memmem(
    void *haystack,
    size_t haystack_len,
    const void *needle,
    size_t needle_len
    )
{
    return CONST_CAST(void*, internal_memmem(haystack, haystack_len, needle, needle_len));
}
#endif

#endif

// token from linux kernel
/**
 * strlcpy - Copy a %NUL terminated string into a sized buffer
 * @param dest Where to copy the string to
 * @param src Where to copy the string from
 * @param size size of destination buffer
 * @return the total length of the string tried to create
 *
 * Compatible with *BSD: the result is always a valid
 * NUL-terminated string that fits in the buffer (unless,
 * of course, the buffer size is zero). It does not pad
 * out the result like strncpy() does.
 */
COMPATIBLE_INLINE size_t strlcpy(char *dest, const char *src, size_t size)
{
    size_t ret = strlen(src);

    if (size) {
        size_t len = (ret >= size) ? size - 1 : ret;
        memcpy(dest, src, len);
        dest[len] = '\0';
    }
    return ret;
}

/**
 * strlcat - Append a length-limited, %NUL-terminated string to another
 * @param dest The string to be appended to
 * @param src The string to append to it
 * @param count The size of the destination buffer.
 * @return the total length of the string tried to create
 */
size_t strlcat(char *dest, const char *src, size_t count)
{
    size_t dsize = strlen(dest);
    size_t len = strlen(src);
    size_t res = dsize + len;

    if (dsize >= count) {
        dest += dsize;
        count -= dsize;
        if (len >= count)
            len = count - 1;
        memcpy(dest, src, len);
        dest[len] = '\0';
    }
    return res;
}

#ifdef _MSC_VER

COMPATIBLE_INLINE void* memrchr(void* start, int c, size_t len)
{
    char* end = REINTERPRET_CAST(char*, start) + len;

    while (--end, len--)
    {
        if (*end == STATIC_CAST(char, c))
            return end;
    }

    return NULL;
}

COMPATIBLE_INLINE const void* memrchr(const void* start, int c, size_t len)
{
    return memrchr(CONST_CAST(void*, start), c, len);
}

COMPATIBLE_INLINE int strcasecmp(const char *s1, const char *s2)
{
    return _stricmp(s1, s2);
}

COMPATIBLE_INLINE int strncasecmp(const char *s1, const char *s2, size_t len)
{
    return _strnicmp(s1, s2, len);
}

COMPATIBLE_INLINE int strerror_r(int errnum, char *buf, size_t buflen)
{
    return strerror_s(buf, buflen, errnum);
}

#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // COMMON_BASE_COMPATIBLE_STRING_H
