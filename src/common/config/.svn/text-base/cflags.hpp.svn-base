#ifndef CFLAGS_HPP_INCLUDED
#define CFLAGS_HPP_INCLUDED

#ifdef _MSC_VER
#undef _CRT_SECURE_NO_WARNINGS
#endif

/// @file cflags.hpp
/// @brief Command line flags parser
/// @author chen3feng <chen3fengx@hotmail.com>
/// @date Jan 25, 2010
/// @version 1.0

#include <stddef.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string>

namespace cflags
{

/// flag has no default value, if not specified, keep invalid.
/// using by CFlags ctor
static const int NoDefault = 1;

///////////////////////////////////////////////////////////////////////////////
// ParseCommandLine flags

/// when error occurs, don't dump
static const int Silent = 1;

/// ignore unknown flags, unknown flags default handles as error
static const int IgnoreUnknown = 2;

/// don't remove handled flags
static const int NoRemove = 4;

template <typename T>
struct DefaultIsWrapper
{
    const T& value;
};

template <typename T>
inline DefaultIsWrapper<T> DefaultIs(const T& value)
{
    DefaultIsWrapper<T> wrapper = { value };
    return wrapper;
}

template <typename T>
struct LocationWrapper
{
    T* value;
};

template <typename T>
inline LocationWrapper<T> BindTo(T* value)
{
    LocationWrapper<T> wrapper = { value };
    return wrapper;
}

/// flag handler primary template
template <typename T>
struct FlagHandler
{
    /*
       has no any members, so it can caught specified version not defined error
       specifictied version should include following member functions:

       static bool Parse(
       const char* value, bool* result,
       std::string& error_message
       );

       static std::string ToString(const T* value);
    */
};

/// default flag parse function
template <typename T>
bool ParseFlag(const char* value, T* result, std::string& error_message)
{
    return FlagHandler<T>::Parse(value, result, error_message);
}

/// default stringify function
template <typename T>
std::string ToString(const T* value)
{
    return FlagHandler<T>::ToString(value);
}

/// specify for bool type
template <>
struct FlagHandler<bool>
{
public:
    template <size_t N>
    static bool MatchStringSet(const char* value, const char* const (&strset)[N])
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (strcmp(strset[i], value) == 0)
                return true;
        }
        return false;
    }

    static bool Parse(
        const char* value,
        bool* result,
        std::string& error_message
        )
    {
        static const char* const true_names[] = { "true", "yes", "on", "1", "" };
        static const char* const false_names[] = { "false", "no", "off", "0" };

        if (MatchStringSet(value, true_names))
        {
            *result = true;
        }
        else if (MatchStringSet(value, false_names))
        {
            *result = false;
        }
        else
        {
            error_message = "invalid value";
            return false;
        }

        return true;
    }

    static std::string ToString(const bool* value)
    {
        return *value ? "true" : "false";
    }
};

/// specify for signed integer type
template <typename T, T Min, T Max>
struct SignedIntFlagHandler
{
    static bool Parse(const char* value, T* result, std::string& error_message)
    {
        if (value[0] == '\0')
        {
            error_message = "can't be empty";
            return false;
        }

        char* end;
        errno = 0;
        long n = strtol(value, &end, 0);
        if (*end != '\0')
        {
            error_message = "reach unexpected character";
            return false;
        }
        else if (n < Min || n > Max || errno != 0)
        {
            error_message = strerror(ERANGE);
            return false;
        }

        *result = static_cast<T>(n);
        return true;
    }
    static std::string ToString(const T* value)
    {
        char buf[32];
        return std::string(buf, sprintf(buf, "%lld", (long long)*value));
    }
};

/// specify for short
template <>
struct FlagHandler<short> : SignedIntFlagHandler<short, SHRT_MIN, SHRT_MAX>
{
};

/// specify for int
template <>
struct FlagHandler<int> : public SignedIntFlagHandler<int, INT_MIN, INT_MAX>
{
};

/// specify for long
template <>
struct FlagHandler<long> : SignedIntFlagHandler<long, LONG_MIN, LONG_MAX>
{
};

/// specify for unsigned integer
template <typename T>
struct UnsignedIntFlagHandler
{
    static bool Parse(const char* value, T* result, std::string& error_message)
    {
        if (value[0] == '\0')
        {
            error_message = "can't be empty";
            return false;
        }

        char* end;
        errno = 0;
        unsigned long n = strtoul(value, &end, 0);
        if (*end != '\0')
        {
            error_message = "reach unexpected character";
            return false;
        }
        else if (n > (T)~T(0) || errno != 0)
        {
            error_message = strerror(ERANGE);
            return false;
        }

        *result = static_cast<T>(n);
        return true;
    }
    static std::string ToString(const T* value)
    {
        char buf[32];
        unsigned long long ull_value = *value;
        return std::string(buf, sprintf(buf, "%llu", ull_value));
    }
};

/// specify for unsigned short
template <>
struct FlagHandler<unsigned short> : UnsignedIntFlagHandler<unsigned short>
{
};

/// specify for unsigned int
template <>
struct FlagHandler<unsigned int> : UnsignedIntFlagHandler<unsigned int>
{
};

/// specify for unsigned long
template <>
struct FlagHandler<unsigned long> : UnsignedIntFlagHandler<unsigned long>
{
};

/// specify for long long
template <>
struct FlagHandler<long long>
{
    static bool Parse(
        const char* value,
        long long* result,
        std::string& error_message
    )
    {
        if (value[0] == '\0')
        {
            error_message = "can't be empty";
            return false;
        }

        char* end;
        errno = 0;
#ifdef _MSC_VER
        long long n = _strtoi64(value, &end, 0);
#else
        long long n = strtoll(value, &end, 0);
#endif
        if (*end != '\0')
        {
            error_message = "reach unexpected character";
            return false;
        }
        else if (errno != 0)
        {
            error_message = strerror(errno);
            return false;
        }
        *result = n;
        return true;
    }
    static std::string ToString(const long long* value)
    {
        char buf[32];
        return std::string(buf, sprintf(buf, "%lld", *value));
    }
};

/// specify for unsigned long long
template <>
struct FlagHandler<unsigned long long>
{
    static bool Parse(
        const char* value,
        unsigned long long* result,
        std::string& error_message
    )
    {
        if (value[0] == '\0')
        {
            error_message = "can't be empty";
            return false;
        }

        errno = 0;
        char* end;
#ifdef _MSC_VER
        unsigned long long n = _strtoui64(value, &end, 0);
#else
        unsigned long long n = strtoull(value, &end, 0);
#endif
        if (*end != '\0')
        {
            error_message = "reach unexpected character";
            return false;
        }
        else if (errno != 0)
        {
            error_message = strerror(errno);
            return false;
        }
        *result = n;
        return true;
    }
    static std::string ToString(const unsigned long long* value)
    {
        char buf[32];
        return std::string(buf, sprintf(buf, "%llu", *value));
    }
};

/// specify for float
template <>
struct FlagHandler<float>
{
    static bool Parse(
        const char* value,
        float* result,
        std::string& error_message
    )
    {
        if (value[0] == '\0')
        {
            error_message = "can't be empty";
            return false;
        }

        errno = 0;
        char* end;
#ifdef _MSC_VER
        double d = strtod(value, &end);
        float n = static_cast<float>(d);
#else
        float n = strtof(value, &end);
#endif
        if (*end != '\0')
        {
            error_message = "reach unexpected character";
            return false;
        }
        else if (errno != 0)
        {
            error_message = strerror(errno);
            return false;
        }
#ifdef _MSC_VER
        // strtod can handle values out of the range of float, check overflow
        if (d > FLT_MAX || d < -FLT_MAX)
        {
            error_message = strerror(ERANGE);
            return false;
        }
#endif
        *result = n;
        return true;
    }
    static std::string ToString(const float* value)
    {
        char buf[32];
        return std::string(buf, sprintf(buf, "%lg", *value));
    }
};

/// specify for double
template <>
struct FlagHandler<double>
{
    static bool Parse(
        const char* value,
        double* result,
        std::string& error_message
    )
    {
        if (value[0] == '\0')
        {
            error_message = "can't be empty";
            return false;
        }

        errno = 0;
        char* end;
        *result = strtod(value, &end);
        if (*end != '\0')
        {
            error_message = "reach unexpected character";
            return false;
        }
        else if (errno != 0)
        {
            error_message = strerror(errno);
            return false;
        }
        return true;
    }
    static std::string ToString(const double* value)
    {
        char buf[32];
        return std::string(buf, sprintf(buf, "%lg", *value));
    }
};

/// specify for basic_string<char>
template <typename Traits, typename Allocator>
struct FlagHandler<std::basic_string<char, Traits, Allocator> >
{
    static bool Parse(
        const char* value,
        std::basic_string<char, Traits, Allocator>* result,
        std::string& error_message)
    {
        *result = value;

        // trim right
        while (!result->empty() &&
               isspace((unsigned char) (*result)[result->length() - 1]))
        {
            result->resize(result->length() - 1);
        }

        // trim left
        while (!result->empty() && isspace((unsigned char) (*result)[0]))
            result->erase(0, 1);

        return true;
    }
    static std::string ToString(
        const std::basic_string<char, Traits, Allocator>* value
        )
    {
        return "'" + *value + "'";
    }
};

class FlagList;

/// untyped flag base class definituon
class BasicFlag
{
private:
    BasicFlag();
protected:
    /// value parser function pointer type
    typedef bool (*Parser)(
        const char* string,
        void* result,
        std::string& error_message
    );

    /// value validate function pointer type
    typedef bool (*Validator)(const void* result, std::string& error_message);

    /// stringify function pointer type
    typedef std::string (*ToString)(const void* result);

    template <typename T>
    BasicFlag(
        const char* file, ///< source file name
        const char* name,
        T* location, ///< flag variable location
        const char* description, /// detail description
        int flags, ///< bitmask flags of this flag
        bool (*validator)(const T*, std::string& error_message), ///< value validator
        bool (*parser)(const char*, T*, std::string& error_message), ///< value parsing function
        std::string (*to_string)(const T* value) ///< stringify function
    ):
        m_File(file),
        m_Name(name),
        m_Address(location),
        m_Description(description),
        m_Flags(flags),
        m_Parser(reinterpret_cast<Parser>(parser)),
        m_Validator(reinterpret_cast<Validator>(validator)),
        m_ToString(reinterpret_cast<ToString>(to_string)),
        m_IsSet(false)
    {
        Register();
    }

public:
    /// Flag name
    const char* Name() const
    {
        return m_Name;
    }

    /// return detail description
    const char* Description() const
    {
        return m_Description;
    }

    /// whether value is valid
    bool IsValid() const
    {
        return m_IsSet || (m_Flags & NoDefault) == 0;
    }

    /// whether this flag is set by cmd or flagfile explicitly
    bool IsSet() const
    {
        return m_IsSet;
    }

    bool ValidateAfterSet(std::string& error)
    {
        m_IsSet = !m_Validator || m_Validator(m_Address, error);
        return m_IsSet;
    }
private:
    bool Parse(const char* value, std::string& error_message)
    {
        std::string error;
        if (m_Parser(value, m_Address, error))
        {
            if (!ValidateAfterSet(error))
            {
                error_message = "validate error: ";
                error_message += error;
                return false;
            }
            return true;
        }
        else
        {
            error_message = "parse error: ";
            error_message += error;
        }
        return false;
    }

    /// get help information of this flag
    std::string GetHelp() const
    {
        std::string result = m_Description;

        if (m_Flags & NoDefault)
        {
            result += ", has no default value";
        }
        else
        {
            result += ", default is ";
            result += m_ToString(m_Address);
        }

        return result;
    }

    /// get source file
    const char* GetFile() const
    {
        return m_File;
    }

    /// check value error, such as has no default value but not initialized
    bool CheckError(std::string& message) const
    {
        if (!IsValid())
        {
            message = "has no default value, but not initialized";
            return true;
        }
        return false;
    }

    /// dump value information
    /// sample of result:
    ///   <not initialized>
    ///   true (default)
    std::string DumpValue() const
    {
        if (m_IsSet)
        {
            return m_ToString(m_Address);
        }
        else
        {
            if (m_Flags & NoDefault)
                return " <not initialized>";
            else
                return m_ToString(m_Address) + " (default)";
        }
    }

    /// register this flag to CFlags system
    void Register();
private:
    const char* m_File;
    int m_Line;
    const char* m_Name;
    void* m_Address;
    const char* m_Description;
    int m_Flags;
    Parser m_Parser;
    Validator m_Validator;
    ToString m_ToString;
    bool m_IsSet;
private:
    friend class FlagList;
    BasicFlag* Next;
};

/// typed flag class definituon
template <typename T>
class Flag : public BasicFlag
{
public:
    Flag(
        const char* file,
        const char* name,
        const char* description,
        int flags = 0,
        bool (*validator)(const T*, std::string& error_message) = NULL,
        bool (*parser)(const char*, T*, std::string& error_message) = &cflags::ParseFlag<T>,
        std::string (*to_string)(const T*) = &cflags::ToString<T>
    ) :
        BasicFlag(file, name, &m_ValuePlaceHolder, description, flags, validator, parser, to_string),
        m_ValuePlaceHolder(),
        m_Value(m_ValuePlaceHolder)
    {
    }

    template <typename DefaultValueType>
    Flag(
        const char* file,
        const char* name,
        const DefaultIsWrapper<DefaultValueType>& init_value,
        const char* description,
        int flags = 0,
        bool (*validator)(const T*, std::string& error_message) = NULL,
        bool (*parser)(const char*, T*, std::string& error_message) = &cflags::ParseFlag<T>,
        std::string (*to_string)(const T*) = &cflags::ToString<T>
    ) :
        BasicFlag(file, name,
                  &m_ValuePlaceHolder, description, flags,
                  validator, parser, to_string),
        m_ValuePlaceHolder(init_value.value),
        m_Value(m_ValuePlaceHolder)
    {
    }

    Flag(
        const char* file,
        const char* name,
        const LocationWrapper<T>& location,
        const char* description,
        int flags = 0,
        bool (*validator)(const T*, std::string& error_message) = NULL,
        bool (*parser)(const char*, T*, std::string& error_message) = &cflags::ParseFlag<T>,
        std::string (*to_string)(const T*) = &cflags::ToString<T>
    ) :
        BasicFlag(
            file, name, location.value, description,
            flags,
            validator, parser, to_string),
        m_Value(*location.value)
    {
    }

    bool SetValue(const T& value)
    {
        m_Value = value;
        std::string error;
        return ValidateAfterSet(error);
    }
    const T& Value() const
    {
        assert(IsValid());
        return m_Value;
    }

    operator const T&() const
    {
        assert(IsValid());
        return m_Value;
    }
private:
    T m_ValuePlaceHolder;
    T& m_Value;
};

template <typename T, typename U>
T operator+(const Flag<T>& flag, const U& rhs)
{
    return flag.Value() + rhs;
}

/// join all error messages
std::string JoinErrorMessages(
    const std::string& unmatch_error,
    const std::string& parse_error,
    const std::string& checked_error
);

/// All flags's list
/// all flags variable are collected into one single linked list
class FlagList
{
public:
    FlagList():
        First(NULL), Last(NULL)
    {
    }

    static FlagList& Instance()
    {
        static FlagList flag_list;
        return flag_list;
    }

    static void Register(BasicFlag* flag)
    {
        Instance().RegisterFlag(flag);
    }

    bool ParseCommandLine(
        int& argc, char** argv,
        int flags,
        std::string& error_message
    );

    bool ParseFlagsFile(
        const char* filename,
        std::string& parse_error,
        std::string& unmatch_error,
        int flags
    );

    void ShowHelp(const char* app_name, FILE* stream);
    void DumpValues(FILE* stream);

    int CheckFlagErrors(std::string& checked_error);
private:
    void RegisterFlag(BasicFlag* flag);
    /// whether name match flag name
    static bool NameMatch(const char* name, size_t name_length, const char* pattern);

    /// split command line args into name/value pair
    static bool SplitFlag(
        int argc, char** argv,
        int index,
        const char*& name, size_t& name_length,
        const char*& value
    );

    /// split single config file line into name/value pair
    static bool SplitFlagLine(
        const char* line,
        const char*& name,
        size_t& name_length,
        const char*& value, std::string& error_message
    );

    static bool HasIgnoreUnknownFlags(
        int argc,
        char** argv,
        bool* onoff,
        std::string& error_message
    );

    void ShowBuiltinFlagHelps(FILE* stream);

    /// return number of matched flags
    int ParseBuiltinFlags(
        const char* app_name,
        const char* name, size_t name_length,
        const char* value,
        std::string& parse_error,
        std::string& unmatch_error,
        int flags
    );

    /// try to match user defined flags by input flag name/value pair
    int TryParseUserFlags(
        const char* name,
        size_t name_length,
        const char* value, std::string& error_message
    );

private:
    // make a single linked list
    BasicFlag* First; ///< linked list head
    BasicFlag* Last;  ///< linked list tail
};

inline void BasicFlag::Register()
{
    FlagList::Register(this);
}

inline bool FlagList::HasIgnoreUnknownFlags(
    int argc, char** argv,
    bool* onoff,
    std::string& error_message
    )
{
    *onoff = false;

    std::string error;
    bool result = false;
    for (int i = 1; i < argc; ++i)
    {
        const char* name;
        size_t name_length;
        const char* value;
        if (SplitFlag(argc, argv, i, name, name_length, value))
        {
            if (*name == '\0') // reach '--', all remaining args are not flags
                break;
            if (NameMatch(name, name_length, "ignore_unknown") ||
                NameMatch(name, name_length, "ignore-unknown"))
            {
                // don't break here -- posterior flag may overlay previous
                if (FlagHandler<bool>::Parse(value, onoff, error))
                {
                    result = true;
                }
                else
                {
                    if (!error_message.empty())
                        error_message += "\n";
                    error_message += "    --";
                    error_message.append(name, name_length);
                    error_message += " -- ";
                    error_message += error;
                }
            }
        }
    }
    return result;
}

inline bool FlagList::ParseCommandLine(
    int& argc, char** argv,
    int flags,
    std::string& error_message
    )
{
    // only '--help' flag
    if (argc == 2 && strcmp(argv[1], "--help") == 0)
    {
        // show help and quick exit
        ShowHelp(argv[0], stdout);
        return false;
    }

    std::string error;

    std::string unmatch_error;
    std::string parse_error;

    bool ignore_unknown = false;
    if (HasIgnoreUnknownFlags(argc, argv, &ignore_unknown, parse_error))
    {
        if (ignore_unknown)
            flags |= IgnoreUnknown;
        else
            flags &= ~IgnoreUnknown;
    }

    for (int i = 1; i < argc; ++i)
    {
        const char* name;
        size_t name_length;
        const char* value;

        if (SplitFlag(argc, argv, i, name, name_length, value))
        {
            if (!(flags & NoRemove))
            {
                // remove this flag from args
                memmove(argv + i, argv + i + 1, sizeof(argv[0]) * (argc - i - 1));
                --i;
                --argc;
            }

            if (*name == '\0') // reach '--', all remaining args are not flags
                break;

            int num_matches = TryParseUserFlags(name, name_length, value, parse_error);
            num_matches += ParseBuiltinFlags(
                argv[0], name, name_length, value, parse_error, unmatch_error, flags);

            if (num_matches == 0 && ((flags & IgnoreUnknown) == 0))
            {
                unmatch_error += "    --";
                unmatch_error += name;
                unmatch_error += "\n";
            }
        }
    }

    std::string checked_error;
    CheckFlagErrors(checked_error);

    if (!unmatch_error.empty() || !parse_error.empty() || !checked_error.empty())
    {
        error_message = "Command error:\n";
        error_message += JoinErrorMessages(unmatch_error, parse_error, checked_error);
        error_message += '\n';
    }

    return error_message.empty();
}

inline int FlagList::CheckFlagErrors(std::string& checked_error)
{
    int num_errors = 0;
    std::string error;
    for (BasicFlag* flag = First; flag != NULL; flag = flag->Next)
    {
        if (flag->CheckError(error))
        {
            checked_error += "    '--";
            checked_error += flag->Name();
            checked_error += "' error: ";
            checked_error += error;
            checked_error += '\n';
            ++num_errors;
        }
    }
    return num_errors;
}

inline int FlagList::TryParseUserFlags(
    const char* name,
    size_t name_length,
    const char* value, std::string& error_message
    )
{
    int num_matches = 0;
    std::string error;
    for (BasicFlag* flag = First; flag != NULL; flag = flag->Next)
    {
        if (NameMatch(name, name_length, flag->m_Name))
        {
            if (!flag->Parse(value, error))
            {
                error_message += "    ";
                error_message += "'--";
                error_message += flag->Name();
                error_message += "' ";
                error_message += error;
                error_message += ", value='";
                error_message += value;
                error_message += '\'';
                error_message += '\n';
            }
            ++num_matches;
        }
    }
    return num_matches;
}

inline std::string JoinErrorMessages(
    const std::string& unmatch_error,
    const std::string& parse_error,
    const std::string& checked_error
)
{
    std::string error_message;

    if (!unmatch_error.empty())
    {
        error_message += "  Unknown flag(s):\n";
        error_message += unmatch_error;
    }

    if (!parse_error.empty())
    {
        error_message += "  Parse error(s):\n";
        error_message += parse_error;
    }

    if (!checked_error.empty())
    {
        error_message += "  Checked error(s):\n";
        error_message += checked_error;
    }

    return error_message;
}

inline void FlagList::RegisterFlag(BasicFlag* flag)
{
    if (First == NULL)
        First = flag;

    if (Last != NULL)
        Last->Next = flag;
    Last = flag;

    flag->Next = NULL;
}

inline bool FlagList::NameMatch(
    const char* name,
    size_t name_length,
    const char* pattern
    )
{
    if (name_length > 0)
    {
        return strncmp(name, pattern, name_length) == 0 &&
            pattern[name_length] == '\0';
    }
    else
    {
        return strcmp(name, pattern) == 0;
    }
}

inline bool FlagList::SplitFlagLine(
    const char* line,
    const char*& name, size_t& name_length,
    const char*& value,
    std::string& error_message
)
{
    if (line[0] == '#') // comment
        return false;

    if (line[0] == '-' && line[1] == '-') // '--'
    {
        name = line + 2;

        value = strchr(line + 2, '=');
        name_length = 0;
        if (value)
        {
            name_length = value - name;
            ++value; // skip '='
        }
        else
        {
            value = "";
            // try to convert 'no' prefixed flag name to flag=false
            if (name[0] == 'n' && name[1] == 'o') // prefixed by 'no'
            {
                name += 2;
                value = "false";
            }
        }
        return true;
    }

    error_message = "unknow flags: ";
    error_message += line;
    error_message += "\n";
    return false;
}

inline bool FlagList::SplitFlag(
    int argc, char** argv,
    int index,
    const char*& name, size_t& name_length,
    const char*& value
    )
{
    (void)argc;

    char* arg = argv[index];

    if (arg[0] == '-' && arg[1] == '-')
    {
        name = arg + 2; // skip '--'
        name_length = 0;

        value = strchr(name, '=');
        if (value)
        {
            name_length = value - name;
            ++value; // skip '='
        }
        else
        {   // has no '=' in flag
            value = "";
        }

        // flag found
        return true;
    }

    // not a flag, just ignore
    return false;
}

inline void FlagList::ShowHelp(const char* app_name, FILE* stream)
{
    // print help header
    fprintf(stream, "Usage: %s <flags> ...\n\n", app_name);

    // show each flags's help
    BasicFlag* flag = First;
    if (flag)
    {
        fprintf(stream, "Acceptable flags:\n");
        for (; flag != NULL; flag = flag->Next)
        {
            fprintf(stream,
                    "    --%-16s %s, defined in %s.\n",
                    flag->Name(), flag->GetHelp().c_str(), flag->GetFile()
            );
        }
        fputc('\n', stream);
    }

    ShowBuiltinFlagHelps(stream);
}

inline int FlagList::ParseBuiltinFlags(
    const char* app_name,
    const char* name, size_t name_length,
    const char* value,
    std::string& parse_error,
    std::string& unmatch_error,
    int flags
)
{
    if (NameMatch(name, name_length, "help"))
    {
        ShowHelp(app_name, stderr);
        return 1;
    }
    else if (NameMatch(name, name_length, "flagfile"))
    {
        ParseFlagsFile(value, parse_error, unmatch_error, flags);
        return 1;
    }
    else if (NameMatch(name, name_length, "ignore_unknown") ||
             NameMatch(name, name_length, "ignore-unknown"))
    {
        // already parsed, ignore
        return 1;
    }

    // ignore unknown flags
    return 0;
}

inline bool FlagList::ParseFlagsFile(
    const char* filename,
    std::string& parse_error,
    std::string& unmatch_error,
    int flags
    )
{
    FILE* fp = fopen(filename, "r");
    if (fp)
    {
        char line[1024];
        while (fgets(line, sizeof(line), fp))
        {
            // remove CR | LF
            char* end = strpbrk(line, "\r\n");
            if (end)
                *end = '\0';

            // trim tailing spaces
            size_t length = strlen(line);
            for (size_t i = length; i > 0; --i)
            {
                if (isspace(static_cast<unsigned char>(line[i-1])))
                    line[i-1] = '\0';
                else
                    break;
            }

            if (line[0] != '\0')
            {
                const char* name;
                size_t name_length;
                const char* value;

                if (SplitFlagLine(line, name, name_length, value, parse_error))
                {
                    // reach alone '--'
                    if (name[0] == '\0' || isspace(static_cast<unsigned char>(name[0])))
                        break;

                    if (TryParseUserFlags(name, name_length, value, parse_error) == 0 &&
                        !(flags & IgnoreUnknown)
                        )
                    {
                        unmatch_error += "    --";
                        unmatch_error += name;
                        unmatch_error += "\n";
                    }
                }
            }
        }
        fclose(fp);
    }
    else
    {
        int error = errno;
        parse_error += "can't open flags file: ";
        parse_error += filename;
        parse_error += ", ";
        parse_error += strerror(error);
    }
    return true;
}

inline void FlagList::DumpValues(FILE* stream)
{
    /// sample result:
    /// Current flag values:
    ///     --name=chen3feng (default)
    ///     --size=1024
    ///     --size=<not initialized>

    fprintf(stream, "Current flag values:\n");
    for (BasicFlag* flag = First; flag != NULL; flag = flag->Next)
    {
        fprintf(stderr, "    --%s=%s\n",
                flag->Name(), flag->DumpValue().c_str());
    }
    fprintf(stderr, "\n");
}

inline void FlagList::ShowBuiltinFlagHelps(FILE* stream)
{
    fprintf(stream, "Acceptable builtin flags:\n");
    fprintf(stream, "    --help                        show help\n");
    fprintf(stream, "    --flagfile=filename           load flags from file\n");
    fprintf(stream, "    --ignore_unknown[=bool_value] ignore unknown flags\n");
    fputc('\n', stream);
}

/// show help information for this application
/// @brief app_name application's name, should be argv[0] typicaly
/// @stream output target
inline void ShowHelp(const char* app_name, FILE* stream)
{
    return cflags::FlagList::Instance().ShowHelp(app_name, stream);
}

/// parse command line
/// @param argc argc of function 'main'
/// @param argc argv of function 'main'
/// @param flags parse bitmask flags, may be zero or ored DumpError or or IgnoreUnknown
/// @param error_message when fail, receive generated error message
/// @return true on success and false on error
inline bool ParseCommandLine(int& argc, char** argv, int flags = 0)
{
    std::string error_message;

    if (cflags::FlagList::Instance().ParseCommandLine(
            argc, argv, flags, error_message))
    {
        return true;
    }

    if (!(flags & Silent) && !error_message.empty())
    {
        fwrite(error_message.data(), error_message.length(), 1, stderr);
    }

    return false;
}

/// parse command line
/// @param argc argc of function 'main'
/// @param argc argv of function 'main'
/// @param flags parse bitmask flags, may be zero or ored DumpError or or IgnoreUnknown
/// @param error_message when fail, receive generated error message
/// @return true on success and false on error
inline bool ParseCommandLine(int* argc, char*** argv, int flags = 0)
{
    return ParseCommandLine(*argc, *argv, flags);
}

/// parse flags file
/// @brief app_name application's name, should be argv[0] typicaly
/// @param filename name of the flags file
/// @param error_message when fail, receive generated error message
/// @param flags same as flags of ParseCommandLine
/// @return true on success and false on error
inline bool ParseFlagsFile(const char* app_name, const char* filename, int flags = 0)
{
    (void)app_name;

    std::string error_message;

    std::string parse_error, unmatch_error;
    if (cflags::FlagList::Instance().ParseFlagsFile(
            filename, parse_error, unmatch_error, flags))
    {
        return true;
    }

    std::string checked_error;
    FlagList::Instance().CheckFlagErrors(checked_error);

    if (!parse_error.empty() || !unmatch_error.empty() || !checked_error.empty())
    {
        error_message = "Command error:\n";
        error_message += JoinErrorMessages(unmatch_error, parse_error, checked_error);
        error_message += '\n';
    }

    if ((flags & Silent) == 0)
    {
        fwrite(error_message.data(), error_message.length(), 1, stderr);
    }

    return false;
}

/// dump all values into stream
/// @param stream dump target
inline void DumpValues(FILE* stream)
{
    return cflags::FlagList::Instance().DumpValues(stream);
}

} // end namespace cflags

// define in namespace make cflags can only be defined in namespace scope
#define CFLAGS_DEFINE_FLAG(type, name, ...) \
namespace user_cflags { \
    cflags::Flag< type > Flag_##name(__FILE__, #name, ##__VA_ARGS__); \
} \
using user_cflags::Flag_##name

#define CFLAGS_DECLARE_FLAG(type, name) \
namespace user_cflags { \
    extern cflags::Flag< type > Flag_##name; \
} \
using user_cflags::Flag_##name

// deprecated
namespace CFlags = cflags;

#endif // CFLAGS_HPP_INCLUDED
