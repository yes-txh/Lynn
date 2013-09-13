#ifndef RPC_SERIALIZE_HPP_INCLUDED
#define RPC_SERIALIZE_HPP_INCLUDED

#include <assert.h>
#include <vector>
#include "protobuf/message.h"
#include "common/base/serialize/serialize.hpp"

namespace Rpc
{

typedef std::vector<char> SerializeBuffer;

/// 序列化
template <typename T>
bool SerializeNormalObject(const T& object, SerializeBuffer& buffer)
{
    ::Serialize::BinaryEncodeAppend(object, buffer);
    return true;
}

/// 反序列化
/// @return 消耗了多少字节
template <typename T>
bool UnserializeNormalObject(const char*& buffer, size_t& size, T& object)
{
    size_t n = ::Serialize::BinaryDecode(buffer, size, object);
    if (n > 0)
    {
        buffer += n;
        size -= n;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////
// 以下为参数修饰模板，起 IDL attribute 的作用
////////////////////////////////////////////////////////////////

/// 用于修饰入参数，只传入，const 引用，值，常量指针均可
template <typename T>
struct In
{
    In(const T& value) : Value(value) {}
    const T& Value;
};

template <typename T>
struct In<const T&>
{
    In(const T& value) : Value(value) {}
    const T& Value;
};

template <typename T>
struct In<const T*>
{
    In(const T* value) : Value(value) {}
    const T* Value;
};

/// ---------------------------------------------------------
/// Out 用于修饰出参数
/// Out 接受非 const 引用和非常量指针，所以此处不定义
/// 如果这里报错，说明 T 不属于上述两种类型
/// ---------------------------------------------------------
template <typename T> struct Out;
template <typename T> struct Out<const T&>;
template <typename T> struct Out<const T*>;

/// Out 接受非 const 引用类型
template <typename T>
struct Out<T&>
{
    Out(T& place) : Value(place) {}
    T& Value;
};

/// Out 可接受非常量指针类型
template <typename T>
struct Out<T*>
{
    Out(T* place) : Value(place) {}
    T* Value;
};
/// ---------------------------------------------------------
/// InOut定义：用于修饰出入参数，双向传递
/// ---------------------------------------------------------
template <typename T>
struct InOut {};

/// 常量引用和常量指针不允许出现在出参数
template <typename T> struct InOut<const T&>;
template <typename T> struct InOut<const T*>;

/// InOut 接受普通引用类型
template <typename T>
struct InOut<T&>
{
    InOut(T& place) : Value(place) {}
    T& Value;
};

/// InOut 接受普通指针类型
template <typename T>
struct InOut<T*>
{
    InOut(T* place) : Value(place) {}
    T* Value;
};

/// ------------------------------------------------------------
/// 提取参数类型的传递方向
/// ------------------------------------------------------------
/// 是否是入参数
template <typename T> struct IsIn  {
    static const bool Value = true;
};

/// 是否是出参数
template <typename T> struct IsOut {
    static const bool Value = false;
};

// in 修饰的类型是入参数
template <typename T> struct IsIn< In<T> >      {
    static const bool Value = true;
};

// out 修饰的类型不是入参数
template <typename T> struct IsIn< Out<T> >     {
    static const bool Value = false;
};

// in_out 修饰的类型也是入参数
template <typename T> struct IsIn< InOut<T> >   {
    static const bool Value = true;
};

// out 修饰的类型是出参数
template <typename T> struct IsOut< Out<T> >    {
    static const bool Value = true;
};

// in_out 修饰的类型也是出参数
template <typename T> struct IsOut< InOut<T> >  {
    static const bool Value = true;
};

// 引用和指针默认视为出入参数
template <typename T> struct IsOut<T&>          {
    static const bool Value = true;
};
template <typename T> struct IsOut<const T&>    {
    static const bool Value = false;
};

/// 参数是否是指针
template <typename T> struct IsPointer {
    static const bool Value = false;
};

/// 特化类型，判断出入参数是否是指针
template <typename T> struct IsPointer<T*> {
    static const bool Value = true;
};

template <typename T> struct RemovePointer {
    typedef T Type;
};

template <typename T> struct RemovePointer<T*> {
    typedef T Type;
};



/// 去除类型的 in/out/inout 修饰
template <typename T> struct RemoveInOut
{
    typedef T Type;
};

template <typename T> struct RemoveInOut<In<T> >
{
    typedef T Type;
};

template <typename T> struct RemoveInOut<Out<T> >
{
    typedef T Type;
};

template <typename T> struct RemoveInOut<InOut<T> >
{
    typedef T Type;
};

/// --------------------------------------------------------
/// 去除传递方向修饰，获得实际的对象的值
/// --------------------------------------------------------
template <typename T>
const T& ValueOf(const T& object)
{
    return object;
}

/// 传入指针求值
template <typename T>
const T& ValueOf(const T* object)
{
    return *object;
}

template <typename T>
T ValueOf(const In<T> & object)
{
    return object.Value;
}

/// In指针参数求值
template <typename T>
T ValueOf(const In<T*>& object)
{
    return *(object.Value);
}

template <typename T>
T ValueOf(const Out<T> & object)
{
    return object.Value;
}

/// 英雄无用武之地。占个坑吧。
template <typename T>
T ValueOf(const Out<T*>& object)
{
    return *(object.Value);
}

template <typename T>
T ValueOf(const InOut<T> & object)
{
    return object.Value;
}

/// InOut指针参数求值
template <typename T>
T ValueOf(const InOut<T*>& object)
{
    return *(object.Value);
}

/// ----------------------------------------------------------
/// 去除 in/out 修饰，获得实际对象的地址
/// ----------------------------------------------------------
/// 引用类型取地址
template <typename T>
void* AddressOf(const T& object)
{
    return (void*) &object;
}

template <typename T>
void* AddressOf(const In<T> & object)
{
    return (void*) &object.Value;
}

template <typename T>
void* AddressOf(const Out<T> & object)
{
    return (void*) &object.Value;
}

template <typename T>
void* AddressOf(const InOut<T> & object)
{
    return (void*) &object.Value;
}

/// 指针类型直接赋值
template <typename T>
void* AddressOf(const T* object)
{
    return (void*) object;
}

template <typename T>
void* AddressOf(const In<T*>& object)
{
    return (void*) object.Value;
}

template <typename T>
void* AddressOf(const Out<T*>& object)
{
    return (void*) object.Value;
}

template <typename T>
void* AddressOf(const InOut<T*>& object)
{
    return (void*) object.Value;
}

/// 去掉类型的 const 和引用属性
template <typename T>
struct RemoveCvRef
{
    typedef T Type;
};

template <typename T>
struct RemoveCvRef<T&>
{
    typedef T Type;
};

template <typename T>
struct RemoveCvRef<const T>
{
    typedef T Type;
};

template <typename T>
struct RemoveCvRef<const T&>
{
    typedef T Type;
};

/// 去除所有的类型修饰，得到最原始的类型名
template <typename T>
struct RawTypeOf
{
private:
    typedef typename RemoveInOut<T>::Type Type1;
    typedef typename RemovePointer<Type1>::Type Type2;
public:
    typedef typename RemoveCvRef<Type2>::Type Type;
};

class Proxy;
class Stub;

/// 判断一个类是不是从另一个类派生出来的
template <typename Derived, typename Base>
struct IsDerivedFrom
{
    // 检查 Derived 是不是完整类型
    static const bool IsCompleteType = sizeof(Derived) > 0;
private:
    // 如果 Derived 是 Base 的派生类，那么能匹配前者，返回类型为 char，否则匹配后者
    static char TestType(Base*);
    static int TestType(void*);
public:
    static const bool Value = (sizeof(TestType((Derived*)NULL)) == 1);
};

template <bool IsPbObject>
struct SerializeRpcObjectTraits
{
public:
    template <typename T>
    static bool Serialize(const T& object, SerializeBuffer& buffer)
    {
        return SerializeNormalObject(object, buffer);;
    }
    template <typename T>
    static bool Unserialize(const char*& buffer, size_t& size, T& object)
    {
        return UnserializeNormalObject(buffer, size, object);
    }
};

// Protocol Buffer Message 特别处理，用自有的接口进行序列和反序列化
template <>
struct SerializeRpcObjectTraits<true>
{
public:
    template <typename T>
    static bool Serialize(const T& object, SerializeBuffer& buffer)
    {
        std::string result;
        const google::protobuf::Message* message = dynamic_cast<const google::protobuf::Message*>(&object);
        bool ret = message->SerializeToString(&result);
        if (ret)
        {
            size_t length = result.size();
            ret = SerializeNormalObject(length, buffer);
            buffer.insert(buffer.end(), result.begin(), result.end());
        }
        return ret;
    }
    template <typename T>
    static bool Unserialize(const char*& buffer, size_t& size, T& object)
    {
        size_t length = 0;
        if (UnserializeNormalObject(buffer, size, length))
        {
            google::protobuf::Message* message = dynamic_cast<google::protobuf::Message*>(&object);
            bool ret = message->ParseFromArray(buffer, length);
            if (ret)
            {
                buffer += length;
                size -= length;
            }
            return ret;
        }
        return false;
    }
};

template <bool IsRpcObject>
struct SerializeTraits
{
public:
    template <typename T>
    static bool Serialize(const T& object, SerializeBuffer& buffer)
    {
        return SerializeRpcObjectTraits<
            IsDerivedFrom<T, google::protobuf::Message>::Value
            >::Serialize(object, buffer);
    }
    template <typename T>
    static bool Unserialize(const char*& buffer, size_t& size, T& object)
    {
        return SerializeRpcObjectTraits<
            IsDerivedFrom<T, google::protobuf::Message>::Value
            >::Unserialize(buffer, size, object);
    }
};

bool SerializeStub(const Stub& object, SerializeBuffer& buffer);
bool UnserializeProxy(const char*& buffer, size_t& size, Proxy& object);

// RPC stub 和 proxy 要特别序列化和反序列化
template <>
struct SerializeTraits<true>
{
public:
    static bool Serialize(const Stub& object, SerializeBuffer& buffer)
    {
        return SerializeStub(object, buffer);
    }

    static bool Unserialize(const char*& buffer, size_t& size, Stub& object)
    {
        assert(!"should not reach here");
        return false;
    }

    static bool Serialize(const Proxy& object, SerializeBuffer& buffer)
    {
        assert(!"should not reach here");
        return false;
    }

    static bool Unserialize(const char*& buffer, size_t& size, Proxy& object)
    {
        return UnserializeProxy(buffer, size, object);
    }
};

/// 序列化
template <typename T>
bool SerializeObject(const T& object, SerializeBuffer& buffer)
{
    return SerializeTraits<IsDerivedFrom<T, Proxy>::Value ||
        IsDerivedFrom<T, Stub>::Value>::Serialize(object, buffer);
}

template <typename T>
bool UnserializeObject(const char*& buffer, size_t& size, T& object)
{
    return SerializeTraits<
        IsDerivedFrom<T, Proxy>::Value || IsDerivedFrom<T, Stub>::Value
        >::Unserialize(buffer, size, object);
}

template <typename T>
bool UnserializeType(const char*& buffer, size_t& size, void* object)
{
    T* obj = static_cast<T*>(object);
    return UnserializeObject(buffer, size, *obj);
}

/// 调用消息的消息头的大小，用于序列化前预留空间
extern const size_t InvokeMessageHeaderSize;

/// 返回消息的消息头的大小，用于序列化前预留空间
extern const size_t ReturnMessageHeaderSize;

} // end namespace Rpc

#endif//RPC_SERIALIZE_HPP_INCLUDED
