#ifndef RPC_PROXY_HPP_INCLUDE
#define RPC_PROXY_HPP_INCLUDE

#include <stddef.h>
#include <assert.h>

#include <string>

#include <common/rpc/types.hpp>
#include <common/rpc/serialization.hpp>
#include <common/rpc/async_token.hpp>

namespace Rpc
{

// 传出参数解析器，调用之前初始化后存入调用表，调用完成后用于处理出参数。
class OutputParameterHandler
{
    typedef bool (*ParameterUnserializeFunction)(
        const char*& buffer, size_t& size,
        void* objects[], size_t count,
        size_t& index
    );

public:
    OutputParameterHandler():
        Count(0), ParamUnserializer(NULL)
    {
        Address[0] = NULL;
    }

    OutputParameterHandler(const OutputParameterHandler& rhs):
        Count(rhs.Count),
        ParamUnserializer(rhs.ParamUnserializer)
    {
        // 出参数总数可能未到最大值，只需要拷贝前 Count 个
        memcpy(Address, rhs.Address, Count * sizeof(Address[0]));
    }
    OutputParameterHandler& operator=(const OutputParameterHandler& rhs)
    {
        Count = rhs.Count;
        ParamUnserializer = rhs.ParamUnserializer;
        memcpy(Address, rhs.Address, Count * sizeof(Address[0]));
        return *this;
    }

    template <typename T>
    void PushArgument(T a)
    {
        if (IsOut<T>::Value)
            Address[Count++] = AddressOf(a);
    }

    /// 初始化出参数处理器
    template <typename T1>
    void Initialize(T1 a1)
    {
        PushArgument<T1>(a1);
        ParamUnserializer = UnserializeParameters<T1>;
    }

    template <typename T1, typename T2>
    void Initialize(T1 a1, T2 a2)
    {
        PushArgument<T1>(a1);
        PushArgument<T2>(a2);
        ParamUnserializer = UnserializeParameters<T1, T2>;
    }

    template <typename T1, typename T2, typename T3>
    void Initialize(T1 a1, T2 a2, T3 a3)
    {
        PushArgument<T1>(a1);
        PushArgument<T2>(a2);
        PushArgument<T3>(a3);
        ParamUnserializer = UnserializeParameters<T1, T2, T3>;
    }

    template <typename T1, typename T2, typename T3, typename T4>
    void Initialize(T1 a1, T2 a2, T3 a3, T4 a4)
    {
        PushArgument<T1>(a1);
        PushArgument<T2>(a2);
        PushArgument<T3>(a3);
        PushArgument<T4>(a4);
        ParamUnserializer = UnserializeParameters<T1, T2, T3, T4>;
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5>
    void Initialize(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5)
    {
        PushArgument<T1>(a1);
        PushArgument<T2>(a2);
        PushArgument<T3>(a3);
        PushArgument<T4>(a4);
        PushArgument<T5>(a5);
        ParamUnserializer = UnserializeParameters<T1, T2, T3, T4, T5>;
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void Initialize(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6)
    {
        PushArgument<T1>(a1);
        PushArgument<T2>(a2);
        PushArgument<T3>(a3);
        PushArgument<T4>(a4);
        PushArgument<T5>(a5);
        PushArgument<T6>(a6);
        ParamUnserializer = UnserializeParameters<T1, T2, T3, T4, T5, T6>;
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    void Initialize(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7)
    {
        PushArgument<T1>(a1);
        PushArgument<T2>(a2);
        PushArgument<T3>(a3);
        PushArgument<T4>(a4);
        PushArgument<T5>(a5);
        PushArgument<T6>(a6);
        PushArgument<T7>(a7);
        ParamUnserializer = UnserializeParameters<T1, T2, T3, T4, T5, T6, T7>;
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    void Initialize(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8)
    {
        PushArgument<T1>(a1);
        PushArgument<T2>(a2);
        PushArgument<T3>(a3);
        PushArgument<T4>(a4);
        PushArgument<T5>(a5);
        PushArgument<T6>(a6);
        PushArgument<T7>(a7);
        PushArgument<T8>(a8);
        ParamUnserializer = UnserializeParameters<T1, T2, T3, T4, T5, T6, T7, T8>;
    }
    // 收到返回的消息后，RPC 调度器根据 call id，调用本函数修改需要受影响的出参数。
    bool ExtractParamters(const char*& buffer, size_t& size)
    {
        size_t index = 0;
        if (ParamUnserializer)
            return ParamUnserializer(buffer, size, Address, Count, index) && index == Count;
        return true;
    }

private:
    /// 反序列化单个参数
    template <typename Type>
    static bool UnserializeParameter(const char*& buffer, size_t& size, void* objects[], size_t count, size_t& index)
    {
        // 跳过非出参数，视为成功，但是 index 不增加
        if (!IsOut<Type>::Value)
            return true;

        // 检查是否越界
        if (index >= count)
            return false;

        if (UnserializeType<typename RawTypeOf<Type>::Type>(buffer, size, objects[index]))
        {
            ++index;    // 下一个出参数
            return true;
        }

        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    // 不同参数个数的出参数反序列化函数，初始化时 ParamUnserializer 根据参数个
    // 数被设置为下面某一个。

    /// 用于一个参数的函数的反序列化函数
    template <typename T1>
    static bool UnserializeParameters(const char*& buffer, size_t& size, void* objects[], size_t count, size_t& index)
    {
        return UnserializeParameter<T1>(buffer, size, objects, count, index);
    }

    /// 用于两个参数的函数的反序列化函数
    template <typename T1, typename T2>
    static bool UnserializeParameters(const char*& buffer, size_t& size, void* objects[], size_t count, size_t& index)
    {
        return UnserializeParameters<T1>(buffer, size, objects, count, index) &&
               UnserializeParameter<T2>(buffer, size, objects, count, index);
    }

    /// 用于三个参数的函数的反序列化函数
    template <typename T1, typename T2, typename T3>
    static bool UnserializeParameters(const char*& buffer, size_t& size, void* objects[], size_t count, size_t& index)
    {
        return UnserializeParameters<T1, T2>(buffer, size, objects, count, index) &&
               UnserializeParameter<T3>(buffer, size, objects, count, index);
    }

    /// 用于四个参数的函数的反序列化函数
    template <typename T1, typename T2, typename T3, typename T4>
    static bool UnserializeParameters(const char*& buffer, size_t& size, void* objects[], size_t count, size_t& index)
    {
        return UnserializeParameters<T1, T2, T3>(buffer, size, objects, count, index) &&
               UnserializeParameter<T4>(buffer, size, objects, count, index);
    }

    /// 用于五个参数的函数的反序列化函数
    template <typename T1, typename T2, typename T3, typename T4, typename T5>
    static bool UnserializeParameters(const char*& buffer, size_t& size, void* objects[], size_t count, size_t& index)
    {
        return UnserializeParameters<T1, T2, T3, T4>(buffer, size, objects, count, index) &&
               UnserializeParameter<T5>(buffer, size, objects, count, index);
    }

    /// 用于六个参数的函数的反序列化函数
    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    static bool UnserializeParameters(const char*& buffer, size_t& size, void* objects[], size_t count, size_t& index)
    {
        return UnserializeParameters<T1, T2, T3, T4, T5>(buffer, size, objects, count, index) &&
               UnserializeParameter<T6>(buffer, size, objects, count, index);
    }

    /// 用于七个参数的函数的反序列化函数
    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    static bool UnserializeParameters(const char*& buffer, size_t& size, void* objects[], size_t count, size_t& index)
    {
        return UnserializeParameters<T1, T2, T3, T4, T5, T6>(buffer, size, objects, count, index) &&
               UnserializeParameter<T7>(buffer, size, objects, count, index);
    }

    /// 用于八个参数的函数的反序列化函数
    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    static bool UnserializeParameters(const char*& buffer, size_t& size, void* objects[], size_t count, size_t& index)
    {
        return UnserializeParameters<T1, T2, T3, T4, T5, T6, T7>(buffer, size, objects, count, index) &&
               UnserializeParameter<T8>(buffer, size, objects, count, index);
    }
private:
    // 传出参数按顺序记录在数组里，收到返回的缓冲区时按顺序解析
    size_t Count;
    void* Address[MaxParameterCount];
    ParameterUnserializeFunction ParamUnserializer;
};

///////////////////////////////////////////////////////////////////////////////
// 客户端的代理的基类
class Proxy
{
protected:
    Proxy():
        m_Pid(0),
        m_ObjectId(0)
    {
    }

    virtual ~Proxy() {}
public:
    /// 返回对象所在的端点
    const std::string& RpcEndPoint() const
    {
        return m_EndPoint;
    }

    /// 返回对象所在的进程的 PID
    int RpcPid() const
    {
        return m_Pid;
    }

    /// 返回对象的 ID
    ObjectId_t RpcObjectId() const
    {
        return m_ObjectId;
    }

    /// 绑定到指定的对象
    void RpcBind(const std::string& endpoint, int pid, ObjectId_t object_id)
    {
        m_EndPoint = endpoint;
        m_Pid = pid;
        m_ObjectId = object_id;
    }

    bool IsValid() const
    {
        return m_ObjectId > 0;
    }
protected:
    virtual const char* GetClassName() const = 0;
    /// 发起异步调用
    Status_t AsyncInvoke(
        const char* method_name,                        ///< 函数名
        int& method_id,                                 ///< 函数 id
        SerializeBuffer& input,                         ///< 输入参数
        const OutputParameterHandler& output_handler,   ///< 输出参数的处理器
        AsyncToken* token,                              ///< 异步完成令牌
        AsyncCallback callback,                         ///< 回调通知函数
        void* object,                                   ///< 回调通知对象指针
        void* param,                                    ///< 传给 callback 的任意额外参数
        int timeout                                     ///< 超时，单位毫秒
    );

private:
    std::string m_EndPoint;  ///< 对象所在的位置
    int        m_Pid;       ///< 远程对象所在进程的 ID, 用于验证对象是否还存在
    ObjectId_t m_ObjectId;  ///< 远程对象的 ID, 用于找到相应的对象
};

/// RPC 异常类，用于同步调用时的错误处理
class Exception : public std::runtime_error
{
public:
    Exception(
        Status_t status,        ///< 状态码
        const char* class_name, ///< 发生异常的类名
        const char* method_name ///< 发生异常的函数名
    ) :
        std::runtime_error(BuildErrorMessage(status, class_name, method_name))
    {
    }
private:
    static std::string BuildErrorMessage(Status_t status, const char* class_name, const char* method_name)
    {
        std::string result = "Rpc::Exception: ";
        result += class_name;
        result += "::";
        result += method_name;
        result += ", ";
        result += StatusString(status, "<Unknown status code>");
        return result;
    }
};

///////////////////////////////////////////////////////////////////////////////
/// 框架提供的 Proxy 的通用实现方式
class ProxyImpl : public Proxy
{
protected:
    ///////////////////////////////////////////////////////////////////////////
    // 被下面的 RPC_PROXY_METHOD 宏所使用的代理函数

    /// 无参数的版本
    /// @return 状态码
    Status_t ProxyAsyncInvoke0(
        const char* method_name,///< 方法名
        int& method_id,         ///< 方法 ID
        AsyncToken* token,      ///< 异步完成令牌
        AsyncCallback callback, ///< 回调函数指针
        void* object,           ///< 回调函数的 context 参数
        void* param,            ///< 回调函数的 param 参数
        int timeout             ///< 超时
    )
    {
        if (!IsValid())
            return Status_InvalidObject;

        SerializeBuffer buffer(InvokeMessageHeaderSize);
        OutputParameterHandler output_handler;
        return this->AsyncInvoke(
                   method_name, method_id,
                   buffer, output_handler,
                   token,
                   callback, object, param,
                   timeout
               );
    }

    /// 代理函数
    /// 一个参数的版本
    /// @return 状态码
    template <typename ArgType1>
    Status_t ProxyAsyncInvoke1(
        const char* method_name,///< 方法名
        int& method_id,         ///< 方法 ID
        ArgType1 a1,            ///< 实际方法的第一个参数
        AsyncToken* token,      ///< 异步完成令牌
        AsyncCallback callback, ///< 回调函数指针
        void* object,           ///< 回调函数的 context 参数
        void* param,            ///< 回调函数的 param 参数
        int timeout             ///< 超时
    )
    {
        if (!IsValid())
            return Status_InvalidObject;

        // 输入参数打包
        SerializeBuffer buffer(InvokeMessageHeaderSize);
        if (!SerializeParameters_1<ArgType1>(a1, buffer))
            return Status_LocalInvalidInputArguments;

        // 设置输出参数的解码器
        OutputParameterHandler output_handler;
        output_handler.Initialize<ArgType1>(a1);

        return this->AsyncInvoke(
                   method_name, method_id,
                   buffer, output_handler,
                   token,
                   callback, object, param,
                   timeout
               );
    }

    /// 代理函数
    /// 两个参数的版本
    /// @return 状态码
    template <typename ArgType1, typename ArgType2>
    Status_t ProxyAsyncInvoke2(
        const char* method_name,///< 方法名
        int& method_id,         ///< 方法 ID
        ArgType1 a1,            ///< 实际方法的第一个参数
        ArgType2 a2,            ///< 实际方法的第二个参数
        AsyncToken* token,      ///< 异步完成令牌
        AsyncCallback callback, ///< 回调函数指针
        void* object,           ///< 回调函数的 context 参数
        void* param,            ///< 回调函数的 param 参数
        int timeout             ///< 超时
    )
    {
        if (!IsValid())
            return Status_InvalidObject;

        // 输入参数打包
        SerializeBuffer buffer(InvokeMessageHeaderSize);
        if (!SerializeParameters_2<ArgType1, ArgType2>(a1, a2, buffer))
            return Status_LocalInvalidInputArguments;

        // 设置输出参数的解码器
        OutputParameterHandler output_handler;
        output_handler.Initialize<ArgType1, ArgType2>(a1, a2);

        return this->AsyncInvoke(
                   method_name, method_id,
                   buffer, output_handler,
                   token,
                   callback, object, param,
                   timeout
               );
    }

    /// 代理函数
    /// 三个参数的版本
    /// @return 状态码
    template <typename ArgType1, typename ArgType2, typename ArgType3>
    Status_t ProxyAsyncInvoke3(
        const char* method_name,///< 方法名
        int& method_id,         ///< 方法 ID
        ArgType1 a1,            ///< 实际方法的第一个参数
        ArgType2 a2,            ///< 实际方法的第二个参数
        ArgType3 a3,            ///< 实际方法的第三个参数
        AsyncToken* token,      ///< 异步完成令牌
        AsyncCallback callback, ///< 回调函数指针
        void* object,           ///< 回调函数的 context 参数
        void* param,            ///< 回调函数的 param 参数
        int timeout             ///< 超时
    )
    {
        if (!IsValid())
            return Status_InvalidObject;

        // 输入参数打包
        SerializeBuffer buffer(InvokeMessageHeaderSize);
        if (!SerializeParameters_3<ArgType1, ArgType2, ArgType3>(a1, a2, a3, buffer))
            return Status_LocalInvalidInputArguments;

        // 设置输出参数的解码器
        OutputParameterHandler output_handler;
        output_handler.Initialize<ArgType1, ArgType2, ArgType3>(a1, a2, a3);

        return this->AsyncInvoke(
                   method_name, method_id,
                   buffer, output_handler,
                   token,
                   callback, object, param,
                   timeout
               );
    }

    /// 代理函数
    /// 四个参数的版本
    /// @return 状态码
    template <typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4>
    Status_t ProxyAsyncInvoke4(
        const char* method_name,///< 方法名
        int& method_id,         ///< 方法 ID
        ArgType1 a1,            ///< 实际方法的第一个参数
        ArgType2 a2,            ///< 实际方法的第二个参数
        ArgType3 a3,            ///< 实际方法的第三个参数
        ArgType4 a4,            ///< 实际方法的第四个参数
        AsyncToken* token,      ///< 异步完成令牌
        AsyncCallback callback, ///< 回调函数指针
        void* object,           ///< 回调函数的 context 参数
        void* param,            ///< 回调函数的 param 参数
        int timeout             ///< 超时
    )
    {
        if (!IsValid())
            return Status_InvalidObject;

        // 输入参数打包
        SerializeBuffer buffer(InvokeMessageHeaderSize);
        if (!SerializeParameters_4<ArgType1, ArgType2, ArgType3, ArgType4>(a1, a2, a3, a4, buffer))
            return Status_LocalInvalidInputArguments;

        // 设置输出参数的解码器
        OutputParameterHandler output_handler;
        output_handler.Initialize<ArgType1, ArgType2, ArgType3, ArgType4>(a1, a2, a3, a4);

        return this->AsyncInvoke(
                   method_name, method_id,
                   buffer, output_handler,
                   token,
                   callback, object, param,
                   timeout
               );
    }

    /// 代理函数
    /// 五个参数的版本
    /// @return 状态码
    template <typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename ArgType5>
    Status_t ProxyAsyncInvoke5(
        const char* method_name,///< 方法名
        int& method_id,         ///< 方法 ID
        ArgType1 a1,            ///< 实际方法的第一个参数
        ArgType2 a2,            ///< 实际方法的第二个参数
        ArgType3 a3,            ///< 实际方法的第三个参数
        ArgType4 a4,            ///< 实际方法的第四个参数
        ArgType5 a5,            ///< 实际方法的第五个参数
        AsyncToken* token,      ///< 异步完成令牌
        AsyncCallback callback, ///< 回调函数指针
        void* object,           ///< 回调函数的 context 参数
        void* param,            ///< 回调函数的 param 参数
        int timeout             ///< 超时
    )
    {
        if (!IsValid())
            return Status_InvalidObject;

        // 输入参数打包
        SerializeBuffer buffer(InvokeMessageHeaderSize);
        if (!SerializeParameters_5<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5>(a1, a2, a3, a4, a5, buffer))
            return Status_LocalInvalidInputArguments;

        // 设置输出参数的解码器
        OutputParameterHandler output_handler;
        output_handler.Initialize<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5>(a1, a2, a3, a4, a5);

        return this->AsyncInvoke(
                   method_name, method_id,
                   buffer, output_handler,
                   token,
                   callback, object, param,
                   timeout
               );
    }

    /// 代理函数
    /// 六个参数的版本
    /// @return RPC 状态码
    template <typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename ArgType5, typename ArgType6>
    Status_t ProxyAsyncInvoke6(
        const char* method_name,///< 方法名
        int& method_id,         ///< 方法 ID
        ArgType1 a1,            ///< 实际方法的第一个参数
        ArgType2 a2,            ///< 实际方法的第二个参数
        ArgType3 a3,            ///< 实际方法的第三个参数
        ArgType4 a4,            ///< 实际方法的第四个参数
        ArgType5 a5,            ///< 实际方法的第五个参数
        ArgType6 a6,            ///< 实际方法的第六个参数
        AsyncToken* token,      ///< 异步完成令牌
        AsyncCallback callback, ///< 回调函数指针
        void* object,           ///< 回调函数的 context 参数
        void* param,            ///< 回调函数的 param 参数
        int timeout             ///< 超时
    )
    {
        if (!IsValid())
            return Status_InvalidObject;

        // 输入参数打包
        SerializeBuffer buffer(InvokeMessageHeaderSize);
        if (!SerializeParameters_6<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6>(
                    a1, a2, a3, a4, a5, a6, buffer)
           )
        {
            return Status_LocalInvalidInputArguments;
        }

        // 设置输出参数的解码器
        OutputParameterHandler output_handler;
        output_handler.Initialize<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6>(a1, a2, a3, a4, a5, a6);

        return this->AsyncInvoke(
                   method_name, method_id,
                   buffer, output_handler,
                   token,
                   callback, object, param,
                   timeout
               );
    }

    /// 代理函数
    /// 七个参数的版本
    /// @return RPC 状态码
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7
             >
    Status_t ProxyAsyncInvoke7(
        const char* method_name,///< 方法名
        int& method_id,         ///< 方法 ID
        ArgType1 a1,            ///< 实际方法的第一个参数
        ArgType2 a2,            ///< 实际方法的第二个参数
        ArgType3 a3,            ///< 实际方法的第三个参数
        ArgType4 a4,            ///< 实际方法的第四个参数
        ArgType5 a5,            ///< 实际方法的第五个参数
        ArgType6 a6,            ///< 实际方法的第六个参数
        ArgType7 a7,            ///< 实际方法的第七个参数
        AsyncToken* token,      ///< 异步完成令牌
        AsyncCallback callback, ///< 回调函数指针
        void* object,           ///< 回调函数的 context 参数
        void* param,            ///< 回调函数的 param 参数
        int timeout             ///< 超时
    )
    {
        if (!IsValid())
            return Status_InvalidObject;

        // 输入参数打包
        SerializeBuffer buffer(InvokeMessageHeaderSize);
        if (!SerializeParameters_7<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7>(
                    a1, a2, a3, a4, a5, a6, a7, buffer)
           )
        {
            return Status_LocalInvalidInputArguments;
        }

        // 设置输出参数的解码器
        OutputParameterHandler output_handler;
        output_handler.Initialize<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7>(a1, a2, a3, a4, a5, a6, a7);

        return this->AsyncInvoke(
                   method_name, method_id,
                   buffer, output_handler,
                   token,
                   callback, object, param,
                   timeout
               );
    }

    /// 代理函数
    /// 八个参数的版本
    /// @return RPC 状态码
    template <
    typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4,
             typename ArgType5, typename ArgType6, typename ArgType7, typename ArgType8
             >
    Status_t ProxyAsyncInvoke8(
        const char* method_name,///< 方法名
        int& method_id,         ///< 方法 ID
        ArgType1 a1,            ///< 实际方法的第一个参数
        ArgType2 a2,            ///< 实际方法的第二个参数
        ArgType3 a3,            ///< 实际方法的第三个参数
        ArgType4 a4,            ///< 实际方法的第四个参数
        ArgType5 a5,            ///< 实际方法的第五个参数
        ArgType6 a6,            ///< 实际方法的第六个参数
        ArgType7 a7,            ///< 实际方法的第七个参数
        ArgType8 a8,            ///< 实际方法的第八个参数
        AsyncToken* token,      ///< 异步完成令牌
        AsyncCallback callback, ///< 回调函数指针
        void* object,           ///< 回调函数的 context 参数
        void* param,            ///< 回调函数的 param 参数
        int timeout             ///< 超时
    )
    {
        if (!IsValid())
            return Status_InvalidObject;

        // 输入参数打包
        SerializeBuffer buffer(InvokeMessageHeaderSize);
        if (!SerializeParameters_8<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7, ArgType8>(
                    a1, a2, a3, a4, a5, a6, a7, a8, buffer)
           )
        {
            return Status_LocalInvalidInputArguments;
        }

        // 设置输出参数的解码器
        OutputParameterHandler output_handler;
        output_handler.Initialize<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7, ArgType8>(
            a1, a2, a3, a4, a5, a6, a7, a8);

        return this->AsyncInvoke(
                   method_name, method_id,
                   buffer, output_handler,
                   token,
                   callback, object, param,
                   timeout
               );
    }

    /// 用来实现同步调用的善后处理函数
    /// 在 token 上等待调用完成并返回结果，以及处理中间发生的错误
    template <typename ReturnType>
    ReturnType SyncInvokeReturn(
        const char* class_name,             ///< 类名
        const char* method_name,            ///< 方法名
        Status_t status,                    ///< 状态码
        AsyncTokenOf<ReturnType>& token,    ///< 异步完成令牌
        int timeout,                        ///< 超时
        Status_t* out_status                ///< 传出状态
    )
    {
        if (status == Status_Pending)
        {
            // 尚未完成，等待其完成
            status = token.Wait(timeout);
            if (status == Status_Timeout)
                token.Cancel(Status_Timeout);
        }

        if (status == Status_Success)
        {
            if (out_status != NULL)
                *out_status = status;
            return token.Result();
        }

        if (out_status == NULL)
        {
            throw Exception(status, class_name, method_name);
        }
        else
        {
            *out_status = status;
            return ReturnType();
        }
    }

private:
    /// 序列化一个参数
    template <typename T>
    static bool SerializeParameters_1(T a, SerializeBuffer& buffer)
    {
        return SerializeParameter<T>(a, buffer);
    }

    /// 序列化两个参数
    template <typename T1, typename T2>
    static bool SerializeParameters_2(T1 a1, T2 a2, SerializeBuffer& buffer)
    {
        return SerializeParameters_1<T1>(a1, buffer) && SerializeParameter<T2>(a2, buffer);
    }

    /// 序列化三个参数
    template <typename T1, typename T2, typename T3>
    static bool SerializeParameters_3(T1 a1, T2 a2, T3 a3, SerializeBuffer& buffer)
    {
        return SerializeParameters_2<T1, T2>(a1, a2, buffer) && SerializeParameter<T3>(a3, buffer);
    }

    /// 序列化四个参数
    template <typename T1, typename T2, typename T3, typename T4>
    static bool SerializeParameters_4(T1 a1, T2 a2, T3 a3, T4 a4, SerializeBuffer& buffer)
    {
        return SerializeParameters_3<T1, T2, T3>(a1, a2, a3, buffer) &&
               SerializeParameter<T4>(a4, buffer);
    }

    /// 序列化五个参数
    template <typename T1, typename T2, typename T3, typename T4, typename T5>
    static bool SerializeParameters_5(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, SerializeBuffer& buffer)
    {
        return SerializeParameters_4<T1, T2, T3, T4>(a1, a2, a3, a4, buffer) &&
               SerializeParameter<T5>(a5, buffer);
    }

    /// 序列化六个参数
    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    static bool SerializeParameters_6(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, SerializeBuffer& buffer)
    {
        return SerializeParameters_5<T1, T2, T3, T4, T5>(a1, a2, a3, a4, a5, buffer) &&
               SerializeParameter<T6>(a6, buffer);
    }

    /// 序列化七个参数
    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    static bool SerializeParameters_7(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, SerializeBuffer& buffer)
    {
        return SerializeParameters_6<T1, T2, T3, T4, T5, T6>(a1, a2, a3, a4, a5, a6, buffer) &&
               SerializeParameter<T7>(a7, buffer);
    }

    /// 序列化七个参数
    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    static bool SerializeParameters_8(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8, SerializeBuffer& buffer)
    {
        return SerializeParameters_7<T1, T2, T3, T4, T5, T6, T7>(a1, a2, a3, a4, a5, a6, a7, buffer) &&
               SerializeParameter<T8>(a8, buffer);
    }

    /// 序列化单个参数
    template <typename T>
    static bool SerializeParameter(T a, SerializeBuffer& buffer)
    {
        // 跳过非入参数
        return !IsIn<T>::Value || SerializeObject(ValueOf(a), buffer);
    }
};

/// 适配成员函数用的通用的 RPC 完成回调函数
template<typename Class, void (Class::*Member)(Rpc::Status_t status, void*)>
void GenericMemberCallback(void* context, Status_t status, void* param)
{
    (static_cast<Class*>(context)->*Member)(status, param);
}


} // end namespace Rpc

/// 把类成员函数转为普通函数用来做回调。
/// 例如存在成员函数：@code
/// void Test::OnComplete(Status_t status, void* param);
/// @endcode
/// 用 RPC_MAKE_MEMBER_CALLBACK(Test, OnComplete)，既可生成可用于异步回调的函数指针，类型为@code
/// void (*)(void* object, Status_t status, void* param)
/// @endcode
/// 注意开头多了一个参数，用来传递 this 指针。
#define RPC_MAKE_MEMBER_CALLBACK(Class, Member) &Rpc::GenericMemberCallback<Class, &Class::Member>

/// 开始代理类的定义，是 RPC_BEGIN_PROXY_CLASS 和 RPC_END_PROXY_CLASS 所定义的块的开头。
/// 范例：@code
/// RPC_BEGIN_PROXY_CLASS(Demo)
///     RPC_PROXY_METHOD_0(void, Method1, 1)
///     RPC_PROXY_METHOD_2(int, Method2, int, int, 1)
/// RPC_END_PROXY_CLASS()
/// @endcode
/// @param Class 类名，所生成的类名为 Class 加上 “Proxy”后缀。
#define RPC_BEGIN_PROXY_CLASS(Class) \
    class Class##Proxy : public Rpc::ProxyImpl \
    { \
        virtual const char* GetClassName() const { return #Class; }

/// @brief 生成无参数的代理方法，放在 RPC_BEGIN_PROXY_CLASS 和 RPC_END_PROXY_CLASS 块中间使用。
/// @param ReturnType 返回值的类型
/// @param Name 函数名
/// @param Timeout 默认的超时时间，单位毫秒
#define RPC_PROXY_METHOD_0(ReturnType, Name, Timeout) \
    public: \
        Rpc::Status_t Async##Name( \
            Rpc::AsyncTokenOf<ReturnType>* token = NULL, \
            Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL, \
            int timeout = Timeout \
        ) \
        { \
            static int method_id = -1; \
            return ProxyAsyncInvoke0(#Name "_0", method_id, token, callback, context, param, timeout); \
        } \
        ReturnType Name(Rpc::Status_t* status = NULL, int timeout = Timeout) \
        { \
            Rpc::AsyncTokenOf<ReturnType> async_token; \
            return SyncInvokeReturn( \
                GetClassName(), #Name, \
                Async##Name(&async_token, NULL, NULL, NULL, -1), \
                async_token, timeout, status \
            ); \
        }

/// @brief 生成一个参数的代理方法，放在 RPC_BEGIN_PROXY_CLASS 和 RPC_END_PROXY_CLASS 块中间使用。
/// @param ReturnType 返回值的类型
/// @param Name 函数名
/// @param ArgType1 第一个参数的类型
/// @param Timeout 默认的超时时间，单位毫秒
#define RPC_PROXY_METHOD_1(ReturnType, Name, ArgType1, Timeout) \
    public: \
        Rpc::Status_t Async##Name( \
            ArgType1 a1, \
            Rpc::AsyncTokenOf<ReturnType>* token = NULL, \
            Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL, \
            int timeout = Timeout \
        ) \
        { \
            static int method_id = -1; \
            return ProxyAsyncInvoke1<ArgType1>(#Name "_1", method_id, a1, token, callback, context, param, timeout); \
        } \
        ReturnType Name(ArgType1 a1, Rpc::Status_t* status = NULL, int timeout = Timeout) \
        { \
            Rpc::AsyncTokenOf<ReturnType> async_token; \
            return SyncInvokeReturn( \
                GetClassName(), #Name, \
                Async##Name(a1, &async_token, NULL, NULL, NULL, -1), \
                async_token, timeout, status \
            ); \
        }

/// @brief 生成两个参数的代理方法，放在 RPC_BEGIN_PROXY_CLASS 和 RPC_END_PROXY_CLASS 块中间使用。
/// @param ReturnType 返回值的类型
/// @param Name 函数名
/// @param ArgType1 第一个参数的类型
/// @param ArgType2 第二个参数的类型
/// @param Timeout 默认的超时时间，单位毫秒
#define RPC_PROXY_METHOD_2(ReturnType, Name, ArgType1, ArgType2, Timeout) \
    public: \
        Rpc::Status_t Async##Name( \
            ArgType1 a1, ArgType2 a2, \
            Rpc::AsyncTokenOf<ReturnType>* token = NULL, \
            Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL, \
            int timeout = Timeout \
        ) \
        { \
            static int method_id = -1; \
            return ProxyAsyncInvoke2<ArgType1, ArgType2>( \
                #Name "_2", method_id, a1, a2, token, callback, context, param, timeout \
                ); \
        } \
        ReturnType Name(ArgType1 a1, ArgType2 a2, Rpc::Status_t* status = NULL, int timeout = Timeout) \
        { \
            Rpc::AsyncTokenOf<ReturnType> async_token; \
            return SyncInvokeReturn( \
                GetClassName(), #Name, \
                Async##Name(a1, a2, &async_token, NULL, NULL, NULL, -1), \
                async_token, timeout, status \
            ); \
        }

/// @brief 生成三个参数的代理方法，放在 RPC_BEGIN_PROXY_CLASS 和 RPC_END_PROXY_CLASS 块中间使用。
/// @param ReturnType 返回值的类型
/// @param Name 函数名
/// @param ArgType1 第一个参数的类型
/// @param ArgType2 第二个参数的类型
/// @param ArgType3 第三个参数的类型
/// @param Timeout 默认的超时时间，单位毫秒
#define RPC_PROXY_METHOD_3(ReturnType, Name, ArgType1, ArgType2, ArgType3, Timeout) \
    public: \
        Rpc::Status_t Async##Name( \
            ArgType1 a1, ArgType2 a2, ArgType3 a3, \
            Rpc::AsyncTokenOf<ReturnType>* token = NULL, \
            Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL, \
            int timeout = Timeout \
        ) \
        { \
            static int method_id = -1; \
            return ProxyAsyncInvoke3<ArgType1, ArgType2, ArgType3>( \
                #Name "_3", method_id, a1, a2, a3, token, callback, context, param, timeout \
                ); \
        } \
        ReturnType Name(ArgType1 a1, ArgType2 a2, ArgType3 a3, \
            Rpc::Status_t* status = NULL, int timeout = Timeout) \
        { \
            Rpc::AsyncTokenOf<ReturnType> async_token; \
            return SyncInvokeReturn( \
                GetClassName(), #Name, \
                Async##Name(a1, a2, a3, &async_token, NULL, NULL, NULL, -1), \
                async_token, timeout, status \
            ); \
        }

/// @brief 生成四个参数的代理方法，放在 RPC_BEGIN_PROXY_CLASS 和 RPC_END_PROXY_CLASS 块中间使用。
/// @param ReturnType 返回值的类型
/// @param Name 函数名
/// @param ArgType1 第一个参数的类型
/// @param ArgType2 第二个参数的类型
/// @param ArgType3 第三个参数的类型
/// @param ArgType4 第四个参数的类型
/// @param Timeout 默认的超时时间，单位毫秒
#define RPC_PROXY_METHOD_4(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, Timeout) \
    public: \
        Rpc::Status_t Async##Name( \
            ArgType1 a1, ArgType2 a2, ArgType3 a3, ArgType4 a4, \
            Rpc::AsyncTokenOf<ReturnType>* token = NULL, \
            Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL, \
            int timeout = Timeout \
        ) \
        { \
            static int method_id = -1; \
            return ProxyAsyncInvoke4<ArgType1, ArgType2, ArgType3, ArgType4>( \
                #Name "_4", method_id, a1, a2, a3, a4, token, callback, context, param, timeout \
                ); \
        } \
        ReturnType Name(ArgType1 a1, ArgType2 a2, ArgType3 a3, ArgType4 a4, \
            Rpc::Status_t* status = NULL, int timeout = Timeout \
        ) \
        { \
            Rpc::AsyncTokenOf<ReturnType> async_token; \
            return SyncInvokeReturn( \
                GetClassName(), #Name, \
                Async##Name(a1, a2, a3, a4, &async_token, NULL, NULL, NULL, -1), \
                async_token, timeout, status \
            ); \
        }

/// @brief 生成五个参数的代理方法，放在 RPC_BEGIN_PROXY_CLASS 和 RPC_END_PROXY_CLASS 块中间使用。
/// @param ReturnType 返回值的类型
/// @param Name 函数名
/// @param ArgType1 第一个参数的类型
/// @param ArgType2 第二个参数的类型
/// @param ArgType3 第三个参数的类型
/// @param ArgType4 第四个参数的类型
/// @param ArgType5 第五个参数的类型
/// @param Timeout 默认的超时时间，单位毫秒
#define RPC_PROXY_METHOD_5(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, Timeout) \
    public: \
        Rpc::Status_t Async##Name( \
            ArgType1 a1, ArgType2 a2, ArgType3 a3, ArgType4 a4, ArgType5 a5, \
            Rpc::AsyncTokenOf<ReturnType>* token = NULL, \
            Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL, \
            int timeout = Timeout \
        ) \
        { \
            static int method_id = -1; \
            return ProxyAsyncInvoke5<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5> \
                (#Name "_5", method_id, a1, a2, a3, a4, a5, token, callback, context, param, timeout \
                ); \
        } \
        ReturnType Name(ArgType1 a1, ArgType2 a2, ArgType3 a3, ArgType4 a4, ArgType5 a5, \
            Rpc::Status_t* status = NULL, int timeout = Timeout \
        ) \
        { \
            Rpc::AsyncTokenOf<ReturnType> async_token; \
            return SyncInvokeReturn( \
                GetClassName(), #Name, \
                Async##Name(a1, a2, a3, a4, a5, &async_token, NULL, NULL, NULL, -1), \
                async_token, timeout, status \
            ); \
        }

/// @brief 生成六个参数的代理方法，放在 RPC_BEGIN_PROXY_CLASS 和 RPC_END_PROXY_CLASS 块中间使用。
/// @param ReturnType 返回值的类型
/// @param Name 函数名
/// @param ArgType1 第一个参数的类型
/// @param ArgType2 第二个参数的类型
/// @param ArgType3 第三个参数的类型
/// @param ArgType4 第四个参数的类型
/// @param ArgType5 第五个参数的类型
/// @param ArgType6 第六个参数的类型
/// @param Timeout 默认的超时时间，单位毫秒
#define RPC_PROXY_METHOD_6(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, Timeout) \
    public: \
        Rpc::Status_t Async##Name( \
            ArgType1 a1, ArgType2 a2, ArgType3 a3, ArgType4 a4, ArgType5 a5, ArgType6 a6, \
            Rpc::AsyncTokenOf<ReturnType>* token = NULL, \
            Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL, \
            int timeout = Timeout \
        ) \
        { \
            static int method_id = -1; \
            return ProxyAsyncInvoke6<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6> \
                (#Name "_6", method_id, a1, a2, a3, a4, a5, a6, token, callback, context, param, timeout \
                ); \
        } \
        ReturnType Name(ArgType1 a1, ArgType2 a2, ArgType3 a3, ArgType4 a4, ArgType5 a5, ArgType6 a6, \
            Rpc::Status_t* status = NULL, int timeout = Timeout \
        ) \
        { \
            Rpc::AsyncTokenOf<ReturnType> async_token; \
            return SyncInvokeReturn( \
                GetClassName(), #Name, \
                Async##Name(a1, a2, a3, a4, a5, a6, &async_token, NULL, NULL, NULL, -1), \
                async_token, timeout, status \
            ); \
        }

/// @brief 生成七个参数的代理方法，放在 RPC_BEGIN_PROXY_CLASS 和 RPC_END_PROXY_CLASS 块中间使用。
/// @param ReturnType 返回值的类型
/// @param Name 函数名
/// @param ArgType1 第一个参数的类型
/// @param ArgType2 第二个参数的类型
/// @param ArgType3 第三个参数的类型
/// @param ArgType4 第四个参数的类型
/// @param ArgType5 第五个参数的类型
/// @param ArgType6 第六个参数的类型
/// @param ArgType7 第七个参数的类型
/// @param Timeout 默认的超时时间，单位毫秒
#define RPC_PROXY_METHOD_7(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7, Timeout) \
    public: \
        Rpc::Status_t Async##Name( \
            ArgType1 a1, ArgType2 a2, ArgType3 a3, ArgType4 a4, ArgType5 a5, ArgType6 a6, ArgType7 a7, \
            Rpc::AsyncTokenOf<ReturnType>* token = NULL, \
            Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL, \
            int timeout = Timeout \
        ) \
        { \
            static int method_id = -1; \
            return ProxyAsyncInvoke7<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7> \
                (#Name "_7", method_id, a1, a2, a3, a4, a5, a6, a7, token, callback, context, param, timeout \
                ); \
        } \
        ReturnType Name(ArgType1 a1, ArgType2 a2, ArgType3 a3, ArgType4 a4, ArgType5 a5, ArgType6 a6, ArgType7 a7, \
            Rpc::Status_t* status = NULL, int timeout = Timeout \
        ) \
        { \
            Rpc::AsyncTokenOf<ReturnType> async_token; \
            return SyncInvokeReturn( \
                GetClassName(), #Name, \
                Async##Name(a1, a2, a3, a4, a5, a6, a7, &async_token, NULL, NULL, NULL, -1), \
                async_token, timeout, status \
            ); \
        }

/// @brief 生成八个参数的代理方法，放在 RPC_BEGIN_PROXY_CLASS 和 RPC_END_PROXY_CLASS 块中间使用。
/// @param ReturnType 返回值的类型
/// @param Name 函数名
/// @param ArgType1 第一个参数的类型
/// @param ArgType2 第二个参数的类型
/// @param ArgType3 第三个参数的类型
/// @param ArgType4 第四个参数的类型
/// @param ArgType5 第五个参数的类型
/// @param ArgType6 第六个参数的类型
/// @param ArgType7 第七个参数的类型
/// @param ArgType7 第八个参数的类型
/// @param Timeout 默认的超时时间，单位毫秒
#define RPC_PROXY_METHOD_8(ReturnType, Name, ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7, ArgType8, Timeout) \
    public: \
        Rpc::Status_t Async##Name( \
            ArgType1 a1, ArgType2 a2, ArgType3 a3, ArgType4 a4, ArgType5 a5, ArgType6 a6, ArgType7 a7, ArgType8 a8, \
            Rpc::AsyncTokenOf<ReturnType>* token = NULL, \
            Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL, \
            int timeout = Timeout \
        ) \
        { \
            static int method_id = -1; \
            return ProxyAsyncInvoke8<ArgType1, ArgType2, ArgType3, ArgType4, ArgType5, ArgType6, ArgType7, ArgType8> \
                (#Name "_8", method_id, a1, a2, a3, a4, a5, a6, a7, a8, token, callback, context, param, timeout \
                ); \
        } \
        ReturnType Name(ArgType1 a1, ArgType2 a2, ArgType3 a3, ArgType4 a4, ArgType5 a5, ArgType6 a6, ArgType7 a7, ArgType8 a8, \
            Rpc::Status_t* status = NULL, int timeout = Timeout \
        ) \
        { \
            Rpc::AsyncTokenOf<ReturnType> async_token; \
            return SyncInvokeReturn( \
                GetClassName(), #Name, \
                Async##Name(a1, a2, a3, a4, a5, a6, a7, a8, &async_token, NULL, NULL, NULL, -1), \
                async_token, timeout, status \
            ); \
        }
/// 结束定义代理类，是 RPC_BEGIN_PROXY_CLASS 和 RPC_END_PROXY_CLASS 所定义的块的开头。
#define RPC_END_PROXY_CLASS() \
    };

#endif//RPC_PROXY_HPP_INCLUDE

