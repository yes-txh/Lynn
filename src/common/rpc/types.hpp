#ifndef RPC_TYPES_HPP_INCLUDED
#define RPC_TYPES_HPP_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/// Base definations for RPC

#include <stddef.h>
#include <errno.h>
#include <stdexcept>
#include <string>

#include <common/base/stdint.h>
#include <common/rpc/config.hpp>

namespace Rpc
{

/// 对象 ID 类型
typedef long long ObjectId_t;

/// 调用 ID 类型
typedef long long InvokeId_t;

/// 最多支持的预定义对象的个数，ID 的前这么多个位置留给双方预先约定。
const ObjectId_t MaxPredefinedObjectId = 0x10000;

const size_t MaxMethodNameLength = 31;

/// 最多支持的参数个数
const size_t MaxParameterCount = 8;

/// 用来描述缓冲区
class Buffer
{
public:
    Buffer(const void* ptr, size_t size):
        m_Ptr(const_cast<void*>(ptr)), m_Size(size)
    {
    }
    Buffer():
        m_Ptr(NULL), m_Size(0)
    {}
    void* Address()
    {
        return m_Ptr;
    }
    const void* Address() const
    {
        return m_Ptr;
    }
    size_t Size() const
    {
        return m_Size;
    }
private:
    void* m_Ptr;
    size_t m_Size;
};

/// RPC 的状态码定义
enum Status_t
{
    Status_Success,                         ///< 操作成功
    Status_Pending,                         ///< 操作尚未完成
    Status_Timeout,                         ///< 超时
    Status_Canceled,                        ///< 调用者主动取消
    Status_MethodFailure,                   ///< RPC 成功，但是函数本身执行错误，有返回值，无出参数
    Status_InvalidObject,                   ///< 无效对象 ID
    Status_InvalidEndPoint,                 ///< 无效 EndPoint
    Status_EndPointInUse,                   ///< EndPoint 已被使用
    Status_ObjectNotFound,                  ///< 对象在对方进程不存在
    Status_MethodNotFound,                  ///< 指定的方法不存在
    Status_ProcessNotFound,                 ///< 目标进程不存在（可能已经终止）
    Status_InvalidInvokeId,                 ///< 调用未找到，可能已被删除
    Status_ConnectionError,                 ///< 连接错误
    Status_LocalInvalidInputArguments,      ///< 本地错误：输入参数无效
    Status_LocalInvalidOutputArguments,     ///< 本地错误：输出参数无效
    Status_LocalInvalidReturnValue,         ///< 本地错误：返回值无效
    Status_RemoteInvalidInputArguments,     ///< 远程错误：输入参数无效
    Status_RemoteInvalidOutputArguments,    ///< 远程错误：输出参数无效
    Status_RemoteInvalidReturnValue,        ///< 远程错误：返回值无效
    Statux_Max                              ///< 仅为自动计算状态码的个数用，不是实际的状态码。
};

/// 状态码是不是一个错误
inline bool Status_IsError(Status_t status)
{
    return status != Status_Success && status != Status_Pending;
}

/// 状态码是不是一个本地（非通讯）错误
inline bool Status_IsLocalError(Status_t status)
{
    return Status_IsError(status) && status != Status_ConnectionError;
}

/// 获得状态码对应的字符串。
/// @param status
/// @undefined_string 未定义时返回的值
inline const char* StatusString(Status_t status, const char* undefined_string = "<Undefined status code>")
{
    switch (status)
    {
#define StatusString_MAKE_CASE(Name) case Status_##Name: return #Name
        StatusString_MAKE_CASE(Success);
        StatusString_MAKE_CASE(Pending);
        StatusString_MAKE_CASE(Timeout);
        StatusString_MAKE_CASE(Canceled);
        StatusString_MAKE_CASE(MethodFailure);
        StatusString_MAKE_CASE(InvalidObject);
        StatusString_MAKE_CASE(InvalidEndPoint);
        StatusString_MAKE_CASE(EndPointInUse);
        StatusString_MAKE_CASE(ObjectNotFound);
        StatusString_MAKE_CASE(MethodNotFound);
        StatusString_MAKE_CASE(ProcessNotFound);
        StatusString_MAKE_CASE(InvalidInvokeId);
        StatusString_MAKE_CASE(ConnectionError);
        StatusString_MAKE_CASE(LocalInvalidInputArguments);
        StatusString_MAKE_CASE(LocalInvalidOutputArguments);
        StatusString_MAKE_CASE(LocalInvalidReturnValue);
        StatusString_MAKE_CASE(RemoteInvalidInputArguments);
        StatusString_MAKE_CASE(RemoteInvalidOutputArguments);
        StatusString_MAKE_CASE(RemoteInvalidReturnValue);
#undef StatusString_MAKE_CASE
    default:
        break;
    }
    return undefined_string;
}

/// @brief 异步完成的回调函数原型
/// @param context 函数自身需要的上下文信息
/// @param status 最终的状态
/// @param token 异步完成令牌
/// @param param 额外的任意参数
typedef void (*AsyncCallback)(void* context, Status_t status, void* param);

} // end namespace Rpc

#endif // RPC_TYPES_HPP_INCLUDED
