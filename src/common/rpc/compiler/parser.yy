%{
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "common/rpc/compiler/idlcompiler.h"
#include "common/rpc/compiler/file_generator.h"
#include "common/base/string/string_algorithm.hpp"

#define YYERROR_VERBOSE

extern "C" {
    void yyerror(const char *s);
    extern int yylex(void);
    extern FILE* yyin;
}

int lineno = 0;

std::string strRawType = "";
std::string strComments = "";
std::string strUserType = "";
std::string strContainer = "";
std::string strBasicType = "";
std::string strReturnType = "";

IdlModule      idlModule;
IdlInterface   idlInterface;
IdlFunction    idlFunction;
IdlFunArgument idlFunArgument;

%}

%union {
    int   ival;
    char* sval;
}

%token <sval> timeout
%token <ival> ytime
%token <sval> amd
%token <sval> nestable
%token <sval> stub_cpp_quote
%token <sval> proxy_cpp_quote
%token <sval> cpp_quote
%token <sval> comment_quote
%token <sval> module
%token <sval> objectname
%token <sval> interface
%token <sval> yconst
%token <sval> yin
%token <sval> yout
%token <sval> ybool
%token <sval> ychar
%token <sval> ysigned
%token <sval> yunsigned
%token <sval> yshort
%token <sval> yint
%token <sval> ylong
%token <sval> yfloat
%token <sval> ydouble
%token <sval> yvoid
%token <sval> callback
%token <sval> line_comment
%token <sval> block_comment
%token <sval> scopeop
%token <sval> ydash

%%

program : file_comment module_def

file_comment :
    | file_comment cpp_quote'(' comment_quote ')' ';'
    {
        std::string str = $4;
        ReplaceFirst(&str, "\"", "");
        ReplaceLast(&str, "\"", "");
        str = ReplaceAll(str, "\\\"", "\"");
        idlModule.m_CppAnnotation += str;
        idlModule.m_CppAnnotation += "\n";
    }
    | file_comment stub_cpp_quote'(' comment_quote ')' ';'
    {
        std::string str = $4;
        ReplaceFirst(&str, "\"", "");
        ReplaceLast(&str, "\"", "");
        str = ReplaceAll(str, "\\\"", "\"");
        idlModule.m_StubAnnotation += str;
        idlModule.m_StubAnnotation += "\n";
    }
    | file_comment proxy_cpp_quote'(' comment_quote ')' ';'
    {
        std::string str = $4;
        ReplaceFirst(&str, "\"", "");
        ReplaceLast(&str, "\"", "");
        str = ReplaceAll(str, "\\\"", "\"");
        idlModule.m_ProxyAnnotation += str;
        idlModule.m_ProxyAnnotation += "\n";
    }
    ;

module_def : single_module_def
    | comments interface_def
    | module_comments module objectname '{' single_module_def '}'
    {
        idlModule.m_ModuleNames.push_back($3);
    }
    ;

single_module_def : module_comments module objectname '{' interface_comments interface_def '}'
    {
        idlModule.m_Exist = true;
        idlModule.m_ModuleNames.push_back($3);
    }
;

interface_def : interface objectname '{' function_comments function_def '}'
    {
        idlInterface.m_InterfaceName = $2;
        idlModule.m_InterfaceVector.push_back(idlInterface);
        idlInterface.Reset();
    }
    | interface_def interface_comments interface objectname '{' function_comments function_def '}'
    {
        idlInterface.m_InterfaceName = $4;
        idlModule.m_InterfaceVector.push_back(idlInterface);
        idlInterface.Reset();
    }
    ;

module_comments : comments
    {
        idlModule.m_ModuleAnnotations.push_back(strComments);
        strComments = "";
    }
    ;
interface_comments : comments
    {
        idlInterface.m_InterfaceAnnotation = strComments;
        strComments = "";
    }
    ;
function_comments: comments
    {
        idlFunction.m_FuncAnnotation = strComments;
        strComments = "";
    }
    ;

function_def : attributes_def prototype_def
    {
        idlInterface.m_FuncVector.push_back(idlFunction);
        idlFunction.Reset();
    }
    | function_def function_comments attributes_def prototype_def
    {
        idlInterface.m_FuncVector.push_back(idlFunction);
        idlFunction.Reset();
    }
    ;

attributes_def :
    | '[' attributes ']'
    ;

attributes : attribute
    | attribute ',' attributes
    ;

attribute : timeout_def
    | amd
    {
        idlFunction.m_IsAmd = true;
    }
    | nestable
    {
        idlFunction.m_IsNestable = true;
    }
    ;

timeout_def : timeout '(' ytime ')'
    {
        idlFunction.m_Timeout = $3;
    }
    | timeout '(' ydash ytime ')'
    {
        idlFunction.m_Timeout = $4;
        idlFunction.m_Timeout *= -1;
    }
    ;
prototype_def : return_type objectname '(' parameter_list ')' ';'
    {
        idlFunction.m_FuncName = $2;
    }
    ;

return_type : user_type
    {
        idlFunction.m_ReturnType = strUserType;
        strUserType = "";
    }
    | basic_type
    {
        idlFunction.m_ReturnType = strBasicType;
    }
    | yvoid
    {
        idlFunction.m_ReturnType = $1;
    }
    ;

basic_type : ybool               { strBasicType = $1; }
    | ychar                      { strBasicType = $1; }
    | ysigned ychar              { strBasicType = $1; }
    | yunsigned ychar            { strBasicType = $1; }
    | yshort                     { strBasicType = $1; }
    | yunsigned yshort           { strBasicType = $1; }
    | yint                       { strBasicType = $1; }
    | yunsigned yint             { strBasicType = $1; }
    | ylong                      { strBasicType = $1; }
    | yunsigned ylong            { strBasicType = $1; }
    | ylong ylong                { strBasicType = $1; }
    | yunsigned ylong ylong      { strBasicType = $1; }
    | yfloat                     { strBasicType = $1; }
    | ydouble                    { strBasicType = $1; }
    | ylong ydouble              { strBasicType = $1; }
    ;

parameter_list :
    | parameter_def
    | parameter_list ',' parameter_def
    ;

parameter_def : param_type_def objectname
    {
        idlFunArgument.m_ArgName = $2;
        idlFunction.m_ArgVector.push_back(idlFunArgument);
        idlFunArgument.Reset();
    }
    | param_type_def
    {
        idlFunction.m_ArgVector.push_back(idlFunArgument);
        idlFunArgument.Reset();
    }
    ;

param_type_def : '[' yin ']' param_type
    {
        idlFunArgument.m_Direction = "in";
    }
    | '[' yin  ']' yconst param_type
    {
        idlFunArgument.m_Direction = "in";
        idlFunArgument.m_IsConst = true;
    }
    | '[' yout ']' param_type
    {
        idlFunArgument.m_Direction = "out";
    }
    | '[' yin ',' yout ']' param_type
    {
        idlFunArgument.m_Direction = "in_out";
    }
    | '[' yout ',' yin ']' param_type
    {
        idlFunArgument.m_Direction = "in_out";
    }
    | '[' callback ']' objectname '&'
    {
        idlFunArgument.m_IsCallBack = true;
        idlFunArgument.m_Direction = "callback";
        idlFunArgument.m_IsReference = true;
        idlFunArgument.m_Type = $4;
    }
    | yconst param_type
    {
        idlFunArgument.m_IsConst = true;
    }
    | param_type
    ;

param_type : raw_type '&'
    {
        idlFunArgument.m_IsReference = true;
        idlFunArgument.m_Type = strRawType;
    }
    | raw_type '*'
    {
        idlFunArgument.m_IsPointer = true;
        idlFunArgument.m_Type = strRawType;
    }
    | raw_type
    {
        idlFunArgument.m_Type = strRawType;
    }
    ;

raw_type : basic_type
    {
        strRawType = strBasicType;
    }
    | user_type
    {
        strRawType = strUserType;
    }
    | container
    {
        strRawType = strContainer;
        strContainer = "";
    }
    ;

container : container_begin container_end
    ;

container_begin : user_type '<'
    {
        strContainer += strUserType + "<";
    }
    ;

container_end : user_type '>'
    {
        strContainer += strUserType + ">";
    }
    | basic_type '>'
    {
        strContainer += strBasicType + ">";
    }
    | container  '>'
    {
        strContainer += " >";
    }
    | raw_type1 ',' raw_type2 '>'
    {
        strContainer += ">";
    }
    ;

raw_type1 : raw_type
    {
        strContainer += strRawType;
        strContainer += ", ";
    }
    | raw_type '*'
    {
        strContainer += strRawType;
        strContainer += "*";
        strContainer += ", ";
    };

raw_type2 : raw_type
    {
        strContainer += strRawType;
        strContainer += " ";
    }
    | raw_type '*'
    {
        strContainer += strRawType;
        strContainer += "*";
    };

user_type : objectname
    {
        strUserType = $1;
    }
    | user_type scopeop objectname
    {
        strUserType += $2;
        strUserType += $3;
    }
    ;

comments :
    | comments comment_def
    ;

comment_def : block_comment
    {
        strComments += $1;
        strComments += "\n";
    }
    | line_comment
    {
        strComments += $1;
        strComments += "\n";
    }
    ;

%%

void yyerror(const char *s)
{
    std::cerr << "line " << lineno << ": " << s << std::endl;
}

#include "common/config/cflags.hpp"

CFLAGS_DEFINE_FLAG(std::string, input, "input file name", CFlags::NoDefault);
CFLAGS_DEFINE_FLAG(std::string, output_dir, "output dir name");

int main(int argc, char** argv)
{
    if (!CFlags::ParseCommandLine(argc, argv))
    {
        return EXIT_FAILURE;
    }
    std::string filename = Flag_input.Value().c_str();
    std::string postfix = ".idl";
    if (filename.find(postfix) != filename.size() - postfix.size())
    {
        std::cerr << "invalid file name. please input the right file name format: $filename.idl" << std::endl;
        return EXIT_FAILURE;
    }
    std::string input_file = filename;
    ReplaceLast(&filename, ".idl", "");
    if (filename.empty())
    {
        std::cerr << "invalid file name. please input the right file name format: $filename.idl" << std::endl;
        return EXIT_FAILURE;
    }
    std::string outdir = Flag_output_dir;
    yyin = fopen(input_file.c_str(), "r");
    if (!yyin)
    {
        std::cerr << "Failed to open idl file: " << input_file << std::endl;
        return EXIT_FAILURE;
    }
    yyparse();
    fclose(yyin);

    //std::cout << "=====================" << std::endl;
    //std::cout << idlModule << std::endl;
    //std::cout << "=====================" << std::endl;

    bool ret = GenerateStub(idlModule, outdir, filename);
    if (!ret)
    {
        std::cerr << "Failed to generate stub file. " << std::endl;
        return EXIT_FAILURE;
    }
    ret = GenerateProxy(idlModule, outdir, filename);
    if (!ret)
    {
        std::cerr << "Failed to generate proxy file. " << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

