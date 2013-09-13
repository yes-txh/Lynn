#ifndef INVOKE_CONTEXT_HPP
#define INVOKE_CONTEXT_HPP

#include <common/rpc/types.hpp>
#include <common/rpc/serialization.hpp>

namespace Rpc
{

struct InvokeInfo;

/// 异步的方式实现 RPC 方法时，实现函数得到的异步调用的上下文，
/// 用于提取调用参数和返回调用结果。函数原型为：
/// @code
/// Rpc::Status_t MethodName(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer);
/// @endcode
/// 可以调用 context.ExtractInputParameters(a1, a2, ...) 提取参数，参数个数和顺序必须跟
/// 实际入参数顺序一致。完成操作时，在任意时候，调用 context.Complete(a1, a2...)
/// 来完成调用，参数顺序必须和出参数顺序完全一致，如果函数返回值类型不为 void，
/// 返回值作为 Complete 的作为最后一个参数来传递。
/// 如果函数体中执行出错，直接返回错误码即可。但是如果是函数返回后， context
/// 被放到别的地方异步完成，此时失败需要自己调用 context.ErrorReturn。
class InvokeContext
{
public:
    InvokeContext();
    InvokeContext(const InvokeInfo& invoke_info);

public:
    /// @brief 从 buffer 中取出实际函数接口定义的所有入参数
    /// @tparam ArgType1 参数1的类型
    /// @param buffer 输入缓冲区
    /// @param a 参数
    /// @return 返回状态
    template <typename ArgType>
    Status_t ExtractInputParameters(const Rpc::Buffer& buffer, ArgType& a) const
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();
        if (!ExtractParameter(p, size, a)) return Status_RemoteInvalidInputArguments;
        if (size != 0) return Status_RemoteInvalidInputArguments;
        return Status_Success;
    }

    /// @brief 从 buffer 中取出实际函数接口定义的所有入参数
    /// @tparam ArgType1 参数1的类型
    /// @tparam ArgType2 参数2的类型
    /// @param buffer 输入缓冲区
    /// @param a1 参数1
    /// @param a2 参数2
    /// @return 返回状态
    template <typename ArgType1, typename ArgType2>
    Status_t ExtractInputParameters(const Rpc::Buffer& buffer, ArgType1& a1, ArgType2& a2) const
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();
        if (!ExtractParameter(p, size, a1)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a2)) return Status_RemoteInvalidInputArguments;
        if (size != 0) return Status_RemoteInvalidInputArguments;
        return Status_Success;
    }

    /// @brief 从 buffer 中取出实际函数接口定义的所有入参数
    /// @tparam ArgType1 参数1的类型
    /// @tparam ArgType2 参数2的类型
    /// @tparam ArgType3 参数3的类型
    /// @param buffer 输入缓冲区
    /// @param a1 参数1
    /// @param a2 参数2
    /// @param a3 参数3
    /// @return 返回状态
    template <typename ArgType1, typename ArgType2, typename ArgType3>
    Status_t ExtractInputParameters(const Rpc::Buffer& buffer, ArgType1& a1, ArgType2& a2, ArgType3& a3) const
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();
        if (!ExtractParameter(p, size, a1)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a2)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a3)) return Status_RemoteInvalidInputArguments;
        if (size != 0) return Status_RemoteInvalidInputArguments;
        return Status_Success;
    }

    /// @brief 从 buffer 中取出实际函数接口定义的所有入参数
    /// @tparam ArgType1 参数1的类型
    /// @tparam ArgType2 参数2的类型
    /// @tparam ArgType3 参数3的类型
    /// @tparam ArgType4 参数4的类型
    /// @param buffer 输入缓冲区
    /// @param a1 参数1
    /// @param a2 参数2
    /// @param a3 参数3
    /// @param a4 参数4
    /// @return 返回状态
    template <typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4>
    Status_t ExtractInputParameters(
        const Rpc::Buffer& buffer,
        ArgType1& a1, ArgType2& a2, ArgType3& a3, ArgType4& a4
    ) const
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();
        if (!ExtractParameter(p, size, a1)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a2)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a3)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a4)) return Status_RemoteInvalidInputArguments;
        if (size != 0) return Status_RemoteInvalidInputArguments;
        return Status_Success;
    }

    /// @brief 从 buffer 中取出实际函数接口定义的所有入参数
    /// @tparam ArgType1 参数1的类型
    /// @tparam ArgType2 参数2的类型
    /// @tparam ArgType3 参数3的类型
    /// @tparam ArgType4 参数4的类型
    /// @tparam ArgType5 参数5的类型
    /// @param buffer 输入缓冲区
    /// @param a1 参数1
    /// @param a2 参数2
    /// @param a3 参数3
    /// @param a4 参数4
    /// @param a5 参数5
    /// @return 返回状态
    template <typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename ArgType5>
    Status_t ExtractInputParameters(
        const Rpc::Buffer& buffer,
        ArgType1& a1, ArgType2& a2, ArgType3& a3, ArgType4& a4, ArgType5& a5
    ) const
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();
        if (!ExtractParameter(p, size, a1)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a2)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a3)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a4)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a5)) return Status_RemoteInvalidInputArguments;
        if (size != 0) return Status_RemoteInvalidInputArguments;
        return Status_Success;
    }

    /// @brief 从 buffer 中取出实际函数接口定义的所有入参数
    /// @tparam ArgType1 参数1的类型
    /// @tparam ArgType2 参数2的类型
    /// @tparam ArgType3 参数3的类型
    /// @tparam ArgType4 参数4的类型
    /// @tparam ArgType5 参数5的类型
    /// @tparam ArgType6 参数6的类型
    /// @param buffer 输入缓冲区
    /// @param a1 参数1
    /// @param a2 参数2
    /// @param a3 参数3
    /// @param a4 参数4
    /// @param a5 参数5
    /// @param a6 参数6
    /// @return 返回状态
    template <typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename ArgType5, typename ArgType6>
    Status_t ExtractInputParameters(
        const Rpc::Buffer& buffer,
        ArgType1& a1, ArgType2& a2, ArgType3& a3, ArgType4& a4, ArgType5& a5, ArgType6& a6
    ) const
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();
        if (!ExtractParameter(p, size, a1)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a2)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a3)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a4)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a5)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a6)) return Status_RemoteInvalidInputArguments;
        if (size != 0) return Status_RemoteInvalidInputArguments;
        return Status_Success;
    }

    /// @brief 从 buffer 中取出实际函数接口定义的所有入参数
    /// @tparam ArgType1 参数1的类型
    /// @tparam ArgType2 参数2的类型
    /// @tparam ArgType3 参数3的类型
    /// @tparam ArgType4 参数4的类型
    /// @tparam ArgType5 参数5的类型
    /// @tparam ArgType6 参数6的类型
    /// @tparam ArgType7 参数7的类型
    /// @param buffer 输入缓冲区
    /// @param a1 参数1
    /// @param a2 参数2
    /// @param a3 参数3
    /// @param a4 参数4
    /// @param a5 参数5
    /// @param a6 参数6
    /// @param a7 参数7
    /// @return 返回状态
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7
             >
    Status_t ExtractInputParameters(
        const Rpc::Buffer& buffer,
        ArgType1& a1, ArgType2& a2, ArgType3& a3, ArgType4& a4, ArgType5& a5, ArgType6& a6, ArgType7& a7
    ) const
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();
        if (!ExtractParameter(p, size, a1)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a2)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a3)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a4)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a5)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a6)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a7)) return Status_RemoteInvalidInputArguments;
        if (size != 0) return Status_RemoteInvalidInputArguments;
        return Status_Success;
    }

    /// @brief 从 buffer 中取出实际函数接口定义的所有入参数
    /// @tparam ArgType1 参数1的类型
    /// @tparam ArgType2 参数2的类型
    /// @tparam ArgType3 参数3的类型
    /// @tparam ArgType4 参数4的类型
    /// @tparam ArgType5 参数5的类型
    /// @tparam ArgType6 参数6的类型
    /// @tparam ArgType7 参数7的类型
    /// @tparam ArgType8 参数8的类型
    /// @param buffer 输入缓冲区
    /// @param a1 参数1
    /// @param a2 参数2
    /// @param a3 参数3
    /// @param a4 参数4
    /// @param a5 参数5
    /// @param a6 参数6
    /// @param a7 参数7
    /// @param a8 参数8
    /// @return 返回状态
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7, typename ArgType8
             >
    Status_t ExtractInputParameters(
        const Rpc::Buffer& buffer,
        ArgType1& a1, ArgType2& a2, ArgType3& a3, ArgType4& a4, ArgType5& a5,
        ArgType6& a6, ArgType7& a7, ArgType8& a8
    ) const
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();
        if (!ExtractParameter(p, size, a1)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a2)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a3)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a4)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a5)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a6)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a7)) return Status_RemoteInvalidInputArguments;
        if (!ExtractParameter(p, size, a8)) return Status_RemoteInvalidInputArguments;
        if (size != 0) return Status_RemoteInvalidInputArguments;
        return Status_Success;
    }
private:
    /// 从输入缓冲区中提取单个参数
    /// retval true 成功，size 减小消耗掉的字节数，缓冲区指针往前移
    /// retval true 成功，缓冲区指针和 size 不动
    template <typename ArgType>
    static bool ExtractParameter(const char*& buffer, size_t& size, ArgType& a)
    {
        return UnserializeObject(buffer, size, a);
    }

public:
    /// 输出出参数并完成调用，用于无出参数和返回值的函数
    Status_t Complete() const
    {
        if (TotalResultCount() > 0)
        {
            Status_t status = m_HasReturnValue ? Status_RemoteInvalidReturnValue : Status_RemoteInvalidOutputArguments;
            Abort(status);
            return status;
        }
        else
        {
            SerializeBuffer buffer(ReturnMessageHeaderSize);
            return Return(buffer);
        }
    }

    /// 输出出参数并完成调用，用于出参数+返回值总数为一个的函数
    template <typename ArgType>
    Status_t Complete(const ArgType& a) const
    {
        if (TotalResultCount() != 1)
            Abort(Status_RemoteInvalidOutputArguments);

        SerializeBuffer buffer(ReturnMessageHeaderSize);

        if (!SerializeObject(a, buffer))
        {
            Status_t status = m_HasReturnValue ? Status_RemoteInvalidReturnValue : Status_RemoteInvalidOutputArguments;
            Abort(status);
            return status;
        }

        return Return(buffer);
    }

    /// 输出出参数并完成调用，用于出参数+返回值总数为两个的函数
    template <typename ArgType1, typename ArgType2>
    Status_t Complete(const ArgType1& a1, const ArgType2& a2) const
    {
        if (TotalResultCount() != 2)
            Abort(Status_RemoteInvalidOutputArguments);

        SerializeBuffer buffer(ReturnMessageHeaderSize);

        Status_t status = Status_RemoteInvalidOutputArguments;
        if (!SerializeObject(a1, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a2, buffer))
        {
            if (m_HasReturnValue)
                status = Status_RemoteInvalidReturnValue;
            Abort(status);
            return status;
        }

        return Return(buffer);
    }

    /// 输出出参数并完成调用，用于出参数+返回值总数为三个的函数
    template <typename ArgType1, typename ArgType2, typename ArgType3>
    Status_t Complete(const ArgType1& a1, const ArgType2& a2, const ArgType3& a3) const
    {
        if (TotalResultCount() != 3)
            Abort(Status_RemoteInvalidOutputArguments);

        SerializeBuffer buffer(ReturnMessageHeaderSize);

        Status_t status = Status_RemoteInvalidOutputArguments;
        if (!SerializeObject(a1, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a2, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a3, buffer))
        {
            if (m_HasReturnValue)
                status = Status_RemoteInvalidReturnValue;
            Abort(status);
            return status;
        }

        return Return(buffer);
    }

    /// 输出出参数并完成调用，用于出参数+返回值总数为四个的函数
    template <typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4>
    Status_t Complete(const ArgType1& a1, const ArgType2& a2, const ArgType3& a3, const ArgType4& a4) const
    {
        if (TotalResultCount() != 4)
            Abort(Status_RemoteInvalidOutputArguments);

        SerializeBuffer buffer(ReturnMessageHeaderSize);

        Status_t status = Status_RemoteInvalidOutputArguments;
        if (!SerializeObject(a1, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a2, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a3, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a4, buffer))
        {
            if (m_HasReturnValue)
                status = Status_RemoteInvalidReturnValue;
            Abort(status);
            return status;
        }

        return Return(buffer);
    }

    /// 输出出参数并完成调用，用于出参数+返回值总数为五个的函数
    template <typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename ArgType5>
    Status_t Complete(const ArgType1& a1, const ArgType2& a2, const ArgType3& a3, const ArgType4& a4, const ArgType5& a5) const
    {
        if (TotalResultCount() != 5)
            Abort(Status_RemoteInvalidOutputArguments);

        SerializeBuffer buffer(ReturnMessageHeaderSize);

        Status_t status = Status_RemoteInvalidOutputArguments;
        if (!SerializeObject(a1, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a2, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a3, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a4, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a5, buffer))
        {
            if (m_HasReturnValue)
                status = Status_RemoteInvalidReturnValue;
            Abort(status);
            return status;
        }

        return Return(buffer);
    }

    /// 输出出参数并完成调用，用于出参数+返回值总数为六个的函数
    template <typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename ArgType5, typename ArgType6>
    Status_t Complete(const ArgType1& a1, const ArgType2& a2, const ArgType3& a3, const ArgType4& a4, const ArgType5& a5, const ArgType6& a6) const
    {
        if (TotalResultCount() != 6)
            Abort(Status_RemoteInvalidOutputArguments);

        SerializeBuffer buffer(ReturnMessageHeaderSize);

        Status_t status = Status_RemoteInvalidOutputArguments;
        if (!SerializeObject(a1, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a2, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a3, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a4, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a5, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a6, buffer))
        {
            if (m_HasReturnValue)
                status = Status_RemoteInvalidReturnValue;
            Abort(status);
            return status;
        }

        return Return(buffer);
    }

    /// 输出出参数并完成调用，用于出参数+返回值总数为七个的函数
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7
             >
    Status_t Complete(
        const ArgType1& a1, const ArgType2& a2, const ArgType3& a3,
        const ArgType4& a4, const ArgType5& a5, const ArgType6& a6,
        const ArgType7& a7
    ) const
    {
        if (TotalResultCount() != 7)
            Abort(Status_RemoteInvalidOutputArguments);

        SerializeBuffer buffer(ReturnMessageHeaderSize);

        Status_t status = Status_RemoteInvalidOutputArguments;
        if (!SerializeObject(a1, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a2, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a3, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a4, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a5, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a6, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a7, buffer))
        {
            if (m_HasReturnValue)
                status = Status_RemoteInvalidReturnValue;
            Abort(status);
            return status;
        }

        return Return(buffer);
    }

    /// 输出出参数并完成调用，用于出参数+返回值总数为七个的函数
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7, typename ArgType8
             >
    Status_t Complete(
        const ArgType1& a1, const ArgType2& a2, const ArgType3& a3,
        const ArgType4& a4, const ArgType5& a5, const ArgType6& a6,
        const ArgType7& a7, const ArgType8& a8
    ) const
    {
        if (TotalResultCount() != 7)
            Abort(Status_RemoteInvalidOutputArguments);

        SerializeBuffer buffer(ReturnMessageHeaderSize);

        Status_t status = Status_RemoteInvalidOutputArguments;
        if (!SerializeObject(a1, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a2, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a3, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a4, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a5, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a6, buffer))
        {
            Abort(status);
            return status;
        }

        if (!SerializeObject(a7, buffer))
        {
            if (m_HasReturnValue)
                status = Status_RemoteInvalidReturnValue;
            Abort(status);
            return status;
        }

        if (!SerializeObject(a8, buffer))
        {
            if (m_HasReturnValue)
                status = Status_RemoteInvalidReturnValue;
            Abort(status);
            return status;
        }

        return Return(buffer);
    }
    /// 输出出参数并完成调用，只序列化返回值，不处理出参数，
    /// 例如 read 失败时，出参数无意义
    template <typename ReturnType>
    Status_t CompleteError(const ReturnType& a) const
    {
        SerializeBuffer buffer(ReturnMessageHeaderSize);
        if (m_HasReturnValue && SerializeObject(a, buffer))
        {
            return Return(buffer, Status_MethodFailure);
        }
        else
        {
            Abort(Status_RemoteInvalidReturnValue);
            return Status_RemoteInvalidReturnValue;
        }
    }

    /// 通知客户端调用失败
    Status_t Abort(Status_t status) const
    {
        return ErrorReturn(status);
    }

private:
    /// 向调用者返回调用结果
    Status_t Return(SerializeBuffer& result, Status_t status = Status_Success) const;

    /// 向调用者返回出错的调用结果
    Status_t ErrorReturn(Status_t status) const;

public:
    size_t TotalResultCount() const
    {
        return m_OutputParamCount + m_HasReturnValue;
    }
public:
    /// 设置参数处理器，根据模板类型参数的不同设置不同的出参数处理函数
    template <typename T1>
    void SetParameterHandlers()
    {
        m_OutputParamCount = IsOut<T1>::Value;
    }

    /// 设置参数处理器，根据模板类型参数的不同设置不同的出参数处理函数
    template <typename T1, typename T2>
    void SetParameterHandlers()
    {
        m_OutputParamCount = IsOut<T1>::Value + IsOut<T2>::Value;
    }

    /// 设置参数处理器，根据模板类型参数的不同设置不同的出参数处理函数
    template <typename T1, typename T2, typename T3>
    void SetParameterHandlers()
    {
        m_OutputParamCount = IsOut<T1>::Value + IsOut<T2>::Value + IsOut<T3>::Value;
    }

    /// 设置参数处理器，根据模板类型参数的不同设置不同的出参数处理函数
    template <typename T1, typename T2, typename T3, typename T4>
    void SetParameterHandlers()
    {
        m_OutputParamCount = IsOut<T1>::Value + IsOut<T2>::Value + IsOut<T3>::Value + IsOut<T4>::Value;
    }

    /// 设置参数处理器，根据模板类型参数的不同设置不同的出参数处理函数
    template <typename T1, typename T2, typename T3, typename T4, typename T5>
    void SetParameterHandlers()
    {
        m_OutputParamCount =
            IsOut<T1>::Value + IsOut<T2>::Value + IsOut<T3>::Value + IsOut<T4>::Value + IsOut<T5>::Value;
    }

    /// 设置参数处理器，根据模板类型参数的不同设置不同的出参数处理函数
    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void SetParameterHandlers()
    {
        m_OutputParamCount =
            IsOut<T1>::Value + IsOut<T2>::Value + IsOut<T3>::Value + IsOut<T4>::Value + IsOut<T5>::Value + IsOut<T6>::Value;
    }

    /// 设置参数处理器，根据模板类型参数的不同设置不同的出参数处理函数
    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    void SetParameterHandlers()
    {
        m_OutputParamCount =
            IsOut<T1>::Value + IsOut<T2>::Value + IsOut<T3>::Value + IsOut<T4>::Value +
            IsOut<T5>::Value + IsOut<T6>::Value + IsOut<T7>::Value;
    }

    /// 设置参数处理器，根据模板类型参数的不同设置不同的出参数处理函数
    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    void SetParameterHandlers()
    {
        m_OutputParamCount =
            IsOut<T1>::Value + IsOut<T2>::Value + IsOut<T3>::Value + IsOut<T4>::Value +
            IsOut<T5>::Value + IsOut<T6>::Value + IsOut<T7>::Value + IsOut<T8>::Value;
    }
public:
    /// 设置返回值处理函数
    template <typename T>
    void SetReturnHandler(T*)
    {
        m_HasReturnValue = true;
    }

    /// void 类型不处理返回值，m_ReturnValueHandler 保持为空
    void SetReturnHandler(void*)
    {
        m_HasReturnValue = false;
    }

private:
    unsigned int m_OutputParamCount;        ///< 出参数个数
    bool m_HasReturnValue;                  ///< 是否有返回值
    std::string m_LocalEndPoint;            ///< 对方端点
    std::string m_RemoteEndPoint;           ///< 对方端点
    InvokeId_t m_InvokeId;                  ///< 调用 ID
    int m_MethodId;                         ///< 方法 ID
};

} // end namespace Rpc

#endif//INVOKE_CONTEXT_HPP
