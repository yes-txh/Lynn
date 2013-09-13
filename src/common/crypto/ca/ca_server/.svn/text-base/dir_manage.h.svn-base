//////////////////////////////////////////////////////////////////////////
// @file:   dir_manage
// @brief:  一些对目录和文件的建立和删除的功能
// @author: joeytian@tencent.com
// @time:   2010-11-24
// 修改历史:
//          <author>    <time>
//////////////////////////////////////////////////////////////////////////
#ifndef COMMON_CRYPTO_CA_CA_SERVER_DIR_MANAGE_H_
#define COMMON_CRYPTO_CA_CA_SERVER_DIR_MANAGE_H_

#ifdef WIN32
#include <stdlib.h>
#include <direct.h>
#include <io.h>
#include <sys/stat.h>

// linux和windows上文件API不同
#define stat _stat
#define access _access
#define R_OK 04
#define W_OK 02
#define S_IFREG _S_IFREG
#define S_IFDIR _S_IFDIR
#else
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/resource.h>
#endif
#include <stdio.h>
#include <string>
#include <fcntl.h>
#include "common/crypto/ca/ca_public/ca_struct.h"

namespace ca {

class DirManage {
public:
    DirManage();
    ~DirManage();

    bool MkDir(const char* dir_name);
    bool IsDir(const char* dir_name);
    bool IsFile(const char* file_name);
    bool IsDirExist(const char* dir_name);
    bool IsFileExist(const char* file_name);
    bool IsFileReadable(const char *file_name);
    bool RecursivelyRmDir(const char* path);

    // 这个函数从common库里提取出来并修改，读取文件时，增加了按指定的delim来分割每一行
    //std::string ReadTextFile(const char *text_file_name, const char* delim = "\r\n");

    // 功能：追加方式写文件，支持可变参数，使用类似printf
    // 这个函数从common库里file_utility.h提取出来，该头文件即将被删除
    bool WriteFile(const char *file_name, const char* fmt, ...);

    // 功能：读文本文件（不要用它读2进制文件）
    // 这个函数从common库里file_utility.h提取出来，该头文件即将被删除
    std::string ReadTextFile(const char *file_name);

    // 功能：读文本文件（不要用它读2进制文件）
    // 可指定读取文件的第几行到第几行, 并可以返回文件的总行数和成功读取的行数
    // got_line_num: 成功读取的行数
    // total_line_num: 文件的总行数
    std::string ReadTextFile(const char *file_name, int32_t start_line_num, int32_t end_line_num,
                             int32_t* got_line_num, int32_t* total_line_num);
};

} // namespace ca
#endif // COMMON_CRYPTO_CA_CA_SERVER_DIR_MANAGE_H_
