#include "common/system/memory/unaligned/check_direct_include.hpp"
#include "common/base/type_cast.hpp"

#if defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC) || defined(_M_IA64) || defined(_M_AMD64)
// microsoft c support __unaligned keyword under align sensitive archs
template <typename T>
T GetUnaligned(const void* p)
{
    return *static_cast<const __unaligned T*>(p);
}

template <typename T, typename U>
void PutUnaligned(void* p, const U& value)
{
    *static_cast<__unaligned T*>(p) = implicit_cast<T>(value);
}
#else
// fallback to generic implementation
#include "common/system/memory/unaligned/generic.hpp"
#endif

