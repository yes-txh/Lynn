// Copyright (c) 2008, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef SERIALIZE_TYPE_HPP_INCLUDED
#define SERIALIZE_TYPE_HPP_INCLUDED

// Serialize framework defination

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include <common/base/serialize/base.hpp>
#include <common/base/serialize/stream.hpp>
#include <common/base/serialize/binary_codec.hpp>

// GLOBAL_NOLINT(runtime/int)

namespace Serialize
{

template <typename EncoderType>
void Encode(EncoderType& encoder, bool value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, char value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, signed char value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, unsigned char value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, short value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, unsigned short value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, int value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, unsigned int value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, long value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, unsigned long value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, long long value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, unsigned long long value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, float value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, double value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

template <typename EncoderType>
void Encode(EncoderType& encoder, const std::string& value, const char* name)
{
    encoder.EncodePrimitive(value, name);
}

///////////////////////////////////////////////////////////////////////////////
// decode functions
///////////////////////////////////////////////////////////////////////////////
template <typename DecoderType>
bool Decode(DecoderType& decoder, bool& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, char& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, signed char& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, unsigned char& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, short& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, unsigned short& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, int& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, unsigned int& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, long& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, unsigned long& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, long long& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, unsigned long long& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, float& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, double& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

template <typename DecoderType>
bool Decode(DecoderType& decoder, std::string& value, const char* name)
{
    return decoder.DecodePrimitive(value, name);
}

///////////////////////////////////////////////////////////////////////////////
// Handle record mamber serialize
///////////////////////////////////////////////////////////////////////////////

// remove any cv-qualifiers and reference modifier from given type
template<typename T>
struct RemoveCvAndReference
{
    typedef T Type;
};

template<typename T>
struct RemoveCvAndReference<const T>
{
    typedef T Type;
};

template<typename T>
struct RemoveCvAndReference<volatile T>
{
    typedef T Type;
};

template<typename T>
struct RemoveCvAndReference<const volatile T>
{
    typedef T Type;
};

template<typename T>
struct RemoveCvAndReference<T&>
{
    typedef T Type;
};

template<typename T>
struct RemoveCvAndReference<const T&>
{
    typedef T Type;
};

template<typename T>
struct RemoveCvAndReference<volatile T&>
{
    typedef T Type;
};

template<typename T>
struct RemoveCvAndReference<const volatile T&>
{
    typedef T Type;
};

// overload for array
template <typename EncoderType, typename Type, size_t N>
void Encode(EncoderType& encoder, const Type(&value)[N], const char* name)
{
    encoder.EncodeListBegin(N, name);
    for (size_t i = 0; i < N; ++i)
        Encode(encoder, value[i]);
    encoder.EncodeListEnd();
}

template <typename DecoderType, typename Type, size_t N>
bool Decode(DecoderType& decoder, Type (&value)[N], const char* name)
{
    typename DecoderType::Result result(decoder);

    size_t size = 0;
    if (!decoder.DecodeListBegin(size, name))
        return false;

    if (size != N)
        return false;

    for (size_t i = 0; i < N; ++i)
    {
        if (!Decode(decoder, value[i]))
            return false;
    }

    result = decoder.DecodeListEnd();

    return result;
}

// overload for array, with explicit length
template <typename EncoderType, typename Type, size_t N, typename SizeType>
void EncodeArray(EncoderType& encoder, const Type(&value)[N], SizeType size, const char* name)
{
    size_t list_size = size;
    if (list_size <= N)
    {
        encoder.EncodeListBegin(size, name);
        for (size_t i = 0; i < list_size; ++i)
            Encode(encoder, value[i]);
        encoder.EncodeListEnd();
    }
    else
    {
        throw OutputError("Array overflow");
    }
}

template <typename DecoderType, typename Type, size_t N, typename SizeType>
bool DecodeArray(DecoderType& decoder, Type (&value)[N], SizeType& size, const char* name)
{
    typename DecoderType::Result result(decoder);

    size_t list_size = 0;

    if (!decoder.DecodeListBegin(list_size, name))
        return false;

    if (list_size > N)
        return false;

    size = list_size;
    for (size_t i = 0; i < list_size; ++i)
    {
        if (!Decode(decoder, value[i]))
            return false;
    }

    result = decoder.DecodeListEnd();

    return result;
}

// overload for string

// string with explicit length
template <typename EncoderType, size_t N, typename SizeType>
void EncodeString(EncoderType& encoder, const char (&value)[N], SizeType size, const char* name)
{
    if ((size_t)size <= N)
        encoder.EncodeString(value, size, name);
    else
        throw OutputError("String overflow");
}

template <typename EncoderType, size_t N, typename SizeType>
void EncodeString(EncoderType& encoder, const unsigned char (&value)[N], SizeType size, const char* name)
{
    return EncodeString(encoder, (const char (&)[N])value, size, name);
}

template <typename EncoderType, typename SizeType>
void EncodeString(EncoderType& encoder, char* const& value, SizeType size, const char* name)
{
    encoder.EncodeString(value, size, name);
}

template <typename EncoderType, typename SizeType>
void EncodeString(EncoderType& encoder, unsigned char* const& value, SizeType size, const char* name)
{
    encoder.EncodeString((char*)value, size, name);
}

template <typename DecoderType, size_t N, typename SizeType>
bool DecodeString(DecoderType& decoder, char (&value)[N], SizeType& size, const char* name)
{
    size_t encoded_size = 0;
    bool result = decoder.DecodeString(value, N, encoded_size, name);
    size = encoded_size;
    return result;
}

template <typename DecoderType, size_t N, typename SizeType>
bool DecodeString(DecoderType& decoder, unsigned char (&value)[N], SizeType& size, const char* name)
{
    return DecodeString(decoder, (char (&)[N])value, size, name);
}

/// Decode string to preallocated buffer
template <typename DecoderType, typename SizeType1, typename SizeType2>
bool DecodeStringToBuffer(DecoderType& decoder, char* value, SizeType1 buffer_size, SizeType2& result_size, const char* name)
{
    size_t encoded_size = 0;
    bool result = decoder.DecodeString(value, buffer_size, encoded_size, name);
    result_size = encoded_size;
    return result;
}

/// Decode string to preallocated buffer
template <typename DecoderType, typename SizeType1, typename SizeType2>
bool DecodeStringToBuffer(DecoderType& decoder, unsigned char* value, SizeType1 buffer_size, SizeType2& result_size, const char* name)
{
    return DecodeStringToBuffer(decoder, (char*)value, buffer_size, result_size, name);
}

/// Decode string to preallocated buffer
template <typename DecoderType, typename SizeType1, typename SizeType2>
bool DecodeStringToBuffer(DecoderType& decoder, signed char* value, SizeType1 buffer_size, SizeType2& result_size, const char* name)
{
    return DecodeStringToBuffer(decoder, (char*)value, buffer_size, result_size, name);
}

/// Decode string, using new[] to allocate memory
template <typename DecoderType, typename SizeType>
bool DecodeStringNew(DecoderType& decoder, char *&value, SizeType& size, const char* name)
{
    std::string str;
    if (decoder.DecodePrimitive(str, name))
    {
        delete[] value;
        value = new char[str.length() + 1];
        memcpy(value, str.data(), str.length());
        value[str.length()] = '\0';
        size = str.length();
        return true;
    }
    return false;
}

/// Decode string, using new[] to allocate memory
template <typename DecoderType, typename SizeType>
bool DecodeStringNew(DecoderType& decoder, unsigned char *&value, SizeType& size, const char* name)
{
    return DecodeStringNew(decoder, (char*&)value, size, name);
}

/// Decode string, using malloc to allocate memory
template <typename DecoderType, typename SizeType>
bool DecodeStringMalloc(DecoderType& decoder, char *&value, SizeType& size, const char* name)
{
    std::string str;
    if (decoder.DecodePrimitive(str, name))
    {
        free(value);
        value = (char*) malloc(str.length() + 1);
        if (value)
        {
            memcpy(value, str.data(), str.length());
            value[str.length()] = '\0';
            size = str.length();
            return true;
        }
    }
    return false;
}

/// Decode string, using malloc to allocate memory
template <typename DecoderType, typename SizeType>
bool DecodeStringMalloc(DecoderType& decoder, unsigned char *&value, SizeType& size, const char* name)
{
    return DecodeStringMalloc(decoder, (char*&)value, size, name);
}


/// Decode string, using realloc to allocate memory
template <typename DecoderType, typename SizeType>
bool DecodeStringRealloc(DecoderType& decoder, char *&value, SizeType& size, const char* name)
{
    std::string str;
    if (decoder.DecodePrimitive(str, name))
    {
        char* p = (char*) realloc(value, str.length() + 1);
        if (p)
        {
            value = p;
            memcpy(value, str.data(), str.length());
            value[str.length()] = '\0';
            size = str.length();
            return true;
        }
    }
    return false;
}

/// Decode string, using realloc to allocate memory
template <typename DecoderType, typename SizeType>
bool DecodeStringRealloc(DecoderType& decoder, unsigned char *&value, SizeType& size, const char* name)
{
    return DecodeStringRealloc(decoder, (char*&)value, size, name);
}


// string terminated with '\0'
template <typename EncoderType, size_t N>
void EncodeString(EncoderType& encoder, const char(&value)[N], const char* name)
{
    size_t length = strlen(value);
    if (length < N)
        encoder.EncodeString(value, length, name);
    else
        throw OutputError("String overflow");
}
template <typename EncoderType, size_t N>
void EncodeString(EncoderType& encoder, const unsigned char(&value)[N], const char* name)
{
    return EncodeString(encoder, (const char(&)[N]) value, name);
}

template <typename EncoderType>
void EncodeString(EncoderType& encoder, char* & value, const char* name)
{
    size_t length = strlen(value);
    encoder.EncodeString(value, length, name);
}

template <typename EncoderType>
void EncodeString(EncoderType& encoder, char* const& value, const char* name)
{
    size_t length = strlen(value);
    encoder.EncodeString(value, length, name);
}

template <typename EncoderType>
void EncodeString(EncoderType& encoder, unsigned char* const& value, const char* name)
{
    return EncodeString(encoder, (char*&)value, name);
}

template <typename DecoderType, size_t N>
bool DecodeString(DecoderType& decoder, char (&value)[N], const char* name)
{
    size_t encoded_size = 0;
    return decoder.DecodeString(value, N, encoded_size, name) && encoded_size < N;
}
template <typename DecoderType, size_t N>
bool DecodeString(DecoderType& decoder, unsigned char (&value)[N], const char* name)
{
    return DecodeString(decoder, (char (&)[N])value, name);
}

template <typename DecoderType>
bool DecodeStringNew(DecoderType& decoder, char *&value, const char* name)
{
    size_t size = 0;
    return DecodeStringNew(decoder, value, size, name);
}
template <typename DecoderType>
bool DecodeStringNew(DecoderType& decoder, unsigned char *&value, const char* name)
{
    return DecodeStringNew(decoder, (char*&)value, name);
}

template <typename DecoderType>
bool DecodeStringMalloc(DecoderType& decoder, char *&value, const char* name)
{
    size_t size = 0;
    return DecodeStringMalloc(decoder, value, size, name);
}
template <typename DecoderType>
bool DecodeStringMalloc(DecoderType& decoder, unsigned char *&value, const char* name)
{
    return DecodeStringMalloc(decoder, (char*&)value, name);
}

template <typename DecoderType>
bool DecodeStringRealloc(DecoderType& decoder, char *&value, const char* name)
{
    size_t size = 0;
    return DecodeStringRealloc(decoder, value, size, name);
}
template <typename DecoderType>
bool DecodeStringRealloc(DecoderType& decoder, unsigned char *&value, const char* name)
{
    return DecodeStringRealloc(decoder, (char*&)value, name);
}

/////////////////////////////////////////////////////////////////////
// overload insert/extract operators

template <typename EncoderType,typename Type>
void Encode(EncoderType& encoder, const Type& value)
{
    Encode(encoder, value, (const char*)NULL);
}

template <typename DecoderType, typename Type>
bool Decode(DecoderType& decoder, Type& value)
{
    return Decode(decoder, value, (const char*)NULL);
}

/////////////////////////////////////////////////////////////////////
// for enum types
template <typename EncoderType, typename Type>
void EncodeEnum(EncoderType& encoder, const Type& value, const char* name)
{
    Type test_sign = Type(-1);
    if (test_sign < 0)
    {
        if (sizeof(value) <= sizeof(int))
            Encode(encoder, (int)value, name);
        else if (sizeof(value) <= sizeof(long))
            Encode(encoder, (long)value, name);
        else
            Encode(encoder, (long long)value, name);
    }
    else
    {
        if (sizeof(value) <= sizeof(unsigned int))
            Encode(encoder, (unsigned int)value, name);
        else if (sizeof(value) <= sizeof(unsigned long))
            Encode(encoder, (unsigned long)value, name);
        else
            Encode(encoder, (unsigned long long)value, name);
    }
}

template <typename IntegerType, typename DecoderType, typename Type>
bool DecodeEnumAsInteger(DecoderType& decoder, Type& value, const char* name)
{
    IntegerType n;
    bool result = Decode(decoder, n, name);
    value = static_cast<Type>(n);
    return result;
}

template <typename DecoderType,typename Type>
bool DecodeEnum(DecoderType& decoder, Type& value, const char* name)
{
    bool result = false;

    Type test_sign = Type(-1);
    if (test_sign < 0)
    {
        if (sizeof(value) == sizeof(int))
            result = DecodeEnumAsInteger<int>(decoder, value, name);
        else if (sizeof(value) == sizeof(signed char))
            result = DecodeEnumAsInteger<signed char>(decoder, value, name);
        else if (sizeof(value) == sizeof(short))
            result = DecodeEnumAsInteger<short>(decoder, value, name);
        else  if (sizeof(value) == sizeof(long))
            result = DecodeEnumAsInteger<long>(decoder, value, name);
        else  if (sizeof(value) == sizeof(long long))
            result = DecodeEnumAsInteger<long long>(decoder, value, name);
    }
    else
    {
        if (sizeof(value) == sizeof(unsigned char))
            result = DecodeEnumAsInteger<unsigned char>(decoder, value, name);
        else if (sizeof(value) == sizeof(unsigned short))
            result = DecodeEnumAsInteger<unsigned short>(decoder, value, name);
        else if (sizeof(value) == sizeof(unsigned int))
            result = DecodeEnumAsInteger<unsigned int>(decoder, value, name);
        else  if (sizeof(value) == sizeof(unsigned long))
            result = DecodeEnumAsInteger<unsigned long>(decoder, value, name);
        else  if (sizeof(value) == sizeof(unsigned long long))
            result = DecodeEnumAsInteger<unsigned long long>(decoder, value, name);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// encode/decode for property
template <typename DecoderType, typename ObjectType, typename ReturnType, typename PropertyType>
bool DecodeObjectPropertySetter(
    DecoderType& decoder,
    ObjectType& object,
    ReturnType (ObjectType::*Setter)(PropertyType value),
    const char* name
    )
{
    typename RemoveCvAndReference<PropertyType>::Type value;
    if (Decode(decoder, value, name))
    {
        (object.*Setter)(value);
        return true;
    }
    return false;
}

template <typename DecoderType, typename ObjectType, typename ReturnType, typename PropertyType, typename DefaultValueType>
void DecodeObjectPropertySetter(
    DecoderType& decoder,
    ObjectType& object,
    ReturnType (ObjectType::*Setter)(PropertyType value),
    const char* name,
    DefaultValueType default_value
    )
{
    typename RemoveCvAndReference<PropertyType>::Type value;
    if (Decode(decoder, value, name))
        (object.*Setter)(value);
    else
        (object.*Setter)(default_value);
}

///////////////////////////////////////////////////////////////////////////////
// set object to defaut value
template <typename T, typename U>
void SetDefault(T &a, const U& value)
{
    a = value;
}

template <typename T, size_t N, typename U>
void SetDefault(T (&a)[N], const U& value)
{
    for (size_t i = 0; i < N; ++i)
        a[i] = value;
}


///////////////////////////////////////////////////////////////////////////////
// Dummy encoder and decoder to be used in macros
class DummyEncoder : public EncoderConcept<DummyEncoder>
{
public:
    void EncodeRecordBegin(const char* name, const char* field_name) {}
    void EncodeRecordEnd(const char* name) {}
    void EncodeListBegin(size_t size, const char* name) {}
    void EncodeListEnd() {}
    void EncodePrimitive(bool value, const char* name) {}
    void EncodePrimitive(char value, const char* name) {}
    void EncodePrimitive(signed char value, const char* name) {}
    void EncodePrimitive(unsigned char value, const char* name) {}
    void EncodePrimitive(short value, const char* name) {}
    void EncodePrimitive(unsigned short value, const char* name) {}
    void EncodePrimitive(int value, const char* name) {}
    void EncodePrimitive(unsigned int value, const char* name) {}
    void EncodePrimitive(long value, const char* name) {}
    void EncodePrimitive(unsigned long value, const char* name) {}
    void EncodePrimitive(long long value, const char* name) {}
    void EncodePrimitive(unsigned long long value, const char* name) {}
    void EncodePrimitive(float value, const char* name) {}
    void EncodePrimitive(double value, const char* name) {}
    void EncodePrimitive(const std::string& value, const char* name) {}
    void EncodeString(const char* value, size_t length, const char* name) {}
    void EncodeNull() {}
};


class DummyDecoder : public DecoderConcept<DummyDecoder>
{
public:
    typedef size_t PosType;
    size_t Mark() const { return 0; }
    void Rollback(size_t pos) {}
    bool DecodeRecordBegin(const char* record_name, const char* field_name) { return false; }
    bool DecodeRecordEnd(const char* name) { return false; }
    bool DecodeListBegin(size_t& size, const char* name) { return false; }
    bool DecodeListEnd() { return false; }
    bool DecodePrimitive(bool& value, const char* name) { return false; }
    bool DecodePrimitive(char& value, const char* name) { return false; }
    bool DecodePrimitive(signed char& value, const char* name) { return false; }
    bool DecodePrimitive(unsigned char& value, const char* name) { return false; }
    bool DecodePrimitive(short& value, const char* name) { return false; }
    bool DecodePrimitive(unsigned short& value, const char* name) { return false; }
    bool DecodePrimitive(int& value, const char* name) { return false; }
    bool DecodePrimitive(unsigned int& value, const char* name) { return false; }
    bool DecodePrimitive(long& value, const char* name) { return false; }
    bool DecodePrimitive(unsigned long& value, const char* name) { return false; }
    bool DecodePrimitive(long long& value, const char* name) { return false; }
    bool DecodePrimitive(unsigned long long& value, const char* name) { return false; }
    bool DecodePrimitive(float& value, const char* name) { return false; }
    bool DecodePrimitive(double& value, const char* name) { return false; }
    bool DecodePrimitive(std::string& value, const char* name) { return false; }
    bool DecodeString(char* buffer, size_t buffer_size, size_t& length, const char* name) { return false; }
    bool SkipNext(std::string& name) { return false; }
    template <typename EncoderType>
    bool CopyNext(EncoderType& encoder) { return false; }
};

/// store any other unknown struct members
class OtherMembers
{
public:
    OtherMembers() : m_Count(0)
    {
    }

    /// store to encoder
    template <typename Encoder>
    bool StoreTo(Encoder& encoder) const
    {
        VectorInputStream stream(m_Data);
        BinaryDecoder<VectorInputStream> decoder(stream);
        for (size_t i = 0; i < m_Count; ++i)
        {
            if (!decoder.CopyNext(encoder))
                return false;
        }
        return true;
    }

    /// load from decoder
    template <typename Decoder>
    bool LoadFrom(Decoder& decoder)
    {
        VectorOutputStream stream(m_Data);
        BinaryEncoder<VectorOutputStream> encoder(stream);

        m_Count = 0;
        while (decoder.CopyNext(encoder))
        {
            ++m_Count;
        }
        return true;
    }

    /// get member count
    size_t Count() const
    {
        return m_Count;
    }

    /// clear all data
    void Clear()
    {
        m_Data.clear();
        m_Count = 0;
    }
private:
    std::vector<char> m_Data;
    size_t m_Count;
};

} //end namespace Serialize

//////////////////////////////////////////////////////////////////////////
// type descript macros, put out of namespace

#define SERIALIZE_REGISTER_STRUCT(Type) \
template <bool IsWriting, typename EncoderType, typename DecoderType> \
bool SerializeRecord( \
    EncoderType& encoder, DecoderType& decoder, \
    Type& object, \
    const char* name \
); \
 \
template <typename EncoderType> \
void Encode(EncoderType& encoder, const Type& value, const char* name) \
{ \
    Serialize::DummyDecoder decoder; \
    SerializeRecord<true>(encoder, decoder, const_cast<Type&>(value), name); \
} \
 \
template <typename DecoderType> \
bool Decode(DecoderType& decoder, Type& value, const char* name) \
{ \
    Serialize::DummyEncoder encoder; \
    return SerializeRecord<false>(encoder, decoder, value, name); \
} \
 \
template <bool IsWriting, typename EncoderType, typename DecoderType> \
bool SerializeRecord( \
    EncoderType& encoder, DecoderType& decoder, \
    Type& object, const char* name \
) \
{ \
    typedef Type RecordType; \
    const Type& const_object = object; \
    (void) const_object; \
    const char* RecordTypeName = #Type; \
    typename DecoderType::Result result(decoder); \
    if (IsWriting) \
    { \
        encoder.EncodeRecordBegin(#Type, name); \
    } \
    else \
    { \
        if (!decoder.DecodeRecordBegin(#Type, name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, "<Begin>", __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_BASE(Name) \
    if (IsWriting) \
    { \
        Encode(encoder, static_cast<const Name&>(const_object), #Name); \
    } \
    else \
    { \
        if (!Decode(decoder, static_cast<Name&>(object), #Name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, "Base:" #Name, __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_MEMBER(Name) \
    if (IsWriting) \
    { \
        Encode(encoder, const_object.Name, #Name); \
    } \
    else \
    { \
        if (!Decode(decoder, object.Name, #Name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, #Name, __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_OPTIONAL_MEMBER(Name, DefaultValue) \
    if (IsWriting) \
    { \
        Encode(encoder, const_object.Name, #Name); \
    } \
    else \
    { \
        if (!Decode(decoder, object.Name, #Name)) \
            Serialize::SetDefault(object.Name, DefaultValue); \
    }

#define SERIALIZE_PROPERTY(Name, Getter, Setter) \
    if (IsWriting) \
    { \
        Encode(encoder, const_object.Getter(), #Name); \
    } \
    else \
    { \
        if (!Serialize::DecodeObjectPropertySetter(decoder, object, &RecordType::Setter, #Name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, #Name, __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_OPTIONAL_PROPERTY(Name, Getter, Setter, DefaultValue) \
    if (IsWriting) \
        Encode(encoder, const_object.Getter(), #Name); \
    else \
        ::Serialize::DecodeObjectPropertySetter(decoder, object, &RecordType::Setter, #Name, DefaultValue); \

#define SERIALIZE_MEMBER_ARRAY(Name, Length) SERIALIZE_BIND_MEMBER(Name, Length)

#define SERIALIZE_MEMBER_ARRAY_LENGTH(Name, Length) \
    if (IsWriting) \
    { \
        ::Serialize::EncodeArray(encoder, const_object.Name, const_object.Length, #Name); \
    } \
    else \
    { \
        if (!::Serialize::DecodeArray(decoder, object.Name, object.Length, #Name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, #Name, __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_OPTIONAL_MEMBER_ARRAY_LENGTH(Name, Length) \
    if (IsWriting) \
    { \
        ::Serialize::EncodeArray(encoder, const_object.Name, const_object.Length, #Name); \
    } \
    else \
    { \
        if (::Serialize::DecodeArray(decoder, object.Name, object.Length, #Name)) \
            object.Length = 0; \
    }

#define SERIALIZE_MEMBER_STRING(Name) \
    if (IsWriting) \
    { \
        ::Serialize::EncodeString(encoder, const_object.Name, #Name); \
    } \
    else \
    { \
        if (!::Serialize::DecodeString(decoder, object.Name, #Name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, #Name, __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_MEMBER_STRING_NEW(Name) \
    if (IsWriting) \
    { \
        ::Serialize::EncodeString(encoder, const_object.Name, #Name); \
    } \
    else \
    { \
        if (!::Serialize::DecodeStringNew(decoder, object.Name, #Name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, #Name, __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_MEMBER_STRING_MALLOC(Name) \
    if (IsWriting) \
    { \
        ::Serialize::EncodeString(encoder, const_object.Name, #Name); \
    } \
    else \
    { \
        if (!::Serialize::DecodeStringMalloc(decoder, object.Name, #Name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, #Name, __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_MEMBER_STRING_REALLOC(Name) \
    if (IsWriting) \
    { \
        ::Serialize::EncodeString(encoder, const_object.Name, #Name); \
    } \
    else \
    { \
        if (!::Serialize::DecodeStringRealloc(decoder, object.Name, #Name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, #Name, __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_OPTIONAL_MEMBER_STRING(Name, Length) \
    if (IsWriting) \
    { \
        ::Serialize::EncodeString(encoder, const_object.Name, const_object.Length, #Name); \
    } \
    else \
    { \
        if (::Serialize::DecodeString(decoder, object.Name, object.Length, #Name)) \
            object.Name[0] = '\0', object.Length = 0; \
    }

#define SERIALIZE_MEMBER_STRING_LENGTH(Name, Length) \
    if (IsWriting) \
    { \
        ::Serialize::EncodeString(encoder, const_object.Name, const_object.Length, #Name); \
    } \
    else \
    { \
        if (!::Serialize::DecodeString(decoder, object.Name, object.Length, #Name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, #Name, __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_MEMBER_STRING_LENGTH_NEW(Name, Length) \
    if (IsWriting) \
    { \
        ::Serialize::EncodeString(encoder, const_object.Name, const_object.Length, #Name); \
    } \
    else \
    { \
        if (!::Serialize::DecodeStringNew(decoder, object.Name, object.Length, #Name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, #Name, __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_MEMBER_STRING_LENGTH_MALLOC(Name, Length) \
    if (IsWriting) \
    { \
        ::Serialize::EncodeString(encoder, const_object.Name, const_object.Length, #Name); \
    } \
    else \
    { \
        if (!::Serialize::DecodeStringMalloc(decoder, object.Name, object.Length, #Name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, #Name, __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_MEMBER_STRING_LENGTH_REALLOC(Name, Length) \
    if (IsWriting) \
    { \
        ::Serialize::EncodeString(encoder, const_object.Name, const_object.Length, #Name); \
    } \
    else \
    { \
        if (!::Serialize::DecodeStringRealloc(decoder, object.Name, object.Length, #Name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, #Name, __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_OPTIONAL_MEMBER_STRING_LENGTH(Name, Length) \
    if (IsWriting) \
    { \
        ::Serialize::EncodeString(encoder, const_object.Name, const_object.Length, #Name); \
    } \
    else \
    { \
        if (::Serialize::DecodeString(decoder, object.Name, object.Length, #Name)) \
            object.Length = 0; \
    }


#define SERIALIZE_OTHER_MEMBERS(Name) \
    if (IsWriting) \
    { \
        const_object.Name.StoreTo(encoder); \
    } \
    else \
    { \
        if (!object.Name.LoadFrom(decoder)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, #Name, __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_NULL_MEMBER() \
    if (IsWriting) \
    { \
        encoder.EncodeNull(); \
    } \
    else \
    { \
        std::string ignored_name; \
        if (!decoder.SkipNext(ignored_name)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, "<Null>", __FILE__, __LINE__); \
            return false; \
        } \
    }

#define SERIALIZE_IGNORE_OTHER_MEMBERS() \
    if (!IsWriting) \
    { \
        std::string ignored_name; \
        while (decoder.SkipNext(ignored_name)) \
            ; \
    }

#define SERIALIZE_UNION_SWITCH(Selector) \
    SERIALIZE_MEMBER(Selector) \
    switch (object.Selector) { \

#define SERIALIZE_UNION_CASE(Value, ...) \
    case Value: __VA_ARGS__; break;

#define SERIALIZE_UNION_DEFAULT(Action) \
    default: Action; break; \

#define SERIALIZE_UNION_NO_DEFAULT() \
    default: \
        if (IsWriting) \
            throw ::Serialize::OutputError("Unknown union selector"); \
        else \
        { \
            decoder.ErrorStack.Push(RecordTypeName, "<UnionSelector>", __FILE__, __LINE__); \
            return false; \
        } \
        break;

#define SERIALIZE_UNION_END() \
    }

#define SERIALIZE_END_STRUCT(...) \
    __VA_ARGS__ \
    if (IsWriting) \
    { \
        encoder.EncodeRecordEnd(RecordTypeName); \
    } \
    else \
    { \
        if (!decoder.DecodeRecordEnd(RecordTypeName)) \
        { \
            decoder.ErrorStack.Push(RecordTypeName, "<End>", __FILE__, __LINE__); \
            return false; \
        } \
    } \
 \
    result = true; \
    return true; \
}

#define SERIALIZE_FRIEND(Type) \
    template <bool IsWriting, typename EncoderType, typename DecoderType> \
    friend bool SerializeRecord(EncoderType& encoder, DecoderType& decoder, Type& object, const char* name);

#define SERIALIZE_REGISTER_ENUM(Type) \
template <typename EncoderType> \
void Encode(EncoderType& encoder, const Type& value, const char* name) \
{ \
    ::Serialize::EncodeEnum(encoder, value, name); \
} \
 \
template <typename DecoderType> \
bool Decode(DecoderType& decoder, Type& value, const char* name) \
{ \
    return ::Serialize::DecodeEnum(decoder, value, name); \
}


namespace Serialize
{
/// this struct can be used to hold any struct members
struct Struct
{
    OtherMembers Members;
};

SERIALIZE_REGISTER_STRUCT(Struct)
SERIALIZE_END_STRUCT(SERIALIZE_OTHER_MEMBERS(Members))

}

#endif//SERIALIZE_TYPE_HPP_INCLUDED
