#ifndef COMMON_HASHVAL_HPP
#define COMMON_HASHVAL_HPP

/**
 * @file hashval.hpp
 * @brief 
 * @author welkin
 * @date 2011-06-02
 */

#include <string>
#include <ostream>

template <int BYTE_SIZE>
class HashVal
{
public:
    HashVal()
    {
        memset(m_value, 0, sizeof(m_value));
    }

    HashVal(const unsigned char * value, const unsigned int byte_size)
    {
        SetValue(value, byte_size);
    }

    HashVal(const char * str)
    {
        SetString(str);
    }

    inline void Reset()
    {
        memset(m_value, 0, sizeof(m_value));
    }

    void SetValue(const unsigned char * value, const unsigned int byte_size)
    {
        if (byte_size == sizeof(m_value))
        {
            memcpy(m_value, value, sizeof(m_value));
        }
        else
        {
            memset(m_value, 0, sizeof(m_value));
        }
    }

    void SetString(const char * str)
    {
        memset(m_value, 0, sizeof(m_value));

        if (NULL != str)
        {
            int len = strlen(str);
            if (len <= sizeof(m_value) * 2)
            {
                if (0 == (len % 2))
                {
                    int index = sizeof(m_value) - len / 2;
                    int i = 0;
                    while (i < len)
                    {
                        m_value[index] = Hex2Char(str[i], str[i + 1]);
                        i += 2;
                        index++;
                    }
                }
                else
                {
                    int index = sizeof(m_value) - len / 2 - 1;
                    m_value[index++] = Hex2Char('0', str[0]);
                    int i = 1;
                    while (i < len)
                    {
                        m_value[index] = Hex2Char(str[i], str[i + 1]);
                        i += 2;
                        index++;
                    } 
                }
            }
        }
    }

    std::string ToString(bool no_padding = true) const
    {
        std::string strHex;
        int i = 0;
        int len = sizeof(m_value);
        static const char HEX_CHAR[] = "0123456789abcdef";

        if (no_padding)
        {
            while (i < len)
            {
                if (0 != static_cast<int>(m_value[i]))
                {
                    if (0 == ((m_value[i] & 0xF0) >> 4))
                    {
                        strHex.append(1, HEX_CHAR[m_value[i] & 0x0F]);
                        i++;
                    }
                    break;
                }
                i++;
            }
        }

        while (i < len)
        {
            strHex.append(1, HEX_CHAR[(m_value[i] & 0xF0) >> 4]);
            strHex.append(1, HEX_CHAR[m_value[i] & 0x0F]);
            i++;
        }

        return (strHex.empty()) ? "0" : strHex;
    }

    friend std::ostream & operator << (std::ostream & os,  const HashVal & v)
    {
        os << v.ToString();
        return os;
    }

    inline bool IsZero() const
    {
        static const unsigned char ZERO[BYTE_SIZE] = {0};
        return (0 == memcmp(m_value, ZERO, sizeof(m_value)));
    }

    inline bool operator == (const HashVal & rhs) const
    {
        return (0 == memcmp(m_value, rhs.m_value, sizeof(m_value)));
    }

    inline bool operator != (const HashVal & rhs) const
    {
        return (0 != memcmp(m_value, rhs.m_value, sizeof(m_value)));
    }

    inline bool operator < (const HashVal & rhs) const
    {
        return (0 > memcmp(m_value, rhs.m_value, sizeof(m_value)));
    }

    inline bool operator > (const HashVal & rhs) const
    {
        return (0 < memcmp(m_value, rhs.m_value, sizeof(m_value)));
    }

    inline HashVal & operator = (const HashVal & rhs)
    {
        memcpy(m_value, rhs.m_value, sizeof(m_value));
        return *this;
    }

    inline unsigned char Hex2Char(const char ch1, const char ch2) const
    {
        return (Hex2Int(ch1) * 16 + Hex2Int(ch2));
    }

    inline unsigned int Hex2Int(const char ch) const
    {
        if (ch >= '0' && ch <= '9')
        {
            return (ch - '0');
        }
        else if (ch >= 'a' && ch <= 'f')
        {
            return (ch - 'a' + 10);
        }
        else if (ch >= 'A' && ch <= 'F')
        {
            return (ch - 'A' + 10);
        }

        return 0;
    }

public:
    unsigned char m_value[BYTE_SIZE];
};

typedef HashVal<64/8> Md5Hash64;
typedef HashVal<96/8> Md5Hash96;
typedef HashVal<128/8> Md5Hash;
typedef HashVal<160/8> Sha1Hash;

#endif
