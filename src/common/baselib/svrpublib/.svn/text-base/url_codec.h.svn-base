// Copyright 2011, Tencent Inc.
// Author: XiaoDong Chen (donniechen@tencent.com)
// url code and decode

#ifndef COMMON_BASELIB_SVRPUBLIB_URL_CODEC_H_
#define COMMON_BASELIB_SVRPUBLIB_URL_CODEC_H_

#include <string>
#include <ctype.h>

class UrlCodec {
public:
    static std::string Encode(const std::string &input) {
        std::string output;
        for (size_t ix = 0; ix < input.size(); ix++) {      
            if (isalnum((uint8_t)input[ix])
                || input[ix] == '-'
                || input[ix] == '_'
                || input[ix] == '\\') {
                output.push_back(input[ix]);
            } else if (input[ix] == ' ') {
                // 貌似把空格编码成%20或者+都可以，这里编成+号
                output.push_back('+');
            } else {
                output.push_back('%');
                output.push_back(ToHex((uint8_t)input[ix] >> 4));
                output.push_back(ToHex((uint8_t)input[ix] % 16));
            }
        }
        return output;
    };

    // TODO(donnie) : 增加DecodeAppend追加模式
    static bool DecodeAppend(const std::string &input, std::string* output) {
        for (size_t ix = 0; ix < input.size(); ix++) {
            uint8_t ch = 0;
            if(input[ix]=='%') {
                if (ix + 2 > input.size()) {
                    // 后面的数据不完整了，返回吧
                    return false;
                }

                if (!IsValid(input[ix+1]) || !IsValid(input[ix+2]))
                    return false;

                ch = (FromHex(input[ix+1])<<4);
                ch |= FromHex(input[ix+2]);
                ix += 2;
            } else if (input[ix] == '+') {
                ch = ' ';
            } else {
                ch = input[ix];
            }
            *output += (char)ch;
        }
        return true;
    }

    static bool Decode(const std::string &input, std::string* output) {
        output->clear();
        return DecodeAppend(input, output);
    }
private:
    static uint8_t ToHex(const uint8_t x) {
        return x > 9 ? x -10 + 'A': x + '0';
    }

    static uint8_t FromHex(const uint8_t x) {
        uint8_t X = toupper(x);
        return isdigit(X) ? X-'0' : X-'A'+10;
    }

    static bool IsValid(const uint8_t x) {
        uint8_t X = toupper(x);
        if (isdigit(X) || (X >= 'A' && X <= 'F'))
            return true;
        return false;
    }

};

#endif  // COMMON_BASELIB_SVRPUBLIB_URL_CODEC_H_
