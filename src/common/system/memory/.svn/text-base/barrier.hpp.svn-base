#ifndef COMMON_SYSTEM_MEMORY_BARRIER_HPP
#define COMMON_SYSTEM_MEMORY_BARRIER_HPP

/// @file
/// @brief memory barrier defines
/// @author phongchen <phongchen@tencent.com>
/// @date Nov 24, 2010

/*
/// full memory barrier
void MemoryBarrier();
/// read barrier
void MemoryReadBarrier();
/// write barrier
void MemoryWriteBarrier();
*/
//////////////////////////////////////////////////////////////////////////////
// implementation

#if defined _MSC_VER
#include <intrin.h>
inline void CompilerBarrier() { volatile int n = 0; n = 0; }
// MemoryBarrier() is already defined in intrin.h
inline void MemoryReadBarrier() { _ReadBarrier(); }
inline void MemoryWriteBarrier() { _WriteBarrier(); }

#elif __GNUC__
inline void CompilerBarrier() { __asm__ __volatile__("": : :"memory"); }
# if defined __i386__ || defined __x86_64__
inline void MemoryBarrier() { __asm__ __volatile__("mfence": : :"memory"); }
inline void MemoryReadBarrier() { __asm__ __volatile__("lfence" ::: "memory"); }
inline void MemoryWriteBarrier() { __asm__ __volatile__("sfence" ::: "memory"); }
# else
inline void MemoryBarrier() { CompilerBarrier(); }
inline void MemoryReadBarrier() { MemoryBarrier(); }
inline void MemoryWriteBarrier() { MemoryBarrier(); }
# endif
#else
inline void CompilerBarrier() { volatile int n = 0; n = 0; }
inline void MemoryBarrier() { CompilerBarrier(); }
inline void MemoryReadBarrier() { MemoryBarrier(); }
inline void MemoryWriteBarrier() { MemoryBarrier(); }
#endif

#endif // COMMON_SYSTEM_MEMORY_BARRIER_HPP

