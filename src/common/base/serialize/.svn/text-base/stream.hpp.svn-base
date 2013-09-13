// Copyright (c) 2008, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef SERIALIZE_STREAM_HPP
#define SERIALIZE_STREAM_HPP

// Define some common used serialize streams

#include <assert.h>
#include <stdio.h>
#include <iosfwd>
#include <vector>

#include <common/base/serialize/base.hpp>

// GLOBAL_NOLINT(runtime/int)

#ifdef __GNUC__
#define stdio_ftell64 ftello64
#define stdio_fseek64 fseeko64
#elif defined _MSC_VER
#if _MSC_VER >= 1400
#define stdio_ftell64 _ftelli64
#define stdio_fseek64 _fseeki64
#else
#include <io.h>
#define stdio_ftell64(fp) _telli64(fileno(fp))
#define stdio_fseek64(fp, offset, whence) _lseeki64(fileno(fp), offset, whence)
#endif
#endif

namespace Serialize
{

class NullOutputStream
{
public:
    typedef long long PosType;
    NullOutputStream() : m_pos(0)
    {
    }
    PosType Tell() const
    {
        return m_pos;
    }
    void Seek(PosType pos)
    {
        m_pos = pos;
    }
    size_t Write(char ch)
    {
        (void)ch;
        ++m_pos;
        return 1;
    }
    size_t Write(const void* buffer, size_t size)
    {
        (void)buffer;
        m_pos += size;
        return size;
    }
private:
    long long m_pos;
};

class BufferInputStream
{
public:
    typedef size_t PosType;
    explicit BufferInputStream(const void* buffer, size_t size):
        m_buffer(static_cast<const char*>(buffer)),
        m_size(size),
        m_pos(0)
    {
    }
public:
    size_t Tell() const
    {
        return m_pos;
    }

    void Seek(size_t pos)
    {
        if (pos <= m_size)
            m_pos = pos;
        else
            throw InputError("Seeking out of range");
    }

    bool End() const
    {
        return m_pos >= m_size;
    }

    bool Peek(void* buffer, size_t size)
    {
        if (size > 0)
        {
            if (m_pos + size <= m_size)
            {
                memcpy(buffer, &m_buffer[m_pos], size);
                return true;
            }
            return false;
        }
        return true;
    }

    bool Skip(size_t size)
    {
        if (m_pos + size <= m_size)
        {
            m_pos += size;
            return true;
        }
        return false;
    }

    bool Read(void* buffer, size_t size)
    {
        if (Peek(buffer, size))
        {
            m_pos += size;
            return true;
        }
        return false;
    }
private: // forbid copy
    BufferInputStream(const BufferInputStream& src);
    BufferInputStream& operator=(const BufferInputStream& rhs);
private:
    const char* m_buffer;
    size_t m_size;
    size_t m_pos;
};

class BufferOutputStream
{
public:
    typedef size_t PosType;
    explicit BufferOutputStream(void* buffer, size_t size):
        m_buffer(static_cast<char*>(buffer)),
        m_size(size),
        m_pos(0)
    {
    }
public:
    size_t Tell() const
    {
        return m_pos;
    }

    void Seek(size_t pos)
    {
        if (pos > m_size)
            throw OutputError("BufferOutputStream: Seeking overflow");
        m_pos = pos;
    }

    size_t Write(char ch)
    {
        if (m_pos < m_size)
            m_buffer[m_pos++] = ch;
        else
            Overflow();
        return 1;
    }
    size_t Write(const void* buffer, size_t size)
    {
        size_t remain_size = m_size - m_pos;
        if (remain_size >= size)
        {
            const char* data = static_cast<const char*>(buffer);
            switch (size)
            {
            case 4:
                m_buffer[m_pos++] = *(data++);
                // fall through
            case 3:
                m_buffer[m_pos++] = *(data++);
                // fall through
            case 2:
                m_buffer[m_pos++] = *(data++);
                // fall through
            case 1:
                m_buffer[m_pos++] = *data;
                break;
            default:
                memcpy(&m_buffer[m_pos], buffer, size);
                m_pos += size;
                break;
            }
        }
        else
        {
            Overflow();
        }

        return size;
    }
private: // forbid copy
    BufferOutputStream(const BufferOutputStream& src);
    BufferOutputStream& operator=(const BufferOutputStream& rhs);
private:
    static void Overflow()
    {
        throw OutputError("BufferOutputStream: Writing overflow");
    }
private:
    char* m_buffer;
    size_t m_size;
    size_t m_pos;
};

class UncheckedBufferOutputStream
{
public:
    typedef size_t PosType;
    explicit UncheckedBufferOutputStream(void* buffer):
        m_buffer(static_cast<char*>(buffer)),
        m_pos(0)
    {
    }
public:
    size_t Tell() const
    {
        return m_pos;
    }

    void Seek(size_t pos)
    {
        m_pos = pos;
    }

    size_t Write(char ch)
    {
        m_buffer[m_pos++] = ch;
        return 1;
    }
    size_t Write(const void* buffer, size_t size)
    {
        const char* data = static_cast<const char*>(buffer);
        switch (size)
        {
        case 4:
            m_buffer[m_pos++] = *(data++);
            // fall through
        case 3:
            m_buffer[m_pos++] = *(data++);
            // fall through
        case 2:
            m_buffer[m_pos++] = *(data++);
            // fall through
        case 1:
            m_buffer[m_pos++] = *data;
            break;
        default:
            memcpy(&m_buffer[m_pos], buffer, size);
            m_pos += size;
            break;
        }

        return size;
    }
private: // forbid copy
    UncheckedBufferOutputStream(const UncheckedBufferOutputStream& src);
    UncheckedBufferOutputStream& operator=(const UncheckedBufferOutputStream& rhs);
private:
    char* m_buffer;
    size_t m_pos;
};

class VectorInputStream : public BufferInputStream
{
public:
    explicit VectorInputStream(const std::vector<char>& v):
        BufferInputStream(&v[0], v.size())
    {
    }
private: // forbid copy
    VectorInputStream(const VectorInputStream& src);
    VectorInputStream& operator=(const VectorInputStream& rhs);
private:
};

class VectorOutputStream
{
public:
    typedef size_t PosType;
    explicit VectorOutputStream(std::vector<char>& v):
        m_vector(v)
    {
        m_vector.clear();
    }
    explicit VectorOutputStream(std::vector<char>& v, size_t skip):
        m_vector(v)
    {
        m_vector.resize(skip);
    }
public:
    size_t Tell() const
    {
        return m_vector.size();
    }
    size_t Write(char ch)
    {
        m_vector.push_back(ch);
        return 1;
    }
    size_t Write(const void* buffer, size_t size)
    {
        const char* data = static_cast<const char*>(buffer);
        m_vector.insert(m_vector.end(), data, data + size);
        return size;
    }
private: // forbid copy
    VectorOutputStream(const VectorOutputStream& src);
    VectorOutputStream& operator=(const VectorOutputStream& rhs);
private:
    std::vector<char>& m_vector;
};

class StringInputStream : public BufferInputStream
{
public:
    explicit StringInputStream(const std::string& s):
        BufferInputStream(s.data(), s.size())
    {
    }
private: // forbid copy
    StringInputStream(const StringInputStream& src);
    StringInputStream& operator=(const StringInputStream& rhs);
private:
};

class StringOutputStream
{
public:
    typedef size_t PosType;
    explicit StringOutputStream(std::string& s):
        m_string(s)
    {
        m_string.clear();
    }
    explicit StringOutputStream(std::string& s, size_t skip):
        m_string(s)
    {
        m_string.resize(skip);
    }
public:
    size_t Tell() const
    {
        return m_string.size();
    }
    size_t Write(char ch)
    {
        m_string.push_back(ch);
        return 1;
    }
    size_t Write(const void* buffer, size_t size)
    {
        const char* data = static_cast<const char*>(buffer);
        m_string.insert(m_string.end(), data, data + size);
        return size;
    }
private: // forbid copy
    StringOutputStream(const StringOutputStream& src);
    StringOutputStream& operator=(const StringOutputStream& rhs);
private:
    std::string& m_string;
};

class StdinStream
{
public:
    typedef long long PosType;

    explicit StdinStream(FILE* fp, bool auto_flush = true) :m_fp(fp), m_AutoFlush(auto_flush)
    {
    }
    ~StdinStream()
    {
        if (m_AutoFlush)
            fflush(m_fp);
    }
    PosType Tell() const
    {
        return stdio_ftell64(m_fp);
    }

    void Seek(PosType pos)
    {
        if (stdio_fseek64(m_fp, pos, SEEK_SET) != 0)
            throw InputError("StdinStream: Seeking error");
    }

    bool End() const
    {
        return feof(m_fp) != 0;
    }

    bool Peek(void* buffer, size_t size)
    {
        if (size == 0)
            return true;
        if (End())
            throw InputError("Reach end of stream");
        PosType offset = stdio_ftell64(m_fp);
        size_t n = fread(buffer, size, 1, m_fp);
        stdio_fseek64(m_fp, offset, SEEK_SET);
        return n == 1;
    }

    bool Skip(size_t size)
    {
        if (stdio_fseek64(m_fp, size, SEEK_CUR) != 0)
            throw InputError("StdinStream: Seeking error");
        return true;
    }

    bool Read(void* buffer, size_t size)
    {
        if (size != 0)
            return fread(buffer, size, 1, m_fp) == 1;
        else
            return true;
    }
private:
    StdinStream(const StdinStream& src);
    StdinStream& operator=(const StdinStream& rhs);
private:
    FILE* m_fp;
    bool m_AutoFlush;
};

class StdoutStream
{
public:
    typedef long long PosType;
    explicit StdoutStream(FILE* fp, bool auto_flush = true) : m_fp(fp), m_AutoFlush(auto_flush)
    {
    }
    ~StdoutStream()
    {
        if (m_AutoFlush)
            fflush(m_fp);
    }
public:
    PosType Tell() const
    {
        return stdio_ftell64(m_fp);
    }
    void Seek(PosType pos)
    {
        stdio_fseek64(m_fp, pos, SEEK_SET);
    }
    size_t Write(char ch)
    {
        return size_t(putc(ch, m_fp) != EOF);
    }
    size_t Write(const void* buffer, size_t size)
    {
        return fwrite(buffer, 1, size, m_fp);
    }
private: // forbid copy
    StdoutStream(const StdoutStream& src);
    StdoutStream& operator=(const StdoutStream& rhs);
private:
    FILE* m_fp;
    bool m_AutoFlush;
};

class CxxOutputStream
{
public:
    typedef long long PosType;
    CxxOutputStream(std::ostream& os) : m_os(os)
    {
    }
    long long Tell() const
    {
        return m_os.tellp();
    }
    void Seek(PosType pos)
    {
        m_os.seekp(static_cast<std::streamoff>(pos));
    }
    size_t Write(char ch)
    {
        m_os.put(ch);
        return 1;
    }
    size_t Write(const void* buffer, size_t size)
    {
        m_os.write(static_cast<const char*>(buffer), size);
        return m_os ? size : 0;
    }
private:
    std::ostream& m_os;
};

template <typename UnderlyStreamType>
class HexOutputStreamFilter
{
public:
    HexOutputStreamFilter(UnderlyStreamType& stream):
        m_stream(stream)
    {
    }
public:
    typename UnderlyStreamType::PosType Tell() const
    {
        return m_stream.Tell() / 2;
    }
    void Seek(typename UnderlyStreamType::PosType pos)
    {
        return m_stream.Seek(pos / 2);
    }
    size_t Write(const void* buffer, size_t size)
    {
        const unsigned char* p = static_cast<const unsigned char*>(buffer);
        for (size_t i = 0; i < size; ++i)
        {
            static const char to_hex[] = "0123456789ABCDEF";
            char buf[2];
            buf[0] = to_hex[p[i] >> 4];
            buf[1] = to_hex[p[i] & 0x0F];
            m_stream.Write(buf, 2);
        }
        return size;
    }
private: // forbid copy
    HexOutputStreamFilter(const HexOutputStreamFilter& src);
    HexOutputStreamFilter& operator=(const HexOutputStreamFilter& rhs);
private:
    UnderlyStreamType& m_stream;
};

}

#undef stdio_ftell64
#undef stdio_fseek64

#endif//SERIALIZE_STREAM_HPP
