#ifndef LOOPBACK_RPC_CHANNEL_HPP
#define LOOPBACK_RPC_CHANNEL_HPP

#include <common/rpc/channel.hpp>

class LoopbackRpcChannel : public Rpc::Channel
{
public:
    LoopbackRpcChannel() : m_EventHandler(NULL)
    {
    }

    /// 连接到指定的 endpoint
    virtual Rpc::Status_t Connect(const std::string& endpoint)
    {
        return Rpc::Status_Success;
    }

    /// 断开
    virtual Rpc::Status_t Disconnect(const std::string& endpoint)
    {
        return Rpc::Status_Success;
    }

    /// 向指定的 endpoint 发数据
    virtual Rpc::Status_t Send(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,
        const void* buffer,
        size_t size
    )
    {
        m_EventHandler->OnReceived(remote_endpoint, local_endpoint, buffer, size);
        return Rpc::Status_Success;
    }

    /// 在制定的 endpoint 上监听
    virtual Rpc::Status_t Listen(const std::string& endpoint)
    {
        return Rpc::Status_Success;
    }

    /// 设置事件处理器
    virtual Rpc::Channel::EventHandler* SetEventHandler(Rpc::Channel::EventHandler* handler)
    {
        Rpc::Channel::EventHandler* old = m_EventHandler;
        m_EventHandler = handler;
        return old;
    }

    std::string GetLocalEndPoint()
    {
        return "loopback";
    }

    bool IsValidEndPoint(const std::string& endpoint)
    {
        return true;
    }

    bool WaitForSendingComplete(int timeout = 1000)
    {
        return true;
    }

    void SetPriority(int) {}
private:
    Rpc::Channel::EventHandler* m_EventHandler;
};

#endif//LOOPBACK_RPC_CHANNEL_HPP
