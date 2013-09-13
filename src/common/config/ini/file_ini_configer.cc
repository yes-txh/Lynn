#include "common/config/ini/file_ini_configer.h"

bool FileIniConfiger::Load(const std::string& configure_file)
{
    m_ini_file.SetPath(configure_file);
    return m_ini_file.ReadFile();
}

bool FileIniConfiger::Save(const std::string& configure_file)
{
    m_ini_file.SetPath(configure_file);
    return m_ini_file.WriteFile(true);
}

bool FileIniConfiger::GetStringValue(
    const std::string& field,
    const std::string& key,
    std::string* value) const
{
    // not easy to check existance, using tricky way to bypass
    std::string default_value = "<kNotFound>";
    *value = m_ini_file.GetValue(field, key, default_value);
    return *value != default_value;
}

bool FileIniConfiger::SetStringValue(
    const std::string& field,
    const std::string& key,
    const std::string& value)
{
    return m_ini_file.SetValue(field, key, value);
}

