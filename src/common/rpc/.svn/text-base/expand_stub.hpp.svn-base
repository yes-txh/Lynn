#include <common/rpc/UndefineInterfaceMacros.hpp>

#include <common/rpc/Stub.hpp>

#define RPC_CALLBACK(Type) const Type##Proxy&

#define RPC_BEGIN_INTERFACE(Interface) \
    class Interface; \
    class Interface##Proxy; \
    RPC_BEGIN_STUB_CLASS(Interface)

#define RPC_METHOD_0(ReturnType, Name, Timeout) \
    RPC_STUB_METHOD_0(ReturnType, Name)

#define RPC_METHOD_1(ReturnType, Name, ArgType1, Timeout) \
    RPC_STUB_METHOD_1(ReturnType, Name, ArgType1)

#define RPC_METHOD_2(ReturnType, Name, ArgType1, ArgType2, Timeout) \
    RPC_STUB_METHOD_2(ReturnType, Name, ArgType1, ArgType2)

#define RPC_METHOD_3(ReturnType, Name, ArgType1, ArgType2, ArgType3, Timeout) \
    RPC_STUB_METHOD_3(ReturnType, Name, ArgType1, ArgType2, ArgType3)

#define RPC_METHOD_4(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, Timeout) \
    RPC_STUB_METHOD_4(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4)

#define RPC_METHOD_5(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, Timeout) \
    RPC_STUB_METHOD_5(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5)

#define RPC_METHOD_6(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, Timeout) \
    RPC_STUB_METHOD_6(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6)

#define RPC_ASYNC_METHOD_0(ReturnType, Name, Timeout) \
    RPC_STUB_ASYNC_METHOD_0(ReturnType, Name)

#define RPC_ASYNC_METHOD_1(ReturnType, Name, ArgType1, Timeout) \
    RPC_STUB_ASYNC_METHOD_1(ReturnType, Name, ArgType1)

#define RPC_ASYNC_METHOD_2(ReturnType, Name, ArgType1, ArgType2, Timeout) \
    RPC_STUB_ASYNC_METHOD_2(ReturnType, Name, ArgType1, ArgType2)

#define RPC_ASYNC_METHOD_3(ReturnType, Name, ArgType1, ArgType2, ArgType3, Timeout) \
    RPC_STUB_ASYNC_METHOD_3(ReturnType, Name, ArgType1, ArgType2, ArgType3)

#define RPC_ASYNC_METHOD_4(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, Timeout) \
    RPC_STUB_ASYNC_METHOD_4(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4)

#define RPC_ASYNC_METHOD_5(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, Timeout) \
    RPC_STUB_ASYNC_METHOD_5(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5)

#define RPC_ASYNC_METHOD_6(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, Timeout) \
    RPC_STUB_ASYNC_METHOD_6(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6)

#define RPC_END_INTERFACE() RPC_END_STUB_CLASS()

#ifndef RPC_INTERFACE_FILENAME
#error Please define "RPC_INTERFACE_FILENAME" before include this file.
#endif

#include RPC_INTERFACE_FILENAME

#include <common/rpc/UndefineInterfaceMacros.hpp>

#define RPC_BEGIN_INTERFACE(Interface) RPC_BEGIN_STUB_DISPATCH(Interface)

#define RPC_METHOD_0(ReturnType, Name, Timeout) \
    RPC_STUB_DISPATCH(Name, 0)

#define RPC_METHOD_1(ReturnType, Name, ArgType1, Timeout) \
    RPC_STUB_DISPATCH(Name, 1)

#define RPC_METHOD_2(ReturnType, Name, ArgType1, ArgType2, Timeout) \
    RPC_STUB_DISPATCH(Name, 2)

#define RPC_METHOD_3(ReturnType, Name, ArgType1, ArgType2, ArgType3, Timeout) \
    RPC_STUB_DISPATCH(Name, 3)

#define RPC_METHOD_4(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, Timeout) \
    RPC_STUB_DISPATCH(Name, 4)

#define RPC_METHOD_5(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, Timeout) \
    RPC_STUB_DISPATCH(Name, 5)

#define RPC_METHOD_6(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, Timeout) \
    RPC_STUB_DISPATCH(Name, 6)

#define RPC_ASYNC_METHOD_0 RPC_METHOD_0
#define RPC_ASYNC_METHOD_1 RPC_METHOD_1
#define RPC_ASYNC_METHOD_2 RPC_METHOD_2
#define RPC_ASYNC_METHOD_3 RPC_METHOD_3
#define RPC_ASYNC_METHOD_4 RPC_METHOD_4
#define RPC_ASYNC_METHOD_5 RPC_METHOD_5
#define RPC_ASYNC_METHOD_6 RPC_METHOD_6

#define RPC_END_INTERFACE() RPC_END_STUB_DISPATCH()

#include RPC_INTERFACE_FILENAME

#include <common/rpc/UndefineInterfaceMacros.hpp>

#undef RPC_INTERFACE_FILENAME

