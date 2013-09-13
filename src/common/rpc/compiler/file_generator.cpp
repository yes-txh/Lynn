#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>

#include "common/rpc/compiler/idlcompiler.h"
#include "common/rpc/compiler/file_generator.h"
#include "common/base/string/string_number.hpp"
#include "common/base/string/string_algorithm.hpp"

using namespace std;

string ReplaceFirst(string* str, const string& src, const string& obj)
{
    string::size_type pos = str->find(src);
    if (pos != string::npos)
    {
        *str = str->replace(pos, src.size(), obj);
    }
    return *str;
}

string ReplaceLast(string* str, const string& src, const string& obj)
{
    string::size_type pos = str->rfind(src);
    if (pos != string::npos)
    {
       *str = str->replace(pos, src.size(), obj);
    }
    return *str;
}

string GetParmType(const IdlFunArgument& idl_argument, bool is_proxy)
{
    string param;
    if (idl_argument.m_IsConst || idl_argument.m_IsCallBack)
    {
        param = "const ";
    }
    param += idl_argument.m_Type;
    if(idl_argument.m_Direction == "callback" && !is_proxy)
    {
        param += "Proxy";
    }
    if (idl_argument.m_IsReference)
    {
        param += "&";
    }
    if (idl_argument.m_IsPointer)
    {
        param += "*";
    }

    if (idl_argument.m_Direction == "in")
    {
        param = "Rpc::In<" + param + " > ";
    }
    else if (idl_argument.m_Direction == "out")
    {
        param = "Rpc::Out<" + param + " > ";
    }
    else if (idl_argument.m_Direction == "in_out")
    {
        param = "Rpc::InOut<" + param + " > ";
    }
    return param;
}

ostream& GetStubFunString(ostream* out, const IdlFunction& function)
{
    size_t arguments_num = function.m_ArgVector.size();
    string num = IntegerToString(arguments_num);
    ostream& fout = *out;
    string func_string;
    if (function.m_IsAmd)
    {
        fout << "    Rpc::Status_t Stub" + function.m_FuncName + "(\n";
        fout << "        const Rpc::InvokeInfo& invoke_info,\n";
        fout << "        const Rpc::Buffer& buffer\n";
        fout << "    )\n";
        fout << "    {\n";
        fout << "        Rpc::InvokeContext context(invoke_info);\n";
        if (arguments_num > 0)
        {
            fout << "        context.SetParameterHandlers<";
            for (size_t i = 0; i < arguments_num; i++)
            {
                if (arguments_num > 1) fout << "\n            ";
                fout << GetParmType(function.m_ArgVector[i], false);
                if (arguments_num > 1 && i != arguments_num - 1) fout << ",";
            }
            if (arguments_num > 1) fout << "\n        ";
            fout << ">();\n";
        }
        fout << "        context.SetReturnHandler(static_cast<" + function.m_ReturnType + "*>(NULL));\n";
        fout << "        return (static_cast<ImplType*>(this))->" + function.m_FuncName + "(context, buffer);\n";
        fout << "    }";
    }
    else
    {
        fout << "    Rpc::Status_t Stub" + function.m_FuncName + "(\n";
        fout << "        const Rpc::InvokeInfo& invoke_info,\n";
        fout << "        const Rpc::Buffer& buffer\n";
        fout << "    )\n";
        fout << "    {\n";
        fout << "        return this->InvokeStub" + num + "<";
        if (arguments_num > 0)
        {
            fout << "\n            ";
        }
        fout << function.m_ReturnType + ", ImplType";
        for (size_t i = 0; i < arguments_num; i++)
        {
            fout << ",\n            " + GetParmType(function.m_ArgVector[i], false);
            if (i == arguments_num - 1) fout << "\n        ";
        }
        fout << ">(\n";
        fout << "            invoke_info,\n";
        fout << "            buffer,\n";
        fout << "            &ImplType::" + function.m_FuncName + ",\n";
        fout << "            typename Rpc::ReturnTypeTag<" + function.m_ReturnType + ">::Type()\n";
        fout << "        );\n";
        fout << "    }";
    }
    return fout;
}

ostream& GetProxyFunString(ostream* out, const IdlFunction& function)
{
    size_t arguments_num = function.m_ArgVector.size();
    string num = IntegerToString(arguments_num);
    ostream& fout = *out;
    fout << "    Rpc::Status_t Async" + function.m_FuncName + "(\n";
    for (size_t i = 0; i < arguments_num; i++)
    {
        fout << "        " + GetParmType(function.m_ArgVector[i], true) + " a" + IntegerToString(i + 1) + ",\n";
    }
    fout << "        Rpc::AsyncTokenOf<" + function.m_ReturnType + ">* token = NULL,\n";
    fout << "        Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,\n";
    fout << "        int timeout = " + IntegerToString(function.m_Timeout) + "\n";
    fout << "    )\n";
    fout << "    {\n";
    fout << "        static int method_id = -1;\n";
    fout << "        return ProxyAsyncInvoke" + num;
    if (arguments_num > 0)
    {
        if (arguments_num == 1)
        {
            fout << "<" << GetParmType(function.m_ArgVector[0], true) << ">";
        }
        else
        {
            fout << "<\n";
            for (size_t i = 0; i < arguments_num; i++)
            {
               fout << "            " << GetParmType(function.m_ArgVector[i], true);
               if (i != arguments_num - 1)
                   fout << ",";
               fout << "\n";
            }
            fout << "        >";
        }
    }
    fout << "(";
    if (arguments_num > 1)
    {
        fout << "\n            ";
    }
    fout << "\"" + function.m_FuncName + "\" \"_" + num + "\", method_id, ";
    if (arguments_num > 0)
    {
        if (arguments_num > 1)
        {
            fout << "\n            ";
        }
        for (size_t i = 0; i < arguments_num; i++)
        {
            fout << "a" + IntegerToString(i + 1) + ", ";
        }
        if (arguments_num > 1)
        {
            fout << "\n            ";
        }
    }
    fout << "token, callback, context, param, ";
    if (arguments_num > 1)
    {
        fout << "\n            ";
    }
    fout << "timeout";
    if (arguments_num > 1)
    {
        fout << "\n        ";
    }
    fout << ");\n";
    fout << "    }\n\n";


    fout << "    " + function.m_ReturnType + " " + function.m_FuncName + "(";
    if (arguments_num > 0)
    {
        for (size_t i = 0; i < arguments_num; i++)
        {
            if (arguments_num > 1)
            {
                fout << "\n        ";
            }
            fout << GetParmType(function.m_ArgVector[i], true);
            fout << " a" + IntegerToString(i + 1) + ", ";
        }
    }
    if (arguments_num > 1)
    {
        fout << "\n        ";
    }
    fout << "Rpc::Status_t* status = NULL, ";
    if (arguments_num > 1)
    {
        fout << "\n        ";
    }
    fout << "int timeout = " + IntegerToString(function.m_Timeout);
    if (arguments_num > 1)
    {
        fout << "\n    ";
    }
    fout << ")\n";
    fout << "    {\n";
    fout << "        Rpc::AsyncTokenOf<" + function.m_ReturnType + "> async_token;\n";
    fout << "        return SyncInvokeReturn(\n";
    fout << "            GetClassName(), \"" + function.m_FuncName + "\",\n";
    fout << "            Async" + function.m_FuncName + "(";
    if (arguments_num > 0)
    {
        for (size_t i = 0; i < arguments_num; i++)
        {
            fout << "a" + IntegerToString(i + 1) + ", ";
        }
    }
    fout << "&async_token, NULL, NULL, NULL, -1),\n";
    fout << "            async_token, timeout, status\n";
    fout << "        );\n";
    fout << "    }";
    return fout;
}

string GetDispatch(const IdlFunction& function)
{
    string argNum = IntegerToString(function.m_ArgVector.size());
    string dispatch = "        { \""
                    + function.m_FuncName +  "\" \"_" + argNum
                    + "\", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::Stub"
                    + function.m_FuncName
                    + ")";
    if (function.m_IsNestable)
    {
        dispatch += ", true";
    }
    dispatch += " },";
    return dispatch;
}

bool GenerateStub(const IdlModule& module, const string& outdir, const string& filename)
{
    string stub_filename = filename + "_stub.h";
    if (outdir != "")
    {
        stub_filename = outdir + "/" + stub_filename;
    }

    ofstream fout(stub_filename.c_str());
    if (!fout)
    {
        cerr << "can't open file to write:" << stub_filename << endl;
        return false;
    }

    fout << endl;
    fout << "// Auto generated code. DO NOT edit this file, edit corresponding IDL file instead." << endl;
    fout << endl;

    string uppercase_filename = ReplaceAllChars(UpperString(filename), "+-./", '_');
    fout << "#ifndef " + uppercase_filename + "_STUB_H" << endl;
    fout << "#define " + uppercase_filename + "_STUB_H" << endl;
    fout << endl;

    if (! module.m_CppAnnotation.empty())
    {
        fout << module.m_CppAnnotation << endl;
    }
    if (! module.m_StubAnnotation.empty())
    {
        fout << module.m_StubAnnotation << endl;
    }

    if (module.m_Exist)
    {
        int n = (int)module.m_ModuleAnnotations.size();
        for (int i = (int)module.m_ModuleAnnotations.size() - 1; i >= 0; i--)
        {
            if (! module.m_ModuleAnnotations[n - i - 1].empty())
            {
                vector<string> lines;
                SplitString(module.m_ModuleAnnotations[n - i -1], "\n", &lines);
                for (size_t m = 0; m < lines.size(); m++)
                {
                    if (!lines[m].empty())
                    {
                        fout << lines[m] << endl;
                    }
                }
            }
            fout << "namespace " << module.m_ModuleNames[i] << endl;
            fout << "{" << endl;
            fout << endl;
        }
    }

    size_t interfaceNum = module.m_InterfaceVector.size();
    for (size_t i = 0; i < interfaceNum; i++)
    {
        const IdlInterface& interface = module.m_InterfaceVector[i];
        fout << interface.m_InterfaceAnnotation;

        fout << "template <typename T>\n";
        fout << "class " + interface.m_InterfaceName + "Stub: public Rpc::StubImpl\n";
        fout << "{\n";
        fout << "    typedef T ImplType;\n";
        fout << "    typedef " + interface.m_InterfaceName + "Stub ThisType;\n";
        fout << "    typedef Rpc::StubImpl BaseType;\n";
        fout << "public:\n";
        fout << "    " + interface.m_InterfaceName + "Stub() {}\n";
        fout << "    " + interface.m_InterfaceName + "Stub(Rpc::ObjectId_t id) : Rpc::StubImpl(id) {}\n";
        fout << "    virtual const typename Rpc::StubImpl::DispatchTable& GetDispatchTable() const;\n";
        fout << "\n";

        string dispatch;
        size_t funcNum = interface.m_FuncVector.size();
        for (size_t k = 0; k < funcNum; k++)
        {
            const IdlFunction& function = interface.m_FuncVector[k];
            vector<string> lines;
            bool is_block_comment = false;
            int  block_comment_line = 0;
            SplitString(function.m_FuncAnnotation, "\n", &lines);
            for (size_t m = 0; m < lines.size(); m++)
            {
                if (lines[m] != "")
                {
                    if (!is_block_comment && HasPrefixString(lines[m], "/*"))
                        is_block_comment = true;
                    if (is_block_comment)
                    {
                        if (block_comment_line > 0)
                            fout << lines[m];
                        else   // first line, need a indention
                            fout << "    " << lines[m];
                        block_comment_line++;
                    }
                    else
                    {
                        fout << "    " << lines[m];
                    }
                    if (is_block_comment && HasSuffixString(lines[m], "*/"))
                    {
                        is_block_comment = false;
                        block_comment_line = 0;
                    }
                }
                fout << endl;
            }
            GetStubFunString(&fout, function) << "\n";
            if (k < funcNum - 1)
            {
                fout << endl;
            }
            dispatch = dispatch + GetDispatch(function) + "\n";
        }

        fout << "};\n";
        fout << "\n";

        fout << "template <typename T>\n";
        fout << "const typename Rpc::StubImpl::DispatchTable& " + interface.m_InterfaceName +"Stub<T>::GetDispatchTable() const\n";
        fout << "{\n";
        fout << "    static const typename Rpc::StubImpl::DispatchEntry entries[] =\n";
        fout << "    {\n";
        fout <<          dispatch ;
        fout << "        { NULL, NULL }\n";
        fout << "    };\n";
        fout << "\n";
        fout << "    static const typename Rpc::StubImpl::DispatchTable table(entries, sizeof(entries)/sizeof(entries[0]) - 1);\n";
        fout << "    return table;\n";
        fout << "};\n";
        fout << "\n";
    }

    if (module.m_Exist)
    {
        for (size_t i = 0; i < module.m_ModuleAnnotations.size(); i++)
        {
            fout << endl;
            fout << "} // end of namespace " << module.m_ModuleNames[i] << endl;
        }
    }

    fout << endl;
    fout << "#endif // end of define "  << uppercase_filename << "_STUB_H" << endl;
    fout << endl;
    return true;
}

bool GenerateProxy(const IdlModule& module, const string& outdir, const string& filename)
{
    string proxy_filename = filename + "_proxy.h";
    if ( outdir != "" )
    {
        proxy_filename = outdir + "/" + proxy_filename;
    }

    ofstream fout(proxy_filename.c_str());
    if ( !fout )
    {
        cerr << "can't open file to write: " << proxy_filename << endl;
        return false;
    }

    fout << endl;
    fout << "// Auto generated code. DO NOT edit this file, edit corresponding IDL file instead." << endl;
    fout << endl;

    string uppercase_filename = ReplaceAllChars(UpperString(filename), "+-./", '_');
    fout << "#ifndef " + uppercase_filename + "_PROXY_H" << endl;
    fout << "#define " + uppercase_filename + "_PROXY_H" << endl;
    fout << endl;

    if (module.m_CppAnnotation != "")
    {
        fout << module.m_CppAnnotation << endl;
    }
    if (module.m_ProxyAnnotation != "")
    {
        fout << module.m_ProxyAnnotation << endl;
    }

    if (module.m_Exist)
    {
        int n = (int)module.m_ModuleAnnotations.size();
        for (int i = (int)module.m_ModuleAnnotations.size() - 1; i >= 0; i--)
        {
            if (! module.m_ModuleAnnotations[n - i - 1].empty())
            {
                vector<string> lines;
                SplitString(module.m_ModuleAnnotations[n - i -1], "\n", &lines);
                for (size_t m = 0; m < lines.size(); m++)
                {
                    if (!lines[m].empty())
                    {
                        fout << lines[m] << endl;
                    }
                }
            }
            fout << "namespace " << module.m_ModuleNames[i] << endl;
            fout << "{" << endl;
            fout << endl;
        }
    }

    size_t interfaceNum = module.m_InterfaceVector.size();
    for (size_t i = 0; i < interfaceNum; i++)
    {
        const IdlInterface& interface = module.m_InterfaceVector[i];
        fout << interface.m_InterfaceAnnotation;

        fout << "class " + interface.m_InterfaceName + "Proxy : public Rpc::ProxyImpl\n";
        fout << "{\n";
        fout << "public:\n";
        fout << "    virtual const char* GetClassName() const { return \"" + interface.m_InterfaceName + "Proxy\"; }\n";
        fout << "\n";

        size_t func_num = interface.m_FuncVector.size();
        for (size_t k = 0; k < func_num; k++)
        {
            const IdlFunction& function = interface.m_FuncVector[k];
            vector<string> lines;
            bool is_block_comment = false;
            int  block_comment_line = 0;
            SplitString(function.m_FuncAnnotation, "\n", &lines);
            for (size_t m = 0; m < lines.size(); m++)
            {
                if (lines[m] != "")
                {
                    if (!is_block_comment && HasPrefixString(lines[m], "/*"))
                        is_block_comment = true;
                    if (is_block_comment)
                    {
                        if (block_comment_line > 0)
                            fout << lines[m];
                        else   // first line, need a indention
                            fout << "    " << lines[m];
                        block_comment_line++;
                    }
                    else
                    {
                        fout << "    " << lines[m];
                    }
                    if (is_block_comment && HasSuffixString(lines[m], "*/"))
                    {
                        is_block_comment = false;
                        block_comment_line = 0;
                    }
                }
                fout << endl;
            }

            GetProxyFunString(&fout, function) << "\n";
            if (k < func_num - 1)
            {
                fout << endl;
            }
        }

        fout << "};\n";
        fout << endl;
    }

    if (module.m_Exist)
    {
        for (size_t i = 0; i < module.m_ModuleAnnotations.size(); i++)
        {
            fout << endl;
            fout << "} // end of namespace " << module.m_ModuleNames[i] << endl;
        }
    }

    fout << endl;
    fout << "#endif // end of define "  << uppercase_filename << "_PROXY_H" << endl;
    fout << endl;
    return true;
}

