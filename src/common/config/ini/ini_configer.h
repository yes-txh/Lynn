/// @author swordshao, modified by ivanhuang
/// @date 2006.9.17
/// @version 1.0
/// @brief 配置信息读写模块

#ifndef COMMON_CONFFIG_INI_CONFIGER_INTERFACE_H_ // NOLINT
#define COMMON_CONFFIG_INI_CONFIGER_INTERFACE_H_

#include <stdio.h>
#include <string>
#include "common/base/inttypes.h"

/// @brief 各类服务器的公用配置信息读写虚拟基类
class IniConfiger
{
protected:
    IniConfiger() {}
public:
    virtual ~IniConfiger() {}
public:
    /// @brief 获取配置文件中指定分类下的指定字段的整数类型的值
    /// @param field 分类名
    /// @param key 字段名
    /// @param value 配置项值
    /// @return 是否执行成功 true or false
    bool GetValue(const std::string& field, const std::string& key, std::string* value) const;
    bool GetValue(const std::string& field, const std::string& key, signed char* value) const;
    bool GetValue(const std::string& field, const std::string& key, unsigned char* value) const;
    bool GetValue(const std::string& field, const std::string& key, short* value) const;
    bool GetValue(const std::string& field, const std::string& key, unsigned short* value) const;
    bool GetValue(const std::string& field, const std::string& key, int* value) const;
    bool GetValue(const std::string& field, const std::string& key, unsigned int* value) const;
    bool GetValue(const std::string& field, const std::string& key, long* value) const;
    bool GetValue(const std::string& field, const std::string& key, unsigned long* value) const;
    bool GetValue(const std::string& field, const std::string& key, long long* value) const;
    bool GetValue(const std::string& field, const std::string& key, unsigned long long* value) const;

    /// @brief 获取配置文件中指定分类下的指定字段的浮点类型（32/64位）的值
    /// @param field 分类名
    /// @param key 字段名
    /// @param value 配置项值
    /// @return 是否执行成功 true or false
    bool GetValue(const std::string& field, const std::string& key, float* value) const;
    bool GetValue(const std::string& field, const std::string& key, double* value) const;

    /// @brief 设置配置文件中指定分类下的指定字段的string类型的值
    /// @param field 分类名
    /// @param key 字段名
    /// @param value 配置项值
    /// 2return 是否执行成功 true or false
    bool SetValue(const std::string& field, const std::string& key, const std::string& value);

    /// @brief 设置配置文件中指定分类下的指定字段的整数类型的值
    /// @param field 分类名
    /// @param key 字段名
    /// @param value 配置项值
    /// @return 是否执行成功 true or false
    bool SetValue(const std::string& field, const std::string& key, int value);
    bool SetValue(const std::string& field, const std::string& key, unsigned int value);
    bool SetValue(const std::string& field, const std::string& key, long value);
    bool SetValue(const std::string& field, const std::string& key, unsigned long value);
    bool SetValue(const std::string& field, const std::string& key, long long value);
    bool SetValue(const std::string& field, const std::string& key, unsigned long long value);

    /// @brief 设置配置文件中指定分类下的指定字段的浮点类型（32/64位）的值
    /// @param field 分类名
    /// @param key 字段名
    /// @param value 配置项值
    /// @return 是否执行成功 true or false
    bool SetValue(const std::string& field, const std::string& key, double value);
private:
    /// @brief 获取配置文件中指定分类下的指定字段的string类型的值
    /// @param field 分类名
    /// @param key 字段名
    /// @param value 配置项值
    /// @return 是否执行成功 true or false
    virtual bool GetStringValue(const std::string& field, const std::string& key, std::string* value) const = 0;

    /// @brief 设置配置文件中指定分类下的指定字段的string类型的值
    /// @param field 分类名
    /// @param key 字段名
    /// @param value 配置项值
    /// @return 是否执行成功 true or false
    virtual bool SetStringValue(const std::string& field, const std::string& key, const std::string& value) = 0;
};

#endif // COMMON_CONFFIG_INI_CONFIGER_INTERFACE_H_ // NOLINT
