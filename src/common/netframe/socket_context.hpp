#ifndef COMMON_NETFRAME_SOCKET_CONTEXT_HPP
#define COMMON_NETFRAME_SOCKET_CONTEXT_HPP

#include <string>
#include <deque>
#include <vector>
#include "common/base/uncopyable.hpp"
#include "common/system/concurrency/mutex.hpp"
#include "common/netframe/packet.hpp"
#include "common/netframe/socket_handler.hpp"
#include "common/system/memory/mempool.hpp"
#include "common/netframe/netframe.hpp"
#include "common/netframe/command_queue.hpp"

namespace netframe {

struct IoEvent;
struct CommandEvent;
class  EventPoller;

class SocketContext
{
public:
    SocketContext(
        NetFrame* netframe,
        const SocketAddress* local_address,
        const SocketAddress* remote_address,
        SocketId sock_id,
        size_t  max_packet_size,
        const NetFrame::EndPointOptions& options
    );
    virtual ~SocketContext();

    void SetEventPoller(EventPoller* event_poller)
    {
        m_EventPoller = event_poller;
    }

    /// 处理到达的Socket事件
    /// @retval >0 事件已被处理
    /// @retval =0 事件未被处理
    /// @retval <0 致命错误
    int ProcessIoEvent(const IoEvent& event);

    /// @brief 处理命令
    /// @retval >0 事件已被处理
    /// @retval =0 事件未被处理
    /// @retval <0 致命错误
    int ProcessCommandEvent(const CommandEvent& event);

    void UpdateEventRequest(); ///< 事件处理完毕之后的操作

    SocketId GetSockId() const
    {
        return m_SockId;
    }

    int GetFd() const
    {
        return m_SockId.SockFd;
    }

    void Close();           ///< 关闭Socket，清理退出

protected: // 派生类可重载的
    /// 处理到达的Socket事件
    /// @param event_mask 期望的事件掩码
    /// @retval >0 事件已被处理
    /// @retval =0 事件未被处理
    /// @retval <0 致命错误
    virtual int HandleIoEvent(const IoEvent& event) = 0;

    /// @brief 处理命令
    /// @param event_mask 期望的事件掩码
    /// @retval >0 事件已被处理
    /// @retval =0 事件未被处理
    /// @retval <0 致命错误
    virtual int HandleCommandEvent(const CommandEvent& event) = 0;

    virtual unsigned int GetWantedEventMask() const;

    void SetEventHandler(SocketHandler* handler)
    {
        m_EventHandler = handler;
    }

    virtual SocketHandler* GetEventHandler() const ///< 该Socket上的事件处理器
    {
        return m_EventHandler;
    }

    void DecreaseBufferedLength(size_t size)
    {
        m_NetFrame->DecreaseBufferedLength(size);
    }

    void DecreaseBufferedPacket()
    {
        m_NetFrame->DecreaseBufferedPacket();
    }

    virtual void HandleClose();             ///< 关闭Socket的清理操作

    void HandleSendingFailed(Packet* packet, int error_code);
    void ClearCommandQueue();

protected:
    NetFrame* m_NetFrame;
    EventPoller* m_EventPoller;
    SocketHandler* m_EventHandler;
    SocketId m_SockId;                      ///< socket标识
    NetFrame::EndPointOptions m_EndPointOptions;
    size_t m_MaxPacketLength;

    SocketAddressStorage m_LocalAddress;    ///< Socket的本地地址
    SocketAddressStorage m_RemoteAddress;   ///< 远端的地址

    unsigned int m_RequestedEvent;          ///< 上次所请求的Socket事件
    bool m_IsFirstRequested;                ///< 是否第一次请求Socket事件
    int m_LastError;                        ///< 出错时最后的错误码

    CommandQueue m_CommandQueue; ///< 命令队列
    DECLARE_UNCOPYABLE(SocketContext);
};

} // namespace netframe

#endif // COMMON_NETFRAME_SOCKET_CONTEXT_HPP
