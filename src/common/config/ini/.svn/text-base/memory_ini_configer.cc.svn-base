#include "common/config/ini/memory_ini_configer.h"

bool MemoryIniConfiger::GetStringValue(
    const std::string& field,
    const std::string& key,
    std::string* value
    ) const
{
    FieldKeyValueMap::const_iterator it =
        m_key_value_map.find(std::make_pair(field, key));
    if (it == m_key_value_map.end()) {
        return false;
    } else {
        *value = it->second;
        return true;
    }
}

bool MemoryIniConfiger::SetStringValue(
    const std::string& field,
    const std::string& key,
    const std::string& value)
{
    m_key_value_map[std::make_pair(field, key)] = value;
    return true;
}

