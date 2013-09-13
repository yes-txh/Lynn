#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "common/rpc/compiler/idlcompiler.h"

std::ostream& operator << (std::ostream &out, const IdlModule& module)
{
    if (! module.m_CppAnnotation.empty())
    {
        out << module.m_CppAnnotation;
    }
    if (! module.m_StubAnnotation.empty())
    {
        out << module.m_StubAnnotation;
    }
    if (! module.m_ProxyAnnotation.empty())
    {
        out << module.m_ProxyAnnotation;
    }
    if (module.m_Exist)
    {
        int n = (int)module.m_ModuleAnnotations.size();
        for (int i = (int)module.m_ModuleAnnotations.size() - 1; i >= 0; i--)
        {
            out << module.m_ModuleAnnotations[n - i - 1] << std::endl;
            out << "module " << module.m_ModuleNames[i] << " {" << std::endl;
        }
    }
    size_t n = module.m_InterfaceVector.size();
    for (size_t i = 0; i < n; i++)
    {
        out << module.m_InterfaceVector[i];
    }
    if (module.m_Exist)
    {
        for (int i = (int)module.m_ModuleAnnotations.size() - 1; i >= 0; i--)
        {
            out << std::endl;
            out << "}" ;
        }
    }
    out << std::endl;
    return out;
}

std::ostream& operator << (std::ostream &out, const IdlInterface& interface)
{
    if (! interface.m_InterfaceAnnotation.empty())
    {
        out << interface.m_InterfaceAnnotation;
    }
    out << "interface " << interface.m_InterfaceName << " {" << std::endl;
    size_t n = interface.m_FuncVector.size();
    for (size_t i = 0; i < n; i++)
    {
        out << interface.m_FuncVector[i];
    }
    out << "}" << std::endl;
    return out;
}

std::ostream& operator << (std::ostream &out, const IdlFunction& function)
{
    if (! function.m_FuncAnnotation.empty())
    {
        out << function.m_FuncAnnotation;
    }
    if (function.m_Timeout > 0 || function.m_IsAmd)
    {
        out << "[";
        if (function.m_Timeout > 0) out << "timeout(" << function.m_Timeout << ")";
        if (function.m_IsAmd) out << ", amd";
        out << "] ";
    }
    out << function.m_ReturnType << " ";
    out << function.m_FuncName << "(";
    size_t n = function.m_ArgVector.size();
    for (size_t i = 0; i < n; i++)
    {
        if (i > 0)  out << ", ";
        out << function.m_ArgVector[i];
    }
    out << ")" << std::endl;
    out << std::endl;
    return out;
}

std::ostream& operator << (std::ostream& out, const IdlFunArgument& argument)
{
    if (! argument.m_Direction.empty())
    {
        out << "[" << argument.m_Direction << "] ";
    }
    if (argument.m_IsConst) out << "const ";
    out << argument.m_Type;
    if (argument.m_IsReference) out << "&";
    if (argument.m_IsPointer) out << "*";
    out << " " << argument.m_ArgName;
    return out;
}

