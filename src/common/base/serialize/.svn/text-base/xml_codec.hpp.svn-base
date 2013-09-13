// Copyright (c) 2008, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef SERIALIZE_XML_CODEC_HPP
#define SERIALIZE_XML_CODEC_HPP

// Xml format encoder, for debugging mainly

#include <sstream>
#include <common/base/serialize/base.hpp>

// GLOBAL_NOLINT(runtime/int)

namespace Serialize
{

template <typename OutputStreamType>
class XmlEncoder : public EncoderConcept<XmlEncoder<OutputStreamType> >
{
public:
    XmlEncoder(OutputStreamType& stream) : m_stream(stream), m_indent(0)
    {
    }
public:
    void EncodeRecordBegin(const char* name, const char* field_name)
    {
        Indent();
        WriteStringToStream("<record");
        if (name)
        {
            WriteStringToStream(" type=\"");
            WriteStringToStream(name);
            WriteStringToStream("\"");
        }
        if (field_name)
        {
            WriteStringToStream(" name=\"");
            WriteStringToStream(field_name);
            WriteStringToStream("\"");
        }
        WriteStringToStream(">\n");
        ++m_indent;
    }

    void EncodeRecordEnd(const char* name)
    {
        --m_indent;
        Indent();
        WriteStringToStream("</record>\n");
    }

    void EncodeListBegin(size_t size, const char* name)
    {
        Indent();
        WriteStringToStream("<list");
        if (name)
        {
            WriteStringToStream(" name=\"");
            WriteStringToStream(name);
            WriteStringToStream("\"");
        }
        if (size)
        {
            WriteStringToStream(" size=\"");
            char buf[12];
            int length = sprintf(buf, "%u", (unsigned int)size);
            m_stream.Write(buf, length);
            WriteStringToStream("\"");
        }
        WriteStringToStream(">\n");
        ++m_indent;
    }

    void EncodeListEnd()
    {
        --m_indent;
        Indent();
        WriteStringToStream("</list>\n");
    }

    void EncodePrimitive(bool value, const char* name)
    {
        Indent();
        EncodePrimitiveHeader("bool", name);
        WriteStringToStream(value ? "true" : "false");
        EncodePrimitiveTail("bool");
    }

    void EncodePrimitive(char value, const char* name)
    {
        Indent();
        EncodePrimitiveHeader("char", name);
        m_stream.Write(&value, 1);
        EncodePrimitiveTail("char");
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
        Indent();
        EncodePrimitiveHeader("string", name);
        WriteStringToStream(" \"");
        m_stream.Write(value, size);
        WriteStringToStream("\" ");
        EncodePrimitiveTail("string");
    }
    void EncodePrimitive(const std::string& value, const char* name)
    {
        EncodeString(value.data(), value.size(), name);
    }
    void EncodeNull()
    {
        Indent();
        return WriteStringToStream("<null />\n");
    }

private:
    void Indent()
    {
        for (int i = 0; i < m_indent; ++i)
            m_stream.Write('\t');
    }
    void WriteStringToStream(const char* str)
    {
        m_stream.Write(str, strlen(str));
    }
    void EncodePrimitiveHeader(const char* type_name, const char* name)
    {
        WriteStringToStream("<");
        WriteStringToStream(type_name);
        if (name)
        {
            WriteStringToStream(" name=\"");
            WriteStringToStream(name);
            WriteStringToStream("\"");
        }
        WriteStringToStream(">");
    }

    void EncodePrimitiveTail(const char* type_name)
    {
        WriteStringToStream("</");
        WriteStringToStream(type_name);
        WriteStringToStream(">\n");
    }

    template <typename Type>
    void EncodePrimitiveWithTypeName(const char* type_name, Type value, const char* name)
    {
        Indent();
        EncodePrimitiveHeader(type_name, name);
        std::ostringstream oss;
        oss << value;
        const std::string& str = oss.str();
        WriteStringToStream(" ");
        m_stream.Write(str.data(), str.size());
        WriteStringToStream(" ");
        EncodePrimitiveTail(type_name);
    }
private: // forbid copy
    XmlEncoder(const XmlEncoder& src);
    XmlEncoder&operator=(const XmlEncoder&);

private:
    OutputStreamType& m_stream;
    int m_indent;
};

} // end namespace Serialize

#endif//SERIALIZE_XML_CODEC_HPP
