/*
 * =====================================================================================
 *       Filename:  idlcompiler.h
 *
 *    Description:  Definition of idl module
 *
 *        Version:  1.0
 *        Created:  10/20/2010 11:15:39 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  wilkenli
 *       Modified:  hsiaokangliu
 * =====================================================================================
 */
#ifndef IDL_COMPILER_H_
#define IDL_COMPILER_H_

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

struct IdlInterface;
struct IdlFunction;
struct IdlFunArgument;


struct IdlModule
{
    IdlModule () : m_Exist (false) {}
    bool m_Exist;
    std::vector<std::string>  m_ModuleAnnotations;
    std::vector<std::string>  m_ModuleNames;
    std::vector<IdlInterface> m_InterfaceVector;

    std::string m_CppAnnotation;
    std::string m_StubAnnotation;
    std::string m_ProxyAnnotation;

    void Reset ()
    {
        m_Exist = false;
        m_ModuleAnnotations.clear();
        m_ModuleNames.clear();
        m_InterfaceVector.clear();
        m_CppAnnotation.clear();
        m_StubAnnotation.clear();
        m_ProxyAnnotation.clear();
    }
};

struct IdlInterface
{
    std::string m_InterfaceAnnotation;
    std::string m_InterfaceName;
    std::vector<IdlFunction> m_FuncVector;
    void Reset ()
    {
        m_InterfaceAnnotation.clear();
        m_InterfaceName.clear();
        m_FuncVector.clear();
    }
};

struct IdlFunction
{
    IdlFunction (): m_Timeout (-1), m_IsAmd (false), m_IsNestable(false) {}
    std::string m_FuncAnnotation;
    int  m_Timeout;
    bool m_IsAmd;           ///< 异步调用
    bool m_IsNestable;      ///< 可嵌套
    std::string m_FuncName;
    std::string m_ReturnType;
    std::vector<IdlFunArgument> m_ArgVector;
    void Reset ()
    {
        m_Timeout = -1;
        m_IsAmd = false;
        m_IsNestable = false;
        m_FuncAnnotation.clear();
        m_FuncName.clear();
        m_ReturnType.clear();
        m_ArgVector.clear();
    }
};

struct IdlFunArgument
{
    IdlFunArgument () : m_IsCallBack (false),
                        m_IsConst (false),
                        m_IsReference (false),
                        m_IsPointer (false) {}
    std::string m_Direction; // [in], [out], [in, out], [callback], ""
    std::string m_Type;
    bool m_IsCallBack;
    bool m_IsConst;
    bool m_IsReference;
    bool m_IsPointer;
    std::string m_ArgName;
    void Reset ()
    {
        m_Direction.clear();
        m_Type.clear();
        m_IsCallBack = false;
        m_IsConst = false;
        m_IsReference = false;
        m_IsPointer = false;
        m_ArgName.clear();
    }
};

extern std::ostream& operator << (std::ostream &out, const IdlModule& );
extern std::ostream& operator << (std::ostream &out, const IdlInterface& );
extern std::ostream& operator << (std::ostream &out, const IdlFunction& );
extern std::ostream& operator << (std::ostream &out, const IdlFunArgument& );

#endif

