#ifndef PENDING_INVOKE_TABLE_HPP
#define PENDING_INVOKE_TABLE_HPP

#include <common/rpc/types.hpp>
#include <common/rpc/proxy.hpp>
#include <common/system/concurrency/thread_pool.hpp>
#include <map>

namespace Rpc
{

class AsyncToken;

/// 未决调用表
/// 未决调用表保存着所有已经发起但是尚未完成的调用的信息，每次插入均会生成一个
/// 调用 ID，可以 ID 为 key 进行对调用状态进行查询和修改。
class PendingInvokeTable
{
    /// 通知信息
    struct Notify
    {
        ConditionVariable* Cond;                ///< 条件变量
        AsyncCallback Callback;                 ///< 回调函数的地址
        void* Context;                          ///< 异步完成回调函数的 context 参数
        void* Param;                            ///< 异步完成回调函数的 param 参数
        /// 通知完成
        void Signal()
        {
            if (Cond)
                Cond->Signal();
        }
        void ExecuteCallback(Status_t status)
        {
            if (Callback)
            {
                Callback(Context, status, Param);
            }
        }
    };

    /// 表项
    struct PendingInvoke
    {
        const char* ClassName;
        const char* MethodName;
        volatile Status_t* StatusPtr;           ///< 指向 Token->m_Status，如果 Token 为空，指向 StatusPlaceHolder
        volatile Status_t StatusPlaceHolder;    ///< Token 为空的时候用来被 StatusPtr 指向
        int* MethodId;                          ///< 指向方法 ID 的指针，返回时填充
        OutputParameterHandler OutputHandler;   ///< 处理出参数的处理器
        AsyncToken* Token;                      ///< 所关联的 Token
        struct Notify Notify;
        long long TimerId; ///< 保存的定时器 ID
        bool CallbackScheduled; ///< 已处于回调状态，不能删除
    public:
        ~PendingInvoke();
        /// 获得当前 Status
        Status_t Status() const
        {
            return *StatusPtr;
        }

        /// 设置 status
        void SetStatus(Status_t status)
        {
            *StatusPtr = status;
        }
    };

public:
    PendingInvokeTable();

    /// 添加一个调用项
    InvokeId_t Insert(
        const char* class_name,
        const char* method_name,
        int& method_id,                                 ///< 函数 id
        const OutputParameterHandler& output_handler,   ///< 输出参数的处理器
        AsyncToken* token,                              ///< 异步完成令牌
        AsyncCallback callback,                         ///< 回调通知函数
        void* object,                                   ///< 回调通知对象指针
        void* param                                     ///< 传给 callback 的任意额外参数
    );

    /// 从表中删除
    bool Remove(InvokeId_t id);

    /// 终止一个调用
    bool Abort(InvokeId_t id, Status_t status);

    /// 等待一个调用
    bool Wait(AsyncToken* token, int timeout);

    /// 等待任意一个调用完成
    bool WaitAny(AsyncToken** tokens, size_t count, int timeout);

    /// 等待所有指定的调用完成
    bool WaitAll(AsyncToken** tokens, size_t count, int timeout);

    /// 处理返回消息
    void HandleReturn(InvokeId_t invoke_id, int method_id, Status_t status, const char* message, size_t size);

    long long GetTimerId(InvokeId_t invoke_id) const;
    bool SetTimerId(InvokeId_t invoke_id, long long timer_id);
private:
    /// 反序列化结果
    bool UnserializeResult(
        InvokeId_t invoke_id,
        PendingInvoke& invoke,
        const char* message,
        size_t size
    );

    /// 反序列化函数返回值
    bool UnserializeReturnValue(
        InvokeId_t invoke_id,
        PendingInvoke& invoke,
        const char* message,
        size_t size
    );

    /// 为一组 token 设置条件变量
    /// @return 是否完全成功
    bool SetCond(AsyncToken** tokens, size_t count, ConditionVariable* cond);

    /// 完成后在线程池中执行用户的回调函数
    void ExecuteCallback(InvokeId_t invoke_id);
private:
    mutable SimpleMutex m_Mutex;
    InvokeId_t m_NextInvokeId;
    typedef std::map<InvokeId_t, PendingInvoke> MapType;
    std::map<InvokeId_t, PendingInvoke> m_Table;
    ThreadPool* m_CallbackThreadPool;
};

} // namespace Rpc

#endif//PENDING_INVOKE_TABLE_HPP
