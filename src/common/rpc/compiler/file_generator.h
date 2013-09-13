#ifndef _FILE_GENERATOR_H
#define _FILE_GENERATOR_H

#include <string>
#include "common/rpc/compiler/idlcompiler.h"

std::string ReplaceFirst(std::string* str, const std::string& src, const std::string& obj);
std::string ReplaceLast(std::string* str, const std::string& src, const std::string& obj);

bool GenerateProxy(const IdlModule& idlmodule, const std::string& outdir, const std::string& filename);
bool GenerateStub(const IdlModule& idlmodule, const std::string& outdir, const std::string& filename);

std::string GetProxyFunString(const IdlFunction& func);
std::string GetStubFunString(const IdlFunction& func);

std::string GetDispatch(const IdlFunction& func);

#endif

