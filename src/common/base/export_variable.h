// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_EXPORT_VARIABLE_H
#define COMMON_BASE_EXPORT_VARIABLE_H

#include <stddef.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include "common/system/concurrency/mutex.hpp"

class ExportedVariable
{
protected:
    ExportedVariable(){}
public:
    virtual ~ExportedVariable() {}
    virtual std::string ToString() const = 0;
private:
    ExportedVariable(const ExportedVariable&);
    ExportedVariable& operator=(const ExportedVariable&);
};

template <typename T>
class ExportedNormalVariable : public ExportedVariable
{
public:
    explicit ExportedNormalVariable(const T* ptr) : m_ptr(ptr) {}
    virtual std::string ToString() const // override
    {
        std::ostringstream os;
        os << *m_ptr;
        return os.str();
    }
private:
    const T* m_ptr;
};

template <typename T>
class ExportedFunctionVariable : public ExportedVariable
{
public:
    explicit ExportedFunctionVariable(T (*function)()) : m_function(function) {}
    virtual std::string ToString() const // override
    {
        std::ostringstream os;
        os << m_function();
        return os.str();
    }
private:
    T (*m_function)();
};

template <typename T, typename Class>
class ExportedMethodVariable : public ExportedVariable
{
    typedef T (Class::*MethodType)() const;
public:
    explicit ExportedMethodVariable(
        const Class* object,
        T (Class::*method)() const)
    :
        m_object(object), m_method(method)
    {
    }
    virtual std::string ToString() const // override
    {
        std::ostringstream os;
        os << (m_object->*m_method)();
        return os.str();
    }
private:
    const Class* m_object;
    MethodType m_method;
};

template <typename T, typename Class>
class FieldExportedVariable : public ExportedVariable
{
    typedef const T (Class::*FieldPtrType);
public:
    explicit FieldExportedVariable(
        const Class* object,
        T (Class::*field_ptr)() const)
    :
        m_object(object), m_field_ptr(field_ptr)
    {
    }
    virtual std::string ToString() const // override
    {
        std::ostringstream os;
        os << (m_object->*m_field_ptr);
        return os.str();
    }
private:
    const Class* m_object;
    FieldPtrType m_field_ptr;
};

class ExportedVariables
{
    typedef std::map<std::string, ExportedVariable*> MapType;
public: // iteration
    static bool GetFirst(std::string* name, ExportedVariable** var)
    {
        return Instance().First(name, var);
    }
    ExportedVariable* Find(const std::string& name)
    {
        return Instance().FindByName(name);
    }
    static bool Register(const char* name, ExportedVariable* var)
    {
        return Instance().RegisterVariable(name, var);
    }
    static ExportedVariable* Unregister(const char* name)
    {
        return Instance().UnregisterVariable(name);
    }
    static void Dump(std::ostream& os)
    {
        return Instance().DumpToStream(os);
    }
private:
    bool First(std::string* name, ExportedVariable** var)
    {
        MutexLocker locker(&m_mutex);
        if (!m_variables.empty())
        {
            MapType::iterator iter = m_variables.begin();
            *name = iter->first;
            *var = iter->second;
            return true;
        }
        return false;
    }
    ExportedVariable* FindByName(const std::string& name)
    {
        MutexLocker locker(&m_mutex);
        MapType::const_iterator iter = m_variables.find(name);
        if (iter != m_variables.end())
        {
            return iter->second;
        }
        return NULL;
    }

    bool RegisterVariable(const char* name, ExportedVariable* var)
    {
        MutexLocker locker(&m_mutex);
        MapType::iterator iter = m_variables.find(name);
        if (iter != m_variables.end())
        {
            return false;
        }
        m_variables[name] = var;
        return true;
    }
    ExportedVariable* UnregisterVariable(const char* name)
    {
        MutexLocker locker(&m_mutex);
        MapType::iterator iter = m_variables.find(name);
        ExportedVariable* result = NULL;
        if (iter != m_variables.end())
        {
            result = iter->second;
            m_variables.erase(iter);
        }
        return result;
    }
    void DumpToStream(std::ostream& os)
    {
        MutexLocker locker(&m_mutex);
        for (MapType::iterator i = m_variables.begin();
             i != m_variables.end(); ++i)
        {
            os << i->first << " = " << i->second->ToString() << '\n';
        }
    }
private:
    static ExportedVariables& Instance()
    {
        static ExportedVariables instance;
        return instance;
    }
private:
    SimpleMutex m_mutex;
    std::map<std::string, ExportedVariable*> m_variables;
};

/// register/unregister variable automically
class VariableRegister
{
public:
    template <typename T>
    explicit VariableRegister(const char* name, const T* address)
        : m_name(name)
    {
        ExportedVariable* variable = new ExportedNormalVariable<T>(address);
        ExportedVariables::Register(name, variable);
    }

    template <typename T>
    explicit VariableRegister(const char* name, T (*function)())
        : m_name(name)
    {
        ExportedVariable* variable = new ExportedFunctionVariable<T>(function);
        ExportedVariables::Register(name, variable);
    }

    template <typename T, typename Class>
    explicit VariableRegister(
        const char* name,
        const Class* object,
        T (Class::*method)() const
    )
        : m_name(name)
    {
        ExportedVariable* variable = new ExportedMethodVariable<T, Class>(object, method);
        ExportedVariables::Register(name, variable);
    }
    ~VariableRegister()
    {
        ExportedVariable* var = ExportedVariables::Unregister(m_name);
        delete var;
    }
private:
    VariableRegister(const VariableRegister&);
    VariableRegister& operator=(const VariableRegister&);
private:
    const char* m_name;
};

#define EXPORT_VARIABLE(name, ...) \
namespace registered_variables { VariableRegister g_##variable_##name##_register(#name, __VA_ARGS__); }

#endif // COMMON_BASE_EXPORT_VARIABLE_H
