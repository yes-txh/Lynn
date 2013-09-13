#include <iostream>
#include <fstream>

#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/crypto/ca/ca_server/dir_manage.h"
#include "common/base/string/string_algorithm.hpp"
#include "common/system/io/file.hpp"

namespace ca {

DirManage::DirManage() {
}

DirManage::~DirManage() {
}

bool DirManage::IsDir(const char* dir_name) {
    if (!dir_name)
        return false;

    struct stat buf;

    if (!(stat(dir_name, &buf) == 0))
        return false;

    return ((buf.st_mode & S_IFDIR) == S_IFDIR);
}

bool DirManage::IsDirExist(const char* dir_name) {
    return (IsDir(dir_name));
}


bool DirManage::IsFile(const char* file_name) {
    if (!file_name)
        return false;
    struct stat buf;

    if (!(stat(file_name, &buf) == 0))
        return false;

    return ((buf.st_mode & S_IFREG) == S_IFREG);
}

bool DirManage::IsFileExist(const char* file_name) {
    return IsFile(file_name);
}


bool DirManage::IsFileReadable(const char *file_name)
{
    if (!IsFileExist(file_name))
        return false;
    return access(file_name, R_OK) == 0;
}


bool DirManage::MkDir(const char* dir_name) {
#ifdef WIN32
    return (_mkdir(dir_name) == 0);
#else
    return (mkdir(dir_name, 0777) == 0);
#endif
}


bool DirManage::RecursivelyRmDir(const char* path) {
    if (!path)
        return false;

#ifndef WIN32
    if (IsDir(path)) {
        DIR* dir = opendir(path);
        struct dirent* dir_info = NULL;
        char dir_path[MAX_PATH] = {0};

        if (dir) {
            while ((dir_info = readdir(dir))) {
                if (   strcmp(dir_info->d_name, ".") ==  0
                        || strcmp(dir_info->d_name, "..") == 0)
                    continue;

                safe_snprintf(dir_path, sizeof(dir_path), "%s/%s", path, dir_info->d_name);
                RecursivelyRmDir(dir_path);
            }

            closedir(dir);
        } else
            return false;

        rmdir(path);
    } else
        unlink(path);

    return true;
#else
    char cmd[MAX_PATH];
    safe_snprintf(cmd, sizeof(cmd), "rmdir %s /S /Q", path);
    int32_t ret_system = (system(cmd));
    return (0 == ret_system) ? true : false;
#endif
}

// 功能：追加方式写文本文件，支持可变参数，使用类似printf
// 说明：只能用来写小块数据(< 16K)
// 示例：FileUtility::WriteFile("/data/1.dat", "%s\t%d\n", szName, dwIndex);
bool DirManage::WriteFile(const char *file_name, const char* fmt, ...)
{
    FILE *fp = fopen(file_name, "at");
    if (fp != NULL)
    {
        char temp_buf[1024*64];
        int dwPos = 0;

        memset(temp_buf, 0, sizeof(temp_buf));

        // 把内容先写到缓冲区
        va_list ap;
        va_start(ap, fmt);
#ifndef _WIN32
        vsnprintf(temp_buf + dwPos, sizeof(temp_buf) - dwPos, fmt, ap);
#else
        _vsnprintf(temp_buf + dwPos, sizeof(temp_buf) - dwPos, fmt, ap);
#endif
        va_end(ap);

        // 把缓冲数据写入到文件
        fprintf(fp, "%s", temp_buf);
        fclose(fp);
        return true;
    }
    else
    {
#ifdef _DEBUG
        fprintf(stdout, "open file : %s failed", file_name);
#endif
        return false;
    }
}

// 功能：读文本文件（不要用它读2进制文件）
std::string DirManage::ReadTextFile(const char *file_name)
{
    std::string str_file_name(file_name);
    StringTrim(&str_file_name);

    // 首先检查文件是否有读权限
     if (!IsFileReadable(str_file_name.c_str()))
         return "";


    std::ifstream fs;
    fs.open(str_file_name.c_str(), std::ios_base::in);

    if (fs.fail())
    {
        // std::cout << " The file isn't exist. " << file_name << std::endl;
        return "";
    }

    std::string str_document, str_line;

    while (getline(fs, str_line))
        str_document += str_line + "\n";

    fs.close();

    return str_document;
}


// 功能：读文本文件（不要用它读2进制文件）
// 可指定读取文件的第几行到第几行, 并可以返回文件的总行数
std::string DirManage::ReadTextFile(const char *file_name, int32_t start_line_num,
                                    int32_t end_line_num, int32_t* got_line_num,
                                    int32_t* total_line_num) {
    if (start_line_num > end_line_num)
        return "";
    if (!file_name || !got_line_num || !total_line_num)
        return "";

    std::string str_file_name(file_name);
    StringTrim(&str_file_name);

    // 首先检查文件是否有读权限
    if (!IsFileReadable(str_file_name.c_str()))
        return "";

    std::ifstream fs;
    fs.open(str_file_name.c_str(), std::ios_base::in);

    if (fs.fail())
    {
        return "";
    }

    std::string str_document, str_line;
    int32_t current_line_num = 0;

    while (getline(fs, str_line)) {
        ++current_line_num;
        if (current_line_num >= start_line_num && current_line_num <= end_line_num) {
            str_document += str_line + "\n";
            ++(*got_line_num);
        }
    }
    *total_line_num = current_line_num;
    fs.close();

    return str_document;
}
} // namespace ca
