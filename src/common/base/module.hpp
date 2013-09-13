// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/18/11
// Description: reliable and expandable global module definition/using mechanism

#ifndef COMMON_BASE_MODULE_HPP
#define COMMON_BASE_MODULE_HPP
#pragma once

/*
//////////////////////////////////////////////////////////////////////////////
// examplaes

// ===========================================================================
// in foo.hpp
int FooDoSomeThing();

// ===========================================================================
// in foo.cpp
DEFINE_MODULE(foo)
{
    // do some staff
    g_foo = InitializeFoo();
    // return true to report success
    return true;
}

// ===========================================================================
// in bar.cpp
// import foo declare
#include "foo.hpp"

DEFINE_MODULE(bar)
{
    // bar requires foo be initialized firstly
    g_bar = InitializeBar(FooDoSomeThing());
    return true;
}

// declare this module depends on foo
USING_MODULE(foo);

// ===========================================================================
// in main.cpp
#include "common/base/module.hpp"

int main(int argc, char** argv)
{
    // initialize all modules firstly
    InitAllModules(&argc, &argv, true);

    // ok, all modules were initialized, do other things
    ...
    return 0;
}

// ===========================================================================
// in BUILD
cc_library(
    name = 'foo',
    srcs = 'foo.cpp',
    link_all_symbols = 1 # NOTE: this is required by module registerers
)

cc_library(
    name = 'bar',
    srcs = 'bar.cpp',
    deps = ':foo',
    link_all_symbols = 1 # NOTE: this is required by module registerers
)

cc_binary(
    name = 'main',
    srcs = 'main.cpp',
    deps = ':bar'
)
*/

#include <stddef.h> // for NULL

namespace testing {
void InitGoogleTest(int* argc, char** argv);
}

namespace base {
namespace internal {

class ModuleRegisterer;

typedef bool (*ModuleCtor)();
typedef void (*ModuleDtor)();

/// Manage all registered modules and their dependancies
class ModuleManager
{
private:
    ModuleManager();
public:
    /// register module, used by the DEFINE_MODULE macro.
    static void RegisterModuleCtor(
        const char* filename,
        int line,
        const char* name,
        ModuleCtor ctor
    );

    static void RegisterModuleDtor(
        const char* filename,
        int line,
        const char* name,
        ModuleDtor dtor
    );

    /// register module dependency, used by the USING_MODULE macro.
    static void RegisterDependency(
        const char* filename,
        int line,
        const char* this_module,
        const char* dep_name,
        ModuleRegisterer* module_registerer
    );

    /// initialize all registered modules after parsing command line flags
    static void InitializeAll(int* argc, char*** argv, bool remove_flags);
};

/// Register module to manager automatically
/// used by DEFINE_MODULE
class ModuleRegisterer
{
public:
    ModuleRegisterer(
        const char* filename,
        int line,
        const char* name,
        ModuleCtor ctor)
    {
        ModuleManager::RegisterModuleCtor(filename, line, name, ctor);
    }
};

/// Register module dtor to manager automatically
/// used by DEFINE_MODULE_DTOR
class ModuleDtorRegisterer
{
public:
    ModuleDtorRegisterer(
        const char* filename,
        int line,
        const char* name,
        ModuleDtor dtor)
    {
        ModuleManager::RegisterModuleDtor(filename, line, name, dtor);
    }
};

/// register module dependency to module manager automatically
/// used by USING_MODULE macro
class ModuleDependencyRegisterer
{
public:
    ModuleDependencyRegisterer(
        const char* filename,
        int line,
        const char* this_module_name,
        const char* dependency_name,
        ModuleRegisterer* module_registerer = NULL
    )
    {
        ModuleManager::RegisterDependency(
            filename, line,
            this_module_name, dependency_name,
            module_registerer);
    }
};

} // end namespace internal

/// Initialize all registered modules in dependency order.
/// @param argc pointer to argc of main
/// @param argv pointer to argv of main
/// @param remove_flags whether remove processed flags
///
/// @note
/// The *argc and *argv may be changed if remove_flags is specified.
///
/// This function always initialize gflags and glog internally, you should
/// never do it again.
///
/// Gtest defined some its own flags and only process and remove recognized
/// flags to it, but gflags is always trying process and remove all flags.
/// If you are using gtest, make sure to call InitGoogleTesting befor this
/// function, or gflags will report unknown flags and gtest will not receive
/// its flags.
inline void InitAllModules(int* argc, char*** argv, bool remove_flags = true)
{
    ::base::internal::ModuleManager::InitializeAll(argc, argv, remove_flags);
}

/// Initialize all registered modules and also gtest
inline void InitAllModulesAndTest(int* argc, char*** argv, bool remove_flags = true)
{
    ::testing::InitGoogleTest(argc, *argv);
    InitAllModules(argc, argv, remove_flags);
}

} // end namespace base

//////////////////////////////////////////////////////////////////////////////
// helper macros to make module easier to use

/// macro to make module registration easier
/// @param name symbolic module name, should be global unique
/// @note DEFINE_MODULE should be used in GLOBAL namespace
#define DEFINE_MODULE(name) \
static const char THIS_MODULE_NAME[] = #name; \
static bool name##_ModuleCtor(); \
namespace modules { \
::base::internal::ModuleRegisterer name##_module_registerer( \
    __FILE__, __LINE__, #name, &name##_ModuleCtor); \
} \
\
static bool name##_ModuleCtor()

/// macro to make module registration easier
/// @param name symbolic module name, should be global unique
/// @note DEFINE_MODULE_DTOR should be used in GLOBAL namespace
#define DEFINE_MODULE_DTOR(name) \
static void name##_ModuleDtor(); \
namespace modules { \
::base::internal::ModuleDtorRegisterer name##_module_dtor_registerer( \
    __FILE__, __LINE__, #name, &name##_ModuleDtor); \
} \
\
static void name##_ModuleDtor()

/// macro to make module dependency registration easier.
/// using compile time symbolic name, name must be declared using DECLARE_MODULE
/// @note USING_MODULE should be used in GLOBAL namespace
#define USING_MODULE(dep_name) \
namespace modules { \
extern ::base::internal::ModuleRegisterer dep_name##_module_registerer; \
static ::base::internal::ModuleDependencyRegisterer using_module_##dep_name( \
    __FILE__, __LINE__, \
    THIS_MODULE_NAME, #dep_name, &dep_name##_module_registerer); \
}

// make it easy to use for end users
using ::base::InitAllModules;
using ::base::InitAllModulesAndTest;

#endif // COMMON_BASE_MODULE_HPP
