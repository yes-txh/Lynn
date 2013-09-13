#ifndef RPC_SCHEDULER_HPP_INCLUDED
#define RPC_SCHEDULER_HPP_INCLUDED

/// @file Scheduler.hpp
/// @brief RPC 调度器类

#include <string>
#include <common/base/stdint.h>
#include <common/rpc/types.hpp>
#include <common/rpc/channel.hpp>
#include <common/rpc/serialization.hpp>
#include <common/system/concurrency/thread_pool.hpp>

namespace Rpc
{

class Proxy;
class Stub;
class OutputParameterHandler;
class AsyncToken;

class SchedulerImpl;

/// RPC 调度器类，处理整个 RPC 底层的调度，超时等
class Scheduler : private Channel::EventHandler
{
public:
    Scheduler();
    ~Scheduler();

private:
    Scheduler(const Scheduler&);
    Scheduler& operator=(const Scheduler&);

public:
    /// 把 RPC 存根注册到本地
    bool RegisterStub(const Rpc::Stub* object, bool replace = false);

    /// 注销 Stub
    bool UnregisterStub(const Rpc::Stub* object);

    /// 得到线程池指针
    ThreadPool* GetThreadPool()
    {
        return &m_ThreadPool;
    }

    /// 客户端：发出调用
    Status_t IssueInvoke(
        Proxy* proxy,                                   ///< 对象代理
        const char* class_name,                         ///< 函数名
        const char* method_name,                        ///< 函数名
        int& method_id,                                 ///< 函数 id
        SerializeBuffer& input,                         ///< 输入参数
        const OutputParameterHandler& output_handler,   ///< 输出参数的处理器
        AsyncToken* token,                              ///< 异步完成令牌
        AsyncCallback callback,                         ///< 回调通知函数
        void* callback_context,                         ///< 回调通知对象指针
        void* callback_param,                           ///< 传给 callback 的任意额外参数
        int timeout                                     ///< 超时
    );

    /// 处理收到的消息
    bool HandleMessage(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,
        const void* data,
        size_t size
    );

    /// 服务器端：执行返回
    Status_t Return(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,  ///< 调用者
        InvokeId_t invoke_id,               ///< 调用 ID
        int method_id,                      ///< 返回的方法 ID
        SerializeBuffer& result,            ///< 输出缓冲区
        Status_t status
    );

    /// 服务器端：执行出错返回
    Status_t ErrorReturn(
        const std::string& local_endpoint,
        const std::string& endpoint,     ///< 调用者
        InvokeId_t invoke_id,           ///< 调用 ID
        int method_id,                  ///< 返回的方法 ID
        Status_t status                 ///< 状态码
    );

    /// 删除活动调用
    bool RemoveInvoke(InvokeId_t invoke_id);

    /// 终止调用
    bool AbortInvoke(InvokeId_t invoke_id, Status_t status);

    /// 等待调用完成
    bool WaitInvoke(AsyncToken* token, int timeout);

    /// 等待任意调用完成
    /// @return 等到的个数
    /// @retval -1 出现错误
    int WaitAnyInvoke(AsyncToken** tokens, size_t count, int timeout);

    /// 等待所有调用完成
    /// @return 等到的个数
    /// @retval -1 出现错误
    int WaitAllInvokes(AsyncToken** tokens, size_t count, int timeout);

    /// 设置通讯通道
    /// @return 先前的通讯通道
    Channel* SetChannel(Channel* channel);
    Channel* GetChannel() const;

    Status_t Listen(const std::string& address);

    std::string GetLocalEndPoint() const;

    bool IsValidEndPoint(const std::string& endpoint);

    /// 唯一的单体
    static Scheduler& Instance()
    {
        static Scheduler* instance = new Scheduler;
        return *instance;
    }

    /// 等待所有的包发送完毕
    /// @param timeout 等待时间，毫秒，默认为10s
    bool WaitForSendingComplete(int timeout = 10000);

private: // 实现 EventHandler 的方法
    virtual void OnReceived(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,
        const void* buffer, size_t size
    );
    virtual void OnSendFailed(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,
        const void* buffer, size_t size
    );
    virtual void OnConnected(
        const std::string& local_endpoint,
        const std::string& remote_endpoint
    );
    virtual void OnClosed(
        const std::string& local_endpoint,
        const std::string& remote_endpoint
    );

private:
    /// 处理调用消息
    bool HandleInvokeMessage(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,
        const void* data,
        size_t size
    );

    /// 处理嵌套调用
    void HandleNestableInvoke(
        std::string local_endpoint,
        std::string remote_endpoint,
        const void* data,
        size_t size
    );

    /// 处理返回消息
    bool HandleReturnMessage(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,
        const void* data,
        size_t size
    );

    /// 处理超时
    bool CancelTimer(InvokeId_t invoke_id);
    void OnTimeout(InvokeId_t invoke_id, uint64_t timer_id);

private:
    ThreadPool m_ThreadPool;                ///< 线程池
    SchedulerImpl* m_pImpl;
};

} // namespace Rpc

#endif//RPC_SCHEDULER_HPP_INCLUDED
