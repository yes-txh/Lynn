/// @author swordshao, modified by ivanhuang
/// @date 2006.9.17
/// @version 1.0
/// @brief ������Ϣ��дģ��

#ifndef COMMON_CONFFIG_INI_CONFIGER_INTERFACE_H_ // NOLINT
#define COMMON_CONFFIG_INI_CONFIGER_INTERFACE_H_

#include <stdio.h>
#include <string>
#include "common/base/inttypes.h"

/// @brief ����������Ĺ���������Ϣ��д�������
class IniConfiger
{
protected:
    IniConfiger() {}
public:
    virtual ~IniConfiger() {}
public:
    /// @brief ��ȡ�����ļ���ָ�������µ�ָ���ֶε��������͵�ֵ
    /// @param field ������
    /// @param key �ֶ���
    /// @param value ������ֵ
    /// @return �Ƿ�ִ�гɹ� true or false
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

    /// @brief ��ȡ�����ļ���ָ�������µ�ָ���ֶεĸ������ͣ�32/64λ����ֵ
    /// @param field ������
    /// @param key �ֶ���
    /// @param value ������ֵ
    /// @return �Ƿ�ִ�гɹ� true or false
    bool GetValue(const std::string& field, const std::string& key, float* value) const;
    bool GetValue(const std::string& field, const std::string& key, double* value) const;

    /// @brief ���������ļ���ָ�������µ�ָ���ֶε�string���͵�ֵ
    /// @param field ������
    /// @param key �ֶ���
    /// @param value ������ֵ
    /// 2return �Ƿ�ִ�гɹ� true or false
    bool SetValue(const std::string& field, const std::string& key, const std::string& value);

    /// @brief ���������ļ���ָ�������µ�ָ���ֶε��������͵�ֵ
    /// @param field ������
    /// @param key �ֶ���
    /// @param value ������ֵ
    /// @return �Ƿ�ִ�гɹ� true or false
    bool SetValue(const std::string& field, const std::string& key, int value);
    bool SetValue(const std::string& field, const std::string& key, unsigned int value);
    bool SetValue(const std::string& field, const std::string& key, long value);
    bool SetValue(const std::string& field, const std::string& key, unsigned long value);
    bool SetValue(const std::string& field, const std::string& key, long long value);
    bool SetValue(const std::string& field, const std::string& key, unsigned long long value);

    /// @brief ���������ļ���ָ�������µ�ָ���ֶεĸ������ͣ�32/64λ����ֵ
    /// @param field ������
    /// @param key �ֶ���
    /// @param value ������ֵ
    /// @return �Ƿ�ִ�гɹ� true or false
    bool SetValue(const std::string& field, const std::string& key, double value);
private:
    /// @brief ��ȡ�����ļ���ָ�������µ�ָ���ֶε�string���͵�ֵ
    /// @param field ������
    /// @param key �ֶ���
    /// @param value ������ֵ
    /// @return �Ƿ�ִ�гɹ� true or false
    virtual bool GetStringValue(const std::string& field, const std::string& key, std::string* value) const = 0;

    /// @brief ���������ļ���ָ�������µ�ָ���ֶε�string���͵�ֵ
    /// @param field ������
    /// @param key �ֶ���
    /// @param value ������ֵ
    /// @return �Ƿ�ִ�гɹ� true or false
    virtual bool SetStringValue(const std::string& field, const std::string& key, const std::string& value) = 0;
};

#endif // COMMON_CONFFIG_INI_CONFIGER_INTERFACE_H_ // NOLINT
