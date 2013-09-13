// Copyright (c) 2008, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_SERIALIZE_BASE_HPP
#define COMMON_BASE_SERIALIZE_BASE_HPP

#include <stddef.h>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>

// GLOBAL_NOLINT(runtime/int)

namespace Serialize
{
/////////////////////////////////////////////////////////////////////
// Serialization exceptions
class Error : public std::runtime_error
{
public:
    explicit Error(const std::string& message) :
        std::runtime_error(message)
    {
    }
};

class InputError : public Error
{
public:
    explicit InputError(const std::string& message = "Serialize input error"):
        Error(message)
    {
    }
};

class OutputError : public Error
{
public:
    explicit OutputError(const std::string& message = "Serialize Output error"):
        Error(message)
    {
    }
};

class FormatError : public InputError
{
public:
    explicit FormatError(const std::string& message = "Serialize input format error")
        : InputError(message)
    {
    }
};

class TypeError : public FormatError
{
public:
    TypeError(const std::string& expected_type, const std::string& actual_type)
        : FormatError("Serialize input type error, except " +
                      expected_type + ", actual " + actual_type)
    {
    }
};

// track error location
struct ErrorLocation
{
public:
    const char* StructName;
    const char* MemberName;
    const char* FileName;
    int LineNumber;
};

class ErrorStack
{
    static const int MaxDepth = 4;
public:
    ErrorStack() : m_Depth(0), m_IsFull(false) {}
public:
    void Push(
        const char* struct_name, const char* member_name,
        const char* filename, int line_number)
    {
        if (m_Depth < MaxDepth)
        {
            m_Locations[m_Depth].StructName = struct_name;
            m_Locations[m_Depth].MemberName = member_name;
            m_Locations[m_Depth].FileName = filename;
            m_Locations[m_Depth].LineNumber = line_number;
            ++m_Depth;
        }
        else
        {
            m_IsFull = true;
        }
    }
    void Clear()
    {
        m_Depth = 0;
    }
    bool IsEmpty() const
    {
        return m_Depth == 0;
    }
    std::string ToString() const
    {
        std::ostringstream oss;
        if (!IsEmpty())
        {
            oss << "Error stack:\n";
            for (int i = 0; i < m_Depth; ++i)
            {
                oss <<
                    "\t[" << i << "] " <<
                    m_Locations[i].FileName << ":" << m_Locations[i].LineNumber << ": " <<
                    m_Locations[i].StructName << "::" << m_Locations[i].MemberName << "\n";
            }
            if (m_IsFull)
                oss << "\t[...] Stack full\n";
        }
        return oss.str();
    }
private:
    ErrorLocation m_Locations[MaxDepth];
    int m_Depth;
    bool m_IsFull;
};

#if 0
///////////////////////////////////////////////////////////////////////////////
// Streams defination
///////////////////////////////////////////////////////////////////////////////

class InputStreamConcept
{
public:
    size_t Tell() const;
    void Seek(size_t pos);
    bool End() const;
    size_t Peek(void* buffer, size_t size);
    void Skip(size_t size);
    size_t Read(void* buffer, size_t size);
};

class OutputStreamConcept
{
public:
    size_t Tell() const;
    void Seek(size_t pos);
    size_t Write(const void* buffer, size_t size);
    size_t Write(const char* str);
};
#endif

///////////////////////////////////////////////////////////////////////////////
// encoder
template <typename EncoderType>
class EncoderConcept
{
protected:
    EncoderConcept()
    {
        (void)(void*)&Check;
    }
public:
    template <typename Type>
    EncoderType& operator<<(const Type& value)
    {
        EncoderType& encoder = *static_cast<EncoderType*>(this);
        Encode(encoder, value);
        return encoder;
    }
    class ErrorStack ErrorStack;
private:
    static EncoderType& MakeEncoder()
    {
        return *reinterpret_cast<EncoderType*>(0x1);
    }
    static void Check()
    {
        EncoderType& encoder = MakeEncoder();

        (void)encoder.EncodeRecordBegin("", "");
        (void)encoder.EncodeRecordEnd("");
        (void)encoder.EncodeListBegin(1, "");
        (void)encoder.EncodeListEnd();
        (void)encoder.EncodePrimitive((bool)false, "");
        (void)encoder.EncodePrimitive((char)0, "");
        (void)encoder.EncodePrimitive((signed char)(0), "");
        (void)encoder.EncodePrimitive((unsigned char)(0), "");
        (void)encoder.EncodePrimitive((short)0, "");
        (void)encoder.EncodePrimitive((unsigned short)0, "");
        (void)encoder.EncodePrimitive((int)0, "");
        (void)encoder.EncodePrimitive((unsigned int)0, "");
        (void)encoder.EncodePrimitive((long)0, "");
        (void)encoder.EncodePrimitive((unsigned long)0, "");
        (void)encoder.EncodePrimitive((long long)0, "");
        (void)encoder.EncodePrimitive((unsigned long long)0, "");
        (void)encoder.EncodePrimitive((float)0, "");
        (void)encoder.EncodePrimitive((double)0, "");
        (void)encoder.EncodePrimitive(std::string(), "");
        (void)encoder.EncodeString("", 0, "");
        (void)encoder.EncodeNull();
    }
};


///////////////////////////////////////////////////////////////////////////////
// decoder
template <typename DecoderType>
class DecoderConcept
{
public:
    // using to save old position and restore if decoding falure
    class Result
    {
    public:
        explicit Result(DecoderType* decoder) :
            m_decoder(*decoder),
            m_mark(decoder->Mark()),
            m_result(false)
        {
        }
        explicit Result(DecoderType& decoder) :
            m_decoder(decoder),
            m_mark(decoder.Mark()),
            m_result(false)
        {
        }
        ~Result()
        {
            if (!m_result)
                m_decoder.Rollback(m_mark);
        }
        Result& operator=(bool value)
        {
            m_result = value;
            return *this;
        }
        operator bool() const
        {
            return m_result;
        }
    private: // forbid copy
        Result(const Result& src);
        Result& operator=(const Result& rhs);
    private:
        DecoderType& m_decoder;
        typename DecoderType::PosType m_mark;
        bool m_result;
    };
protected:
    DecoderConcept()
    {
        (void)(void*)&Check;
    }
public:
    class ErrorStack ErrorStack;
    template <typename Type>
    DecoderType& operator>>(Type& value)
    {
        DecoderType& decoder = *static_cast<DecoderType*>(this);
        if (!Decode(decoder, value))
        {
            if (decoder.ErrorStack.IsEmpty())
            {
                throw InputError("Serialization Decode error");
            }
            else
            {
                throw InputError("Serialization Decode error" + decoder.ErrorStack.ToString());
            }
        }
        return decoder;
    }
private:
    static int ReturnBool(bool);
    template <typename Type>
    static Type Get();

    static void Check()
    {
        DecoderType* decoder = NULL;
        (void)(sizeof(ReturnBool(decoder->DecodeRecordBegin("", ""))));
        (void)(sizeof(ReturnBool(decoder->DecodeRecordEnd(""))));
        (void)(sizeof(ReturnBool(decoder->DecodeListBegin(Get<size_t&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodeListEnd())));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<bool&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<char&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<signed char&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<unsigned char&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<short&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<unsigned short&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<int&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<unsigned int&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<long&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<unsigned long&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<long long&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<unsigned long long&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<float&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<double&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->DecodePrimitive(Get<std::string&>(), ""))));
        (void)(sizeof(ReturnBool(decoder->SkipNext(Get<std::string&>()))));
        (void)(sizeof(ReturnBool(decoder->CopyNext(Get<int&>()))));
}
};


} // end namespace Serialize


#endif // COMMON_BASE_SERIALIZE_BASE_HPP

