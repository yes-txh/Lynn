#ifndef COMMON_SYSTEM_CPU_RDTSC_HPP
#define COMMON_SYSTEM_CPU_RDTSC_HPP

#ifdef _MSC_VER
inline __int64 rdtsc()
{
    __asm rdtsc
}
#elif defined __GNUC__
#if defined __i386__
inline long rdtsc()
{
    long val;
    __asm__ __volatile__("rdtsc" : "=A" (val));
    return val;
}
#elif defined __x86_64__
inline long rdtsc()
{
     unsigned int __a,__d;
     __asm__ __volatile__("rdtsc" : "=a" (__a), "=d" (__d));
     return ((unsigned long)__a) | (((unsigned long)__d)<<32);
}
#endif
#endif

#endif // COMMON_SYSTEM_CPU_RDTSC_HPP
