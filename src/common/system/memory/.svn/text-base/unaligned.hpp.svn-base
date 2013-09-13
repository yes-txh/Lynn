#ifndef COMMON_SYSTEM_MEMORY_UNALIGNED_HPP
#define COMMON_SYSTEM_MEMORY_UNALIGNED_HPP

#include <stddef.h>

/// @file
/// @author phongchen <phongchen@tencent.com>
/// @brief portable unaligned memory access.
/// @note for portable reason, T should be POD type, and sizeof(T) should be
/// 1 2 4 8 bytes.

//////////////////////////////////////////////////////////////////////////////
// interface declaration

/// @brief get value from unaligned address
/// @tparam T type to get
/// @param p pointer to get value from
/// @return get result
/// @details usage: uint32_t n = GetUnaligned<uint32_t>(p);
template <typename T>
T GetUnaligned(const void* p);

/// @brief put value into unaligned address
/// @tparam T type to get
/// @tparam U introduce U make T must be given explicitly
/// @param p pointer to get value
/// @param value value to put into p
/// @details usage: PutUnaligned<uint32_t>(p, 100);
template <typename T, typename U>
void PutUnaligned(void* p, const U& value);

//////////////////////////////////////////////////////////////////////////////
// implementation

#include "common/base/platform_features.hpp"

#if defined ALIGNMENT_INSENSITIVE_PLATFORM
# include "common/system/memory/unaligned/align_insensitive.hpp"
#else
# if defined __GNUC__
#  include "common/system/memory/unaligned/gcc.hpp"
# elif defined _MSC_VER
#  include "common/system/memory/unaligned/msc.hpp"
# else
#  include "common/system/memory/unaligned/generic.hpp"
# endif // compiler detect
#endif // arch detect

/// @brief round up pointer to next nearest aligned address
/// @param p the pointer
/// @param align alignment, must be power if 2
template <typename T>
T* RoundUpPtr(T* p, size_t align)
{
    size_t address = reinterpret_cast<size_t>(p);
    return reinterpret_cast<T*>((address + align - 1) & ~(align - 1U));
}

/// @brief round down pointer to previous nearest aligned address
/// @param p the pointer
/// @param align alignment, must be power if 2
template <typename T>
T* RoundDownPtr(T* p, size_t align)
{
    size_t address = reinterpret_cast<size_t>(p);
    return reinterpret_cast<T*>(address & ~(align - 1U));
}

#endif // COMMON_SYSTEM_MEMORY_UNALIGNED_HPP
