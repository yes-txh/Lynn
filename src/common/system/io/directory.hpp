#ifndef COMMON_SYSTEM_IO_DIRECTORY_H
#define COMMON_SYSTEM_IO_DIRECTORY_H

/// @brief direcotry operations.
/// @author hsiaokangliu
/// @author 2010-11-20

#include <string>
#include <vector>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#endif


namespace io
{

#define MAX_PATH_LEN 1024 * 10

namespace directory
{
    bool Exists(const char* dir);
    bool Exists(const std::string& dir);
    bool IsDir(const char* path);
    bool IsDir(const std::string& dir);
    bool IsReadable(const char* dir);
    bool IsReadable(const std::string& dir);
    bool IsWritable(const char* dir);
    bool IsWritable(const std::string& dir);

    bool Create(const char* dir, int mode = 0755);
    bool Create(const std::string& dir, int mode = 0755);
    bool Delete(const char* dir);
    bool Delete(const std::string& dir);

    bool SetCurrentDir(const char* dir);
    bool SetCurrentDir(const std::string& dir);
    std::string GetCurrentDir();

    std::string ToWindowsFormat(const char* dir);
    std::string ToWindowsFormat(const std::string& dir);
    std::string ToUnixFormat(const char* dir);
    std::string ToUnixFormat(const std::string& dir);

    time_t GetAccessTime(const char* dir);
    time_t GetAccessTime(const std::string& dir);
    time_t GetCreateTime(const char* dir);
    time_t GetCreateTime(const std::string& dir);
    time_t GetLastModifyTime(const char* dir);
    time_t GetLastModifyTime(const std::string& dir);

    bool GetFiles(const char* dir, std::vector<std::string>* files);
    bool GetFiles(const std::string& dir, std::vector<std::string>* files);
    bool GetAllFiles(const char* dir, std::vector<std::string>* files);
    bool GetAllFiles(const std::string& dir, std::vector<std::string>* files);

    bool GetSubDirs(const char* dir, std::vector<std::string>* subdirs);
    bool GetSubDirs(const std::string& dir, std::vector<std::string>* subdirs);
    bool GetAllSubDirs(const char* dir, std::vector<std::string>* subdirs);
    bool GetAllSubDirs(const std::string& dir, std::vector<std::string>* subdirs);

    bool RecursiveDelete(const char* dir);
    bool RecursiveDelete(const std::string& dir);
} // namespace io::directory

class DirectoryIterator
{
public:
    enum Flags
    {
        INVALID = 0x00,
        FILE = 0x01,
        DIRECTORY = 0x02,
        SYMBOLLINK = 0x04,
        ALL = FILE | DIRECTORY | SYMBOLLINK,
    };
public:
    DirectoryIterator(
        const char* dir, ///< 目录名
        int flags = ALL, ///< 类型过滤
        const char* filter = NULL /// 通配符过滤
        );
    DirectoryIterator();
    ~DirectoryIterator()
    {
        Close();
    }

    bool Open(
        const char* dir, ///< 目录名
        int flags = ALL, ///< 类型过滤
        const char* filter = NULL /// 通配符过滤
        );

    bool Open(
        const std::string& dir, ///< 目录名
        int flags = ALL,        ///< 类型过滤
        const char* filter = NULL ///通配符过滤
        );

    std::string Name() const;
    std::string FullPath();
    bool Next();
    bool Close();
    bool IsEnd();
    int  GetType() const;
private:
    void Initialize();
    bool SkipCurrent();
    DirectoryIterator(const DirectoryIterator&);
    DirectoryIterator& operator = (const DirectoryIterator&);
private:
#ifdef _WIN32
    WIN32_FIND_DATA m_FindFileData;
    HANDLE m_Handle;
#else
    DIR* m_Dir;
    struct dirent* m_Dirent;
#endif
    bool m_End;
    int  m_Flags;
    const char* m_OpenDir;
    const char* m_FilterPattern;
}; // class DirectoryIterator

} // namespace io

#endif
