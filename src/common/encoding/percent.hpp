// Copyright 2011, Tencent Inc.
// All rights reserved.

/// @author XiaoDong Chen (donniechen@tencent.com)
///         CHEN Feng (phongchen@tencent.com)
/// @brief percent encoding process
/// @date Mar 31, 2011

#ifndef COMMON_ENCODING_PERCENT_HPP
#define COMMON_ENCODING_PERCENT_HPP

#include <ctype.h>
#include <string>
#include "common/base/stdint.h"

/// @brief percent encoding, majorly for url
/// @see http://en.wikipedia.org/wiki/Percent-encoding
struct PercentEncoding {
public:
    /// @brief ����׷������� output
    static void EncodeAppend(const std::string &input, std::string* output);

    /// @brief ��������� output
    static void EncodeTo(const std::string &input, std::string* output);

    /// @brief ���������滻ԭ��������
    static void Encode(std::string *str);

    /// @brief ���ر����Ľ��
    static std::string Encode(const std::string &input);

    /// @brief �����׷�ӷ�ʽ����� output ��
    static bool DecodeAppend(const std::string &input, std::string* output);

    /// @brief ���������� output ��
    /// @return �Ƿ�ɹ�
    static bool DecodeTo(const std::string &input, std::string* output);

    /// @brief ���������滻ԭ��������
    static bool Decode(std::string* str);
private:
    static uint8_t ToHex(const uint8_t x);
    static uint8_t FromHex(const uint8_t x);
    static bool IsValidEncodingChar(const uint8_t x);
};

#endif // COMMON_ENCODING_PERCENT_HPP
