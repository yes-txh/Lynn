#ifndef RPC_CHANNEL_HPP_INCLUDED
#define RPC_CHANNEL_HPP_INCLUDED

#include <common/rpc/types.hpp>

namespace Rpc
{

/// RPC ͨѶͨ������ӿڶ���
class Channel
{
public:
    /// �¼�������
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

    /// ���ӵ�ָ���� endpoint
    virtual Status_t Connect(const std::string& endpoint) = 0;

    /// �Ͽ�
    virtual Status_t Disconnect(const std::string& endpoint) = 0;

    /// ��ָ���� endpoint ������
    virtual Status_t Send(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,
        const void* buffer,
        size_t size
    ) = 0;

    /// ���ƶ��� endpoint �ϼ���
    virtual Status_t Listen(const std::string& endpoint) = 0;

    /// �����¼�������
    virtual EventHandler* SetEventHandler(EventHandler* handler) = 0;

    virtual std::string GetLocalEndPoint() = 0;

    virtual bool IsValidEndPoint(const std::string& endpoint) = 0;

    virtual bool WaitForSendingComplete(int timeout) = 0;

    virtual void SetPriority(int priority) = 0;
};

} // end namespace Rpc

#endif//RPC_CHANNEL_HPP_INCLUDED

