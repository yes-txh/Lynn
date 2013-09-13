#ifndef RPC_SERIALIZE_HPP_INCLUDED
#define RPC_SERIALIZE_HPP_INCLUDED

#include <assert.h>
#include <vector>
#include "protobuf/message.h"
#include "common/base/serialize/serialize.hpp"

namespace Rpc
{

typedef std::vector<char> SerializeBuffer;

/// ���л�
template <typename T>
bool SerializeNormalObject(const T& object, SerializeBuffer& buffer)
{
    ::Serialize::BinaryEncodeAppend(object, buffer);
    return true;
}

/// �����л�
/// @return �����˶����ֽ�
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
// ����Ϊ��������ģ�壬�� IDL attribute ������
////////////////////////////////////////////////////////////////

/// ���������������ֻ���룬const ���ã�ֵ������ָ�����
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
/// Out �������γ�����
/// Out ���ܷ� const ���úͷǳ���ָ�룬���Դ˴�������
/// ������ﱨ��˵�� T ������������������
/// ---------------------------------------------------------
template <typename T> struct Out;
template <typename T> struct Out<const T&>;
template <typename T> struct Out<const T*>;

/// Out ���ܷ� const ��������
template <typename T>
struct Out<T&>
{
    Out(T& place) : Value(place) {}
    T& Value;
};

/// Out �ɽ��ܷǳ���ָ������
template <typename T>
struct Out<T*>
{
    Out(T* place) : Value(place) {}
    T* Value;
};
/// ---------------------------------------------------------
/// InOut���壺�������γ��������˫�򴫵�
/// ---------------------------------------------------------
template <typename T>
struct InOut {};

/// �������úͳ���ָ�벻��������ڳ�����
template <typename T> struct InOut<const T&>;
template <typename T> struct InOut<const T*>;

/// InOut ������ͨ��������
template <typename T>
struct InOut<T&>
{
    InOut(T& place) : Value(place) {}
    T& Value;
};

/// InOut ������ָͨ������
template <typename T>
struct InOut<T*>
{
    InOut(T* place) : Value(place) {}
    T* Value;
};

/// ------------------------------------------------------------
/// ��ȡ�������͵Ĵ��ݷ���
/// ------------------------------------------------------------
/// �Ƿ��������
template <typename T> struct IsIn  {
    static const bool Value = true;
};

/// �Ƿ��ǳ�����
template <typename T> struct IsOut {
    static const bool Value = false;
};

// in ���ε������������
template <typename T> struct IsIn< In<T> >      {
    static const bool Value = true;
};

// out ���ε����Ͳ��������
template <typename T> struct IsIn< Out<T> >     {
    static const bool Value = false;
};

// in_out ���ε�����Ҳ�������
template <typename T> struct IsIn< InOut<T> >   {
    static const bool Value = true;
};

// out ���ε������ǳ�����
template <typename T> struct IsOut< Out<T> >    {
    static const bool Value = true;
};

// in_out ���ε�����Ҳ�ǳ�����
template <typename T> struct IsOut< InOut<T> >  {
    static const bool Value = true;
};

// ���ú�ָ��Ĭ����Ϊ�������
template <typename T> struct IsOut<T&>          {
    static const bool Value = true;
};
template <typename T> struct IsOut<const T&>    {
    static const bool Value = false;
};

/// �����Ƿ���ָ��
template <typename T> struct IsPointer {
    static const bool Value = false;
};

/// �ػ����ͣ��жϳ�������Ƿ���ָ��
template <typename T> struct IsPointer<T*> {
    static const bool Value = true;
};

template <typename T> struct RemovePointer {
    typedef T Type;
};

template <typename T> struct RemovePointer<T*> {
    typedef T Type;
};



/// ȥ�����͵� in/out/inout ����
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
/// ȥ�����ݷ������Σ����ʵ�ʵĶ����ֵ
/// --------------------------------------------------------
template <typename T>
const T& ValueOf(const T& object)
{
    return object;
}

/// ����ָ����ֵ
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

/// Inָ�������ֵ
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

/// Ӣ��������֮�ء�ռ���Ӱɡ�
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

/// InOutָ�������ֵ
template <typename T>
T ValueOf(const InOut<T*>& object)
{
    return *(object.Value);
}

/// ----------------------------------------------------------
/// ȥ�� in/out ���Σ����ʵ�ʶ���ĵ�ַ
/// ----------------------------------------------------------
/// ��������ȡ��ַ
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

/// ָ������ֱ�Ӹ�ֵ
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

/// ȥ�����͵� const ����������
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

/// ȥ�����е��������Σ��õ���ԭʼ��������
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

/// �ж�һ�����ǲ��Ǵ���һ��������������
template <typename Derived, typename Base>
struct IsDerivedFrom
{
    // ��� Derived �ǲ�����������
    static const bool IsCompleteType = sizeof(Derived) > 0;
private:
    // ��� Derived �� Base �������࣬��ô��ƥ��ǰ�ߣ���������Ϊ char������ƥ�����
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

// Protocol Buffer Message �ر��������еĽӿڽ������кͷ����л�
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

// RPC stub �� proxy Ҫ�ر����л��ͷ����л�
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

/// ���л�
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

/// ������Ϣ����Ϣͷ�Ĵ�С���������л�ǰԤ���ռ�
extern const size_t InvokeMessageHeaderSize;

/// ������Ϣ����Ϣͷ�Ĵ�С���������л�ǰԤ���ռ�
extern const size_t ReturnMessageHeaderSize;

} // end namespace Rpc

#endif//RPC_SERIALIZE_HPP_INCLUDED
