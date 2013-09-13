#ifndef RPC_CONFIG_HPP_INCLUDED
#define RPC_CONFIG_HPP_INCLUDED

/// @file Config.hpp
/// brief 支持一定程度的定制
/// 此处多为占位实现，实际要与其他库对接。

#include <common/system/concurrency/mutex.hpp>
#include <common/system/concurrency/condition_variable.hpp>
#include <common/system/concurrency/thread.hpp>

namespace Rpc
{

#if 0
/// 互斥
class Mutex
{
public:
    void Lock() {}
    void Unlock() {}
};
#endif

#if 0
/// Mutex 的自动 Lock
class MutexLock
{
public:
    MutexLock(Mutex&) {}
};
#endif

#if 0
/// 条件变量
class Condition
{
public:
    void Signal() {}
    void Broadcast() {}
    bool Wait(Mutex&, int timeout) {
        return false;
    }
};
#endif

typedef ::ConditionVariable ConditionVariable;

} // end namespace Rpc

#endif//RPC_CONFIG_HPP_INCLUDED
