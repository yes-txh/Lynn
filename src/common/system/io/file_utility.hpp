/**
 * ��̬�ļ����ߺ���, �������ʽ�ṩ��ʵ���Ƕ�����C����
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

// ����ļ����ĳ���
#define MAX_FILE_NAME_LENGTH   1024

#ifndef _WIN32
    #define MAXFDS 1000000
#endif

// �����ļ���ʱ���Ƿ񸲸�д
#define FILECOPY_OVERWRITE_ON  1
#define FILECOPY_OVERWRITE_OFF 2

class FileUtility
{
public:

    /////////////////////////////////////////////////////////////////
    // �������ļ�����

    /// ���ܣ����ı��ļ�����Ҫ������2�����ļ���
    static std::string ReadTextFile(const char *pcFileName);

    /// ���ܣ����ı��ļ�����Ҫ������2�����ļ���
    static std::string ReadTextFile(std::string strFileName);

    /// ���ܣ����������ļ�
    static bool ReadBinFile(char* pcContent, int& dwReadLen, const char *pcFileName);

    /// ���ܣ�д�������ļ�
    static bool WriteBinFile(char* pcContent, int dwWriteLen, const char *pcFileName);

    /// ���ܣ�׷�ӷ�ʽд�ļ���֧�ֿɱ������ʹ������printf
    static bool WriteFile(const char *pcFileName, const char* fmt, ...);

    /// ���ܣ�׷�ӷ�ʽ��һ���ַ���д���ļ�
    static bool Write(std::string& strFileName, std::string& strValue, bool bPrint = false);

    /// ���ܣ�׷�ӷ�ʽ��һ������д���ļ�
    static bool Write(std::string& strFileName, int dwValue, bool bPrint = false);

    /// ���ܣ�׷�ӷ�ʽ��һ������д���ļ�
    static bool Write(std::string& strFileName, double lfValue, bool bPrint = false);

    /// ���ܣ������ļ�
    static bool Copy(const char *pcDestFile, const char* pcSrcFile, char bFlags = FILECOPY_OVERWRITE_ON);

    /// ���ܣ�ɾ���ļ�
    static bool Delete(const char* pcFileName);

    /// ���ܣ��ļ�����
    static bool Rename(const char *pcNewFile, const char* pcOldFile);

    /// ���ܣ��ж��ļ��Ƿ����
    static bool IsFileExist(const char *pcFileName);

    /// ���ܣ�һ���ַ����ǲ���һ���ļ���
    static bool IsFile(const char *pcFileName);

    /// ���ܣ��ж��ļ��Ƿ�ɶ�
    static bool IsFileReadable(const char *pcFileName);

    /// ���ܣ��ж��ļ��Ƿ��д
    static bool IsFileWritable(const char *pcFileName);

    /// ���ܣ�ȡ���ļ���С
    static long long GetFileSize(const char *pcFileName);

    /// ���ܣ�ȡ���ļ�������ʱ��
    static std::string GetFileAccessTime(const char *pcFileName);

    /// ���ܣ�ȡ���ļ�����ʱ��
    static std::string GetFileCreateTime(const char *pcFileName);

    /// ���ܣ�ȡ���ļ�����޸�ʱ��
    static std::string GetFileLastModifyTime(const char *pcFileName);

    /// ���ܣ����������ļ�·������ȡ���ļ���
    static std::string ExtractFileName(char *pcFilePath);

    /// ���ܣ����ļ�����ȡ����չ��
    static std::string ExtractFileExtName(std::string& strFileName);

    /// ���ܣ���url���ļ���չ���ж��Ƿ���html�ļ�
    static bool IsHtmlUrl(std::string& strFileName);

    /// ���ܣ�����ļ�����Ŀ¼�Ĳ��
    static int GetFilePathLevel(const char *pcFilePath);

    /////////////////////////////////////////////////////////////////
    // ������Ŀ¼����

    /// ���ܣ��ж�Ŀ¼�Ƿ����
    static bool IsDirExist(const char *pcDirName);

    /// ���ܣ�һ���ַ����ǲ���һ��Ŀ¼��
    static bool IsDir(const char *pcDirName);

    /// ���ܣ��ж�Ŀ¼�Ƿ�ɶ�
    static bool IsDirReadable(const char *pcDirName);

    /// ���ܣ��ж�Ŀ¼�Ƿ��д
    static bool IsDirWritable(const char *pcDirName);

    /// ���ܣ�����һ��Ŀ¼
    static bool CreateDir(const char *pcDirName);

    /// ���ܣ�ɾ��һ��Ŀ¼
    static bool DeleteDir(const char *pcDirName);

    /// ���ܣ����õ�ǰĿ¼
    static bool SetCurrentDir(const char *pcDirName);

    /// ���ܣ���õ�ǰĿ¼
    static std::string GetCurrentDir(void);

    /// ���ܣ����ļ�·����ʽ�� linuxתΪwindows
    #ifdef _WIN32
    static void MakeWindowsPath(char* pcFilePath);
    #endif

    /// ��windows�ļ�·���ָ����滻��linux��ʽ�ģ����ѽ�β����ķָ���ȥ��
    static void NormalizePath(char* pcFilePath);

    /////////////////////////////////////////////////////////////////
    // ��������������

    /// ���ļ�����һ��set, ��ʽҪ����ÿ��һ��
    static bool Load2Set(std::set<std::string>& sDict, const char* pcFileName);

    /// ��һ��set�����һ���ļ���
    static bool DumpSet(std::set<std::string>& sDict, std::string strFileName);

    /// ���ļ����ص�һ��vector, ��ʽҪ����ÿ��һ��
    static bool Load2Vector(std::vector<std::string>& vDict, const char* pcFileName);

    /// ���ļ�����һ��map, ��ʽҪ����ÿ�ж���<key = value>
    static bool Load2Map(std::map<std::string, std::string>& mDict, const char* pcFileName, std::string strSeparator = "=");

    /// ���ļ�����һ��map, ��ʽҪ����ÿ�ж���<key = value>
    static bool Load2Map(std::map<std::string, int>& mDict, const char* pcFileName, std::string strSeparator = "=");

    /// ���ļ�����һ��map, ��ʽҪ����ÿ�ж���<key = value>
    static bool Load2Map(std::map<std::string, double>& mDict, const char* pcFileName, std::string strSeparator = "=");

    /// ��һ��map�����һ���ļ���
    static bool DumpMap(std::map<std::string, int>& mDict, std::string strFileName, std::string strSeparator = "=");

    /// ��һ��map�����һ���ļ���
    static bool DumpMap(std::map<std::string, std::string>& mDict, std::string strFileName, std::string strSeparator = "=");

    /// ����һ��Ŀ¼����Ŀ¼�������ļ�ִ��ĳ������
    static bool DirRoamer(const char* szPath, std::vector<std::string>& vFileName);

    // �ļ�ϵͳ������
#ifndef _WIN32
    static void SetFileSystemLimit();
#endif
};

#endif
