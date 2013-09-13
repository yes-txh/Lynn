// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/18/11
// Description:

#include "common/base/module.hpp"

#include <string.h>
#include <algorithm>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include "common/base/module/dependency_map.hpp"
#include "common/base/singleton.hpp"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace base {
namespace internal {

class ModuleManagerImpl : public Singleton<ModuleManagerImpl>
{
    struct Module
    {
        const char* filename; // for diagnose
        int line;             // for diagnose
        const char* name;
        ModuleCtor ctor;
        ModuleDtor dtor;
    public:
        Module():
            filename(NULL),
            line(0),
            name(NULL),
            ctor(NULL),
            dtor(NULL)
        {
        }
    };
public:
    void RegisterModuleCtor(
        const char* filename, int line,
        const char* name,
        ModuleCtor ctor
    );
    void RegisterModuleDtor(
        const char* filename, int line,
        const char* name,
        ModuleDtor dtor
    );
    void RegisterDependency(const char* this_module, const char* dep_name);
    void InitializeAll(int* argc, char*** argv, bool remove_flags);
private:
    void SortModules(std::vector<const Module*>* sorted_modules) const;
private:
    typedef std::map<std::string, Module> ModuleMap;
    ModuleMap m_modules;
    DependencyMap m_depends;
};

void ModuleManagerImpl::RegisterModuleCtor(
    const char* filename,
    int line,
    const char* name,
    ModuleCtor ctor
    )
{
    Module& module = m_modules[name];
    if (module.ctor != NULL)
    {
        LOG(FATAL) << name << ": Duplicated module definition: "
            << "first in " << module.filename << ":" << module.line << ", "
            << "second in " << filename << ":" << line;
    }

    module.filename = filename;
    module.line = line;
    module.name = name;
    module.ctor = ctor;

    // make sure this module is registered into the depends
    m_depends.insert(std::make_pair(name, std::set<std::string>()));
}

void ModuleManagerImpl::RegisterModuleDtor(
    const char* filename,
    int line,
    const char* name,
    ModuleDtor dtor
    )
{
    Module& module = m_modules[name];
    if (module.dtor != NULL)
    {
        LOG(FATAL) << name << ": Duplicated module dtor registered: "
            << "in " << filename << ":" << line;
    }
    CHECK_STREQ(module.name, name);
    module.dtor = dtor;
}

void ModuleManagerImpl::RegisterDependency(const char* this_module, const char* dep_name)
{
    // register this module to dependency module
    m_depends[this_module].insert(dep_name);
}

void ModuleManagerImpl::SortModules(std::vector<const Module*>* sorted_modules) const
{
    std::vector<std::string> sorted_names;
    if (!TopologicalSort(m_depends, &sorted_names))
    {
        // Dump all dependencies
        std::ostringstream oss;
        oss << "Cyclic dependency detected:\n";
        for (DependencyMap::const_iterator i = m_depends.begin();
             i != m_depends.end(); ++i)
        {
            // exclude acyclic module
            if (std::find(sorted_names.begin(), sorted_names.end(), i->first) !=
                sorted_names.end())
            {
                continue;
            }

            oss << i->first << ":";
            const std::set<std::string>& deps = i->second;
            for (std::set<std::string>::const_iterator j = deps.begin();
                 j != deps.end(); ++j)
            {
                oss << " " << *j;
            }
            oss << "\n";
        }
        LOG(FATAL) << oss.str();
    }

    std::reverse(sorted_names.begin(), sorted_names.end());

    for (size_t i = 0; i < sorted_names.size(); ++i)
    {
        const std::string& module = sorted_names[i];
        ModuleMap::const_iterator it = m_modules.find(module);
        CHECK(it != m_modules.end()) << "Unknown module name: " << module
            << ", did you forget to link corresponding library?";
        sorted_modules->push_back(&it->second);
    }
}

void ModuleManagerImpl::InitializeAll(int* argc, char*** argv, bool remove_flags)
{
    google::ParseCommandLineFlags(argc, argv, remove_flags);
    google::InitGoogleLogging((*argv)[0]);

    std::vector<const Module*> sorted_modules;
    SortModules(&sorted_modules);
    for (std::vector<const Module*>::iterator i = sorted_modules.begin();
         i != sorted_modules.end(); ++i)
    {
        const Module* module = *i;
        LOG(INFO) << "Initializing module " << module->name;
        if (!module->ctor())
        {
            LOG(FATAL) << "Module initialization error:" << "defined in "
                << module->filename << ":" << module->line;
        }

        // register dtor to atexit
        // ATEXIT_MAX is large enough under glibc, so can be used unlimitedly
        if (module->dtor)
            atexit(module->dtor);
    }
}

///////////////////////////////////////////////////////////////////////////////
// forward ModuleManager mtthods to Impl

void ModuleManager::RegisterModuleCtor(
    const char* filename,
    int line,
    const char* name,
    ModuleCtor ctor
)
{
    ModuleManagerImpl::Instance().RegisterModuleCtor(filename, line, name, ctor);
}

void ModuleManager::RegisterModuleDtor(
    const char* filename,
    int line,
    const char* name,
    ModuleDtor dtor
)
{
    ModuleManagerImpl::Instance().RegisterModuleDtor(filename, line, name, dtor);
}

void ModuleManager::RegisterDependency(
    const char* filename,
    int line,
    const char* this_module_name,
    const char* dep_name,
    ModuleRegisterer* module_registerer
    )
{
    // module_registerer is used to verify the name and type of depended module
    // and make a symbolic linking dependency forcely (can be checked by linker if missing)
    // but it become useless here, discard it now.
    ModuleManagerImpl::Instance().RegisterDependency(this_module_name, dep_name);
}

void ModuleManager::InitializeAll(int* argc, char*** argv, bool remove_flags)
{
    ModuleManagerImpl::Instance().InitializeAll(argc, argv, remove_flags);
}

} // end namespace internal
} // end namespace base
