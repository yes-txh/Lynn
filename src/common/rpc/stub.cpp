///////////////////////////////////////////////////////////////////////////////
/// Stub 相关的实现

#define _CRT_NONSTDC_NO_WARNINGS 1

#include <common/rpc/stub.hpp>

#include <map>

#ifdef unix
#include <unistd.h>
#elif defined _WIN32
#include <process.h>
#else
#endif

#include "common/rpc/scheduler.hpp"

namespace Rpc
{

bool SerializeStub(const Stub& object, SerializeBuffer& buffer)
{
    return SerializeNormalObject(Scheduler::Instance().GetLocalEndPoint(), buffer) &&
           SerializeNormalObject(getpid(), buffer) &&
           SerializeNormalObject(object.RpcObjectId(), buffer);
}

Stub::Stub() : m_ObjectId(GenerateObjectId())
{
    // 新产生 ID
    Scheduler::Instance().RegisterStub(this);
}

/// 允许直接指定对象 ID
Stub::Stub(ObjectId_t object_id) : m_ObjectId(object_id)
{
    assert(object_id < MaxPredefinedObjectId);
    Scheduler::Instance().RegisterStub(this);
}

Stub::Stub(const Stub& src) : m_ObjectId(GenerateObjectId())
{
    // Stub 对象一般不拷贝，这里访问属性置为保护的
    // 如果拷贝，也不拷贝 ID，重新生成。
    Scheduler::Instance().RegisterStub(this);
}

Stub& Stub::operator=(const Stub& src)
{
    // 只是避免拷贝对象 ID
    return *this;
}

Stub::~Stub()
{
    Scheduler::Instance().UnregisterStub(this);
}

// 处理函数调用的返回
Status_t Stub::Return(
    const std::string& local_endpoint,
    const std::string& remote_endpoint,
    InvokeId_t invoke_id,
    int method_id,
    SerializeBuffer& result,
    Status_t status
)
{
    return Scheduler::Instance().Return(local_endpoint, remote_endpoint, invoke_id, method_id, result, status);
}

/// 处理函数的错误返回
Status_t Stub::ErrorReturn(
    const std::string& local_endpoint,
    const std::string& remote_endpoint,
    InvokeId_t invoke_id,
    int method_id,
    Status_t status
)
{
    assert(status != Status_Success);
    return Scheduler::Instance().ErrorReturn(local_endpoint, remote_endpoint, invoke_id, method_id, status);
}

/// 实现基类的 Dispatch
Status_t StubImpl::Dispatch(
    const std::string& local_endpoint,
    const std::string& remote_endpoint,
    InvokeId_t invoke_id,
    const char* name,
    int method_id,
    const void* message,
    size_t size
)
{
    const DispatchTable& dispatch_table = GetDispatchTable();

    InvokeInfo invoke_info = {
        local_endpoint,
        remote_endpoint,
        invoke_id,
        method_id,
    };

    Status_t status = dispatch_table.Invoke(this, invoke_info, name, Buffer(message, size));

    // 通讯错误之外的错误都向客户端发送错误消息，否则可能通讯已经处于故障状态，不发送
    if (Status_IsLocalError(status))
        return ErrorReturn(local_endpoint, remote_endpoint, invoke_id, method_id, status);
    else
        return status;
}

/// 实现基类的 IsNestableMethod
bool StubImpl::IsNestableMethod(const char* name, int method_id)
{
    const DispatchTable& dispatch_table = GetDispatchTable();
    return dispatch_table.IsNestableMethod(name, method_id);
}

bool StubImpl::DispatchTable::IsNestableMethod(
    const char* name,
    int method_id) const
{
    if (method_id < 0)
    {
        method_id = FindMethod(name);
        if (method_id < 0)
            return false;
        return m_Entries[method_id].NestedInvoke;
    }
    else
    {
        if (method_id >= (int)m_Size)
        {
            return false;
        }
        return m_Entries[method_id].NestedInvoke;
    }
    return false;
}

Status_t StubImpl::DispatchTable::Invoke(
    StubImpl* stub,
    InvokeInfo& invoke_info,
    const char* name,
    const Buffer& buffer
) const
{
    if (invoke_info.method_id < 0)
    {
        invoke_info.method_id = FindMethod(name);
        if (invoke_info.method_id < 0)
            return Status_MethodNotFound;
    }
    else
    {
        if (invoke_info.method_id >= (int) m_Size)
            return Status_MethodNotFound;
    }

    return (stub->*m_Entries[invoke_info.method_id].Method)(invoke_info, buffer);
}

// 根据函数名查找 ID
int StubImpl::DispatchTable::FindMethod(const char* name) const
{
    // DispatchTable 以 NULL 名字结尾
    size_t i = 0;
    while (m_Entries[i].Name)
    {
        if (strcmp(m_Entries[i].Name, name) == 0)
            return int(i);
        ++i;
    }
    return -1;
}

} // end namespace Rpc
