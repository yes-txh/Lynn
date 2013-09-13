#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "common/config/cflags.hpp"

///////////////////////////////////////////////////////////////////////////////
// Flag ֵ���Դ洢���ڲ���Ҳ���԰󶨵��ⲿ����

/// ֵ�洢�� Flag ��
CFLAGS_DEFINE_FLAG(bool, create, "whether create");

/// �󶨵��ⲿ������ֵ�洢���ⲿ�ı�����
bool demo = true;
CFLAGS_DEFINE_FLAG(bool, demo, cflags::BindTo(&demo), "whether demo mode");

/// �󶨵��ⲿ������ֵ�洢���ⲿ�ı�����
unsigned long number = 0;
CFLAGS_DEFINE_FLAG(unsigned long, number, cflags::BindTo(&number), "number of xxx", cflags::NoDefault);

/// �󶨵��ⲿ������ֵ�洢���ⲿ�ı�����
std::string username = "chen3feng";
CFLAGS_DEFINE_FLAG(std::string, username, cflags::BindTo(&username), "username of xxx");

/// ָ��Ĭ��ֵ
CFLAGS_DEFINE_FLAG(bool, delete, cflags::DefaultIs(false), "whether delete");

/// �󶨵��ⲿ������ֵ�洢���ⲿ�ı�����
/// ���԰� overwrite �������ŵ�ͷ�ļ��Flag_overwrite �Ķ���ֻ������ʵ����
bool overwrite = false;
CFLAGS_DEFINE_FLAG(bool, overwrite, cflags::BindTo(&overwrite), "whether overwrite");

/// double ���͵�������
/// ��Ȼ��PI �ǳ��������޷����õģ������Ϊ��ʾ
CFLAGS_DEFINE_FLAG(double, PI, cflags::DefaultIs(3.14159265358979323846), "PI");

#include <complex>

/// ��ʾ�����������䣺֧�ָ�������
namespace cflags
{
template <typename T>
struct FlagHandler<std::complex<T> >
{
public:
    static bool Parse(const char* value, std::complex<T>* result, std::string& error_message)
    {
        double real = 0, image = 0;
        if (sscanf(value, "%lf+%lfi", &real, &image) == 2 || // ��ѧ��ʽ
            sscanf(value, "%lf+%lfj", &real, &image) == 2 || // �繤��ʽ
            sscanf(value, "(%lf,%lf)", &real, &image) == 2 || // �񹤸�ʽ
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

/// ��������������
CFLAGS_DEFINE_FLAG(std::complex<double>, complex, "a simple complex demo");

/// ʹ�ú�ķ�ʽ���� Flag
CFLAGS_DEFINE_FLAG(
    std::complex<double>,
    complex2,
    "another simple complex demo"
);

/// ���Էŵ�ͷ�ļ���
CFLAGS_DECLARE_FLAG(std::complex<double>, complex2);

// У�� listen �˿ڷ�Χ�Ƿ��ڷ� root ����� listen ����Ч��Χ֮��
bool ValidateListenPort(const unsigned short* port, std::string& error_message)
{
    // 1024 ����Ϊ��Ȩ�˿ڣ��� root ������ listen
    // 32768-61000 Ϊ Linux ϵͳ��̬���䷶Χ�����ܳ�ͻ��
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

    // �����Ĭ��ֵ
    cflags::DumpValues(stdout);

    if (cflags::ParseCommandLine(argc, argv))
    {
        // �����ɹ��������һ��
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
