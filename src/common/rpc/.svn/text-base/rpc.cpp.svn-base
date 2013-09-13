#define _CRT_NONSTDC_NO_WARNINGS 1

#include <common/rpc/rpc.hpp>

#include <string>

#ifdef _WIN32
#include <process.h>
#elif defined unix
#include <unistd.h>
#endif

#include "common/rpc/scheduler.hpp"
#include "common/rpc/netframe_channel/netframe_channel.hpp"

namespace Rpc
{

static Channel* DoGetDefaultChannel(int netframe_workthread_num = 4)
{
    static NetframeChannel* channel = new NetframeChannel(netframe_workthread_num);
    return channel;
}

Channel* GetDefaultChannel(int netframe_workthread_num = 4)
{
    return DoGetDefaultChannel(netframe_workthread_num);
}

Status_t Initialize(int netframe_workthread_num)
{
    return Initialize(GetDefaultChannel(netframe_workthread_num));
}

Status_t Initialize(Channel* channel)
{
    Scheduler::Instance().SetChannel(channel);
    return Status_Success;
}

Status_t ShutDown(int timeout)
{
    bool ret = Scheduler::Instance().WaitForSendingComplete(timeout);
    if (!ret)
    {
        return Status_Timeout;
    }
    return Status_Success;
}

std::string GetLocalEndPoint()
{
    return Scheduler::Instance().GetLocalEndPoint();
}

Status_t Listen(const std::string& address)
{
    return Scheduler::Instance().Listen(address);
}

Status_t GetRemoteObject(const std::string& endpoint, ObjectId_t object_id, Proxy& proxy)
{
    if (!Scheduler::Instance().IsValidEndPoint(endpoint))
    {
        return Status_InvalidEndPoint;
    }
    proxy.RpcBind(endpoint, 0, object_id);
    return Status_Success;
}

void SetNetworkPriority(int priority)
{
    Scheduler::Instance().GetChannel();
}

}
