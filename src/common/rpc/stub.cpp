///////////////////////////////////////////////////////////////////////////////
/// Stub ��ص�ʵ��

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
    // �²��� ID
    Scheduler::Instance().RegisterStub(this);
}

/// ����ֱ��ָ������ ID
Stub::Stub(ObjectId_t object_id) : m_ObjectId(object_id)
{
    assert(object_id < MaxPredefinedObjectId);
    Scheduler::Instance().RegisterStub(this);
}

Stub::Stub(const Stub& src) : m_ObjectId(GenerateObjectId())
{
    // Stub ����һ�㲻�������������������Ϊ������
    // ���������Ҳ������ ID���������ɡ�
    Scheduler::Instance().RegisterStub(this);
}

Stub& Stub::operator=(const Stub& src)
{
    // ֻ�Ǳ��⿽������ ID
    return *this;
}

Stub::~Stub()
{
    Scheduler::Instance().UnregisterStub(this);
}

// ���������õķ���
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

/// �������Ĵ��󷵻�
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

/// ʵ�ֻ���� Dispatch
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

    // ͨѶ����֮��Ĵ�����ͻ��˷��ʹ�����Ϣ���������ͨѶ�Ѿ����ڹ���״̬��������
    if (Status_IsLocalError(status))
        return ErrorReturn(local_endpoint, remote_endpoint, invoke_id, method_id, status);
    else
        return status;
}

/// ʵ�ֻ���� IsNestableMethod
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

// ���ݺ��������� ID
int StubImpl::DispatchTable::FindMethod(const char* name) const
{
    // DispatchTable �� NULL ���ֽ�β
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
