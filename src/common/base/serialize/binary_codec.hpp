// Copyright (c) 2008, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_SERIALIZE_BINARY_CODEC_HPP
#define COMMON_BASE_SERIALIZE_BINARY_CODEC_HPP

// Define serialize binary format encoder & decoder

#include <cassert>
#include <climits>
#include <vector>

#include "common/base/serialize/base.hpp"
#include "common/base/byte_order.hpp"

// GLOBAL_NOLINT(runtime/int)

namespace Serialize
{

enum TagType
{
    TagType_Bool,
    TagType_Char,
    TagType_FixedSignedInteger,
    TagType_FixedUnsignedInteger,
    TagType_SignedInteger,
    TagType_UnsignedInteger,
    TagType_Float,
    TagType_RawBytes,
    TagType_RecordBegin,
    TagType_RecordEnd,
    TagType_ListBegin,
    TagType_ListEnd,
    TagType_String,
    TagType_Null
};

/*
encoding Header, include Tag and optional field Id

Structure:
in one byte
hi<--->low
[xxxxxxxx] all bits
[****----] type
[----*---] no value
[-----*--] no list size / big endian
[------**] width_shift / bool value
*/
class Tag
{
public:
    enum
    {
        Type_Shift = 4,
        Type_Width = 4,
        Type_Mask = ((1U << Type_Width) - 1) << Type_Shift,

        NoValue_Shift = 3,
        NoValue_Mask = (1U << NoValue_Shift),

        Reserved1_Shift = 2,
        Reserved1_Mask = (1U << Reserved1_Shift),

        Reserved2_Width = 2,
        Reserved2_Mask = ((1U << Reserved2_Width) - 1),
    };
public:
    Tag() : m_value(0)
    {
    }
    Tag(TagType type, bool no_value = 0, bool reserved1 = 0, unsigned int reserved2 = 0):
        m_value(static_cast<unsigned char>(
            (type << Type_Shift) |
            (no_value << NoValue_Shift) |
            (reserved1 << Reserved1_Shift) |
            (reserved2 & Reserved2_Mask)
            )
        )
    {
    }
    TagType Type() const
    {
        return static_cast<TagType>(m_value >> Type_Shift);
    }
    unsigned char Header() const
    {
        return m_value;
    }

    bool NoValue() const
    {
        return (m_value & NoValue_Mask) != 0;
    }

    bool Reserved1() const
    {
        return (m_value & Reserved1_Mask) != 0;
    }

    unsigned int Reserved2() const
    {
        return m_value & Reserved2_Mask;
    }
public:
    static Tag Parse(unsigned char tag_byte)
    {
        Tag tag(tag_byte);
        return tag;
    }
private:
    explicit Tag(unsigned char tag_byte) : m_value(tag_byte)
    {
    }
protected:
    unsigned char m_value;
};

class CharTag : public Tag
{
public:
    CharTag() : Tag(TagType_Char) {}
};

class BoolTag : public Tag
{
public:
    explicit BoolTag(bool value = false) : Tag(TagType_Bool, false, false, value)
    {
    }
    bool Value() const
    {
        return Reserved2() == 1;
    }
};

class FixedNumberTag : public Tag
{
public:
    FixedNumberTag(TagType type, unsigned int width_shift = 0, bool no_value = false)
        :Tag(type, no_value, ByteOrder::IsBigEndian(), width_shift)
    {
    }
    FixedNumberTag() {}
    bool IsBigEndian() const
    {
        return Reserved1();
    }
    size_t Width() const
    {
        return 1U << Reserved2();
    }
};

class FloatTag : public FixedNumberTag
{
public:
    explicit FloatTag(unsigned int width_shift = 0, bool no_value = false)
        : FixedNumberTag(TagType_Float, width_shift, no_value) {}
};

class StringTag : public Tag
{
public:
    StringTag() : Tag(TagType_String) {}
};

class ListBeginTag : public Tag
{
public:
    enum
    {
        NoSize_Shift = 2,
        NoSize_Width = 1,
        NoSize_Mask = (1U << NoSize_Shift),
    };
public:
    explicit ListBeginTag(bool no_value = false, bool no_size = false)
        : Tag(TagType_ListBegin, no_value, no_size)
    {
    }
    bool NoSize() const
    {
        return (m_value & NoSize_Mask) != 0;
    }
};

class ListEndTag : public Tag
{
public:
    ListEndTag() : Tag(TagType_ListEnd) {}
};

///////////////////////////////////////////////////////////////////////////////
// Helper traits

// Get shift width number, i.e. log2
template <size_t N> struct ShiftOf {};
template <> struct ShiftOf<1> { static const size_t Value = 0; };
template <> struct ShiftOf<2> { static const size_t Value = 1; };
template <> struct ShiftOf<4> { static const size_t Value = 2; };
template <> struct ShiftOf<8> { static const size_t Value = 3; };

// get size shift width of type
template <typename T>
struct TypeSizeShiftOf
{
    static const size_t Value = ShiftOf<sizeof(T)>::Value;
};


///////////////////////////////////////////////////////////////////////////////
// Remove sign of integral types
template <typename T> struct RemoveSign {};

// char type is not integral, support it as an extension
template <> struct RemoveSign<char> { typedef unsigned char Type; };

// signed
template <> struct RemoveSign<signed char> { typedef unsigned char Type; };
template <> struct RemoveSign<short>       { typedef unsigned short Type; };
template <> struct RemoveSign<int>         { typedef unsigned int Type; };
template <> struct RemoveSign<long>        { typedef unsigned long Type; };
template <> struct RemoveSign<long long>   { typedef unsigned long long Type; };

// handle unsigned
template <> struct RemoveSign<unsigned char>       { typedef unsigned char Type; };
template <> struct RemoveSign<unsigned short>      { typedef unsigned short Type; };
template <> struct RemoveSign<unsigned int>        { typedef unsigned int Type; };
template <> struct RemoveSign<unsigned long>       { typedef unsigned long Type; };
template <> struct RemoveSign<unsigned long long>  { typedef unsigned long long Type; };


///////////////////////////////////////////////////////////////////////////////
// Add sign of integral types
template <typename T> struct AddSign {};

// char type is not integral, support it as an extension
template <> struct AddSign<char> { typedef signed char Type; };

// handle unsigned
template <> struct AddSign<unsigned char>       { typedef signed char Type; };
template <> struct AddSign<unsigned short>      { typedef short Type; };
template <> struct AddSign<unsigned int>        { typedef int Type; };
template <> struct AddSign<unsigned long>       { typedef long Type; };
template <> struct AddSign<unsigned long long>  { typedef long long Type; };

// signed
template <> struct AddSign<signed char> { typedef signed char Type; };
template <> struct AddSign<short>       { typedef short Type; };
template <> struct AddSign<int>         { typedef int Type; };
template <> struct AddSign<long>        { typedef long Type; };
template <> struct AddSign<long long>   { typedef long long Type; };


///////////////////////////////////////////////////////////////////////////////
// variant length integer encoding, encode 7 bit as a group in an encoding
// byte from the lowest group
namespace VariantInteger
{
// bits used in each encoding byte
const unsigned int ENCODING_WIDTH = CHAR_BIT - 1;

// encoding mask of each byte
const unsigned int ENCODING_MASK = (1U << ENCODING_WIDTH) - 1;

// whether encoding sequence is end
const unsigned int ENCODING_CONTINUE_MASK = 1U << ENCODING_WIDTH;

template <typename Type, typename StreamType>
bool ReadUnsigned(Type& value, StreamType& stream)
{
    // make sure Type is unsigned
    typedef char static_assert_unsigned[Type(-1) > 0 ? 1 : -1];

    value = 0;
    unsigned int shift = 0;
    unsigned char encoding_byte;
    while (stream.Read(&encoding_byte, 1))
    {
        if (shift < sizeof(Type) * CHAR_BIT)
            value |= (static_cast<Type>(encoding_byte) & ENCODING_MASK) << shift;
        shift += ENCODING_WIDTH;
        if ((encoding_byte & ENCODING_CONTINUE_MASK) == 0)
            return true;
    }
    return false;
}

template <typename Type, typename StreamType>
void WriteUnsigned(Type value, StreamType& stream)
{
    // make sure Type is unsigned
    typedef char static_assert_unsigned[Type(-1) > 0 ? 1 : -1];

    while (value > ENCODING_MASK)
    {
        stream.Write(static_cast<char>((value & ENCODING_MASK) | ENCODING_CONTINUE_MASK));
        value >>= ENCODING_WIDTH;
    }

    // last byte has no highest bit set
    stream.Write(static_cast<char>(value));
}

/////////////////////////////////////////////////////////////
// using ZigZag algorithm to map signed integer into unsigned

template <typename Type, typename StreamType>
bool ReadSigned(Type& value, StreamType& stream)
{
    // make sure Type is signed
    typedef char static_assert_signed[Type(-1) < 0  ? 1 : -1];

    // we need large enough type to hold encoding bit
    unsigned long long n;
    if (ReadUnsigned(n, stream))
    {
        // lowest bit is sign bit
        if (n & 1) // minus
            value = -static_cast<Type>((n + 1) >> 1);
        else
            value = static_cast<Type>(n >> 1);
        return true;
    }
    return false;
}

template <typename Type, typename StreamType>
void WriteSigned(Type value, StreamType& stream)
{
    // make sure Type is signed
    typedef char static_assert_signed[Type(-1) < 0  ? 1 : -1];

    // ZigZag encoding, euqals to: n = (abs(n) << 1) - (n < 0)
    value = (value << 1U) ^ (value >> (sizeof(Type) * CHAR_BIT - 1));

    typedef typename RemoveSign<Type>::Type UnsignedType;
    UnsignedType n = static_cast<UnsignedType>(value);
    return WriteUnsigned(n, stream);
}

} // end namespace VariantInteger


///////////////////////////////////////////////////////////////////////////////
// Decoder
template <typename OutputStreamType>
class BinaryEncoder : public EncoderConcept<BinaryEncoder<OutputStreamType> >
{
public:
    explicit BinaryEncoder(OutputStreamType& stream) : m_stream(stream)
    {
    }
public: // overrided base members
    void EncodeRecordBegin(const char* name, const char* field_name)
    {
        WriteTag(Tag(TagType_RecordBegin));
    }
    void EncodeRecordEnd(const char* name)
    {
        WriteTag(Tag(TagType_RecordEnd));
    }

    void EncodeListBegin(size_t size, const char* name)
    {
        WriteTag(Tag(TagType_ListBegin));
        WriteVariantUnsigned(size);
    }

    void EncodeListEnd()
    {
        WriteTag(Tag(TagType_ListEnd));
    }

    void EncodePrimitive(bool value, const char* name)
    {
        BoolTag tag(value);
        WriteTag(tag);
    }
    void EncodePrimitive(char value, const char* name)
    {
        WriteTag(CharTag());
        m_stream.Write(value);
    }
    void EncodePrimitive(signed char value, const char* name)
    {
        EncodeSigned(value);
    }
    void EncodePrimitive(unsigned char value, const char* name)
    {
        EncodeUnsigned(value);
    }
    void EncodePrimitive(short value, const char* name)
    {
        EncodeSigned(value);
    }
    void EncodePrimitive(unsigned short value, const char* name)
    {
        EncodeUnsigned(value);
    }
    void EncodePrimitive(int value, const char* name)
    {
        EncodeSigned(value);
    }
    void EncodePrimitive(unsigned int value, const char* name)
    {
        EncodeUnsigned(value);
    }
    void EncodePrimitive(long value, const char* name)
    {
        EncodeSigned(value);
    }
    void EncodePrimitive(unsigned long value, const char* name)
    {
        EncodeUnsigned(value);
    }
    void EncodePrimitive(long long value, const char* name)
    {
        EncodeSigned(value);
    }
    void EncodePrimitive(unsigned long long value, const char* name)
    {
        EncodeUnsigned(value);
    }
    void EncodePrimitive(float value, const char* name)
    {
        WriteTag(FloatTag(TypeSizeShiftOf<float>::Value));
        m_stream.Write(&value, sizeof(value));
    }
    void EncodePrimitive(double value, const char* name)
    {
        WriteTag(FloatTag(TypeSizeShiftOf<double>::Value));
        m_stream.Write(&value, sizeof(value));
    }

    void EncodePrimitive(const std::string& value, const char* name)
    {
        WriteTag(StringTag());
        WriteVariantUnsigned(value.size());
        m_stream.Write(value.data(), value.size());
    }
    void EncodeString(const char* value, size_t length, const char* name)
    {
        WriteTag(StringTag());
        WriteVariantUnsigned(length);
        m_stream.Write(value, length);
    }
    void EncodeNull()
    {
        WriteTag(Tag(TagType_Null));
    }
private:
    void WriteTag(Tag tag)
    {
        m_stream.Write(tag.Header());
    }

    template <typename Type>
    void WriteVariantSigned(Type value)
    {
        VariantInteger::WriteSigned(value, m_stream);
    }

    template <typename Type>
    void WriteVariantUnsigned(Type value)
    {
        VariantInteger::WriteUnsigned(value, m_stream);
    }

    template <typename Type>
    void EncodeFixedSigned(Type value)
    {
        WriteTag(FixedNumberTag(TagType_FixedSignedInteger, TypeSizeShiftOf<Type>::Value));
        m_stream.Write(&value, sizeof(value));
    }

    template <typename Type>
    void EncodeFixedUnsigned(Type value)
    {
        WriteTag(FixedNumberTag(TagType_FixedUnsignedInteger, TypeSizeShiftOf<Type>::Value));
        m_stream.Write(&value, sizeof(value));
    }

    // Encode fixed integer in compact mode
    void EncodeCompactFixedSigned(signed char value)
    {
        EncodeFixedSigned(value);
    }
    void EncodeCompactFixedSigned(short value)
    {
        if (value >= SCHAR_MIN && value <= SCHAR_MAX)
            EncodeFixedSigned((signed char)value);
        else
            EncodeFixedSigned(value);
    }
    void EncodeCompactFixedSigned(int value)
    {
        if (value >= SCHAR_MIN && value <= SCHAR_MAX)
            EncodeFixedSigned((signed char)value);
        else if (value >= SHRT_MIN && value <= SHRT_MAX)
            EncodeFixedSigned((short)value);
        else
            EncodeFixedSigned(value);
    }
    void EncodeCompactFixedSigned(long value)
    {
        if (value >= SCHAR_MIN && value <= SCHAR_MAX)
            EncodeFixedSigned((signed char)value);
        else if (value >= SHRT_MIN && value <= SHRT_MAX)
            EncodeFixedSigned((short)value);
        else if (value >= INT_MIN && value <= INT_MAX)
            EncodeFixedSigned((int)value);
        else
            EncodeFixedSigned(value);
    }
    void EncodeCompactFixedSigned(long long value)
    {
        if (value >= SCHAR_MIN && value <= SCHAR_MAX)
            EncodeFixedSigned((signed char)value);
        else if (value >= SHRT_MIN && value <= SHRT_MAX)
            EncodeFixedSigned((short)value);
        else if (value >= INT_MIN && value <= INT_MAX)
            EncodeFixedSigned((int)value);
        else if (value >= LONG_MIN && value <= LONG_MAX)
            EncodeFixedSigned((long)value);
        else
            EncodeFixedSigned(value);
    }

    // Encode fixed unsigned integer in compact mode
    void EncodeCompactFixedUnsigned(unsigned char value)
    {
        EncodeFixedUnsigned(value);
    }
    void EncodeCompactFixedUnsigned(unsigned short value)
    {
        if (value <= UCHAR_MAX)
            EncodeFixedUnsigned((unsigned char)value);
        else
            EncodeFixedUnsigned(value);
    }
    void EncodeCompactFixedUnsigned(unsigned int value)
    {
        if (value <= UCHAR_MAX)
            EncodeFixedUnsigned((unsigned char)value);
        else if (value <= USHRT_MAX)
            EncodeFixedUnsigned((unsigned short)value);
        else
            EncodeFixedUnsigned(value);
    }
    void EncodeCompactFixedUnsigned(unsigned long value)
    {
        if (value <= UCHAR_MAX)
            EncodeFixedUnsigned((unsigned char)value);
        else if (value <= USHRT_MAX)
            EncodeFixedUnsigned((unsigned short)value);
        else if (value <= UINT_MAX)
            EncodeFixedUnsigned((unsigned int)value);
        else
            EncodeFixedUnsigned(value);
    }
    void EncodeCompactFixedUnsigned(unsigned long long value)
    {
        if (value <= UCHAR_MAX)
            EncodeFixedUnsigned((unsigned char)value);
        else if (value <= USHRT_MAX)
            EncodeFixedUnsigned((unsigned short)value);
        else if (value <= UINT_MAX)
            EncodeFixedUnsigned((unsigned int)value);
        else if (value <= ULONG_MAX)
            EncodeFixedUnsigned((unsigned long)value);
        else
            EncodeFixedUnsigned(value);
    }

    template <typename Type>
    void EncodeVariantSigned(Type value)
    {
        WriteTag(Tag(TagType_SignedInteger));
        WriteVariantSigned(value);
    }

    template <typename Type>
    void EncodeVariantUnsigned(Type value)
    {
        WriteTag(Tag(TagType_UnsignedInteger));
        WriteVariantUnsigned(value);
    }

    template <typename Type>
    void EncodeSigned(Type value)
    {
        // EncodeCompactFixedSigned(value);
        EncodeVariantSigned(value);
    }

    template <typename Type>
    void EncodeUnsigned(Type value)
    {
        // EncodeCompactFixedUnsigned(value);
        EncodeVariantUnsigned(value);
    }
private: // forbid copy
    BinaryEncoder(const BinaryEncoder& src);
    BinaryEncoder&operator=(const BinaryEncoder&);
private:
    OutputStreamType& m_stream;
};

///////////////////////////////////////////////////////////////////////////////
// Decoder
template <typename InputStreamType>
class BinaryDecoder : public DecoderConcept<BinaryDecoder<InputStreamType> >
{
public:
    typedef typename InputStreamType::PosType PosType;
    typedef typename DecoderConcept<BinaryDecoder<InputStreamType> >::Result Result;
    explicit BinaryDecoder(InputStreamType& stream) : m_stream(stream)
    {
    }
public:
    size_t Mark() const
    {
        return m_stream.Tell();
    }

    void Rollback(PosType pos)
    {
        m_stream.Seek(pos);
    }
public: // overrided base members
    bool DecodeRecordBegin(const char* record_name, const char* field_name)
    {
        (void)record_name;
        (void)field_name;

        Result result(this);
        Tag tag;
        result = (ReadTag(tag) && tag.Type() == TagType_RecordBegin);
        return result;
    }

    bool DecodeRecordEnd(const char* name)
    {
        Result result(this);
        Tag tag;
        result = (ReadTag(tag) && tag.Type() == TagType_RecordEnd);
        return result;
    }

    bool DecodeListBegin(size_t& size, const char* name)
    {
        Result result(this);
        Tag tag;
        if (ReadTag(tag) && tag.Type() == TagType_ListBegin)
        {
            result = ReadVariantUnsigned(size);
        }
        return result;
    }

    bool DecodeListEnd()
    {
        Result result(this);
        Tag tag;
        result = (ReadTag(tag) && tag.Type() == TagType_ListEnd);
        return result;
    }

    bool DecodePrimitive(bool& value, const char* name)
    {
        Result result(this);
        BoolTag tag;
        if (ReadTag(tag) && tag.Type() == TagType_Bool)
        {
            value = tag.Value();
            result = true;
        }
        return result;
    }

    bool DecodePrimitive(char& value, const char* name)
    {
        Result result(this);
        Tag tag;
        if (ReadTag(tag) &&
            tag.Type() == TagType_Char &&
            m_stream.Read(&value, sizeof(value))
            )
        {
            result = true;
        }
        return result;
    }

    bool DecodePrimitive(signed char& value, const char* name)
    {
        return DecodeInteger(value);
    }

    bool DecodePrimitive(unsigned char& value, const char* name)
    {
        return DecodeInteger(value);
    }

    bool DecodePrimitive(short& value, const char* name)
    {
        return DecodeInteger(value);
    }

    bool DecodePrimitive(unsigned short& value, const char* name)
    {
        return DecodeInteger(value);
    }

    bool DecodePrimitive(int& value, const char* name)
    {
        return DecodeInteger(value);
    }

    bool DecodePrimitive(unsigned int& value, const char* name)
    {
        return DecodeInteger(value);
    }

    bool DecodePrimitive(long& value, const char* name)
    {
        return DecodeInteger(value);
    }

    bool DecodePrimitive(unsigned long& value, const char* name)
    {
        return DecodeInteger(value);
    }

    bool DecodePrimitive(long long& value, const char* name)
    {
        return DecodeInteger(value);
    }

    bool DecodePrimitive(unsigned long long& value, const char* name)
    {
        return DecodeInteger(value);
    }

    bool DecodePrimitive(float& value, const char* name)
    {
        Result result(this);
        FloatTag tag;
        if (ReadTag(tag) &&
            tag.Type() == TagType_Float
            )
        {
            switch (tag.Width())
            {
            case sizeof(float):
                result = ReadFloat(value, tag.IsBigEndian());
                break;
            case sizeof(double):
                {
                    double d;
                    result = ReadFloat(d, tag.IsBigEndian());
                    if (result)
                        value = (float)d;
                }
                break;
            default:
                break; // result is false
            }
        }
        return result;
    }

    bool DecodePrimitive(double& value, const char* name)
    {
        Result result(this);
        FloatTag tag;
        if (ReadTag(tag) &&
            tag.Type() == TagType_Float
            )
        {
            switch (tag.Width())
            {
            case sizeof(float):
                {
                    float f;
                    result = ReadFloat(f, tag.IsBigEndian());
                    if (result)
                        value = f;
                }
                break;
            case sizeof(double):
                result = ReadFloat(value, tag.IsBigEndian());
                break;
            default:
                break; // result is false
            }
        }
        return result;
    }

    bool DecodePrimitive(std::string& value, const char* name)
    {
        Result result(this);
        Tag tag;
        if (ReadTag(tag) &&
            tag.Type() == TagType_String
            )
        {
            result = ReadString(value);
        }
        return result;
    }

    bool DecodeString(char* buffer, size_t buffer_size, size_t& size, const char* name)
    {
        Result result(this);
        Tag tag;
        if (ReadTag(tag) && tag.Type() == TagType_String)
        {
            if (ReadObjectSize(size) && size <= buffer_size)
            {
                if (m_stream.Read(buffer, size))
                {
                    if (size < buffer_size)
                        buffer[size] = '\0';
                    result = true;
                }
            }
        }
        return result;
    }

    bool SkipNext(std::string& name)
    {
        Result result(this);
        Tag tag;
        if (ReadTag(tag))
        {
            switch (tag.Type())
            {
            case TagType_Bool:
                result = true;
                break;
            case TagType_Char:
                {
                    char value;
                    result = m_stream.Read(&value, sizeof(value));
                }
                break;
            case TagType_FixedSignedInteger:
                {
                    long long value;
                    const FixedNumberTag& number_tag = static_cast<FixedNumberTag&>(tag);
                    result = ReadFixedSigned(number_tag.Width(), value, number_tag.IsBigEndian());
                }
                break;
            case TagType_SignedInteger:
                {
                    long long value;
                    result = ReadVariantSigned(value);
                }
                break;
            case TagType_FixedUnsignedInteger:
                {
                    unsigned long long value;
                    const FixedNumberTag& number_tag = static_cast<FixedNumberTag&>(tag);
                    result = ReadFixedUnsigned(number_tag.Width(), value, number_tag.IsBigEndian());
                }
                break;
            case TagType_UnsignedInteger:
                {
                    unsigned long long value;
                    result = ReadVariantUnsigned(value);
                }
                break;
            case TagType_Float:
                result = m_stream.Skip(static_cast<FixedNumberTag&>(tag).Width());
                break;
            case TagType_RawBytes:
                break;
            case TagType_RecordBegin:
                {
                    std::string member_name;
                    while (SkipNext(member_name))
                    {
                    }
                    result = DecodeRecordEnd(NULL);
                }
                break;
            case TagType_RecordEnd:
                break;
            case TagType_ListBegin:
                {
                    size_t size;
                    if (ReadObjectSize(size))
                    {
                        std::string member_name;
                        for (size_t i = 0; i < size; ++i)
                        {
                            if (!SkipNext(member_name))
                                return false;
                        }
                        result = DecodeListEnd();
                    }
                }
                break;
            case TagType_ListEnd:
                break;
            case TagType_String:
                {
                    size_t size;
                    if (ReadObjectSize(size))
                    {
                        result = m_stream.Skip(size);
                    }
                }
                break;
            default:
                break; // result is false
            }
            name.clear();
        }
        return result;
    }

    template <typename EncoderType>
    bool CopyNext(EncoderType& encoder)
    {
        Result result(this);
        Tag tag;
        if (ReadTag(tag))
        {
            switch (tag.Type())
            {
            case TagType_Bool:
                encoder.EncodePrimitive(static_cast<BoolTag&>(tag).Value(), NULL);
                result = true;
                break;
            case TagType_Char:
                {
                    char value;
                    result = m_stream.Read(&value, sizeof(value));
                    if (result)
                        encoder.EncodePrimitive(value, NULL);
                }
                break;
            case TagType_FixedSignedInteger:
                {
                    long long value = 0;
                    const FixedNumberTag& number_tag = static_cast<FixedNumberTag&>(tag);
                    if (ReadFixedSigned(number_tag.Width(), value, number_tag.IsBigEndian()))
                    {
                        encoder.EncodePrimitive(value, NULL);
                        return true;
                    }
                }
                break;
            case TagType_SignedInteger:
                {
                    long long value;
                    result = ReadVariantSigned(value);
                    if (result)
                        encoder.EncodePrimitive(value, NULL);
                }
                break;
            case TagType_FixedUnsignedInteger:
                {
                    unsigned long long value = 0;
                    const FixedNumberTag& number_tag = static_cast<FixedNumberTag&>(tag);
                    result = ReadFixedUnsigned(number_tag.Width(), value, number_tag.IsBigEndian());
                    if (result)
                        encoder.EncodePrimitive(value, NULL);
                }
                break;
            case TagType_UnsignedInteger:
                {
                    unsigned long long value;
                    result = ReadVariantUnsigned(value);
                    if (result)
                        encoder.EncodePrimitive(value, NULL);
                }
                break;
            case TagType_Float:
                {
                    const FloatTag &float_tag = static_cast<const FloatTag&>(tag);
                    switch (float_tag.Width())
                    {
                    case sizeof(float):
                        {
                            float value;
                            result = ReadFloat(value, float_tag.IsBigEndian());
                            if (result)
                                encoder.EncodePrimitive(value, NULL);
                        }
                        break;
                    case sizeof(double):
                        {
                            double value;
                            result = ReadFloat(value, float_tag.IsBigEndian());
                            if (result)
                                encoder.EncodePrimitive(value, NULL);
                        }
                        break;
                    default:
                        break; // result is false
                    }
                }
                break;
            case TagType_RawBytes:
                break;
            case TagType_RecordBegin:
                {
                    encoder.EncodeRecordBegin(NULL, NULL);
                    while (CopyNext(encoder))
                    {
                    }
                    result = DecodeRecordEnd(NULL);
                    if (result)
                        encoder.EncodeRecordEnd(NULL);
                }
                break;
            case TagType_RecordEnd:
                break;
            case TagType_ListBegin:
                {
                    size_t size;
                    if (ReadObjectSize(size))
                    {
                        encoder.EncodeListBegin(size, NULL);
                        for (size_t i = 0; i < size; ++i)
                        {
                            if (!CopyNext(encoder))
                                return false;
                        }
                        if (DecodeListEnd())
                        {
                            encoder.EncodeListEnd();
                            result = true;
                        }
                    }
                }
                break;
            case TagType_ListEnd:
                break;
            case TagType_String:
                {
                    std::string value;
                    result = ReadString(value);
                    if (result)
                        encoder.EncodePrimitive(value, NULL);
                }
                break;
            default:
                break; // result is false
            }
        }
        return result;
    }
private:
    template <typename Type>
    static void ConvertToLocalByteOrder(Type& value, bool is_big_endian)
    {
        if (is_big_endian != ByteOrder::IsBigEndian())
        {
            ByteOrder::Swap(&value);
        }
    }

    template <typename Type, typename TargetType>
    bool ReadTypedFixed(TargetType& value, bool is_big_endian)
    {
        Type tmp;
        if (m_stream.Read(&tmp, sizeof(tmp)))
        {
            ConvertToLocalByteOrder(tmp, is_big_endian);
            value = static_cast<TargetType>(tmp);
            return true;
        }
        return false;
    }

    template <typename Type>
    bool ReadFixedSigned(size_t size, Type& value, bool is_big_endian)
    {
        if (size == sizeof(signed char))
            return ReadTypedFixed<signed char>(value, is_big_endian);
        else if (size == sizeof(short))
            return ReadTypedFixed<signed short>(value, is_big_endian);
        else if (size == sizeof(int))
            return ReadTypedFixed<int>(value, is_big_endian);
        else if (size == sizeof(long))
            return ReadTypedFixed<int>(value, is_big_endian);
        else if (size == sizeof(long long))
            return ReadTypedFixed<long long>(value, is_big_endian);
        return false;
    }

    template <typename Type>
    bool ReadFixedUnsigned(size_t size, Type& value, bool is_big_endian)
    {
        if (size == sizeof(unsigned char))
            return ReadTypedFixed<unsigned char>(value, is_big_endian);
        else if (size == sizeof(unsigned short))
            return ReadTypedFixed<unsigned short>(value, is_big_endian);
        else if (size == sizeof(unsigned int))
            return ReadTypedFixed<unsigned int>(value, is_big_endian);
        else if (size == sizeof(unsigned long))
            return ReadTypedFixed<unsigned long>(value, is_big_endian);
        else if (size == sizeof(unsigned long long))
            return ReadTypedFixed<unsigned long long>(value, is_big_endian);
        return false;
    }

    template <typename Type>
    bool ReadVariantSigned(Type& value)
    {
        return VariantInteger::ReadSigned(value, m_stream);
    }

    template <typename Type>
    bool ReadVariantUnsigned(Type& value)
    {
        return VariantInteger::ReadUnsigned(value, m_stream);
    }

    bool ReadTag(Tag& tag)
    {
        unsigned char tag_byte;
        if (m_stream.Read(&tag_byte, 1))
        {
            tag = Tag::Parse(tag_byte);
            return true;
        }
        return false;
    }

    bool ReadObjectSize(size_t& size)
    {
        return ReadVariantUnsigned(size);
    }

    bool ReadString(std::string& value)
    {
        size_t size;
        if (ReadObjectSize(size))
        {
            value.clear();
            const size_t buf_size = 4096;
            if (size > buf_size)
            {
                std::vector<char> buffer(size);
                if (m_stream.Read(&buffer[0], size))
                {
                    value.assign(&buffer[0], size);
                    return true;
                }
            }
            else
            {
                char buffer[buf_size];
                if (m_stream.Read(buffer, size))
                {
                    value.assign(buffer, size);
                    return true;
                }
            }
        }
        return false;
    }

    template <typename T>
    bool ReadFloat(T& value, bool is_big_endian)
    {
        if (m_stream.Read(&value, sizeof(value)))
        {
            ConvertToLocalByteOrder(value, is_big_endian);
            return true;
        }
        return false;
    }

    template <typename Type>
    bool DecodeInteger(Type& value)
    {
        Result result(this);
        FixedNumberTag tag;
        if (ReadTag(tag))
        {
            switch (tag.Type())
            {
            case TagType_FixedSignedInteger:
                result = ReadFixedSigned(tag.Width(), value, tag.IsBigEndian());
                break;
            case TagType_FixedUnsignedInteger:
                result = ReadFixedUnsigned(tag.Width(), value, tag.IsBigEndian());
                break;
            case TagType_SignedInteger:
                result = ReadVariantSigned(reinterpret_cast<typename AddSign<Type>::Type&>(value));
                break;
            case TagType_UnsignedInteger:
                result = ReadVariantUnsigned(
                    reinterpret_cast<typename RemoveSign<Type>::Type&>(value));
                break;
            default:
                break;
            }
        }
        return result;
    }
private: // forbid copy
    BinaryDecoder(const BinaryDecoder& src);
    BinaryDecoder&operator=(const BinaryDecoder&);
private:
    InputStreamType& m_stream;
};

} // end namespace Serialize

#endif // COMMON_BASE_SERIALIZE_BINARY_CODEC_HPP
