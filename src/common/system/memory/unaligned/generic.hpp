// generic solution, using memcpy

#include <string.h>
#include "common/base/type_cast.hpp"
#include "common/system/memory/unaligned/check_direct_include.hpp"

template <typename T>
T GetUnaligned(const void* p)
{
    T t;
    memcpy(&t, p, sizeof(t));
    return t;
}

template <typename T, typename U>
void PutUnaligned(void* p, const U& value)
{
    T t = implicit_cast<T>(value);
    memcpy(p, &t, sizeof(t));
}


