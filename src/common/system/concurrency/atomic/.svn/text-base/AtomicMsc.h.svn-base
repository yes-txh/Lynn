#ifndef ATOMIC_MSC_H_INCLUDED
#define ATOMIC_MSC_H_INCLUDED

#ifdef _M_IX86
#include "common/system/concurrency/atomic/AtomicMscX86.h"
#else
#include "common/system/concurrency/atomic/AtomicMscIntrinsic.h"
typedef AtomicMscIntrinsic AtomicImplementation;
#endif

#endif//ATOMIC_MSC_H_INCLUDED

