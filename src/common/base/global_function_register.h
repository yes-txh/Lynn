// Copyright 2011, Tencent Inc.
// Author: Kypo Yin(kypoyin@tencent.com)
//
// Defines several helper macros for registering global function(global function
// and static class method) by a string name and get them later per the registe-
// red name.
// The motivation is to help implement the factory class. C++ doesn't support
// reflection so we define several macros to do this.
//
// All macros defined here are NOT used by final user directly, and they are
// used to create register macros for a specific base class. the example in
// global_function_register_test.h

#ifndef COMMON_BASE_GLOBAL_FUNCTION_REGISTER_H_
#define COMMON_BASE_GLOBAL_FUNCTION_REGISTER_H_

#include <assert.h>
#include <map>
#include <string>

// The first parameter, register_name, should be unique globally.
// Another approach for this is to define a template for function type.
// It would make the code more readable, but the only issue of using template
// is that each function type could have only one register then. It doesn't
// sound very likely that a user wants to have multiple registers for one
// function type, but we keep it as a possibility.

#define GLOBAL_REGISTER_DEFINE_REGISTRY(register_name, function_type) \
class GlobalFunctionRegistry##register_name { \
public: \
    GlobalFunctionRegistry##register_name() {} \
    ~GlobalFunctionRegistry##register_name() {} \
 \
    void AddFunction(const std::string& entry_name, \
                     function_type function_point) { \
    assert(function_registry_.find(entry_name) == \
           function_registry_.end()); \
 \
    function_registry_.insert(make_pair(entry_name, function_point)); \
    } \
 \
    function_type GetFunction(const std::string& entry_name) { \
        GlobalFunctionRegistry::iterator it \
            = function_registry_.find(entry_name); \
        if (it != function_registry_.end()) { \
            return it->second; \
        } \
        return reinterpret_cast<function_type>(NULL); \
    } \
\
private: \
    typedef std::map<std::string, function_type> GlobalFunctionRegistry; \
    GlobalFunctionRegistry function_registry_; \
}; \
\
inline GlobalFunctionRegistry##register_name& \
    GetGlobalFunctionRegistry##register_name() { \
        static GlobalFunctionRegistry##register_name function_registry; \
        return function_registry; \
} \
\
class GlobalFunctionRegister##register_name{ \
public: \
    GlobalFunctionRegister##register_name(std::string entry_name, \
                                          function_type function_point) { \
    GetGlobalFunctionRegistry##register_name().AddFunction(entry_name, \
                                                           function_point); \
} \
 \
    ~GlobalFunctionRegister##register_name() {} \
};

#define GLOBAL_FUNCTION_REGISTER(register_name, entry_name, function_point) \
    GlobalFunctionRegister##register_name \
        Register##register_name##entry_name(#entry_name, function_point);


#define GET_GLOBAL_FUNCTION(register_name, entry_name_as_name) \
    GetGlobalFunctionRegistry##register_name().GetFunction(entry_name_as_name)

#endif // COMMON_BASE_GLOBAL_FUNCTION_REGISTER_H_
