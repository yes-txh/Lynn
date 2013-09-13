#define _CRT_NONSTDC_NO_WARNINGS 1

#include "common/rpc/scheduler.hpp"

#include <string>
#include <new>

#ifdef unix
#include <unistd.h>
#elif defined _WIN32
#include <process.h>
#else
#endif

#include "common/base/closure.h"
#include "common/base/stdint.h"
#include "common/rpc/message.hpp"
#include "common/rpc/channel.hpp"
#include "common/rpc/local_object_table.hpp"
#include "common/rpc/remote_object_table.hpp"
#include "common/rpc/pending_invoke_table.hpp"
#include "common/system/timer/timer_manager.hpp"

namespace Rpc
{

class SchedulerImpl
{
public:
    LocalObjectTable m_LocalObjectTable;            ///< ���ض�����������еĿɱ� RPC ���ʵı��ض���
    RemoteObjectTable m_RemoteObjectTable;          ///< Զ�̶�����������еĶ������
    PendingInvokeTable m_PendingInvokeTable;        ///< δ�����ñ���������δ��ɵĵ���
    TimerManager m_TimerManager;                    ///< ��ʱ�������࣬����ʱ
    Channel* m_Channel;                             ///< ͨѶͨ��
public:
    SchedulerImpl() : m_Channel(NULL)
    {
    }

    ~SchedulerImpl()
    {
    }
};

Scheduler::Scheduler():
    m_ThreadPool(1, 32)
{
    m_pImpl = new SchedulerImpl();
}

Scheduler::~Scheduler()
{
    delete m_pImpl;
}

bool Scheduler::RemoveInvoke(InvokeId_t invoke_id)
{
    return m_pImpl->m_PendingInvokeTable.Remove(invoke_id);
}

bool Scheduler::AbortInvoke(InvokeId_t invoke_id, Status_t status)
{
    return m_pImpl->m_PendingInvokeTable.Abort(invoke_id, status);
}

bool Scheduler::WaitInvoke(AsyncToken* token, int timeout)
{
    return m_pImpl->m_PendingInvokeTable.Wait(token, timeout);
}

int Scheduler::WaitAnyInvoke(AsyncToken** tokens, size_t count, int timeout)
{
    return m_pImpl->m_PendingInvokeTable.WaitAny(tokens, count, timeout);
}

int Scheduler::WaitAllInvokes(AsyncToken** tokens, size_t count, int timeout)
{
    return m_pImpl->m_PendingInvokeTable.WaitAll(tokens, count, timeout);
}

bool Scheduler::WaitForSendingComplete(int timeout)
{
    return m_pImpl->m_Channel->WaitForSendingComplete(timeout);
}

Channel* Scheduler::SetChannel(Channel* channel)
{
    Channel* old = m_pImpl->m_Channel;
    if (!old)
    {
        m_pImpl->m_Channel = channel;
        channel->SetEventHandler(this);
    }
    return old;
}

Channel* Scheduler::GetChannel() const
{
    return m_pImpl->m_Channel;
}

Status_t Scheduler::Listen(const std::string& address)
{
    return m_pImpl->m_Channel->Listen(address);
}

std::string Scheduler::GetLocalEndPoint() const
{
    return m_pImpl->m_Channel->GetLocalEndPoint();
}

bool Scheduler::IsValidEndPoint(const std::string& endpoint)
{
    return m_pImpl->m_Channel->IsValidEndPoint(endpoint);
}

bool Scheduler::RegisterStub(const Rpc::Stub* object, bool replace)
{
    return m_pImpl->m_LocalObjectTable.Register(object, replace);
}

bool Scheduler::UnregisterStub(const Rpc::Stub* object)
{
    return m_pImpl->m_LocalObjectTable.Unregister(object);
}

Status_t Scheduler::IssueInvoke(
    Proxy* proxy,                                   ///< �������
    const char* class_name,                         ///< ������
    const char* method_name,                        ///< ������
    int& method_id,                                 ///< ���� id
    SerializeBuffer& input,                         ///< �������
    const OutputParameterHandler& output_handler,   ///< ��������Ĵ�����
    AsyncToken* token,                              ///< �첽�������
    AsyncCallback callback,                         ///< �ص�֪ͨ����
    void* callback_context,                         ///< �ص�֪ͨ����ָ��
    void* callback_param,                           ///< ���� callback ������������
    int timeout                                     ///< ��ʱ
)
{
    InvokeId_t invoke_id = m_pImpl->m_PendingInvokeTable.Insert(
        class_name, method_name, method_id,
        output_handler, token,
        callback, callback_context, callback_param
    );

    // construct message header on buffer
    new (&input[0]) InvokeMessageHeader(proxy->RpcPid(), proxy->RpcObjectId(), invoke_id, method_id, method_name, input.size());

    if (timeout >= 0)
    {
        // ����һ��һ���Եĳ�ʱ��ʱ��
        Closure<void, uint64_t>* closure = NewClosure(this, &Scheduler::OnTimeout, invoke_id);
        unsigned long long  timer_id = m_pImpl->m_TimerManager.AddOneshotTimer((int64_t)timeout, closure);

        m_pImpl->m_PendingInvokeTable.SetTimerId(invoke_id, timer_id);
    }

    // ����
    const char* buffer = &input[0];
    size_t buffer_size = input.size();
    Status_t status = m_pImpl->m_Channel->Send("", proxy->RpcEndPoint(), buffer, buffer_size);

    // ת��״̬��
    if (status == Status_Success)
        return Status_Pending;
    else
        return status;
}

bool Scheduler::HandleMessage(
    const std::string& local_endpoint,
    const std::string& remote_endpoint,
    const void* data,
    size_t size
)
{
    UNALIGNED const MessageHeader* header = static_cast<const MessageHeader*>(data);

    if (size < sizeof(MessageHeader))
        return false;

    // ����ֽ���
    if (header->ByteOrder != ByteOrderTester)
    {
        // TODO: �����ֽ���ת��
        return false;
    }

    if (header->Length < sizeof(MessageHeader))
        return false;

    if (memcmp(header->Signature, "RPC", sizeof(header->Signature)) != 0)
        return false;

    switch (header->Type)
    {
    case MessageType_Invoke:
        return HandleInvokeMessage(local_endpoint, remote_endpoint, data, size);
        break;
    case MessageType_Return:
        return HandleReturnMessage(local_endpoint, remote_endpoint, data, size);
        break;
    default:
        break;
    }
    return false;
}

void Scheduler::HandleNestableInvoke(
        std::string local_endpoint,
        std::string remote_endpoint,
        const void* data,
        size_t size)
{
    UNALIGNED const InvokeMessageHeader* header = static_cast<const InvokeMessageHeader*>(data);
    const char* body = static_cast<const char*>(data) + header->HeaderLength;
    size_t body_size = header->Length - header->HeaderLength;
    m_pImpl->m_LocalObjectTable.Dispatch(
            local_endpoint,
            remote_endpoint,
            header->InvokeId,
            header->ObjectId,
            header->MethodName,
            header->MethodId,
            body,
            body_size
            );
    delete [] (const char*)data;
}

bool Scheduler::HandleInvokeMessage(
    const std::string& local_endpoint,
    const std::string& remote_endpoint,
    const void* data,
    size_t size
)
{
    UNALIGNED const InvokeMessageHeader* header = static_cast<const InvokeMessageHeader*>(data);

    if (header->HeaderLength < sizeof(InvokeMessageHeader))
        return false;

    // ��� PID �Ƿ����������֤�������ڵĽ����Ƿ񻹴�
    if (header->Pid != 0 && header->Pid != getpid())
    {
        ErrorReturn(local_endpoint, remote_endpoint, header->InvokeId, header->MethodId, Status_ProcessNotFound);
        return false;
    }

    if (m_pImpl->m_LocalObjectTable.IsNestableMethod(header->ObjectId, header->MethodName, header->MethodId))
    {
        char* buffer = new char[size];
        memcpy(buffer, data, size);
        Closure<void>* callback = NewClosure(this,
                &Scheduler::HandleNestableInvoke,
                local_endpoint,
                remote_endpoint,
                (const void*)buffer,
                size);
        m_ThreadPool.AddTask(callback);
    }
    else
    {
        const char* body = static_cast<const char*>(data) + header->HeaderLength;
        size_t body_size = header->Length - header->HeaderLength;
        // ������������
        m_pImpl->m_LocalObjectTable.Dispatch(
                local_endpoint,
                remote_endpoint,
                header->InvokeId,
                header->ObjectId,
                header->MethodName,
                header->MethodId,
                body,
                body_size
                );
    }
    return true;
}

bool Scheduler::HandleReturnMessage(
    const std::string& local_endpoint,
    const std::string& remote_endpoint,
    const void* data,
    size_t size
)
{
    UNALIGNED const ReturnMessageHeader* header = static_cast<const ReturnMessageHeader*>(data);

    const char* body = static_cast<const char*>(data) + header->HeaderLength;
    size_t body_size = header->Length - header->HeaderLength;

    CancelTimer(header->InvokeId);
    m_pImpl->m_PendingInvokeTable.HandleReturn(header->InvokeId, header->MethodId, (Status_t)header->Status, body, body_size);

    return true;
}

/// �������ˣ�ִ�з���
Status_t Scheduler::Return(
    const std::string& local_endpoint,
    const std::string& remote_endpoint,  ///< ������
    InvokeId_t invoke_id,               ///< ���� ID
    int method_id,                      ///< ���صķ��� ID
    SerializeBuffer& result,            ///< ���������
    Status_t status
)
{
    assert(status == Status_Success || status == Status_MethodFailure);
    // �����л�ʱԤ���Ļ������Ϲ���ͷ
    new (&result[0]) ReturnMessageHeader(invoke_id, method_id, status, result.size());

    const char* result_buffer = &result[0];
    size_t result_size = result.size();

    return m_pImpl->m_Channel->Send(local_endpoint, remote_endpoint, result_buffer, result_size);
}

/// �������ˣ�ִ�г�����
Status_t Scheduler::ErrorReturn(
    const std::string& local_endpoint,
    const std::string& remote_endpoint,
    InvokeId_t invoke_id,
    int method_id,
    Status_t status
)
{
    // ���󷵻ص�ʱ������Ϣ��
    ReturnMessageHeader header(invoke_id, method_id, status, sizeof(ReturnMessageHeader));
    // ���͸�ͨѶ��
    return m_pImpl->m_Channel->Send(local_endpoint, remote_endpoint, &header, sizeof(header));
}

///////////////////////////////////////////////////////////////////////////////
// ������ó�ʱ

bool Scheduler::CancelTimer(InvokeId_t invoke_id)
{
    long long timer_id = m_pImpl->m_PendingInvokeTable.GetTimerId(invoke_id);
    if (timer_id >= 0)
    {
        return m_pImpl->m_TimerManager.RemoveTimer(timer_id);
    }
    return false;
}

void Scheduler::OnTimeout(InvokeId_t invoke_id, uint64_t timer_id)
{
    m_pImpl->m_PendingInvokeTable.Abort(invoke_id, Status_Timeout);
}

///////////////////////////////////////////////////////////////////////////////
// ʵ�� EventHandler �ķ���

void Scheduler::OnReceived(
    const std::string& local_endpoint,
    const std::string& remote_endpoint,
    const void* buffer,
    size_t size
)
{
    HandleMessage(local_endpoint, remote_endpoint, buffer, size);
}

void Scheduler::OnSendFailed(
    const std::string& local_endpoint,
    const std::string& remote_endpoint,
    const void* buffer,
    size_t size
)
{
    UNALIGNED const MessageHeader* header = static_cast<const MessageHeader*>(buffer);
    switch (header->Type)
    {
    case MessageType_Invoke:
    {
        UNALIGNED const InvokeMessageHeader* invoke_header = static_cast<const InvokeMessageHeader*>(header);
        CancelTimer(invoke_header->InvokeId);
        m_pImpl->m_PendingInvokeTable.Abort(invoke_header->InvokeId, Status_ConnectionError);
    }
    break;
    case MessageType_Return:
        // ignore return failure, client will be timeout
        break;
    }
}

void Scheduler::OnConnected(const std::string& local_endpoint, const std::string& remote_endpoint)
{
}

void Scheduler::OnClosed(const std::string& local_endpoint, const std::string& remote_endpoint)
{
}

} // namespace Rpc
