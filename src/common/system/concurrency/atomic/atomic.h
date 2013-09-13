#ifndef COMMON_SYSTEM_CONCURRENCY_ATOMIC_ATOMIC_H_INCLUDED
#define COMMON_SYSTEM_CONCURRENCY_ATOMIC_ATOMIC_H_INCLUDED

/*

Function style interface

///////////////////////////////////////////////////////////////////////////////
// return value
AtomicGet

///////////////////////////////////////////////////////////////////////////////
// Change value and return
AtomicSet
AtomicAdd
AtomicSub
AtomicAnd
AtomicOr
AtomicXor

T AtomicExchange<Operation>(T& target, T value)
Operation:
    atomically
    {
        target operation value;
        return target;
    }

///////////////////////////////////////////////////////////////////////////////
// change value and return old value

AtomicExchangeSet
AtomicExchangeAdd
AtomicExchangeSub
AtomicExchangeAnd
AtomicExchangeOr
AtomicExchangeXor

Prototype:
    T AtomicExchange<Operation>(T& target, T value)

Operation:
    atomically
    {
        T old = target;
        target operation value;
        return old;
    }


///////////////////////////////////////////////////////////////////////////////
// compare and change

Prototype:
    bool AtomicCompareExchange(T& value, T compare, T exchange, T& old)

Operation:
    atomically
    {
        old = value;
        if (value == compare)
        {
            value = exchange;
            return true;
        }
        return false;
    }

*/

// import implementation for each platform
#if defined _MSC_VER
#include "common/system/concurrency/atomic/AtomicMsc.h"
#elif defined __GNUC__
#include "common/system/concurrency/atomic/AtomicGcc.h"
#else
#error unsupported compiler
#endif

// convert implementation to global namespace
#include "common/system/concurrency/atomic/AtomicAdapter.h"

#endif // COMMON_SYSTEM_CONCURRENCY_ATOMIC_ATOMIC_H_INCLUDED
