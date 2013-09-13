#include "common/rpc/proxy.hpp"
#include "common/rpc/scheduler.hpp"
#include "common/rpc/pending_invoke_table.hpp"

#if 0
#define DEBUG_PRINTF(...) printf(__VA_ARGS__), fflush(stdout)
#else
#define DEBUG_PRINTF(...) (void) 0
#endif

namespace Rpc
{

PendingInvokeTable::PendingInvoke::~PendingInvoke()
{
#ifndef NDEBUG
    memset(this, 0xCF, sizeof(*this));
#else
    StatusPtr = NULL;
    Token = NULL;
#endif
}

PendingInvokeTable::PendingInvokeTable() :
    m_NextInvokeId(0)
{
}


/// 添加一个调用项
InvokeId_t PendingInvokeTable::Insert(
    const char* class_name,
    const char* method_name,
    int& method_id,                                 ///< 函数 id
    const OutputParameterHandler& output_handler,   ///< 输出参数的处理器
    AsyncToken* token,                              ///< 异步完成令牌
    AsyncCallback callback,                         ///< 回调通知函数
    void* object,                                   ///< 回调通知对象指针
    void* param                                     ///< 传给 callback 的任意额外参数
)
{
    MutexLocker lock(m_Mutex);

    // 生成一个新的调用 ID，并在表中加入一项
    InvokeId_t invoke_id = ++m_NextInvokeId;

    assert(m_Table.find(invoke_id) == m_Table.end());
    PendingInvoke& invoke = m_Table[invoke_id];
    DEBUG_PRINTF("INSERT_ID: %lld\n", invoke_id);

    // 初始化
    if (token != NULL)
    {
        assert(token->m_Status != Status_Pending); // 尚未完成的 token 不能被复用

        invoke.StatusPtr = &token->m_Status;
        invoke.StatusPlaceHolder = Status_InvalidInvokeId;
        token->m_InvokeId = invoke_id;
    }
    else
    {
        invoke.StatusPtr = &invoke.StatusPlaceHolder;
    }

    invoke.ClassName = class_name;
    invoke.MethodName = method_name;
    invoke.SetStatus(Status_Pending);

    invoke.MethodId = &method_id;
    invoke.OutputHandler = output_handler;
    invoke.Token = token;
    invoke.Notify.Cond = NULL;
    invoke.Notify.Callback = callback;
    invoke.Notify.Context = object;
    invoke.Notify.Param = param;
    invoke.TimerId = -1;
    invoke.CallbackScheduled = false;

    return invoke_id;
}

bool PendingInvokeTable::Remove(InvokeId_t id)
{
    DEBUG_PRINTF("%s: invoke_id=%lld\n", __func__, id);

    MutexLocker lock(m_Mutex);

    if (m_Table.erase(id) == 1)
    {
        DEBUG_PRINTF("%s: ERASE_ID: %lld\n", __func__, id);
        return true;
    }

    return false;
}

bool PendingInvokeTable::Abort(InvokeId_t id, Status_t status)
{
    assert(status != Status_Success);
    assert(status != Status_Pending);

    bool result = false;
    DEBUG_PRINTF("%s: invoke_id=%lld, %s\n", __func__, id, StatusString(status));

    MutexLocker lock(m_Mutex);

    MapType::iterator i = m_Table.find(id);
    if (i != m_Table.end())
    {
        PendingInvoke& invoke = i->second;
        if (invoke.Status() == Status_Pending)
        {
            invoke.SetStatus(status);
            if (invoke.Notify.Callback != NULL)
            {
                invoke.CallbackScheduled = true;
                Scheduler::Instance().GetThreadPool()->AddTask(
                    MAKE_PARAMETERIZED_THREAD_CALLBACK(PendingInvokeTable, ExecuteCallback, InvokeId_t),
                    this, id
                );
            }
            else
            {
                invoke.Notify.Signal();
                m_Table.erase(i);
                DEBUG_PRINTF("%s, callback ERASE_ID: %lld\n", __func__, id);
            }
            result = true;
        }
        else
        {
            if (!invoke.CallbackScheduled)
            {
                assert(invoke.Status() != Status_Success);
                DEBUG_PRINTF(
                    "%s, no callback ERASE_ID: %lld, status = %s\n", __func__ ,
                    id, Rpc::StatusString(invoke.Status()));
                m_Table.erase(i);
            }
        }
    }

    return result;
}

long long PendingInvokeTable::GetTimerId(InvokeId_t invoke_id) const
{
    MutexLocker lock(m_Mutex);
    MapType::const_iterator i = m_Table.find(invoke_id);
    if (i != m_Table.end())
    {
        return i->second.TimerId;
    }
    return -1;
}

bool PendingInvokeTable::SetTimerId(InvokeId_t invoke_id, long long timer_id)
{
    MutexLocker lock(m_Mutex);
    MapType::iterator i = m_Table.find(invoke_id);
    if (i != m_Table.end())
    {
        i->second.TimerId = timer_id;
        return true;
    }
    return false;
}

bool PendingInvokeTable::Wait(AsyncToken* token, int timeout)
{
    MutexLocker lock(m_Mutex);

    // 加锁后再次检查
    if (token->Status() != Status_Pending)
        return true;

    InvokeId_t id = token->InvokeId();

    MapType::iterator i = m_Table.find(id);
    if (i == m_Table.end())
        return false;

    // Lock
    PendingInvoke* invoke = &i->second;
    if (invoke->Status() == Status_Pending)
    {
        ConditionVariable cond;
        invoke->Notify.Cond = &cond;
        cond.Wait(m_Mutex, timeout);
        i = m_Table.find(id);
        if (i != m_Table.end())
        {
            invoke = &i->second;
            invoke->Notify.Cond = NULL;
        }
    }

    return true;
}

bool PendingInvokeTable::SetCond(
    AsyncToken** tokens,
    size_t count,
    ConditionVariable* cond
    )
{
    bool result = true;
    for (size_t i = 0; i < count; ++i)
    {
        MapType::iterator it = m_Table.find(tokens[i]->m_InvokeId);
        if (it != m_Table.end())
        {
            PendingInvoke& invoke = it->second;
            invoke.Notify.Cond = cond;
        }
        else
        {
            result = false;
        }
    }
    return result;
}

bool PendingInvokeTable::WaitAny(AsyncToken** tokens, size_t count, int timeout)
{
    MutexLocker lock(m_Mutex);

    ConditionVariable cond;

    // Set condition to all tokens
    if (!SetCond(tokens, count, &cond))
    {
        SetCond(tokens, count, NULL);
        return false;
    }

    cond.Wait(m_Mutex, timeout);

    bool result = false;
    for (size_t i = 0; i < count; ++i)
    {
        if (tokens[i]->Status() != Status_Pending)
            result = true;
    }

    SetCond(tokens, count, NULL);
    return result;
}

bool PendingInvokeTable::WaitAll(AsyncToken** tokens, size_t count, int timeout)
{
    MutexLocker lock(m_Mutex);
    ConditionVariable cond;
    if (!SetCond(tokens, count, &cond))
    {
        SetCond(tokens, count, NULL);
        return false;
    }

    cond.Wait(m_Mutex, timeout);

    bool result = true;
    for (size_t i = 0; i < count; ++i)
    {
        if (tokens[i]->Status() == Status_Pending)
        {
            result = false;
            break;
        }
    }

    SetCond(tokens, count, NULL);
    return result;
}

bool PendingInvokeTable::UnserializeReturnValue(
    InvokeId_t invoke_id,
    PendingInvoke& invoke,
    const char* message,
    size_t size)
{
    // 反序列化返回值
    if (invoke.Token)
    {
        if (!invoke.Token->ParseResult(message, size))
        {
            invoke.SetStatus(Status_LocalInvalidReturnValue);
            return false;
        }
    }

    DEBUG_PRINTF("%s: SetStatus, invoke_id = %lld, status = Success\n", __func__, invoke_id);
    invoke.SetStatus(Status_Success);
    return true;
}


bool PendingInvokeTable::UnserializeResult(
    InvokeId_t invoke_id,
    PendingInvoke& invoke,
    const char* message,
    size_t size)
{
    // 反序列化出参数
    if (!invoke.OutputHandler.ExtractParamters(message, size))
    {
        invoke.SetStatus(Status_LocalInvalidOutputArguments);
        return false;
    }

    if (!UnserializeReturnValue(invoke_id, invoke, message, size))
        return false;

    DEBUG_PRINTF("%s: SetStatus, invoke_id = %lld, status = Success\n", __func__, invoke_id);
    invoke.SetStatus(Status_Success);
    return true;
}

void PendingInvokeTable::HandleReturn(
    InvokeId_t invoke_id,
    int method_id,
    Status_t status,
    const char* message,
    size_t size
)
{
    DEBUG_PRINTF("%s: invoke_id=%lld\n", __func__, invoke_id);
    MutexLocker lock(m_Mutex);
    DEBUG_PRINTF("%s: invoke_id=%lld, enter lock\n", __func__, invoke_id);

    MapType::iterator i = m_Table.find(invoke_id);
    if (i != m_Table.end())
    {
        PendingInvoke& invoke = i->second;
        DEBUG_PRINTF("%s: invoke_id=%lld, invoke found\n", __func__, invoke_id);

        if (invoke.Status() == Status_Pending)
        {
            DEBUG_PRINTF("%s: invoke_id=%lld, status=pending\n", __func__, invoke_id);
            if (method_id >= 0)
                *invoke.MethodId = method_id;

            switch (status)
            {
            case Status_Success:
                UnserializeResult(invoke_id, invoke, message, size);
                break;
            case Status_MethodFailure:
                UnserializeReturnValue(invoke_id, invoke, message, size);
                break;
            default:
                invoke.SetStatus(status);
                break;
            }

            assert(status != Status_Pending);

            if (invoke.Notify.Callback)
            {
                DEBUG_PRINTF("%s: invoke_id=%lld, prepare callback\n", __func__, invoke_id);
                invoke.CallbackScheduled = true;
                Scheduler::Instance().GetThreadPool()->AddTask(
                    MAKE_PARAMETERIZED_THREAD_CALLBACK(PendingInvokeTable, ExecuteCallback, InvokeId_t),
                    this, invoke_id
                );
            }
            else
            {
                DEBUG_PRINTF("%s: invoke_id=%lld, signal\n", __func__, invoke_id);
                invoke.Notify.Signal();
                m_Table.erase(i);
                DEBUG_PRINTF("%s: ERASE_ID: %lld\n", __func__, invoke_id);
            }
        }
        else
        {
            if (!invoke.CallbackScheduled)
            {
                m_Table.erase(i);
                DEBUG_PRINTF("%s: no call back ERASE_ID: %lld\n", __func__,  invoke_id);
            }
        }
    }
}

void PendingInvokeTable::ExecuteCallback(InvokeId_t invoke_id)
{
    DEBUG_PRINTF("%s: invoke_id=%lld\n", __func__, invoke_id);

    PendingInvoke invoke;

    // lock scope
    {
        MutexLocker lock(m_Mutex);
        MapType::iterator i = m_Table.find(invoke_id);
        if (i != m_Table.end())
        {
            // make a local copy before erase it
            invoke = i->second;

            DEBUG_PRINTF("%s, ERASE_ID: %lld\n", __func__, invoke_id);
            m_Table.erase(i);
        }
        else
        {
            assert(!"ExecuteCallback: invoke not found");
            return;
        }
    }

    Status_t status = invoke.Status();
    assert(status != Status_Pending);

    DEBUG_PRINTF("%s: callback executing invoke_id=%lld\n", __func__, invoke_id);
    invoke.Notify.ExecuteCallback(status);
    invoke.Notify.Signal();
}


} // end namespace Rpc

