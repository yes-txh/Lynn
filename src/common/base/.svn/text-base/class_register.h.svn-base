// Copyright 2010, Tencent Inc.
// Author: Yi Wang (yiwang@tencent.com)
//         Hangjun Ye (hansye@tencent.com)

#ifndef COMMON_BASE_CLASS_REGISTER_H_
#define COMMON_BASE_CLASS_REGISTER_H_

// Defines several helper macros for registering class and its singleton by a
// string name and creating/retrieving them later per the registered name.
// The motivation is to help implement the factory class. C++ doesn't support
// reflection so we define several macros to do this.
//
// All macros defined here are NOT used by final user directly, and they are
// used to create register macros for a specific base class. Here is an example:
/*
   mapper.h (the interface definition):
   #include "common/base/class_register.h"
   class Mapper {
   };

   CLASS_REGISTER_DEFINE_REGISTRY(mapper_register, Mapper);

   #define REGISTER_MAPPER(mapper_name) \
       CLASS_REGISTER_OBJECT_CREATOR( \
           mapper_register, Mapper, #mapper_name, mapper_name) \

   #define CREATE_MAPPER(mapper_name_as_string) \
       CLASS_REGISTER_CREATE_OBJECT(mapper_register, mapper_name_as_string)

   hello_mapper.cc (an implementation of Mapper):
   #include "mapper.h"
   class HelloMapper : public Mapper {
   };
   REGISTER_MAPPER(HelloMapper);

   mapper_user.cc (the final user of all registered mappers):
   #include "mapper.h"
   Mapper* mapper = CREATE_MAPPER("HelloMapper");
*/
// Another usage is to register class by an arbitrary string instead of its
// class name, and register a singleton instance for each registered name.
// Here is an example:
/*
   file_impl.h (the interface definition):
   class FileImpl {
   };

   CLASS_REGISTER_DEFINE_REGISTRY(file_impl_register, FileImpl);

   #define REGISTER_DEFAULT_FILE_IMPL(path_prefix_as_string, file_impl_name) \
       CLASS_REGISTER_DEFAULT_OBJECT_CREATOR_WITH_SINGLETON( \
           file_impl_register, FileImpl, path_prefix_as_string, file_impl_name)

   #define REGISTER_FILE_IMPL(path_prefix_as_string, file_impl_name) \
       CLASS_REGISTER_OBJECT_CREATOR_WITH_SINGLETON( \
           file_impl_register, FileImpl, path_prefix_as_string, file_impl_name)

   #define CREATE_FILE_IMPL(path_prefix_as_string) \
       CLASS_REGISTER_CREATE_OBJECT(file_impl_register, path_prefix_as_string)

   #define GET_FILE_IMPL_SINGLETON(path_prefix_as_string) \
       CLASS_REGISTER_GET_SINGLETON(file_impl_register, path_prefix_as_string)

   #define FILE_IMPL_COUNT() \
       CLASS_REGISTER_CREATOR_COUNT(file_impl_register)

   #define FILE_IMPL_NAME(i) \
       CLASS_REGISTER_CREATOR_NAME(file_impl_register, i)

   local_file.cc (an implementation of FileImpl):
   #include "file.h"
   class LocalFileImpl : public FileImpl {
   };
   REGISTER_FILE_IMPL("/local", LocalFileImpl);

   file_user.cc (the final user of all registered file implementations):
   #include "file_impl.h"
   FileImpl* file_impl = CREATE_FILE_IMPL("/local");
   FileImpl* file_impl_singleton = GET_FILE_IMPL_SINGLETON("/local");
*/

#include <assert.h>
#include <map>
#include <string>
#include <vector>

// The first parameter, register_name, should be unique globally.
// Another approach for this is to define a template for base class. It would
// make the code more readable, but the only issue of using template is that
// each base class could have only one register then. It doesn't sound very
// likely that a user wants to have multiple registers for one base class,
// but we keep it as a possibility.
// We would switch to using template class if necessary.
#define CLASS_REGISTER_DEFINE_REGISTRY(register_name, base_class_name) \
    class ObjectCreatorRegistry_##register_name { \
    public: \
        typedef base_class_name* (*Creator)(); \
        typedef base_class_name* (*Singleton)(); \
 \
        ObjectCreatorRegistry_##register_name() {} \
        ~ObjectCreatorRegistry_##register_name() {} \
 \
        void AddCreator(const std::string& entry_name, \
                        Creator creator, \
                        Singleton singleton = NULL) { \
            assert(m_creator_registry.find(entry_name) == \
                   m_creator_registry.end()); \
            m_creator_registry[entry_name] = creator; \
            m_singleton_registry[entry_name] = singleton; \
            m_creator_names.push_back(entry_name); \
        } \
 \
        size_t CreatorCount() const { \
            return m_creator_names.size(); \
        } \
 \
        const std::string& CreatorName(size_t i) const { \
            assert(i < m_creator_names.size()); \
            return m_creator_names[i]; \
        } \
 \
        base_class_name* CreateObject(const std::string& entry_name) const { \
            CreatorRegistry::const_iterator it = \
                m_creator_registry.find(entry_name); \
            if (it == m_creator_registry.end()) { \
                return NULL; \
            } \
            return (*(it->second))(); \
        } \
 \
        base_class_name* GetSingleton(const std::string& entry_name) const { \
            SingletonRegistry::const_iterator it = \
                m_singleton_registry.find(entry_name); \
            if (it == m_singleton_registry.end()) { \
                return NULL; \
            } \
            return (*(it->second))(); \
        } \
 \
    private: \
        typedef std::map<std::string, Creator> CreatorRegistry; \
        typedef std::map<std::string, Singleton> SingletonRegistry; \
        std::vector<std::string> m_creator_names; \
        CreatorRegistry m_creator_registry; \
        SingletonRegistry m_singleton_registry; \
    }; \
 \
    inline ObjectCreatorRegistry_##register_name& \
    GetRegistry_##register_name() { \
        static ObjectCreatorRegistry_##register_name registry; \
        return registry; \
    } \
 \
    class ObjectCreatorRegister_##register_name { \
    public: \
        ObjectCreatorRegister_##register_name( \
            const std::string& entry_name, \
            ObjectCreatorRegistry_##register_name::Creator creator, \
            ObjectCreatorRegistry_##register_name::Singleton singleton = NULL) { \
            GetRegistry_##register_name().AddCreator(entry_name, \
                                                     creator, \
                                                     singleton); \
        } \
        ~ObjectCreatorRegister_##register_name() {} \
    }

// User could select one of following two versions of
// CLASS_REGISTER_OBJECT_CREATOR, with or without singleton, but couldn't use
// both. So if the user decides to use the singleton version, all
// implementations must have public default constructor.
#define CLASS_REGISTER_OBJECT_CREATOR(register_name, \
                                      base_class_name, \
                                      entry_name_as_string, \
                                      class_name) \
    base_class_name* ObjectCreator_##register_name##class_name() { \
        return new class_name; \
    } \
    ObjectCreatorRegister_##register_name \
        g_object_creator_register_##register_name##class_name( \
            entry_name_as_string, \
            ObjectCreator_##register_name##class_name)

#define CLASS_REGISTER_OBJECT_CREATOR_WITH_SINGLETON(register_name, \
                                                     base_class_name, \
                                                     entry_name_as_string, \
                                                     class_name) \
    base_class_name* ObjectCreator_##register_name##class_name() { \
        return new class_name; \
    } \
    base_class_name* ObjectSingleton_##register_name##class_name() { \
        static class_name singleton; \
        return &singleton; \
    } \
    ObjectCreatorRegister_##register_name \
        g_object_creator_register_##register_name##class_name( \
            entry_name_as_string, \
            ObjectCreator_##register_name##class_name, \
            ObjectSingleton_##register_name##class_name)

#define CLASS_REGISTER_CREATE_OBJECT(register_name, entry_name_as_string) \
    GetRegistry_##register_name().CreateObject(entry_name_as_string)

#define CLASS_REGISTER_GET_SINGLETON(register_name, entry_name_as_string) \
    GetRegistry_##register_name().GetSingleton(entry_name_as_string)

#define CLASS_REGISTER_CREATOR_COUNT(register_name) \
    GetRegistry_##register_name().CreatorCount()

#define CLASS_REGISTER_CREATOR_NAME(register_name, i) \
    GetRegistry_##register_name().CreatorName(i)

#endif // COMMON_BASE_CLASS_REGISTER_H_
