//////////////////////////////////////////////////////////////////////////
// modified by ivanhuang at 20101115
// 1.�޸Ĵ���������ע�͡��޸Ĳ��ִ���
// 2.����UnitTest
//////////////////////////////////////////////////////////////////////////

#ifndef COMMON_CONFFIG_INI_CONFIGER_UTILITY_H_ // NOLINT
#define COMMON_CONFFIG_INI_CONFIGER_UTILITY_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "common/base/stdint.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif // _WIN32

enum {
    kInt8Min    = -(2<<7),
    kInt8Max    = (2<<7) - 1,
    kUint8Max   = (2<<8) - 1,
    
    kInt16Min   = -(2<<15),
    kInt16Max   = (2<<15) - 1,
    kUint16Max  = (2<<16) - 1,

    kInt32Min   = -(2<<31),
    kInt32Max   = (2<<31) - 1,
    kUint32Max  = (2<<32) - 1,

    kInt64Min   = -(2<<63),
    kInt64Max   = (2<<63) - 1,
    kUint64Max  = (2<<64) - 1,
};

// ʹ���Լ��ļ򵥽�������
//#define USE_OWN_PARSER

class ConfigerUtility {
public:
    /********************************************************************
    ��������: ��һ���ַ���ת��Ϊ����ֵ(8/16/32/64/, �з���,�޷���)
    �������: s ��ת�����ַ���
    �������: i ת���������
    ���� :   ��
    *********************************************************************/
    inline static bool atoi(const char* s, int8_t& i) {
#ifndef USE_OWN_PARSER
        int temp;
        if (1 != sscanf(s, "%d", &temp))
            return false;

        i = static_cast<int8_t>(temp);
        return true;
#else
        int64_t temp;
        if (!ConfigerUtility::atoi(s, temp)) {
            return false;
        }

        if (temp >= kInt8Min && temp <= kInt8Max) {
            i = static_cast<int8_t>(temp);
            return true;
        } else {
            return false;
        }
#endif
    }

    inline static bool atoi(const char* s, uint8_t& i) {
#ifndef USE_OWN_PARSER
        unsigned temp;
        if (1 != sscanf(s, "%u", &temp))
            return false;

        i = static_cast<uint8_t>(temp);
        return true;
#else
        uint64_t temp;
        if (!ConfigerUtility::atoi(s, temp)) {
            return false;
        }

        if (temp <= kUint8Max) {
            i = static_cast<uint8_t>(temp);
            return true;
        } else {
            return false;
        }
#endif
    }

    inline static bool atoi(const char* s, int16_t& i) {
#ifndef USE_OWN_PARSER
        i = static_cast<int16_t>(::atoi(s));
        return true;
#else
        int64_t temp;
        if (!ConfigerUtility::atoi(s, temp)) {
            return false;
        }

        if (temp >= kInt16Min && temp <= kInt16Max) {
            i = static_cast<int16_t>(temp);
            return true;
        } else {
            return false;
        }
#endif
    }

    inline static bool atoi(const char* s, uint16_t& i) {
#ifndef USE_OWN_PARSER
        i = static_cast<uint16_t>(::atoi(s));
        return true;
#else
        uint64_t temp;
        if (!ConfigerUtility::atoi(s, temp)) {
            return false;
        }

        if (temp <= kUint16Max) {
            i = static_cast<uint16_t>(temp);
            return true;
        } else {
            return false;
        }
#endif
    }

    inline static bool atoi(const char* s, int32_t& i) {
#ifndef USE_OWN_PARSER
        if (1 != sscanf(s, "%d", &i))
            return false;
        return true;
#else
        int64_t temp;
        if (!ConfigerUtility::atoi(s, temp)) {
            return false;
        }

        if (temp >= kInt32Min && temp <= kInt32Max) {
            i = static_cast<int32_t>(temp);
            return true;
        } else {
            return false;
        }
#endif
    }

    inline static bool atoi(const char* s, uint32_t& i) {
#ifndef USE_OWN_PARSER
        if (1 != sscanf(s, "%u", &i))
            return false;
        return true;
#else
        uint64_t temp;
        if (!ConfigerUtility::atoi(s, temp)) {
            return false;
        }

        if (temp <= kUint32Max) {
            i = static_cast<uint32_t>(temp);
            return true;
        } else {
            return false;
        }
#endif
    }

    inline static bool atoi(const char* s, int64_t& i) {
        long long int temp;
#ifdef _WIN32
        if (1 != sscanf(s, "%I64d", &temp))
            return false;
#else
        if (1 != sscanf(s, "%Ld", &temp))
            return false;
#endif
        i = static_cast<int64_t>(temp);
        return true;
    }

    inline static bool atoi(const char* s, uint64_t& i) {
        long long unsigned temp;
#ifdef _WIN32
        if (1 != sscanf(s, "%I64u", &temp))
            return false;
#else
        if (1 != sscanf(s, "%Lu", &temp))
            return false;
#endif
        i = static_cast<uint64_t>(temp);
        return true;
    }

    /********************************************************************
    ��������: ��һ���ַ���ת��Ϊ������
    �������: s ��ת�����ַ���
    �������: f ת����ĸ�����
    ���� :   ��
    *********************************************************************/
    inline static bool atof(const char* s, float& f) {
        f = static_cast<float>(::atof(s));
        return true;
    }

    inline static bool atof(const char* s, double& f) {
        f = ::atof(s);
        return true;
    }

    /********************************************************************
    ��������: ��һ��int32_tֵת����Ϊ�ַ��������Ϊstd::string��ʽ
    �������: i ��ת��������
    �������: s ת������ַ���
    ���� :   ��
    *********************************************************************/
    inline static std::string IntToString(int32_t value) {
        char buff[80];
        snprintf(buff, sizeof(buff), "%d", value);
        return std::string(buff);
    }

    inline static std::string IntToString(uint32_t value) {
        char buff[80];
        snprintf(buff, sizeof(buff), "%u", value);
        return std::string(buff);
    }

    inline static std::string IntToString(int64_t value) {
        char buff[80];
        long long int temp = static_cast<long long int>(value);
#ifdef _WIN32
        snprintf(buff, sizeof(buff), "%I64d", temp);
#else
        snprintf(buff, sizeof(buff), "%lld", temp);
#endif
        return std::string(buff);
    }

    inline static std::string IntToString(uint64_t value) {
        char buff[80];
        long long unsigned temp = static_cast<long long unsigned>(value);
#ifdef  _WIN32
        snprintf(buff, sizeof(buff), "%I64u", temp);
#else
        snprintf(buff, sizeof(buff), "%llu", temp);
#endif
        return std::string(buff);
    }

    /********************************************************************
    ��������: ��һ������ֵת����Ϊ�ַ��������Ϊstd::string��ʽ
    �������: i ��ת���ĸ�����
    �������: s ת������ַ���
    ���� :   ��
    *********************************************************************/
    inline static std::string FloatToString(float value) {
        char buff[80];
        snprintf(buff, sizeof(buff), "%f", value);
        return std::string(buff);
    }

    inline static std::string FloatToString(double value) {
        char buff[80];
        snprintf(buff, sizeof(buff), "%f", value);
        return std::string(buff);
    }

private:
    ConfigerUtility();
    ~ConfigerUtility();

    ConfigerUtility(const ConfigerUtility &rhs);
    ConfigerUtility & operator = (const ConfigerUtility &rhs);
};

#endif // COMMON_CONFFIG_INI_CONFIGER_UTILITY_H_ // NOLINT
