#ifndef RPC_CONFIG_HPP_INCLUDED
#define RPC_CONFIG_HPP_INCLUDED

/// @file Config.hpp
/// brief ֧��һ���̶ȵĶ���
/// �˴���Ϊռλʵ�֣�ʵ��Ҫ��������Խӡ�

#include <common/system/concurrency/mutex.hpp>
#include <common/system/concurrency/condition_variable.hpp>
#include <common/system/concurrency/thread.hpp>

namespace Rpc
{

#if 0
/// ����
class Mutex
{
public:
    void Lock() {}
    void Unlock() {}
};
#endif

#if 0
/// Mutex ���Զ� Lock
class MutexLock
{
public:
    MutexLock(Mutex&) {}
};
#endif

#if 0
/// ��������
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
