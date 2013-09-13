#ifndef RPC_ASYNC_TOKEN_HPP_INCLUDED
#define RPC_ASYNC_TOKEN_HPP_INCLUDED

#include <common/rpc/types.hpp>

namespace Rpc {

class PendingInvokeTable;

/// @brief 异步调用令牌。用于在调用完成之前，对正在进行的调用进行查询，等待，取消，
///        调用后获得返回值。
class AsyncToken
{
    friend class PendingInvokeTable;
protected:
    AsyncToken() :
        m_InvokeId(-1),
        m_Status(Status_InvalidInvokeId)
    {
    }
public:
    virtual ~AsyncToken();

private:
    AsyncToken(const AsyncToken&);
    AsyncToken& operator=(const AsyncToken&);

public:
    InvokeId_t InvokeId() const
    {
        return m_InvokeId;
    }

    bool IsValid() const
    {
        return m_InvokeId >= 0;
    }

    /// 返回调用是否完成
    bool IsComplete() const;

    /// 获得当前进行状态
    Status_t Status() const;

    /// 等待调用
    Status_t Wait(int timeout = -1);

    /// 取消调用
    bool Cancel(Status_t status = Status_Canceled);

private:
    /// 由框架在异步调用收到返回消息后调用传回函数的返回值
    virtual bool ParseResult(const char* buffer, size_t size) = 0;
public:
    /// 等待任意调用完成
    static int WaitAny(AsyncToken** tokens, size_t count, int timeout = -1);

    /// 等待所有调用完成
    static int WaitAll(AsyncToken** tokens, size_t count, int timeout = -1);

private:
    InvokeId_t m_InvokeId;                          ///< 调用 ID 号
    volatile Status_t m_Status;                     ///< 当前的状态
};

// 处理具体返回值类型的异步完成令牌
template <typename ResultType>
class AsyncTokenOf : public AsyncToken
{
private:
    // 接受函数的返回值
    virtual bool ParseResult(const char* buffer, size_t size)
    {
        return UnserializeObject(buffer, size, m_Result) && size == 0;
    }

public:
    // 完成后用来获得函数的返回值
    ResultType& Result()
    {
        assert(Status() == Status_Success);
        return m_Result;
    }
private:
    ResultType m_Result;    ///< 用于保存函数返回值
};

// 对 void 返回值的特化
template <>
class AsyncTokenOf<void> : public AsyncToken
{
private:
    virtual bool ParseResult(const char* buffer, size_t size)
    {
        return size == 0;
    }

public:
    void Result()
    {
    }
};

} // end namespace Rpc

#endif//RPC_ASYNC_TOKEN_HPP_INCLUDED

