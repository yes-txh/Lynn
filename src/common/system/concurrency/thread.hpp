// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2010-06-18

#ifndef COMMON_SYSTEM_CONCURRENCY_THREAD_HPP_INCLUDED
#define COMMON_SYSTEM_CONCURRENCY_THREAD_HPP_INCLUDED

#include "common/base/closure.h"
#include "common/base/function.hpp"
#include "common/system/concurrency/base_thread.hpp"

/// 把类成员函数转为普通函数用来做回调。
/// @param Class 类名
/// @param Member 成员函数名
/// @param ParamType 成员函数的参数类型，只能是指针或者 intptr_t/uintptr_t
/// 例如存在成员函数：@code
/// void Test::XxxThread();
/// @endcode
/// 用 MAKE_MEMBER_CALLBACK(Test, XxxThread)，既可生成可用于线程的函数指针，类型为@code
/// void (*)(void* object, void* param)
/// @endcode
/// 注意开头多了一个参数，用来传递 this 指针。
#define MAKE_THREAD_CALLBACK(Class, Member) &::GenericMemberFunctionAdapter<Class, &Class::Member>

/// 适配成员函数用的通用的线程函数
template<typename Class, void (Class::*Member)()>
void GenericMemberFunctionAdapter(void* context, unsigned long long param)
{
    (static_cast<Class*>(context)->*Member)();
}

/// 把带一个参数的类成员函数转为普通函数用来做回调。
/// @param Class 类名
/// @param Member 成员函数名
/// @param ParamType 成员函数的参数类型，只能是指针或者 intptr_t/uintptr_t
/// 例如存在成员函数：@code
/// void Test::XxxThread(void* param);
/// @endcode
/// 用 MAKE_MEMBER_CALLBACK(Test, XxxThread)，既可生成可用于线程的函数指针，类型为@code
/// void (*)(void* object, void* param)
/// @endcode
/// 注意开头多了一个参数，用来传递 this 指针。
#define MAKE_PARAMETERIZED_THREAD_CALLBACK(Class, Member, ParamType) \
    &::GenericParamMemberFunctionAdapter<Class, ParamType, &Class::Member>
template<typename Class, typename ParamType, void (Class::*Member)(ParamType)>
void GenericParamMemberFunctionAdapter(void* context, unsigned long long param)
{
    (static_cast<Class*>(context)->*Member)(ParamType(param));
}

class Thread : public ::BaseThread
{
public:
    typedef void (*StartRoutine)(void* context, unsigned long long param);

    Thread():
        m_StartRoutine(NULL),
        m_Context(NULL),
        m_Param(0),
        m_closure(NULL)
    {
    }

    explicit Thread(
        StartRoutine start_routine,
        void* context = NULL,
        unsigned long long param = 0
    ):
        m_StartRoutine(start_routine),
        m_Context(context),
        m_Param(param),
        m_closure(NULL)
    {
    }

    explicit Thread(Closure<void>* closure) :
        m_StartRoutine(NULL),
        m_Context(NULL),
        m_Param(0),
        m_closure(closure)
    {
    }

    explicit Thread(const Function<void ()>& function) :
        m_StartRoutine(NULL),
        m_Context(NULL),
        m_Param(0),
        m_closure(NULL),
        m_function(function)
    {
    }

    void Initialize(StartRoutine start_routine, void* context = NULL, unsigned long long param = 0)
    {
        m_StartRoutine = start_routine;
        m_Context = context;
        m_Param = param;
        m_closure = NULL;
        m_function = NULL;
    }

    void Initialize(Closure<void>* closure)
    {
        m_StartRoutine = NULL;
        m_Context = NULL;
        m_Param = 0;
        m_closure = closure;
    }

    void Initialize(const Function<void ()>& function)
    {
        m_StartRoutine = NULL;
        m_Context = NULL;
        m_Param = 0;
        m_closure = NULL;
        m_function = function;
    }

private:
    virtual void Entry()
    {
        if (m_function) {
            m_function();
        } else if (m_closure) {
            m_closure->Run();
        } else if (m_StartRoutine) {
            m_StartRoutine(m_Context, m_Param);
        }
    }

    // Description about the routine.
    StartRoutine m_StartRoutine;
    void* m_Context;
    unsigned long long m_Param;

    // Description about the routine by Closure.
    // If it's set, ignore the above routine.
    Closure<void>* m_closure;

    // Description about the routine by Function.
    // If it's set, ignore the above routine.
    Function<void ()> m_function;
};

#endif // COMMON_SYSTEM_CONCURRENCY_THREAD_HPP_INCLUDED

