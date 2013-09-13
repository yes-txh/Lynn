#include <common/rpc/proxy.hpp>
#include "common/rpc/scheduler.hpp"
#include "common/rpc/pending_invoke_table.hpp"
#include <assert.h>
#include <string>
#include <map>

namespace Rpc
{

///////////////////////////////////////////////////////////////////////////////
// 实现 Proxy 类

bool UnserializeProxy(const char*& buffer, size_t& size, Proxy& object)
{
    std::string endpoint;
    int pid;
    ObjectId_t object_id;
    if (UnserializeObject(buffer, size, endpoint) &&
            UnserializeObject(buffer, size, pid) &&
            UnserializeObject(buffer, size, object_id)
       )
    {
        object.RpcBind(endpoint, pid, object_id);
        return true;
    }
    return false;
}

// 异步调用
Status_t Proxy::AsyncInvoke(
    const char* method_name,                        /// 函数名
    int& method_id,                                 /// 函数 id
    SerializeBuffer& input,                         /// 输入参数
    const OutputParameterHandler& output_handler,   /// 输出参数的处理器
    AsyncToken* token,                              /// 异步完成令牌
    AsyncCallback callback,                         /// 回调通知函数
    void* callback_context,                         /// 回调通知对象指针
    void* callback_param,                           /// 传给 callback 的任意额外参数
    int timeout                                     /// 超时，单位毫秒
)
{
    // 请求调度器调度本次调用
    return Scheduler::Instance().IssueInvoke(
               this,
               GetClassName(),
               method_name,
               method_id,
               input,
               output_handler,
               token,
               callback,
               callback_context,
               callback_param,
               timeout
           );
}

} // end namespace Rpc
