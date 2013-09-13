///////////////////////////////////////////////////////////////////////////////
/// RPC 桩类

#ifndef RPC_STUB_HPP_INCLUDED
#define RPC_STUB_HPP_INCLUDED

#include <common/rpc/types.hpp>
#include <common/rpc/serialization.hpp>

#include <string.h>
#include <assert.h>

namespace Rpc
{

class LocalObjectTable;

struct InvokeInfo
{
    const std::string& local_endpoint;   ///< 本地端点
    const std::string& remote_endpoint;  ///< 调用者的来源
    InvokeId_t invoke_id;               ///< 调用的唯一 id
    int method_id;                      ///< 方法 id
};

///////////////////////////////////////////////////////////////////////////////
// 对象在服务器端的桩，所有可通过 RPC 调用的对象均需实现此接口
class Stub
{
    friend class LocalObjectTable;

protected:
    Stub();
    Stub(ObjectId_t object_id);
    Stub(const Stub& src);
    virtual ~Stub();
    Stub& operator=(const Stub& src);

    // 处理调用请求
    virtual Status_t Dispatch(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,  ///< 调用者的来源
        InvokeId_t invoke_id,               ///< 调用的唯一 id
        const char* name,                   ///< 方法名
        int method_id,                      ///< 方法 id
        const void* message,                ///< 参数缓冲区
        size_t size                         ///< 参数缓冲区长度
    ) = 0;

    virtual bool IsNestableMethod(
        const char* name,
        int method_id
    ) = 0;

public:
    /// 获得对象 ID
    ObjectId_t RpcObjectId() const
    {
        return m_ObjectId;
    }

    /// 向调用者返回调用结果
    static Status_t Return(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,  ///< 调用者的来源
        InvokeId_t invoke_id,               ///< 调用的唯一 id
        int method_id,                      ///< 方法 id
        SerializeBuffer& result,
        Status_t status = Status_Success
    );

    static Status_t Return(
        const InvokeInfo& invoke_info,
        SerializeBuffer& result,
        Status_t status = Status_Success
    )
    {
        return Return(
                   invoke_info.local_endpoint,
                   invoke_info.remote_endpoint,
                   invoke_info.invoke_id,
                   invoke_info.method_id,
                   result,
                   status
               );
    }

    /// 向调用者返回出错的调用结果
    static Status_t ErrorReturn(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,  ///< 调用者的来源
        InvokeId_t invoke_id,               ///< 调用的唯一 id
        int method_id,                      ///< 方法 id
        Status_t status                     ///< 完成状态
    );
private:
    /// 以全局共享的计数器递增的方式生成对象 ID，可以确保不重复
    static ObjectId_t GenerateObjectId()
    {
        static ObjectId_t s_NextObjectId = MaxPredefinedObjectId;
        // TODO: 改为原子加？
        return s_NextObjectId++;
    }
private:
    ObjectId_t m_ObjectId;
};

///////////////////////////////////////////////////////////////////////////////
// 以下为 Stub 的实现部分

template <typename ArgType, bool IsPointer>
struct ToArgImpl
{
    template <typename RawType>
    static RawType& Get(RawType& a)
    {
        return a;
    }
};

template <typename ArgType>
struct ToArgImpl<ArgType, true>
{
    template <typename RawType>
    static RawType* Get(RawType& a)
    {
        return &a;
    }
};

template <typename ArgType>
struct ToArg : ToArgImpl<ArgType, IsPointer<typename RemoveInOut<ArgType>::Type>::Value>
{
};

/// 正常返回类型
struct NormalReturnType {};

/// void 返回类型
struct VoidReturnType {};

/// 用于提取返回类型，调用不同的重载 InvokeStub 处理不同的返回类型
template <typename T>
struct ReturnTypeTag
{
    typedef NormalReturnType Type;
};

/// 用于提取返回类型，调用不同的重载 InvokeStub 处理不同的返回类型
template <>
struct ReturnTypeTag<void>
{
    typedef VoidReturnType Type;
};

///////////////////////////////////////////////////////////////////////////////
// 基于表的 Stub 的通用实现方式
class StubImpl : public Stub
{
private:
    /// 实现基类的 Dispatch
    virtual Status_t Dispatch(
        const std::string& local_endpoint,   ///< 本地的端点
        const std::string& remote_endpoint,  ///< 对方的端点
        InvokeId_t invoke_id,               ///< 调用的唯一 id
        const char* name,                   ///< 方法名
        int method_id,                      ///< 方法 id
        const void* message,                ///< 参数缓冲区
        size_t size                         ///< 参数缓冲区长度
    );

    /// 实现基类的 IsNestableMethod
    virtual bool IsNestableMethod(
        const char* name,
        int method_id
    );

protected:
    StubImpl() {}
    explicit StubImpl(ObjectId_t object_id) : Stub(object_id) {}

    typedef Status_t (StubImpl::*StubMethod)(
        const InvokeInfo& invoke_info,
        const Buffer& input
    );

    /// Dispatch 表项
    struct DispatchEntry
    {
        const char* Name;       /// 方法名
        StubMethod Method;      /// Stub 方法
        bool NestedInvoke;      /// 是否可嵌套调用
    };

    class DispatchTable
    {
    public:
        DispatchTable(const DispatchEntry* entries, size_t size):
            m_Entries(entries), m_Size(size)
        {
        }

        Status_t Invoke(
            StubImpl* stub,
            InvokeInfo& invoke_info,
            const char* name,
            const Buffer& buffer
        ) const;
        bool IsNestableMethod(
            const char* name,
            int method_id
        ) const;
    private:
        // 根据函数名查找 ID
        int FindMethod(const char* name) const;
    private:
        const DispatchEntry* m_Entries;
        size_t m_Size;
    };

private:
    /// 获得对象类的 Dispatch 表，
    /// 在 RPC_BEGIN_STUB_CLASS 生成的类中被重载，在 RPC_BEGIN_STUB_DISPATCH 定义的函数中被实现。
    virtual const DispatchTable& GetDispatchTable() const = 0;

protected:
    ///////////////////////////////////////////////////////////////////////////
    // 入参数提取函数

    /// 提取单个出参数
    template <typename ArgType1, typename RawType1>
    static bool ExtractInputParameter(const char*& buffer, size_t& size, RawType1& a1)
    {
        // 非入参数直接返回成功，入参数需反序列化成功
        return !IsIn<ArgType1>::Value || UnserializeObject(buffer, size, a1);
    }

    /// 提取单参数函数的参数
    template <typename ArgType1, typename RawType1>
    static bool ExtractInputParameters(const Buffer& buffer, RawType1& a1)
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();

        if (!ExtractInputParameter<ArgType1>(p, size, a1))
        {
            return false;
        }


        return size == 0;
    }

    /// 提取双参数函数的参数
    template <
    typename ArgType1, typename ArgType2,
             typename RawType1, typename RawType2
             >
    static bool ExtractInputParameters(const Buffer& buffer, RawType1& a1, RawType2& a2)
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();

        if (!ExtractInputParameter<ArgType1>(p, size, a1))
        {
            return false;
        }
        if (!ExtractInputParameter<ArgType2>(p, size, a2))
        {
            return false;
        }

        return size == 0;
    }

    /// 提取三参数函数的参数
    template <
    typename ArgType1, typename ArgType2, typename ArgType3,
             typename RawType1, typename RawType2, typename RawType3
             >
    static bool ExtractInputParameters(const Buffer& buffer, RawType1& a1, RawType2& a2, RawType3& a3)
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();

        if (!ExtractInputParameter<ArgType1>(p, size, a1))
            return false;
        if (!ExtractInputParameter<ArgType2>(p, size, a2))
            return false;
        if (!ExtractInputParameter<ArgType3>(p, size, a3))
            return false;

        return size == 0;
    }

    /// 提取四参数函数的参数
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename RawType1, typename RawType2, typename RawType3, typename RawType4
             >
    static bool ExtractInputParameters(
        const Buffer& buffer,
        RawType1& a1, RawType2& a2, RawType3& a3, RawType4& a4
    )
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();

        if (!ExtractInputParameter<ArgType1>(p, size, a1))
            return false;
        if (!ExtractInputParameter<ArgType2>(p, size, a2))
            return false;
        if (!ExtractInputParameter<ArgType3>(p, size, a3))
            return false;
        if (!ExtractInputParameter<ArgType4>(p, size, a4))
            return false;

        return size == 0;
    }

    /// 提取五参数函数的参数
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename ArgType5,
             typename RawType1, typename RawType2, typename RawType3, typename RawType4, typename RawType5
             >
    static bool ExtractInputParameters(
        const Buffer& buffer,
        RawType1& a1, RawType2& a2, RawType3& a3, RawType4& a4, RawType5& a5
    )
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();

        if (!ExtractInputParameter<ArgType1>(p, size, a1))
            return false;
        if (!ExtractInputParameter<ArgType2>(p, size, a2))
            return false;
        if (!ExtractInputParameter<ArgType3>(p, size, a3))
            return false;
        if (!ExtractInputParameter<ArgType4>(p, size, a4))
            return false;
        if (!ExtractInputParameter<ArgType5>(p, size, a5))
            return false;

        return size == 0;
    }

    /// 提取六参数函数的参数
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename ArgType5, typename ArgType6,
             typename RawType1, typename RawType2, typename RawType3, typename RawType4, typename RawType5, typename RawType6
             >
    static bool ExtractInputParameters(
        const Buffer& buffer,
        RawType1& a1, RawType2& a2, RawType3& a3, RawType4& a4, RawType5& a5, RawType6& a6
    )
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();

        if (!ExtractInputParameter<ArgType1>(p, size, a1))
            return false;
        if (!ExtractInputParameter<ArgType2>(p, size, a2))
            return false;
        if (!ExtractInputParameter<ArgType3>(p, size, a3))
            return false;
        if (!ExtractInputParameter<ArgType4>(p, size, a4))
            return false;
        if (!ExtractInputParameter<ArgType5>(p, size, a5))
            return false;
        if (!ExtractInputParameter<ArgType6>(p, size, a6))
            return false;

        return size == 0;
    }

    /// 提取七参数函数的参数
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7,
             typename RawType1, typename RawType2, typename RawType3, typename RawType4,
             typename RawType5, typename RawType6, typename RawType7
             >
    static bool ExtractInputParameters(
        const Buffer& buffer,
        RawType1& a1, RawType2& a2, RawType3& a3, RawType4& a4, RawType5& a5, RawType6& a6, RawType7& a7
    )
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();

        if (!ExtractInputParameter<ArgType1>(p, size, a1))
            return false;
        if (!ExtractInputParameter<ArgType2>(p, size, a2))
            return false;
        if (!ExtractInputParameter<ArgType3>(p, size, a3))
            return false;
        if (!ExtractInputParameter<ArgType4>(p, size, a4))
            return false;
        if (!ExtractInputParameter<ArgType5>(p, size, a5))
            return false;
        if (!ExtractInputParameter<ArgType6>(p, size, a6))
            return false;
        if (!ExtractInputParameter<ArgType7>(p, size, a7))
            return false;

        return size == 0;
    }

    /// 提取八参数函数的参数
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7, typename ArgType8,
             typename RawType1, typename RawType2, typename RawType3, typename RawType4,
             typename RawType5, typename RawType6, typename RawType7, typename RawType8
             >
    static bool ExtractInputParameters(
        const Buffer& buffer,
        RawType1& a1, RawType2& a2, RawType3& a3, RawType4& a4, RawType5& a5, RawType6& a6, RawType7& a7, RawType8& a8
    )
    {
        const char* p = (const char*) buffer.Address();
        size_t size = buffer.Size();

        if (!ExtractInputParameter<ArgType1>(p, size, a1))
            return false;
        if (!ExtractInputParameter<ArgType2>(p, size, a2))
            return false;
        if (!ExtractInputParameter<ArgType3>(p, size, a3))
            return false;
        if (!ExtractInputParameter<ArgType4>(p, size, a4))
            return false;
        if (!ExtractInputParameter<ArgType5>(p, size, a5))
            return false;
        if (!ExtractInputParameter<ArgType6>(p, size, a6))
            return false;
        if (!ExtractInputParameter<ArgType7>(p, size, a7))
            return false;
        if (!ExtractInputParameter<ArgType8>(p, size, a8))
            return false;

        return size == 0;
    }
protected:
    ///////////////////////////////////////////////////////////////////////////
    // 出参数打包函数，在同步完成时被调用

    /// 单参数函数的出参数打包
    template <typename ArgType1, typename RawType1>
    static bool PackOutputParameters(const RawType1& a1, SerializeBuffer& buffer)
    {
        return !IsOut<ArgType1>::Value || SerializeObject(a1, buffer);
    }

    /// 双参数函数的出参数打包
    template <
    typename ArgType1, typename ArgType2,
             typename RawType1, typename RawType2
             >
    static bool PackOutputParameters(
        const RawType1& a1, const RawType2& a2,
        SerializeBuffer& buffer
    )
    {
        return PackOutputParameters<ArgType1>(a1, buffer) &&
               PackOutputParameters<ArgType2>(a2, buffer);
    }

    /// 三参数函数的出参数打包
    template <
    typename ArgType1, typename ArgType2, typename ArgType3,
             typename RawType1, typename RawType2, typename RawType3
             >
    static bool PackOutputParameters(
        const RawType1& a1, const RawType2& a2, const RawType3& a3,
        SerializeBuffer& buffer
    )
    {
        return PackOutputParameters<ArgType1, ArgType2>(a1, a2, buffer) &&
               PackOutputParameters<ArgType3>(a3, buffer);
    }

    /// 四参数函数的出参数打包
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename RawType1, typename RawType2, typename RawType3, typename RawType4
             >
    static bool PackOutputParameters(
        const RawType1& a1, const RawType2& a2, const RawType3& a3, const RawType4& a4,
        SerializeBuffer& buffer
    )
    {
        return PackOutputParameters<ArgType1, ArgType2, ArgType3>(a1, a2, a3, buffer) &&
               PackOutputParameters<ArgType4>(a4, buffer);
    }

    /// 五参数函数的出参数打包
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename ArgType5,
             typename RawType1, typename RawType2, typename RawType3, typename RawType4, typename RawType5
             >
    static bool PackOutputParameters(
        const RawType1& a1, const RawType2& a2, const RawType3& a3, const RawType4& a4, const RawType5& a5,
        SerializeBuffer& buffer
    )
    {
        return PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4>(a1, a2, a3, a4, buffer) &&
               PackOutputParameters<ArgType5>(a5, buffer);
    }

    /// 六参数函数的出参数打包
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename ArgType5, typename ArgType6,
             typename RawType1, typename RawType2, typename RawType3, typename RawType4, typename RawType5, typename RawType6
             >
    static bool PackOutputParameters(
        const RawType1& a1, const RawType2& a2, const RawType3& a3, const RawType4& a4, const RawType5& a5, const RawType6& a6,
        SerializeBuffer& buffer
    )
    {
        return PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5>(a1, a2, a3, a4, a5, buffer) &&
               PackOutputParameters<ArgType6>(a6, buffer);
    }

    /// 七参数函数的出参数打包
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7,
             typename RawType1, typename RawType2, typename RawType3, typename RawType4,
             typename RawType5, typename RawType6, typename RawType7
             >
    static bool PackOutputParameters(
        const RawType1& a1, const RawType2& a2, const RawType3& a3, const RawType4& a4,
        const RawType5& a5, const RawType6& a6, const RawType7& a7,
        SerializeBuffer& buffer
    )
    {
        return PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6>(a1, a2, a3, a4, a5, a6, buffer) &&
               PackOutputParameters<ArgType7>(a7, buffer);
    }

    /// 八参数函数的出参数打包
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7, typename ArgType8,
             typename RawType1, typename RawType2, typename RawType3, typename RawType4,
             typename RawType5, typename RawType6, typename RawType7, typename RawType8
             >
    static bool PackOutputParameters(
        const RawType1& a1, const RawType2& a2, const RawType3& a3, const RawType4& a4,
        const RawType5& a5, const RawType6& a6, const RawType7& a7, const RawType8& a8,
        SerializeBuffer& buffer
    )
    {
        return PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7>(
                   a1, a2, a3, a4, a5, a6, a7, buffer) &&
               PackOutputParameters<ArgType8>(a8, buffer);
    }
protected:
    /// 打包返回值
    template <typename T>
    bool PackReturnValue(const T& value, SerializeBuffer& buffer)
    {
        return SerializeObject(value, buffer);
    }
protected:

    /// 无参数函数的同步完成桩方法
    template <typename ReturnType, typename ImplType>
    Status_t InvokeStub0(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(),
        NormalReturnType
    )
    {
        ReturnType r = (static_cast<ImplType*>(this)->*method)();

        SerializeBuffer result(ReturnMessageHeaderSize);
        if (!PackReturnValue(r, result))
            return Status_RemoteInvalidReturnValue;

        return Stub::Return(invoke_info, result);
    }

    /// 无参数函数的同步完成桩方法，返回值void类型
    template <typename ReturnType, typename ImplType>
    Status_t InvokeStub0(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(),
        VoidReturnType
    )
    {
        (static_cast<ImplType*>(this)->*method)();

        SerializeBuffer result(ReturnMessageHeaderSize);

        return Return(invoke_info, result);
    }

    /// 一个参数的桩函数，返回值为普通类型
    template <
    typename ReturnType,
             typename ImplType,
             typename ArgType,
             typename ImplArgType1
             >
    Status_t InvokeStub1(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1),
        NormalReturnType
    )
    {
        typename RawTypeOf<ArgType>::Type a;
        if (!ExtractInputParameters<ArgType>(input, a))
        {
            return Status_RemoteInvalidInputArguments;
        }

        ReturnType r = (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType>::Get(a)
                );

        SerializeBuffer result(ReturnMessageHeaderSize);

        if (!PackOutputParameters<ArgType>(a, result))
            return Status_RemoteInvalidOutputArguments;

        if (!PackReturnValue(r, result))
            return Status_RemoteInvalidReturnValue;

        return Return(invoke_info, result);
    }

    template <
    typename ReturnType,
             typename ImplType,
             typename ArgType,
             typename ImplArgType1
             >
    Status_t InvokeStub1(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1),
        VoidReturnType
    )
    {
        // 实际函数的参数
        typename RawTypeOf<ArgType>::Type a;

        if (!ExtractInputParameters<ArgType>(input, a))
        {
            return Status_RemoteInvalidInputArguments;
        }

        // 调用实现函数
        (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType>::Get(a)
                );

        SerializeBuffer result(ReturnMessageHeaderSize);
        if (!PackOutputParameters<ArgType>(a, result))
            return Status_RemoteInvalidOutputArguments;

        return Return(invoke_info, result);
    }

    /// 两个参数的桩函数，返回值为普通类型
    template <
    typename ReturnType,
             typename ImplType,
             typename ArgType1, typename ArgType2,
             typename ImplArgType1, typename ImplArgType2
             >
    Status_t InvokeStub2(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2),
        NormalReturnType
    )
    {
        // 实际函数的参数
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;

        if (!ExtractInputParameters<ArgType1, ArgType2>(input, a1, a2))
        {
            return Status_RemoteInvalidInputArguments;
        }

        // 调用实现函数
        ReturnType r = (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2)
                );

        SerializeBuffer result(ReturnMessageHeaderSize);

        if (!PackOutputParameters<ArgType1, ArgType2>(a1, a2, result))
            return Status_RemoteInvalidOutputArguments;

        if (!PackReturnValue(r, result))
            return Status_RemoteInvalidReturnValue;

        return Return(invoke_info, result);
    }

    template <
    typename ReturnType,
             typename ImplType,
             typename ArgType1, typename ArgType2,
             typename ImplArgType1, typename ImplArgType2
             >
    Status_t InvokeStub2(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2),
        VoidReturnType
    )
    {
        // 实际函数的参数
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;

        if (!ExtractInputParameters<ArgType1, ArgType2>(input, a1, a2))
        {
            return Status_RemoteInvalidInputArguments;
        }

        // 调用实现函数
        (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2)
                );

        SerializeBuffer result(ReturnMessageHeaderSize);

        if (!PackOutputParameters<ArgType1, ArgType2>(a1, a2, result))
            return Status_RemoteInvalidOutputArguments;
        return Return(invoke_info, result);
    }

    // 三个参数
    template <
    typename ReturnType,
             typename ImplType,
             typename ArgType1, typename ArgType2, typename ArgType3,
             typename ImplArgType1, typename ImplArgType2, typename ImplArgType3
             >
    Status_t InvokeStub3(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2, ImplArgType3),
        NormalReturnType
    )
    {
        // 实际函数的参数
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;
        typename RawTypeOf<ArgType3>::Type a3;

        if (!ExtractInputParameters<ArgType1, ArgType2, ArgType3>(input, a1, a2, a3))
            return Status_RemoteInvalidInputArguments;

        // 调用实现函数
        ReturnType r = (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2),
                ToArg<ArgType3>::Get(a3)
                );

        SerializeBuffer result(ReturnMessageHeaderSize);

        if (!PackOutputParameters<ArgType1, ArgType2, ArgType3>(a1, a2, a3, result))
            return Status_RemoteInvalidOutputArguments;

        if (!PackReturnValue(r, result))
            return Status_RemoteInvalidReturnValue;

        return Return(invoke_info, result);
    }

    template <
    typename ReturnType,
             typename ImplType,
             typename ArgType1, typename ArgType2, typename ArgType3,
             typename ImplArgType1, typename ImplArgType2, typename ImplArgType3
             >
    Status_t InvokeStub3(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2, ImplArgType3),
        VoidReturnType
    )
    {
        // 实际函数的参数
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;
        typename RawTypeOf<ArgType3>::Type a3;

        if (!ExtractInputParameters<ArgType1, ArgType2, ArgType3>(input, a1, a2, a3))
            return Status_RemoteInvalidInputArguments;

        // 调用实现函数
        (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2),
                ToArg<ArgType3>::Get(a3)
                );

        SerializeBuffer result(ReturnMessageHeaderSize);
        if (!PackOutputParameters<ArgType1, ArgType2, ArgType3>(a1, a2, a3, result))
            return Status_RemoteInvalidOutputArguments;

        return Return(invoke_info, result);
    }

    // 四个参数
    template <
    typename ReturnType,
             typename ImplType,
             typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ImplArgType1, typename ImplArgType2, typename ImplArgType3, typename ImplArgType4
             >
    Status_t InvokeStub4(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2, ImplArgType3, ImplArgType4),
        NormalReturnType
    )
    {
        // 实际函数的参数
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;
        typename RawTypeOf<ArgType3>::Type a3;
        typename RawTypeOf<ArgType4>::Type a4;

        if (!ExtractInputParameters<ArgType1, ArgType2, ArgType3, ArgType4>(input, a1, a2, a3, a4))
            return Status_RemoteInvalidInputArguments;

        // 调用实现函数
        ReturnType r = (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2),
                ToArg<ArgType3>::Get(a3),
                ToArg<ArgType4>::Get(a4)
                );

        SerializeBuffer result(ReturnMessageHeaderSize);
        if (!PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4>(a1, a2, a3, a4, result))
            return Status_RemoteInvalidOutputArguments;

        if (!PackReturnValue(r, result))
            return Status_RemoteInvalidReturnValue;

        return Return(invoke_info, result);
    }

    template <
    typename ReturnType,
             typename ImplType,
             typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ImplArgType1, typename ImplArgType2, typename ImplArgType3, typename ImplArgType4
             >
    Status_t InvokeStub4(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2, ImplArgType3, ImplArgType4),
        VoidReturnType
    )
    {
        // 实际函数的参数
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;
        typename RawTypeOf<ArgType3>::Type a3;
        typename RawTypeOf<ArgType4>::Type a4;

        // 取出所有的入参数
        if (!ExtractInputParameters<ArgType1, ArgType2, ArgType3, ArgType4>(input, a1, a2, a3, a4))
            return Status_RemoteInvalidInputArguments;

        // 调用实现函数
        (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2),
                ToArg<ArgType3>::Get(a3),
                ToArg<ArgType4>::Get(a4)
                );

        SerializeBuffer result(ReturnMessageHeaderSize);

        if (!PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4>(a1, a2, a3, a4, result))
            return Status_RemoteInvalidOutputArguments;

        return Return(invoke_info, result);
    }

    // 五个参数
    template <
    typename ReturnType,
             typename ImplType,
             typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename ArgType5,
             typename ImplArgType1, typename ImplArgType2, typename ImplArgType3, typename ImplArgType4, typename ImplArgType5
             >
    Status_t InvokeStub5(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2, ImplArgType3, ImplArgType4, ImplArgType5),
        NormalReturnType
    )
    {
        // 实际函数的参数
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;
        typename RawTypeOf<ArgType3>::Type a3;
        typename RawTypeOf<ArgType4>::Type a4;
        typename RawTypeOf<ArgType5>::Type a5;

        // 取出所有的入参数
        if (!ExtractInputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5>(input, a1, a2, a3, a4, a5))
            return Status_RemoteInvalidInputArguments;

        // 调用实现函数
        ReturnType r = (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2),
                ToArg<ArgType3>::Get(a3),
                ToArg<ArgType4>::Get(a4),
                ToArg<ArgType5>::Get(a5)
                );

        /// 打包出参数和返回值
        SerializeBuffer result(ReturnMessageHeaderSize);
        if (!PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5>(a1, a2, a3, a4, a5, result))
            return Status_RemoteInvalidOutputArguments;

        if (!PackReturnValue(r, result))
            return Status_RemoteInvalidReturnValue;

        return Return(invoke_info, result);
    }

    template <
    typename ReturnType, typename ImplType,
             typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename ArgType5,
             typename ImplArgType1, typename ImplArgType2, typename ImplArgType3, typename ImplArgType4, typename ImplArgType5
             >
    Status_t InvokeStub5(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2, ImplArgType3, ImplArgType4, ImplArgType5),
        VoidReturnType
    )
    {
        // 实际函数的参数
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;
        typename RawTypeOf<ArgType3>::Type a3;
        typename RawTypeOf<ArgType4>::Type a4;
        typename RawTypeOf<ArgType5>::Type a5;

        // 取出所有的入参数
        if (!ExtractInputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5>(input, a1, a2, a3, a4, a5))
            return Status_RemoteInvalidInputArguments;

        // 调用实现函数
        (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2),
                ToArg<ArgType3>::Get(a3),
                ToArg<ArgType4>::Get(a4),
                ToArg<ArgType5>::Get(a5)
                );

        /// 打包出参数
        SerializeBuffer result(ReturnMessageHeaderSize);

        if (!PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5>(a1, a2, a3, a4, a5, result))
            return Status_RemoteInvalidOutputArguments;

        return Return(invoke_info, result);
    }

    // 六个参数
    template <
    typename ReturnType, typename ImplType,
             typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename ArgType5, typename ArgType6,
             typename ImplArgType1, typename ImplArgType2, typename ImplArgType3, typename ImplArgType4, typename ImplArgType5, typename ImplArgType6
             >
    Status_t InvokeStub6(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2, ImplArgType3, ImplArgType4, ImplArgType5, ImplArgType6),
        NormalReturnType
    )
    {
        // 实际函数的参数
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;
        typename RawTypeOf<ArgType3>::Type a3;
        typename RawTypeOf<ArgType4>::Type a4;
        typename RawTypeOf<ArgType5>::Type a5;
        typename RawTypeOf<ArgType6>::Type a6;

        // 取出所有的入参数
        if (!ExtractInputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6>(input, a1, a2, a3, a4, a5, a6))
            return Status_RemoteInvalidInputArguments;

        // 调用实现函数
        ReturnType r = (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2),
                ToArg<ArgType3>::Get(a3),
                ToArg<ArgType4>::Get(a4),
                ToArg<ArgType5>::Get(a5),
                ToArg<ArgType6>::Get(a6)
                );

        SerializeBuffer result(ReturnMessageHeaderSize);

        if (!PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6>(a1, a2, a3, a4, a5, a6, result))
            return Status_RemoteInvalidOutputArguments;

        if (!PackReturnValue(r, result))
            return Status_RemoteInvalidReturnValue;

        return Return(invoke_info, result);
    }

    template <
    typename ReturnType,
             typename ImplType,
             typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6,
             typename ImplArgType1, typename ImplArgType2, typename ImplArgType3,
             typename ImplArgType4, typename ImplArgType5, typename ImplArgType6
             >
    Status_t InvokeStub6(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2, ImplArgType3, ImplArgType4, ImplArgType5, ImplArgType6),
        VoidReturnType
    )
    {
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;
        typename RawTypeOf<ArgType3>::Type a3;
        typename RawTypeOf<ArgType4>::Type a4;
        typename RawTypeOf<ArgType5>::Type a5;
        typename RawTypeOf<ArgType6>::Type a6;

        // 取出所有的入参数
        if (!ExtractInputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6>(input, a1, a2, a3, a4, a5, a6))
            return Status_RemoteInvalidInputArguments;

        // 调用实现函数
        (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2),
                ToArg<ArgType3>::Get(a3),
                ToArg<ArgType4>::Get(a4),
                ToArg<ArgType5>::Get(a5),
                ToArg<ArgType6>::Get(a6)
                );

        SerializeBuffer result(ReturnMessageHeaderSize);
        if (!PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6>(a1, a2, a3, a4, a5, a6, result))
            return Status_RemoteInvalidOutputArguments;

        return Return(invoke_info, result);
    }

    // 七个参数
    template <
    typename ReturnType, typename ImplType,
             typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7,
             typename ImplArgType1, typename ImplArgType2, typename ImplArgType3,
             typename ImplArgType4, typename ImplArgType5, typename ImplArgType6,
             typename ImplArgType7
             >
    Status_t InvokeStub7(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2, ImplArgType3, ImplArgType4, ImplArgType5, ImplArgType6, ImplArgType7),
        NormalReturnType
    )
    {
        // 实际函数的参数
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;
        typename RawTypeOf<ArgType3>::Type a3;
        typename RawTypeOf<ArgType4>::Type a4;
        typename RawTypeOf<ArgType5>::Type a5;
        typename RawTypeOf<ArgType6>::Type a6;
        typename RawTypeOf<ArgType7>::Type a7;

        // 取出所有的入参数
        if (!ExtractInputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7>(
                    input, a1, a2, a3, a4, a5, a6, a7))
        {
            return Status_RemoteInvalidInputArguments;
        }

        // 调用实现函数
        ReturnType r = (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2),
                ToArg<ArgType3>::Get(a3),
                ToArg<ArgType4>::Get(a4),
                ToArg<ArgType5>::Get(a5),
                ToArg<ArgType6>::Get(a6),
                ToArg<ArgType7>::Get(a7)
                );

        SerializeBuffer result(ReturnMessageHeaderSize);

        if (!PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7>(
                    a1, a2, a3, a4, a5, a6, a7, result))
        {
            return Status_RemoteInvalidOutputArguments;
        }

        if (!PackReturnValue(r, result))
            return Status_RemoteInvalidReturnValue;

        return Return(invoke_info, result);
    }

    template <
    typename ReturnType,
             typename ImplType,
             typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7,
             typename ImplArgType1, typename ImplArgType2, typename ImplArgType3,
             typename ImplArgType4, typename ImplArgType5, typename ImplArgType6,
             typename ImplArgType7
             >
    Status_t InvokeStub7(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2, ImplArgType3, ImplArgType4, ImplArgType5, ImplArgType6, ImplArgType7),
        VoidReturnType
    )
    {
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;
        typename RawTypeOf<ArgType3>::Type a3;
        typename RawTypeOf<ArgType4>::Type a4;
        typename RawTypeOf<ArgType5>::Type a5;
        typename RawTypeOf<ArgType6>::Type a6;
        typename RawTypeOf<ArgType7>::Type a7;

        // 取出所有的入参数
        if (!ExtractInputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7>(
                    input, a1, a2, a3, a4, a5, a6, a7))
        {
            return Status_RemoteInvalidInputArguments;
        }

        // 调用实现函数
        (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2),
                ToArg<ArgType3>::Get(a3),
                ToArg<ArgType4>::Get(a4),
                ToArg<ArgType5>::Get(a5),
                ToArg<ArgType6>::Get(a6),
                ToArg<ArgType7>::Get(a7)
                );

        SerializeBuffer result(ReturnMessageHeaderSize);
        if (!PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7>(
                    a1, a2, a3, a4, a5, a6, a7, result))
        {
            return Status_RemoteInvalidOutputArguments;
        }

        return Return(invoke_info, result);
    }

    // 八个参数
    template <
    typename ReturnType, typename ImplType,
             typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7, typename ArgType8,
             typename ImplArgType1, typename ImplArgType2, typename ImplArgType3,
             typename ImplArgType4, typename ImplArgType5, typename ImplArgType6,
             typename ImplArgType7, typename ImplArgType8
             >
    Status_t InvokeStub8(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2, ImplArgType3, ImplArgType4, ImplArgType5, ImplArgType6, ImplArgType7, ImplArgType8),
        NormalReturnType
    )
    {
        // 实际函数的参数
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;
        typename RawTypeOf<ArgType3>::Type a3;
        typename RawTypeOf<ArgType4>::Type a4;
        typename RawTypeOf<ArgType5>::Type a5;
        typename RawTypeOf<ArgType6>::Type a6;
        typename RawTypeOf<ArgType7>::Type a7;
        typename RawTypeOf<ArgType8>::Type a8;

        // 取出所有的入参数
        if (!ExtractInputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7, ArgType8>(
                    input, a1, a2, a3, a4, a5, a6, a7, a8))
        {
            return Status_RemoteInvalidInputArguments;
        }

        // 调用实现函数
        ReturnType r = (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2),
                ToArg<ArgType3>::Get(a3),
                ToArg<ArgType4>::Get(a4),
                ToArg<ArgType5>::Get(a5),
                ToArg<ArgType6>::Get(a6),
                ToArg<ArgType7>::Get(a7),
                ToArg<ArgType8>::Get(a8)
                );

        SerializeBuffer result(ReturnMessageHeaderSize);

        if (!PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7, ArgType8>(
                    a1, a2, a3, a4, a5, a6, a7, a8, result))
        {
            return Status_RemoteInvalidOutputArguments;
        }

        if (!PackReturnValue(r, result))
            return Status_RemoteInvalidReturnValue;

        return Return(invoke_info, result);
    }

    template <
    typename ReturnType,
             typename ImplType,
             typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7, typename ArgType8,
             typename ImplArgType1, typename ImplArgType2, typename ImplArgType3,
             typename ImplArgType4, typename ImplArgType5, typename ImplArgType6,
             typename ImplArgType7, typename ImplArgType8
             >
    Status_t InvokeStub8(
        const InvokeInfo& invoke_info,
        const Buffer& input,
        ReturnType (ImplType::*method)(ImplArgType1, ImplArgType2, ImplArgType3, ImplArgType4, ImplArgType5, ImplArgType6, ImplArgType7, ImplArgType8),
        VoidReturnType
    )
    {
        typename RawTypeOf<ArgType1>::Type a1;
        typename RawTypeOf<ArgType2>::Type a2;
        typename RawTypeOf<ArgType3>::Type a3;
        typename RawTypeOf<ArgType4>::Type a4;
        typename RawTypeOf<ArgType5>::Type a5;
        typename RawTypeOf<ArgType6>::Type a6;
        typename RawTypeOf<ArgType7>::Type a7;
        typename RawTypeOf<ArgType8>::Type a8;

        // 取出所有的入参数
        if (!ExtractInputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7, ArgType8>(
                    input, a1, a2, a3, a4, a5, a6, a7, a8))
        {
            return Status_RemoteInvalidInputArguments;
        }

        // 调用实现函数
        (static_cast<ImplType*>(this)->*method)(
                ToArg<ArgType1>::Get(a1),
                ToArg<ArgType2>::Get(a2),
                ToArg<ArgType3>::Get(a3),
                ToArg<ArgType4>::Get(a4),
                ToArg<ArgType5>::Get(a5),
                ToArg<ArgType6>::Get(a6),
                ToArg<ArgType7>::Get(a7),
                ToArg<ArgType8>::Get(a8)
                );


        SerializeBuffer result(ReturnMessageHeaderSize);
        if (!PackOutputParameters<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7, ArgType8>(
                    a1, a2, a3, a4, a5, a6, a7, a8, result))
        {
            return Status_RemoteInvalidOutputArguments;
        }

        return Return(invoke_info, result);
    }
};

} // end namespace Rpc

/// 定义桩类
#define RPC_BEGIN_STUB_CLASS(Class) \
template <typename T> \
class Class##Stub: public Rpc::StubImpl \
{ \
    typedef T ImplType; \
    typedef Class##Stub ThisType; \
    typedef Rpc::StubImpl BaseType; \
public: \
    Class##Stub() {} \
    Class##Stub(Rpc::ObjectId_t id) : Rpc::StubImpl(id) {} \
    virtual const typename Rpc::StubImpl::DispatchTable& GetDispatchTable() const;

#define RPC_STUB_METHOD_0(ReturnType, Name) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        return this->InvokeStub0<ReturnType, ImplType>( \
            invoke_info, buffer, &ImplType::Name, typename Rpc::ReturnTypeTag<ReturnType>::Type() \
            ); \
    }

#define RPC_STUB_METHOD_1(ReturnType, Name, ArgType) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        return this->InvokeStub1<ReturnType, ImplType, ArgType>( \
            invoke_info, buffer, &ImplType::Name, typename Rpc::ReturnTypeTag<ReturnType>::Type() \
            ); \
    }

#define RPC_STUB_METHOD_2(ReturnType, Name, ArgType1, ArgType2) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        return this->InvokeStub2<ReturnType, ImplType, ArgType1, ArgType2>( \
            invoke_info, buffer, &ImplType::Name, typename Rpc::ReturnTypeTag<ReturnType>::Type() \
            ); \
    }

#define RPC_STUB_METHOD_3(ReturnType, Name, ArgType1, ArgType2, ArgType3) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        return this->InvokeStub3<ReturnType, ImplType, ArgType1, ArgType2, ArgType3>( \
            invoke_info, buffer, &ImplType::Name, typename Rpc::ReturnTypeTag<ReturnType>::Type() \
            ); \
    }

#define RPC_STUB_METHOD_4(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        return this->InvokeStub4<ReturnType, ImplType, ArgType1, ArgType2, ArgType3, ArgType4>( \
            invoke_info, buffer, &ImplType::Name, typename Rpc::ReturnTypeTag<ReturnType>::Type() \
            ); \
    }

#define RPC_STUB_METHOD_5(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        return this->InvokeStub5<ReturnType, ImplType, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5>( \
            invoke_info, buffer, &ImplType::Name, typename Rpc::ReturnTypeTag<ReturnType>::Type() \
            ); \
    }

#define RPC_STUB_METHOD_6(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        return this->InvokeStub6<ReturnType, ImplType, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6>( \
            invoke_info, buffer, &ImplType::Name, typename Rpc::ReturnTypeTag<ReturnType>::Type() \
            ); \
    }

#define RPC_STUB_METHOD_7(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        return this->InvokeStub7<ReturnType, ImplType, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7>( \
            invoke_info, buffer, &ImplType::Name, typename Rpc::ReturnTypeTag<ReturnType>::Type() \
            ); \
    }

#define RPC_STUB_METHOD_8(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7, ArgType8) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        return this->InvokeStub8<ReturnType, ImplType, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7, ArgType8>( \
            invoke_info, buffer, &ImplType::Name, typename Rpc::ReturnTypeTag<ReturnType>::Type() \
            ); \
    }

///////////////////////////////////////////////////////////////////////////////
// 下面这组宏生成调用实现提供的实现函数的桩函数，
// 异步调用的实现函数原型要求为：
// Rpc::Status_t Name(const Rpc::InvokeContext& context, const Rpc::Buffer& buffer);
//
// 实现调用 context.GetParameters(buffer, a1, a2, a3 ...); 来提取调用参数
// 调用 context.Complete(result1, result2, ..., return_value) 来返回结果，并
// 调用结束后返回状态码。
// 对于值为 Rpc::Status_Success 和 Rpc::Status_Pending 之外的状态，都会自动调用 Abort。

/// 异步的方式调度成员函数
#define RPC_STUB_ASYNC_METHOD_0(ReturnType, Name) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        Rpc::InvokeContext context(invoke_info); \
        context.SetReturnHandler(static_cast<ReturnType*>(NULL)); \
        return (static_cast<ImplType*>(this))->Name(context, buffer); \
    }

#define RPC_STUB_ASYNC_METHOD_1(ReturnType, Name, ArgType1) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        Rpc::InvokeContext context(invoke_info); \
        context.SetParameterHandlers<ArgType1>(); \
        context.SetReturnHandler(static_cast<ReturnType*>(NULL)); \
        return (static_cast<ImplType*>(this))->Name(context, buffer); \
    }

#define RPC_STUB_ASYNC_METHOD_2(ReturnType, Name, ArgType1, ArgType2) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        Rpc::InvokeContext context(invoke_info); \
        context.SetParameterHandlers<ArgType1, ArgType2>(); \
        context.SetReturnHandler(static_cast<ReturnType*>(NULL)); \
        return (static_cast<ImplType*>(this))->Name(context, buffer); \
    }

#define RPC_STUB_ASYNC_METHOD_3(ReturnType, Name, ArgType1, ArgType2, ArgType3) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        Rpc::InvokeContext context(invoke_info); \
        context.SetParameterHandlers<ArgType1, ArgType2, ArgType3>(); \
        context.SetReturnHandler(static_cast<ReturnType*>(NULL)); \
        return (static_cast<ImplType*>(this))->Name(context, buffer); \
    }

#define RPC_STUB_ASYNC_METHOD_4(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        Rpc::InvokeContext context(invoke_info); \
        context.SetParameterHandlers<ArgType1, ArgType2, ArgType3, ArgType4>(); \
        context.SetReturnHandler(static_cast<ReturnType*>(NULL)); \
        return (static_cast<ImplType*>(this))->Name(context, buffer); \
    }

#define RPC_STUB_ASYNC_METHOD_5(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        Rpc::InvokeContext context(invoke_info); \
        context.SetParameterHandlers<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5>(); \
        context.SetReturnHandler(static_cast<ReturnType*>(NULL)); \
        return (static_cast<ImplType*>(this))->Name(context, buffer); \
    }

#define RPC_STUB_ASYNC_METHOD_6(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        Rpc::InvokeContext context(invoke_info); \
        context.SetParameterHandlers<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6>(); \
        context.SetReturnHandler(static_cast<ReturnType*>(NULL)); \
        return (static_cast<ImplType*>(this))->Name(context, buffer); \
    }

#define RPC_STUB_ASYNC_METHOD_7(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        Rpc::InvokeContext context(invoke_info); \
        context.SetParameterHandlers<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7>(); \
        context.SetReturnHandler(static_cast<ReturnType*>(NULL)); \
        return (static_cast<ImplType*>(this))->Name(context, buffer); \
    }

#define RPC_STUB_ASYNC_METHOD_8(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7, ArgType8) \
    Rpc::Status_t Stub##Name(const Rpc::InvokeInfo& invoke_info, const Rpc::Buffer& buffer) \
    { \
        Rpc::InvokeContext context(invoke_info); \
        context.SetParameterHandlers<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7, ArgType8>(); \
        context.SetReturnHandler(static_cast<ReturnType*>(NULL)); \
        return (static_cast<ImplType*>(this))->Name(context, buffer); \
    }

/// 桩定义结束
#define RPC_END_STUB_CLASS() \
};

/// 调度表开始
#define RPC_BEGIN_STUB_DISPATCH(Class) \
template <typename T> \
const typename Rpc::StubImpl::DispatchTable& Class##Stub<T>::GetDispatchTable() const \
{ \
    static const typename Rpc::StubImpl::DispatchEntry entries[] = \
    {

/// 调度成员函数
#define RPC_STUB_DISPATCH(Name, ArgCount) \
        { #Name "_" #ArgCount, static_cast<Rpc::StubImpl::StubMethod>(&ThisType::Stub##Name) },

/// 嵌套调用成员函数
#define RPC_STUB_NESTABLE_DISPATCH(Name, ArgCount) \
        { #Name "_" #ArgCount, static_cast<Rpc::StubImpl::StubMethod>(&ThisType::Stub##Name), true },

/// 以 NULL 名字结尾表示调度表结束
#define RPC_END_STUB_DISPATCH() \
        { NULL, NULL } \
    }; \
 \
    static const typename Rpc::StubImpl::DispatchTable table(entries, sizeof(entries)/sizeof(entries[0]) - 1); \
    return table; \
};

#include <common/rpc/invoke_context.hpp>

#endif//RPC_STUB_HPP_INCLUDED

