#include <cstring>
#include <stdlib.h>
#include "common/system/io/file.hpp"
#include "common/system/io/directory.hpp"
#include "common/system/io/path.hpp"
using namespace std;

namespace io
{
namespace path
{

bool IsSeparator(char ch)
{
#ifdef _WIN32
    return ch == '\\' || ch == '/';
#else
    return ch == '/';
#endif
}

string GetBaseName(const char* filepath)
{
    size_t len = strlen(filepath);
    int i = int(len - 1);
    for ( ; i >= 0; i--)
    {
        if (IsSeparator(filepath[i]))
            break;
    }
    return string(filepath + i + 1, filepath + len);
}

string GetBaseName(const string& filepath)
{
    return GetBaseName(filepath.c_str());
}

string GetExtension(const char* filepath)
{
    size_t len = strlen(filepath);
    int i = int(len - 1);
    for ( ; i >= 0; i--)
    {
        if (filepath[i] == '.')
            return string(filepath + i, filepath + len);
        if (IsSeparator(filepath[i]))
            return "";
    }
    return "";
}

string GetExtension(const string& filepath)
{
    return GetExtension(filepath.c_str());
}

string GetDirectory(const char* filepath)
{
    size_t len = strlen(filepath);
    int i = (int)len - 1;
    for ( ; i >= 0; i--)
    {
        if (IsSeparator(filepath[i]))
            break;
    }
    if (i >= 0)
        return string(filepath, filepath + i + 1);
    return "";
}

string GetDirectory(const string& filepath)
{
    return GetDirectory(filepath.c_str());
}

string GetFullPath(const char* filepath)
{
    char resolved[MAX_PATH_LEN];
#ifdef _WIN32
    _fullpath(resolved, filepath, MAX_PATH_LEN);
#else
    realpath(filepath, resolved);
#endif
    return resolved;
}

string GetFullPath(const string& filepath)
{
    return GetFullPath(filepath.c_str());
}

} // end of namespace path

} // end of namespace io

