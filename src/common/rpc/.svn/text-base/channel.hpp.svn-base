#ifndef RPC_CHANNEL_HPP_INCLUDED
#define RPC_CHANNEL_HPP_INCLUDED

#include <common/rpc/types.hpp>

namespace Rpc
{

/// RPC 通讯通道抽象接口定义
class Channel
{
public:
    /// 事件处理器
    struct EventHandler
    {
        virtual ~EventHandler() {}

        virtual void OnReceived(
            const std::string& local_endpoint,
            const std::string& remote_endpoint,
            const void* buffer,
            size_t size
        ) = 0;

        virtual void OnSendFailed(
            const std::string& local_endpoint,
            const std::string& remote_endpoint,
            const void* buffer,
            size_t size
        ) = 0;

        virtual void OnConnected(
            const std::string& local_endpoint,
            const std::string& remote_endpoint
        ) = 0;

        virtual void OnClosed(
            const std::string& local_endpoint,
            const std::string& remote_endpoint
        ) = 0;
    };
public:
    virtual ~Channel() {}

    /// 连接到指定的 endpoint
    virtual Status_t Connect(const std::string& endpoint) = 0;

    /// 断开
    virtual Status_t Disconnect(const std::string& endpoint) = 0;

    /// 向指定的 endpoint 发数据
    virtual Status_t Send(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,
        const void* buffer,
        size_t size
    ) = 0;

    /// 在制定的 endpoint 上监听
    virtual Status_t Listen(const std::string& endpoint) = 0;

    /// 设置事件处理器
    virtual EventHandler* SetEventHandler(EventHandler* handler) = 0;

    virtual std::string GetLocalEndPoint() = 0;

    virtual bool IsValidEndPoint(const std::string& endpoint) = 0;

    virtual bool WaitForSendingComplete(int timeout) = 0;

    virtual void SetPriority(int priority) = 0;
};

} // end namespace Rpc

#endif//RPC_CHANNEL_HPP_INCLUDED

