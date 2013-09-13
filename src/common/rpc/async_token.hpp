#ifndef RPC_ASYNC_TOKEN_HPP_INCLUDED
#define RPC_ASYNC_TOKEN_HPP_INCLUDED

#include <common/rpc/types.hpp>

namespace Rpc {

class PendingInvokeTable;

/// @brief �첽�������ơ������ڵ������֮ǰ�������ڽ��еĵ��ý��в�ѯ���ȴ���ȡ����
///        ���ú��÷���ֵ��
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

    /// ���ص����Ƿ����
    bool IsComplete() const;

    /// ��õ�ǰ����״̬
    Status_t Status() const;

    /// �ȴ�����
    Status_t Wait(int timeout = -1);

    /// ȡ������
    bool Cancel(Status_t status = Status_Canceled);

private:
    /// �ɿ�����첽�����յ�������Ϣ����ô��غ����ķ���ֵ
    virtual bool ParseResult(const char* buffer, size_t size) = 0;
public:
    /// �ȴ�����������
    static int WaitAny(AsyncToken** tokens, size_t count, int timeout = -1);

    /// �ȴ����е������
    static int WaitAll(AsyncToken** tokens, size_t count, int timeout = -1);

private:
    InvokeId_t m_InvokeId;                          ///< ���� ID ��
    volatile Status_t m_Status;                     ///< ��ǰ��״̬
};

// ������巵��ֵ���͵��첽�������
template <typename ResultType>
class AsyncTokenOf : public AsyncToken
{
private:
    // ���ܺ����ķ���ֵ
    virtual bool ParseResult(const char* buffer, size_t size)
    {
        return UnserializeObject(buffer, size, m_Result) && size == 0;
    }

public:
    // ��ɺ�������ú����ķ���ֵ
    ResultType& Result()
    {
        assert(Status() == Status_Success);
        return m_Result;
    }
private:
    ResultType m_Result;    ///< ���ڱ��溯������ֵ
};

// �� void ����ֵ���ػ�
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

