// Copyright (c) 2008, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_SERIALIZE_JSON_CODEC_HPP
#define COMMON_BASE_SERIALIZE_JSON_CODEC_HPP

#include <common/base/serialize/base.hpp>
#include <sstream>
#include <stack>

// GLOBAL_NOLINT(runtime/int)

namespace Serialize {

template <typename StreamType>
class JsonEncoder : public EncoderConcept<JsonEncoder<StreamType> >
{
public:
    JsonEncoder(StreamType& stream, bool indent = true, bool comment = true)
        : m_Stream(stream), m_Indent(indent), m_Comment(comment)
    {
        m_CounterStack.push(0);
    }
public:
    void EncodeRecordBegin(const char* type_name, const char* field_name)
    {
        StartMember();
        WriteFieldName(field_name);
        WriteString("{");
        if (type_name && m_Comment)
        {
            WriteString(" /* type = ");
            WriteString(type_name);
            WriteString(" */");
        }
        m_CounterStack.push(0);
    }

    void EncodeRecordEnd(const char* name)
    {
        CloseMember('}');
    }

    void EncodeListBegin(size_t size, const char* name)
    {
        StartMember();
        WriteFieldName(name);
        WriteString("[");
        m_CounterStack.push(0);
    }

    void EncodeListEnd()
    {
        CloseMember(']');
    }

    void EncodePrimitive(bool value, const char* name)
    {
        StartMember();
        WriteFieldName(name);
        WriteString(value ? "true" : "false");
        EndMember();
    }

    void EncodePrimitive(char value, const char* name)
    {
        StartMember();
        WriteFieldName(name);
        m_Stream.Write('"');
        WriteQuotChar(value);
        m_Stream.Write('"');
        EndMember();
    }

    void EncodePrimitive(signed char value, const char* name)
    {
        EncodePrimitiveWithTypeName("integer", (int)value, name);
    }

    void EncodePrimitive(unsigned char value, const char* name)
    {
        EncodePrimitiveWithTypeName("integer", (int)value, name);
    }

    void EncodePrimitive(short value, const char* name)
    {
        EncodePrimitiveWithTypeName("integer", value, name);
    }

    void EncodePrimitive(unsigned short value, const char* name)
    {
        EncodePrimitiveWithTypeName("integer", value, name);
    }

    void EncodePrimitive(int value, const char* name)
    {
        EncodePrimitiveWithTypeName("integer", value, name);
    }

    void EncodePrimitive(unsigned int value, const char* name)
    {
        EncodePrimitiveWithTypeName("integer", value, name);
    }

    void EncodePrimitive(long value, const char* name)
    {
        EncodePrimitiveWithTypeName("integer", value, name);
    }

    void EncodePrimitive(unsigned long value, const char* name)
    {
        EncodePrimitiveWithTypeName("integer", value, name);
    }

    void EncodePrimitive(long long value, const char* name)
    {
        EncodePrimitiveWithTypeName("integer", value, name);
    }

    void EncodePrimitive(unsigned long long value, const char* name)
    {
        EncodePrimitiveWithTypeName("integer", value, name);
    }
    void EncodePrimitive(float value, const char* name)
    {
        EncodePrimitiveWithTypeName("float", value, name);
    }

    void EncodePrimitive(double value, const char* name)
    {
        EncodePrimitiveWithTypeName("float", value, name);
    }

    void EncodeString(const char* value, size_t size, const char* name)
    {
        StartMember();
        WriteFieldName(name);
        WriteString("\"");
        WriteQuotString(value, size);
        WriteString("\"");
        EndMember();
    }
    void EncodePrimitive(const std::string& value, const char* name)
    {
        EncodeString(value.data(), value.size(), name);
    }

    void EncodeNull(const char* name = NULL)
    {
        StartMember();
        WriteFieldName(name);
        WriteString("null");
        EndMember();
    }

private:
    void WriteString(const char* str)
    {
        m_Stream.Write(str, strlen(str));
    }

    void WriteQuotChar(char ch)
    {
        switch (ch)
        {
        case '\a':
            WriteString("\\a");
            break;
        case '\b':
            WriteString("\\b");
            break;
        case '\f':
            WriteString("\\f");
            break;
        case '\n':
            WriteString("\\n");
            break;
        case '\r':
            WriteString("\\r");
            break;
        case '\t':
            WriteString("\\t");
            break;
        case '\v':
            WriteString("\\v");
            break;
        case '\\':
            WriteString("\\\\");
            break;
        case '"':
            WriteString("\\\"");
            break;
        case '\'':
            WriteString("\\'");
            break;
        default:
            if (iscntrl(ch))
            {
                WriteString("\\x");
                static const char hex[] = "0123456789ABCDEF";
                m_Stream.Write(hex[(unsigned char)ch >> 4]);
                m_Stream.Write(hex[(unsigned char)ch & 0x0F]);
            }
            else
            {
                m_Stream.Write(ch);
            }
        }
    }
    void WriteQuotString(const char* str, size_t size)
    {
        for (size_t i = 0; i < size; ++i)
            WriteQuotChar(str[i]);
    }

    void WriteFieldName(const char* field_name)
    {
        if (field_name)
        {
            WriteString("\"");
            WriteQuotString(field_name, strlen(field_name));
            WriteString("\" : ");
        }
    }

    void Indent()
    {
        for (size_t i = 1; i < m_CounterStack.size(); ++i)
            m_Stream.Write('\t');
    }
    void StartMember()
    {
        if (!m_CounterStack.empty())
        {
            if (m_CounterStack.top() > 0)
                m_Stream.Write(',');
            if (m_CounterStack.size() > 1 || m_CounterStack.top() > 0)
            {
                if (m_Indent)
                {
                    WriteString("\n");
                    Indent();
                }
                else
                {
                    WriteString(" ");
                }
            }
        }
    }
    void EndMember()
    {
        if (!m_CounterStack.empty())
            ++m_CounterStack.top();
    }
    void CloseMember(char end_char)
    {
        if (!m_CounterStack.empty())
        {
            if (m_Indent)
            {
                if (m_CounterStack.top() > 0)
                {
                    WriteString("\n");
                    m_CounterStack.pop();
                    Indent();
                }
                else
                {
                    m_CounterStack.pop();
                }
            }
            else
            {
                m_Stream.Write(' ');
            }
            m_Stream.Write(end_char);
        }
        EndMember();
    }

    template <typename Type>
    void EncodePrimitiveWithTypeName(const char* type_name, Type value, const char* name)
    {
        StartMember();
        WriteFieldName(name);
        std::ostringstream oss;
        oss << value;
        const std::string& str = oss.str();
        m_Stream.Write(str.data(), str.size());
        EndMember();
    }
private: // forbid copy
    JsonEncoder(const JsonEncoder& src);
    JsonEncoder&operator=(const JsonEncoder&);

private:
    StreamType& m_Stream;
    std::stack<int> m_CounterStack;
    bool m_Indent;
    bool m_Comment;
};

} // end namespace Serialize

#endif // COMMON_BASE_SERIALIZE_JSON_CODEC_HPP

