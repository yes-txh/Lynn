#include <assert.h>
#include "common/netframe/event_poller.hpp"
#include "common/netframe/netframe.hpp"
#include "common/netframe/command_event.hpp"
#include "common/netframe/work_thread.hpp"
#include "common/netframe/socket_context.hpp"
#include "common/system/concurrency/thread.hpp"

namespace netframe {

SocketContext::SocketContext(
    NetFrame* netframe,
    const SocketAddress* local_address,
    const SocketAddress* remote_address,
    SocketId sock_id,
    size_t max_packet_size,
    const NetFrame::EndPointOptions& options
) : m_NetFrame(netframe),
    m_EventPoller(NULL),
    m_EventHandler(NULL),
    m_SockId(sock_id),
    m_EndPointOptions(options),
    m_MaxPacketLength(max_packet_size),
    m_RequestedEvent(0),
    m_IsFirstRequested(true),
    m_LastError(0),
    m_CommandQueue(options.MaxCommandQueueLength())
{
    socklen_t address_length;

    if (local_address)
    {
        m_LocalAddress = *local_address;
    }
    else
    {
        address_length = m_LocalAddress.Capacity();
        getsockname(GetFd(), m_LocalAddress.Address(), &address_length);
        m_LocalAddress.SetLength(address_length);
    }

    if (remote_address)
    {
        m_RemoteAddress = *remote_address;
    }
    else
    {
        address_length = m_RemoteAddress.Capacity();
        getpeername(GetFd(), m_RemoteAddress.Address(), &address_length);
        m_RemoteAddress.SetLength(address_length);
    }
}

SocketContext::~SocketContext()
{
    ClearCommandQueue();
    int fd = GetFd();
    if (fd >= 0)
    {
        NetFrame::CloseFd(fd);
    }
    delete m_EventHandler;
    m_EventHandler = NULL;
}

unsigned int SocketContext::GetWantedEventMask() const
{
    unsigned int event_mask = IoEvent_Readable;
    if (!m_CommandQueue.IsEmpty())
        event_mask |= IoEvent_Writeable;
    return event_mask;
}

void SocketContext::HandleSendingFailed(Packet* packet, int error_code)
{
    DecreaseBufferedLength(packet->Length());
    DecreaseBufferedPacket();
    if (GetEventHandler()->OnSendingFailed(packet, error_code))
        delete packet;
}

void SocketContext::ClearCommandQueue()
{
    size_t total_length = 0;
    CommandEvent event;
    while (m_CommandQueue.GetFront(&event))
    {
        if (event.Type == CommandEvent_SendPacket ||
            event.Type == CommandEvent_SendUrgentPacket)
        {
            Packet* packet = event.Cookie.CastTo<Packet*>();
            total_length += packet->Length();
            delete packet;
            DecreaseBufferedPacket();
        }
        m_CommandQueue.PopFront();
    }
    DecreaseBufferedLength(total_length);
}

void SocketContext::Close()
{
    HandleClose();
}

// 关闭该Socket
void SocketContext::HandleClose()
{
    // 关闭端点时，清空命令队列
    CommandEvent event;
    while (m_CommandQueue.GetFront(&event))
    {
        if (event.Type == CommandEvent_SendPacket ||
            event.Type == CommandEvent_SendUrgentPacket)
        {
            Packet* packet = event.Cookie.CastTo<Packet*>();
            HandleSendingFailed(packet, ESHUTDOWN);
        }
        m_CommandQueue.PopFront();
    }
    int fd = GetFd();
    GetEventHandler()->OnClose(m_LastError);
    m_EventPoller->ClearEventRequest(fd);
    NetFrame::CloseFd(fd);
    m_SockId.Id = -1;
    GetEventHandler()->SetEndPointId(m_SockId);
}

// 处理到达的Socket事件
int SocketContext::ProcessIoEvent(const IoEvent& event)
{
    int result = HandleIoEvent(event);
    if (result > 0)
        UpdateEventRequest();
    else if (result < 0)
        Close();
    return result;
}

int SocketContext::ProcessCommandEvent(const CommandEvent& event)
{
    int result = HandleCommandEvent(event);
    if (result > 0)
        UpdateEventRequest();
    else if (result < 0)
        Close();
    return result;
}

//事件处理完毕之后的操作
void SocketContext::UpdateEventRequest()
{
    unsigned int event_mask = GetWantedEventMask();
    assert(event_mask != 0);

    if (event_mask != m_RequestedEvent)
    {
        bool  request_success = false;
        if (m_IsFirstRequested)
        {
            m_IsFirstRequested = false;
            request_success = m_EventPoller->RequestEvent(GetFd(), event_mask);
        }
        else
        {
            request_success = m_EventPoller->RerequestEvent(GetFd(), event_mask);
        }
        m_RequestedEvent = event_mask;

        if (!request_success)
        {
            // 请求事件失败，Socket 需要被关闭
            Close();
        }
    }
}

} // namespace netframe
