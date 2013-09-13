#include "common/config/ini/ini_configer.h"
#include "common/base/string/string_algorithm.hpp"
#include "common/base/string/string_number.hpp"

bool IniConfiger::GetValue(
    const std::string& field,
    const std::string& key,
    std::string* value) const
{
    return GetStringValue(field, key, value);
}

template <typename T>
static bool GetNumberValue(
    const IniConfiger* configer,
    const std::string& field,
    const std::string& key,
    T* value
    )
{
    std::string string_value;
    if (configer->GetValue(field, key, &string_value))
    {
        StringTrim(&string_value);
        return StringToNumber(string_value, value);
    }
    return false;
}

bool IniConfiger::GetValue(
    const std::string& field,
    const std::string& key,
    signed char* value) const
{
    return GetNumberValue(this, field, key, value);
}

bool IniConfiger::GetValue(
    const std::string& field,
    const std::string& key,
    unsigned char* value) const
{
    return GetNumberValue(this, field, key, value);
}

bool IniConfiger::GetValue(
    const std::string& field,
    const std::string& key,
    short* value) const
{
    return GetNumberValue(this, field, key, value);
}

bool IniConfiger::GetValue(
    const std::string& field,
    const std::string& key,
    unsigned short* value) const
{
    return GetNumberValue(this, field, key, value);
}

bool IniConfiger::GetValue(
    const std::string& field,
    const std::string& key,
    int* value) const
{
    return GetNumberValue(this, field, key, value);
}

bool IniConfiger::GetValue(
    const std::string& field,
    const std::string& key,
    unsigned int* value) const
{
    return GetNumberValue(this, field, key, value);
}

bool IniConfiger::GetValue(
    const std::string& field,
    const std::string& key,
    long* value) const
{
    return GetNumberValue(this, field, key, value);
}

bool IniConfiger::GetValue(
    const std::string& field,
    const std::string& key,
    unsigned long* value) const
{
    return GetNumberValue(this, field, key, value);
}

bool IniConfiger::GetValue(
    const std::string& field,
    const std::string& key,
    long long* value) const
{
    return GetNumberValue(this, field, key, value);
}

bool IniConfiger::GetValue(
    const std::string& field,
    const std::string& key,
    unsigned long long* value) const
{
    return GetNumberValue(this, field, key, value);
}

bool IniConfiger::GetValue(
    const std::string& field,
    const std::string& key,
    float* value) const
{
    return GetNumberValue(this, field, key, value);
}

bool IniConfiger::GetValue(
    const std::string& field,
    const std::string& key,
    double* value) const
{
    return GetNumberValue(this, field, key, value);
}

bool IniConfiger::SetValue(
    const std::string& field,
    const std::string& key,
    const std::string& value)
{
    return SetStringValue(field, key, value);
}

template <typename T>
static bool SetNumberValue(
    IniConfiger* configer,
    const std::string& field,
    const std::string& key,
    T value
    )
{
    std::string string_value = NumberToString(value);
    return configer->SetValue(field, key, string_value);
}

bool IniConfiger::SetValue(
    const std::string& field,
    const std::string& key,
    int value)
{
    return SetNumberValue(this, field, key, value);
}

bool IniConfiger::SetValue(
    const std::string& field,
    const std::string& key,
    unsigned int value)
{
    return SetNumberValue(this, field, key, value);
}

bool IniConfiger::SetValue(
    const std::string& field,
    const std::string& key,
    long value)
{
    return SetNumberValue(this, field, key, value);
}

bool IniConfiger::SetValue(
    const std::string& field,
    const std::string& key,
    unsigned long value)
{
    return SetNumberValue(this, field, key, value);
}

bool IniConfiger::SetValue(
    const std::string& field,
    const std::string& key,
    long long value)
{
    return SetNumberValue(this, field, key, value);
}

bool IniConfiger::SetValue(
    const std::string& field,
    const std::string& key,
    unsigned long long value)
{
    return SetNumberValue(this, field, key, value);
}

bool IniConfiger::SetValue(
    const std::string& field,
    const std::string& key,
    double value)
{
    return SetNumberValue(this, field, key, value);
}
