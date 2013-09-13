#include <common/rpc/invoke_context.hpp>
#include <common/rpc/stub.hpp>

namespace Rpc
{

InvokeContext::InvokeContext():
    m_OutputParamCount(0), m_HasReturnValue(false), m_InvokeId(0), m_MethodId(0)
{
}

InvokeContext::InvokeContext(const InvokeInfo& invoke_info) :
    m_OutputParamCount(0), m_HasReturnValue(false),
    m_LocalEndPoint(invoke_info.local_endpoint),
    m_RemoteEndPoint(invoke_info.remote_endpoint),
    m_InvokeId(invoke_info.invoke_id),
    m_MethodId(invoke_info.method_id)
{
}

/// 向调用者返回调用结果
Status_t InvokeContext::Return(SerializeBuffer& result, Status_t status) const
{
    return Stub::Return(m_LocalEndPoint, m_RemoteEndPoint, m_InvokeId, m_MethodId, result, status);
}

/// 向调用者返回出错的调用结果
Status_t InvokeContext::ErrorReturn(Status_t status) const
{
    return Stub::ErrorReturn(m_LocalEndPoint, m_RemoteEndPoint, m_InvokeId, m_MethodId, status);
}


} // end namespace Rpc
