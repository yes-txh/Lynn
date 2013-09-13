#include <common/rpc/async_token.hpp>
#include <common/rpc/pending_invoke_table.hpp>
#include <common/rpc/scheduler.hpp>

namespace Rpc
{

bool AsyncToken::IsComplete() const
{
    assert(IsValid());
    return Status() != Status_Pending;
}

AsyncToken::~AsyncToken()
{
    if (IsValid())
        Scheduler::Instance().RemoveInvoke(m_InvokeId);
    m_InvokeId = -m_InvokeId; // for debug propose
    m_Status = Status_InvalidInvokeId;
}

// 返回当前状态
Status_t AsyncToken::Status() const
{
    assert(IsValid());
    return m_Status;
}

// 等待完成
Status_t AsyncToken::Wait(int timeout)
{
    assert(IsValid());
    Status_t status = Status();
    if (status != Status_Pending)
        return status;

    if (Scheduler::Instance().WaitInvoke(this, timeout))
    {
        status = Status();
        if (status == Status_Pending)
            status = Status_Timeout;
    }
    else
    {
        status = Status_InvalidInvokeId;
    }
    return status;
}

// 取消调用
bool AsyncToken::Cancel(Status_t status)
{
    if (!IsValid())
        return false;
    return Scheduler::Instance().AbortInvoke(m_InvokeId, status);
}

// 等待一组中任意一个完成
int AsyncToken::WaitAny(AsyncToken** tokens, size_t count, int timeout)
{
    if (count > 0)
        return Scheduler::Instance().WaitAnyInvoke(tokens, count, timeout);
    else
        return 0;
}

// 等待一组全部完成
int AsyncToken::WaitAll(AsyncToken** tokens, size_t count, int timeout)
{
    if (count > 0)
        return Scheduler::Instance().WaitAllInvokes(tokens, count, timeout);
    else
        return 0;
}

} // end namespace Rpc
