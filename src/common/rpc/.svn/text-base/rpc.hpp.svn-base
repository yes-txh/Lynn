#ifndef RPC_HPP_INCLUDED
#define RPC_HPP_INCLUDED

#include <common/rpc/types.hpp>
#include <common/rpc/channel.hpp>
#include <common/rpc/stub.hpp>
#include <common/rpc/proxy.hpp>

/// RPC 命名空间，所有的 RPC 接口都在 Rpc 命名空间之下
namespace Rpc
{

Status_t Initialize(int netframe_workthread_num = 4);

Status_t Initialize(Channel* channel);

std::string GetLocalEndPoint();

Status_t Listen(const std::string& address);

/// @brief RPC shut down.
/// @param timeout millseconds. wait until all rpc packets are sent, default wait 10 sencods.
Status_t ShutDown(int timeout = 10000);

Status_t GetRemoteObject(const std::string& endpoint, ObjectId_t object_id, Proxy& proxy);

/// @param priority should be 0-6, the larger value represent the higher priority
void SetNetworkPriority(int priority);

}

#endif//RPC_HPP_INCLUDED
