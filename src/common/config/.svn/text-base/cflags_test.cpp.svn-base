#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "common/config/cflags.hpp"

///////////////////////////////////////////////////////////////////////////////
// Flag 值可以存储在内部，也可以绑定到外部变量

/// 值存储在 Flag 内
CFLAGS_DEFINE_FLAG(bool, create, "whether create");

/// 绑定到外部变量，值存储在外部的变量里
bool demo = true;
CFLAGS_DEFINE_FLAG(bool, demo, cflags::BindTo(&demo), "whether demo mode");

/// 绑定到外部变量，值存储在外部的变量里
unsigned long number = 0;
CFLAGS_DEFINE_FLAG(unsigned long, number, cflags::BindTo(&number), "number of xxx", cflags::NoDefault);

/// 绑定到外部变量，值存储在外部的变量里
std::string username = "chen3feng";
CFLAGS_DEFINE_FLAG(std::string, username, cflags::BindTo(&username), "username of xxx");

/// 指定默认值
CFLAGS_DEFINE_FLAG(bool, delete, cflags::DefaultIs(false), "whether delete");

/// 绑定到外部变量，值存储在外部的变量里
/// 可以把 overwrite 的声明放到头文件里，Flag_overwrite 的定义只保留在实现里
bool overwrite = false;
CFLAGS_DEFINE_FLAG(bool, overwrite, cflags::BindTo(&overwrite), "whether overwrite");

/// double 类型的配置项
/// 当然，PI 是常数，是无法配置的，这里仅为演示
CFLAGS_DEFINE_FLAG(double, PI, cflags::DefaultIs(3.14159265358979323846), "PI");

#include <complex>

/// 演示数据类型扩充：支持复数类型
namespace cflags
{
template <typename T>
struct FlagHandler<std::complex<T> >
{
public:
    static bool Parse(const char* value, std::complex<T>* result, std::string& error_message)
    {
        double real = 0, image = 0;
        if (sscanf(value, "%lf+%lfi", &real, &image) == 2 || // 数学格式
            sscanf(value, "%lf+%lfj", &real, &image) == 2 || // 电工格式
            sscanf(value, "(%lf,%lf)", &real, &image) == 2 || // 民工格式
            sscanf(value, "%lf", &real) == 1 ||
            sscanf(value, "%lfi", &image) == 1 ||
            sscanf(value, "%lfj", &image) == 1
        )
        {
            *result = std::complex<T>(real, image);
            return true;
        }

        error_message = "invalod complex format";
        return false;
    }
    static std::string ToString(const std::complex<T>* value)
    {
        char buffer[64];
        return std::string(buffer, sprintf(buffer, "%lg+%lgi", value->real(), value->imag()));
    }
};
}

/// 复数类型配置项
CFLAGS_DEFINE_FLAG(std::complex<double>, complex, "a simple complex demo");

/// 使用宏的方式定义 Flag
CFLAGS_DEFINE_FLAG(
    std::complex<double>,
    complex2,
    "another simple complex demo"
);

/// 可以放到头文件中
CFLAGS_DECLARE_FLAG(std::complex<double>, complex2);

// 校验 listen 端口范围是否在非 root 程序可 listen 的有效范围之内
bool ValidateListenPort(const unsigned short* port, std::string& error_message)
{
    // 1024 以下为特权端口，非 root 程序不能 listen
    // 32768-61000 为 Linux 系统动态分配范围，可能冲突。
    if ((*port > 1024 && *port < 32768) || *port > 61000)
        return true;
    error_message = "invalid listen port value, should between (1024, 32768) or (61000, 65536)";
    return false;
}

CFLAGS_DEFINE_FLAG(unsigned short, port, "port number", 0, ValidateListenPort);

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        cflags::ShowHelp(argv[0], stdout);
        return EXIT_SUCCESS;
    }

    // 先输出默认值
    cflags::DumpValues(stdout);

    if (cflags::ParseCommandLine(argc, argv))
    {
        // 解析成功后再输出一次
        cflags::DumpValues(stdout);
        for (int i = 0; i < argc; ++i)
        {
            printf("arg[%d]=%s\n", i, argv[i]);
        }
    }
    else
    {
        return EXIT_FAILURE;
    }
}
