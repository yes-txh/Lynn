/**
 * 静态文件工具函数, 以类的形式提供，实质是独立的C函数
 * @author jerryzhang@tencent.com
 * @since  2006.04.12
 */

#ifndef     __FILE_UTILITY_H__
#define     __FILE_UTILITY_H__

#ifdef __DEPRECATED
#warning "This file is deprecated and will be removed. please using common/system/file.hpp and directory.hpp"
#endif

#include <string>
#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>

// 最大文件名的长度
#define MAX_FILE_NAME_LENGTH   1024

#ifndef _WIN32
    #define MAXFDS 1000000
#endif

// 复制文件的时候是否覆盖写
#define FILECOPY_OVERWRITE_ON  1
#define FILECOPY_OVERWRITE_OFF 2

class FileUtility
{
public:

    /////////////////////////////////////////////////////////////////
    // 下面是文件操作

    /// 功能：读文本文件（不要用它读2进制文件）
    static std::string ReadTextFile(const char *pcFileName);

    /// 功能：读文本文件（不要用它读2进制文件）
    static std::string ReadTextFile(std::string strFileName);

    /// 功能：读二进制文件
    static bool ReadBinFile(char* pcContent, int& dwReadLen, const char *pcFileName);

    /// 功能：写二进制文件
    static bool WriteBinFile(char* pcContent, int dwWriteLen, const char *pcFileName);

    /// 功能：追加方式写文件，支持可变参数，使用类似printf
    static bool WriteFile(const char *pcFileName, const char* fmt, ...);

    /// 功能：追加方式把一个字符串写入文件
    static bool Write(std::string& strFileName, std::string& strValue, bool bPrint = false);

    /// 功能：追加方式把一个整数写入文件
    static bool Write(std::string& strFileName, int dwValue, bool bPrint = false);

    /// 功能：追加方式把一个整数写入文件
    static bool Write(std::string& strFileName, double lfValue, bool bPrint = false);

    /// 功能：复制文件
    static bool Copy(const char *pcDestFile, const char* pcSrcFile, char bFlags = FILECOPY_OVERWRITE_ON);

    /// 功能：删除文件
    static bool Delete(const char* pcFileName);

    /// 功能：文件改名
    static bool Rename(const char *pcNewFile, const char* pcOldFile);

    /// 功能：判断文件是否存在
    static bool IsFileExist(const char *pcFileName);

    /// 功能：一个字符串是不是一个文件名
    static bool IsFile(const char *pcFileName);

    /// 功能：判断文件是否可读
    static bool IsFileReadable(const char *pcFileName);

    /// 功能：判断文件是否可写
    static bool IsFileWritable(const char *pcFileName);

    /// 功能：取得文件大小
    static long long GetFileSize(const char *pcFileName);

    /// 功能：取得文件最后访问时间
    static std::string GetFileAccessTime(const char *pcFileName);

    /// 功能：取得文件创建时间
    static std::string GetFileCreateTime(const char *pcFileName);

    /// 功能：取得文件最后修改时间
    static std::string GetFileLastModifyTime(const char *pcFileName);

    /// 功能：从完整的文件路径名中取得文件名
    static std::string ExtractFileName(char *pcFilePath);

    /// 功能：从文件名中取得扩展名
    static std::string ExtractFileExtName(std::string& strFileName);

    /// 功能：从url中文件扩展名判断是否是html文件
    static bool IsHtmlUrl(std::string& strFileName);

    /// 功能：获得文件所在目录的层次
    static int GetFilePathLevel(const char *pcFilePath);

    /////////////////////////////////////////////////////////////////
    // 下面是目录操作

    /// 功能：判断目录是否存在
    static bool IsDirExist(const char *pcDirName);

    /// 功能：一个字符串是不是一个目录名
    static bool IsDir(const char *pcDirName);

    /// 功能：判断目录是否可读
    static bool IsDirReadable(const char *pcDirName);

    /// 功能：判断目录是否可写
    static bool IsDirWritable(const char *pcDirName);

    /// 功能：创建一个目录
    static bool CreateDir(const char *pcDirName);

    /// 功能：删除一个目录
    static bool DeleteDir(const char *pcDirName);

    /// 功能：设置当前目录
    static bool SetCurrentDir(const char *pcDirName);

    /// 功能：获得当前目录
    static std::string GetCurrentDir(void);

    /// 功能：把文件路径格式从 linux转为windows
    #ifdef _WIN32
    static void MakeWindowsPath(char* pcFilePath);
    #endif

    /// 把windows文件路径分隔符替换成linux格式的，并把结尾多余的分隔符去掉
    static void NormalizePath(char* pcFilePath);

    /////////////////////////////////////////////////////////////////
    // 下面是容器操作

    /// 从文件加载一个set, 格式要求是每行一项
    static bool Load2Set(std::set<std::string>& sDict, const char* pcFileName);

    /// 把一个set输出到一个文件中
    static bool DumpSet(std::set<std::string>& sDict, std::string strFileName);

    /// 从文件加载到一个vector, 格式要求是每行一项
    static bool Load2Vector(std::vector<std::string>& vDict, const char* pcFileName);

    /// 从文件加载一个map, 格式要求是每行都是<key = value>
    static bool Load2Map(std::map<std::string, std::string>& mDict, const char* pcFileName, std::string strSeparator = "=");

    /// 从文件加载一个map, 格式要求是每行都是<key = value>
    static bool Load2Map(std::map<std::string, int>& mDict, const char* pcFileName, std::string strSeparator = "=");

    /// 从文件加载一个map, 格式要求是每行都是<key = value>
    static bool Load2Map(std::map<std::string, double>& mDict, const char* pcFileName, std::string strSeparator = "=");

    /// 把一个map输出到一个文件中
    static bool DumpMap(std::map<std::string, int>& mDict, std::string strFileName, std::string strSeparator = "=");

    /// 把一个map输出到一个文件中
    static bool DumpMap(std::map<std::string, std::string>& mDict, std::string strFileName, std::string strSeparator = "=");

    /// 遍历一个目录，对目录下所有文件执行某个函数
    static bool DirRoamer(const char* szPath, std::vector<std::string>& vFileName);

    // 文件系统的设置
#ifndef _WIN32
    static void SetFileSystemLimit();
#endif
};

#endif
