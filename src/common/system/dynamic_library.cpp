#include "common/system/dynamic_library.hpp"

#include <string.h> // System

#ifdef _WIN32
#include <common/base/common_windows.h>
#else
#include <dlfcn.h>
#include <stdio.h>
#endif

DynamicLibrary::DynamicLibrary()
{
    m_library = NULL;
}

DynamicLibrary::~DynamicLibrary()
{
    Unload();
}

// Gets the last dll error as string
static const char *DllGetLastError()
{
#ifdef _WIN32
    return "dll error"; // GetLastError();
#else // not win32
    return dlerror();
#endif // WIN32
}

// Loads the dll with LoadLibrary() or dlopen.
bool DynamicLibrary::Load(const char* filename)
{
    if (IsLoaded()) {
        return true;
    }

#ifdef _WIN32
    m_library = LoadLibraryA(filename);
#else
    // RTLD_NOW: resolve all symbols on load
    // RTLD_LOCAL: don't resolve symbols for other libraries
    m_library = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
#endif

    if (!m_library) {
        const char * error = DllGetLastError();
        (void) error;
        return false;
    }

    return true;
}

// Frees one instances of the dynamic library.
bool DynamicLibrary::Unload()
{
    if (!IsLoaded()) {
        return false;
    }

#ifdef _WIN32
    bool retval = FreeLibrary(static_cast<HMODULE>(m_library)) != FALSE;
#else
    bool retval = dlclose(m_library) == 0;
#endif

    m_library = NULL;

    return retval;
}

// Returns the address where symbol funcname is loaded or NULL on failure
void* DynamicLibrary::GetSymbol(const char* name) const
{
    if (!m_library)
    {
        return NULL;
    }

#ifdef _WIN32
    return GetProcAddress(static_cast<HMODULE>(m_library), name);
#else
    return dlsym(m_library, name);
#endif
}
