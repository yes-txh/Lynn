#ifndef _CRT_NONSTDC_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

/**
 * 文件：FileUtility.cpp
 * 功能：公用的文件工具函数
 * @author jerryzhang@tencent.com
 * @since  2006.04.12
 */

#undef __DEPRECATED
#include <common/system/io/file_utility.hpp>

#include <time.h>
#include <common/base/compatible/stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <common/base/common_windows.h>
#include <direct.h>
#include <io.h>

// linux和windows上文件API不同
#ifndef stat
#define stat _stat
#endif
#define access _access
#define R_OK 04
#define W_OK 02
#define S_IFREG _S_IFREG
#define S_IFDIR _S_IFDIR
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/resource.h>
#endif

#include <common/base/string/string_number.hpp>
#include <common/base/string/string_algorithm.hpp>

/** 读文件 */
std::string FileUtility::ReadTextFile(const char *pcFileName)
{
    std::string strFileName(pcFileName);
    StringTrim(&strFileName);

    // 首先检查文件是否有读权限
    if (!IsFileReadable(strFileName.c_str()))
        return "";

    std::ifstream fs;
    fs.open(strFileName.c_str(), std::ios_base::in);

    if (fs.fail())
    {
        // std::cout << " The file isn't exist. " << pcFileName << std::endl;
        return "";
    }

    std::string strDocument, strLine;

    while (getline(fs, strLine))
        strDocument += strLine + "\r\n";

    fs.close();

    return strDocument;
}

/** 读文件 */
std::string FileUtility::ReadTextFile(std::string strFileName)
{
    return ReadTextFile(strFileName.c_str());
}

/** 功能：读二进制文件, 不适合巨大的需要分段读取的文件 */
bool FileUtility::ReadBinFile(char* pcContent, int& dwReadLen, const char *pcFileName)
{
    if (NULL == pcContent || NULL == pcFileName)
        return false;

    dwReadLen = 0;

    // 取得文件的大小
    long long dwFileLen = GetFileSize(pcFileName);
    if (dwFileLen <= 0)
        return false;

    int  dwFd;

#ifdef _WIN32
    dwFd = _open(pcFileName,  O_RDONLY | _O_BINARY);
#else
    dwFd = open(pcFileName, O_RDONLY);
#endif
    if (-1 == dwFd)
    {
        // std::cout << "open file failed : " << pcFileName << std::endl;
        return false;
    }

    // 这里用循环来读是因为读大文件的时候该函数可能被信号中断
    int dwReadBytes = 0;
    while (dwReadLen < dwFileLen)
    {
        dwReadBytes = read(dwFd, pcContent+dwReadLen, (size_t)(dwFileLen-dwReadLen));
        if (dwReadBytes > 0)
            dwReadLen += dwReadBytes;
        else
            break;
    }

    close(dwFd);
    if (dwReadLen < dwFileLen)
        return false;
    else
        return true;
}

/** 功能：写二进制文件 */
bool FileUtility::WriteBinFile(char* pcContent, int dwWriteLen, const char *pcFileName)
{
    int dwFd;
#ifdef _WIN32
    if ((dwFd = _open(pcFileName, _O_BINARY | O_CREAT|O_TRUNC|O_RDWR)) < 0)
#else
    if ((dwFd = open(pcFileName, O_CREAT|O_TRUNC|O_RDWR, 0666)) < 0)
#endif
    {
        return false;
    }
    if (write(dwFd, pcContent, dwWriteLen) < 0)
    {
        close(dwFd);
        return false;
    }
    close(dwFd);
    return true;
}

/** 功能：追加方式写文本文件，支持可变参数，使用类似printf
 *   说明：只能用来写小块数据(< 16K)
 *    示例：FileUtility::WriteFile("/data/1.dat", "%s\t%d\n", szName, dwIndex);
 */
bool FileUtility::WriteFile(const char *pcFileName, const char* fmt, ...)
{
    // 首先检查文件是否有写权限
    // if (!IsFileWritable(pcFileName))
    //  return false;

    FILE *fp = fopen(pcFileName, "at");
    if (fp != NULL)
    {
        char szBuf [1024*64];
        int dwPos = 0;

        memset(szBuf, 0, sizeof(szBuf));

        // 把内容先写到缓冲区
        va_list ap;
        va_start(ap, fmt);
#ifndef _WIN32
        vsnprintf(szBuf + dwPos, sizeof(szBuf) - dwPos, fmt, ap);
#else
        _vsnprintf(szBuf + dwPos, sizeof(szBuf) - dwPos, fmt, ap);
#endif
        va_end(ap);

        // 把缓冲数据写入到文件
        fprintf(fp, "%s\n", szBuf);
        fclose(fp);
        return true;
    }
    else
    {
#ifdef _DEBUG
        fprintf(stdout, "open file : %s failed", pcFileName);
#endif
        return false;
    }
}

/** 功能：追加方式把一个字符串写入文件 */
bool FileUtility::Write(std::string& strFileName, std::string& strValue, bool bPrint)
{
    if (bPrint)
        std::cout << strValue << std::endl;
    return WriteFile(strFileName.c_str(), "%s", strValue.c_str());
}

/** 功能：追加方式把一个整数写入文件 */
bool FileUtility::Write(std::string& strFileName, int dwValue, bool bPrint)
{
    if (bPrint)
        std::cout << dwValue << std::endl;
    return WriteFile(strFileName.c_str(), "%d", dwValue);
}

/** 功能：追加方式把一个整数写入文件 */
bool FileUtility::Write(std::string& strFileName, double lfValue, bool bPrint)
{
    if (bPrint)
        std::cout << lfValue << std::endl;
    return WriteFile(strFileName.c_str(), "%lf", lfValue);
}

/** 功能：复制文件 */
bool FileUtility::Copy(const char *pcDestFile, const char* pcSrcFile, char bFlags)
{
#ifdef _WIN32
    if (bFlags == FILECOPY_OVERWRITE_ON)
    {
        // 覆盖写
        if (TRUE != (CopyFileA(pcSrcFile, pcDestFile , false)))
            return false;
    }
    else if (bFlags == FILECOPY_OVERWRITE_OFF)
    {
        // 不覆盖写
        if (TRUE != (CopyFileA(pcSrcFile, pcDestFile , true)))
            return false;
    }
    else
    {
        return false;
    }

    return true;
#else
    size_t dwRet;
    FILE*  pSrcFile;
    FILE*  pDestFile;
    char   szBuf[1024];

    // 不覆盖写的时候需要检查原文件是否存在
    if (bFlags == FILECOPY_OVERWRITE_OFF)
    {
        if (access(pcDestFile, F_OK) == 0)
        {
            return false;
        }
        else if (errno != ENOENT)
        {
            return false;
        }
    }

    if ((pSrcFile = fopen(pcSrcFile, "r")) == NULL)
    {
        // 源文件不存在
        return false;
    }

    if ((pDestFile = fopen(pcDestFile, "w+")) == NULL)
    {
        // 打开目的文件失败
        fclose(pSrcFile);
        return false;
    }

    while ((dwRet = fread(szBuf, 1, sizeof(szBuf), pSrcFile)) > 0)
    {
        // 把读到写到新文件
        if (fwrite(szBuf, 1, dwRet, pDestFile) != dwRet)
        {
            fclose(pSrcFile);
            fclose(pDestFile);
            return false;
        }
    }

    fclose(pSrcFile);
    fclose(pDestFile);

    return true;
#endif
}

/** 功能：删除文件 */
bool FileUtility::Delete(const char* pcFileName)
{
    if (!IsFileExist(pcFileName))
        return false;
    return remove(pcFileName) == 0;
}

/** 功能：文件改名 */
bool FileUtility::Rename(const char *pcNewFile, const char* pcOldFile)
{
    if (!IsFileExist(pcOldFile))
        return false;
    return rename(pcOldFile, pcNewFile) == 0;

}

/** 功能：判断文件是否存在 */
bool FileUtility::IsFileExist(const char *pcFileName)
{
    return IsFile(pcFileName);
    // return (access(pcFileName, 0)) != -1;
}

/** 功能：一个字符串是不是一个文件名 */
bool FileUtility::IsFile(const char *pcFileName)
{
    struct stat buf;
    if (!(stat(pcFileName, &buf) == 0))
        return false;
    return (buf.st_mode & S_IFREG) != 0;
}

/** 功能：判断文件是否可读 */
bool FileUtility::IsFileReadable(const char *pcFileName)
{
    if (!IsFileExist(pcFileName))
        return false;
    return access(pcFileName, R_OK) == 0;
}

/** 功能：判断文件是否可写 */
bool FileUtility::IsFileWritable(const char *pcFileName)
{
    if (!IsFileExist(pcFileName))
        return false;
    return access(pcFileName, W_OK) == 0;
}

/** 功能：取得文件大小 */
long long FileUtility::GetFileSize(const char *pcFileName)
{
    struct stat stFileStat;
    int dwRet = stat(pcFileName, &stFileStat);
    if (dwRet < 0)
        return -1;
    else
        return stFileStat.st_size;
}

/** 功能：取得文件最后访问时间 */
std::string FileUtility::GetFileAccessTime(const char *pcFileName)
{
    struct stat stFileStat;
    int dwRet = stat(pcFileName, &stFileStat);
    if (dwRet < 0)
        return std::string("");
    else
        return std::string(ctime(&stFileStat.st_atime));
}

/** 功能：取得文件创建时间 */
std::string FileUtility::GetFileCreateTime(const char *pcFileName)
{
    struct stat stFileStat;
    int dwRet = stat(pcFileName, &stFileStat);
    if (dwRet < 0)
        return std::string("");
    else
        return std::string(ctime(&stFileStat.st_ctime));
}

/** 功能：取得文件最后修改时间 */
std::string FileUtility::GetFileLastModifyTime(const char *pcFileName)
{
    struct stat stFileStat;
    int dwRet = stat(pcFileName, &stFileStat);
    if (dwRet < 0)
        return std::string("");
    else
        return std::string(ctime(&stFileStat.st_mtime));
}

/** 功能：从完整的文件路径名中取得文件名 */
std::string FileUtility::ExtractFileName(char *pcFilePath)
{
    char *pStr;
    int    dwLen = 0;

    for (pStr = pcFilePath+strlen(pcFilePath)-1; pStr>pcFilePath; pStr--, dwLen++)
    {
        if (pStr[0] == '/')
        {
            break;
        }
    }
    return std::string(pStr+1, dwLen);
}

/** 功能：从文件名中取得扩展名 */
std::string FileUtility::ExtractFileExtName(std::string& strFileName)
{
    if (strFileName.empty())
        return "";

    size_t i;
    for (i = strFileName.size()-1; i > 0; i--)
    {
        if (strFileName[i] == '.')
        {
            break;
        }
    }
    if ( 0 == i)
        return "";
    else
        return strFileName.substr(i+1);
}

/** 功能：从url中文件扩展名判断是否是html文件 */
bool FileUtility::IsHtmlUrl(std::string& strFileName)
{
    //////////////////////////////////////////////////////////////////////////
    // 初始化Url文件类型参照表;
    // html:         htm, html, shtm, shtml, xhtml, stm
    // cgi:          cgi, jsp, asp, aspx, php, php3, php4, pl
    // flash:        swf
    // video:        wmv, asf, asx, mov, avi, mpeg, mpg, mpa, mpe, rm, ram, 3gp, 3g2
    // audio:        mp3, wav, wma, ogg, ra, mid, midi
    // doc:          doc, pdf, ps, ppt, pps, xls, rtf, wps
    // text:         txt, text, c, h
    // image:        gif, jpg, jpeg, png, bmp, ico, jfif
    // compressed:   zip, rar, tar, z, gz, tgz, taz, bz2
    // exe:          exe,  dll
    // bin:          class, o, bin, jar,torrent
    // js:           js
    // css:          css
    // xml:           xsl
    // wml:
    static char szInValidExtName[][6] = {
        "3g2", "3gp",
        "asf", "asx", "avi",
        "bin", "bmp", "bz2",
        "c", "class", "css",
        "dll", "doc",
        "exe",
        "gif", "gz",
        "h",
        "ico",
        "jar", "jfif", "jpeg", "jpg", "js",
        "mid", "midi", "mov", "mp3", "mpa", "mpe", "mpeg", "mpg",
        "o", "ogg",
        "pdf", "png", "pps", "ppt", "ps", "psd",
        "ra", "ram", "rar", "rm", "rtf",
        "svg", "swf",
        "tar", "text", "tgz", "txt",
        "wav", "wma", "wmv", "wps",
        "xls", "xsl",
        "z", "zip"
    };
    static int dwInValidExtNameCnt = sizeof(szInValidExtName)/6;
    if (strFileName.empty())
        return true;

    // 先取出扩展名
    std::string strFileExtName = ExtractFileExtName(strFileName);
    if (strFileExtName.empty())
        return true;

    if (strFileExtName == "torrent")
    {
        return false;
    }
    // 扩展名长度大于5，那么应该是url的动态参数部分
    if (strFileExtName.size() > 5)
        return true;

    strFileExtName = LowerString(strFileExtName.c_str());

    // 这是最常见的情况，所以先判断，但是html文件的扩展名不一定是这几个
    if (strFileExtName.compare(0, 4, "html") == 0
            || strFileExtName.compare(0, 3, "htm") == 0
            || strFileExtName.compare(0, 5, "shtml") == 0
            || strFileExtName.compare(0, 4, "aspx") == 0
            || strFileExtName.compare(0, 3, "asp") == 0
            || strFileExtName.compare(0, 3, "php") == 0
            || strFileExtName.compare(0, 3, "jsp") == 0)
    {
        return true;
    }

    // 常见的不是html的类型
    for (int i = 0; i < dwInValidExtNameCnt; i++)
    {
        if (strFileExtName == szInValidExtName[i])
        {
            return false;
        }
    }

    // 大部分不能明确的都认为是html文件
    return true;
}

/** 功能：获得文件所在目录的层次 */
int FileUtility::GetFilePathLevel(const char *pcFilePath)
{
    const char *pStr;
    int    dwLevel = 0;

    pStr = pcFilePath + strlen(pcFilePath) - 1;
    for (; pStr > pcFilePath; pStr--)
    {
        // 需要把多余的跳过
        if  (pStr[0] == '/' && pStr-1 > pcFilePath && (*(pStr-1) != '/') && *(pStr-1) != '\\')
            dwLevel++;
        if  (pStr[0] == '\\' && pStr-1 > pcFilePath && (*(pStr-1) != '/') && *(pStr-1) != '\\')
            dwLevel++;

        if  (pStr[0] == ':')
        {
            if (dwLevel > 0 && (pStr[1] == '/' || pStr[1] == '\\'))
                dwLevel--;
            break;
        }
    }
    return dwLevel;
}

/** 功能：判断目录是否存在 */
bool FileUtility::IsDirExist(const char *pcDirName)
{
    return IsDir(pcDirName);
}

/** 功能：一个字符串是不是一个目录名 */
bool FileUtility::IsDir(const char *pcDirName)
{
    struct stat buf;
    if (!(stat(pcDirName, &buf) == 0))
        return false;
    return (buf.st_mode & S_IFDIR) != 0;
}

/** 功能：判断目录是否可读 */
bool FileUtility::IsDirReadable(const char *pcDirName)
{
    if (!IsDirExist(pcDirName))
        return false;
    return access(pcDirName, R_OK) == 0;
}

/** 功能：判断目录是否可写 */
bool FileUtility::IsDirWritable(const char *pcDirName)
{
    if (!IsDirExist(pcDirName))
        return false;
    return access(pcDirName, W_OK) == 0;
}

/** 功能：创建一个目录 */
bool FileUtility::CreateDir(const char *pcDirName)
{
#ifdef _WIN32
    return (_mkdir(pcDirName) == 0);
#else
    return (mkdir(pcDirName, 0777) == 0);
#endif
}

/** 功能：删除一个目录 */
bool FileUtility::DeleteDir(const char *pcDirName)
{
    // 首先检查目录是否存在
    if (!IsDirExist(pcDirName))
        return false;

#ifdef _WIN32
    return (_rmdir(pcDirName) == 0);
#else
    std::string strCmd("rm -r ");
    strCmd += pcDirName;
    int n = system(strCmd.c_str());
    (void) n;
    return true;
    // return (rmdir(pcDirName, 0777) == 0);
#endif
}

/** 功能：设置当前目录 */
bool FileUtility::SetCurrentDir(const char *pcDirName)
{
    // 首先检查目录是否存在
    if (!IsDirExist(pcDirName))
        return false;

#ifdef _WIN32
    // 返回非0是成功
    return (SetCurrentDirectoryA(pcDirName) != 0);
#else
    // 返回0是成功
    return (chdir(pcDirName) == 0);
#endif
}

/** 功能：获得当前目录 */
std::string FileUtility::GetCurrentDir(void)
{
#ifdef _WIN32
    char szPath[MAX_PATH];
    return std::string(_fullpath(szPath, ".", MAX_PATH));
#else
    char szPath[PATH_MAX];
    char* wd = getcwd(szPath, PATH_MAX);
    (void) wd;
    return std::string(szPath);
#endif
}

/** 功能：把文件路径格式从 linux转为windows */
#ifdef _WIN32
void FileUtility::MakeWindowsPath(char* pcFilePath)
{
    char* cTmp;

    for (cTmp = pcFilePath; *cTmp; cTmp++)
    {
        if ( '/' == *cTmp )
            *cTmp = '\\';
    }
}
#endif

/** 把windows文件路径分隔符替换成linux格式的，并把结尾多余的分隔符去掉 */
void FileUtility::NormalizePath(char* pcFilePath)
{
    size_t  dwLen = strlen( pcFilePath );
    char *cTmp;

    // windows下这段才有用
    for (cTmp = pcFilePath; *cTmp; cTmp++ )
    {
        if ( '\\' == *cTmp )
            *cTmp = '/';
    }

    while( dwLen > 1 && pcFilePath[dwLen - 1] == '/' )
    {
        pcFilePath[dwLen-1] = '\0';
        dwLen--;
    }
}

/** 加载一个词典, 格式要求是每行一项 */
bool FileUtility::Load2Set(std::set<std::string>& sDict, const char* pcFileName)
{
    std::ifstream fs;
    fs.open(pcFileName, std::ios_base::in);

    if (fs.fail())
    {
        std::cout << " Sorry ! The file isn't exist. " << pcFileName << std::endl;
        return false;
    }

    std::string strLine;

    while (getline(fs, strLine))
    {
        //strLine = StringUtility::TrimBothSides(strLine, " \t\r\n");
        if (!strLine.empty())
            sDict.insert(strLine);
    }

    fs.close();

    return true;
}

/** 把一个set输出到一个文件中 */
bool FileUtility::DumpSet(std::set<std::string>& sDict, std::string strFileName)
{
    std::string strLine;
    std::set<std::string>::iterator it;
    bool bRet = true;

    for (it = sDict.begin(); it != sDict.end(); it++)
        bRet &= WriteFile(strFileName.c_str(), "%s", (*it).c_str());
    return bRet;
}

/** 从文件加载到一个vector, 格式要求是每行一项 */
bool FileUtility::Load2Vector(std::vector<std::string>& vDict, const char* pcFileName)
{
    // 这里不调用Load2Set是为了保持文件的原有顺序
    std::ifstream fs;
    fs.open(pcFileName, std::ios_base::in);

    if (fs.fail())
    {
        std::cout << " Sorry ! The file isn't exist. " << pcFileName << std::endl;
        return false;
    }

    std::string strLine;

    while (getline(fs, strLine))
    {
        if (!strLine.empty())
            vDict.push_back(strLine);
    }

    fs.close();
    return true;
}

/** 从文件中加载一个map, 格式要求是每行都是<key = value> */
bool FileUtility::Load2Map(std::map<std::string, std::string>& mDict, const char* pcFileName, std::string strSeparator)
{
    std::ifstream fs;
    fs.open(pcFileName, std::ios_base::in);

    if (fs.fail())
    {
        std::cout << " Sorry ! The file isn't exist. " << pcFileName << std::endl;
        return false;
    }

    std::string strLine;
    std::string strKey;
    std::string strValue;
    size_t  dwPos;

    while (getline(fs, strLine))
    {
        dwPos = strLine.find(strSeparator);
        if (std::string::npos != dwPos)
        {
            strKey = strLine.substr(0, dwPos);
            StringTrim(&strKey);
            if (strKey.empty())
                continue;

            strValue   = strLine.substr(dwPos+strSeparator.size());
            StringTrim(&strValue);
            if (strValue.empty())
                continue;

            mDict[strKey] = strValue;
        }
    }

    fs.close();

    return true;
}

/** 从文件中加载一个map, 格式要求是每行都是<key = value> */
bool FileUtility::Load2Map(std::map<std::string, int>& mDict, const char* pcFileName, std::string strSeparator)
{
    // 首先调用上面的函数把数据导入到临时map中
    std::map<std::string, std::string> mTempDict;
    bool bRet = Load2Map(mTempDict, pcFileName, strSeparator);
    if (bRet == false)
        return false;

    int      dwValue = 1;
    std::map<std::string, std::string>::iterator it;
    for (it = mTempDict.begin(); it != mTempDict.end(); it++)
    {

        if (!StringToNumber(it->second, &dwValue))
            return false;
        mDict[it->first] = dwValue;
    }

    return true;
}

/** 从文件中加载一个map, 格式要求是每行都是<key = value> */
bool FileUtility::Load2Map(std::map<std::string, double>& mDict, const char* pcFileName, std::string strSeparator)
{
    // 首先调用上面的函数把数据导入到临时map中
    std::map<std::string, std::string> mTempDict;
    bool bRet = Load2Map(mTempDict, pcFileName, strSeparator);
    if (bRet == false)
        return false;

    double lfValue;
    std::map<std::string, std::string>::iterator it;
    for (it = mTempDict.begin(); it != mTempDict.end(); it++)
    {
        if (!StringToNumber(it->second, &lfValue))
            return false;
        mDict[it->first] = lfValue;
    }

    return true;
}

/** 把一个map输出到一个文件中 */
bool FileUtility::DumpMap(std::map<std::string, int>& mDict, std::string strFileName, std::string strSeparator)
{
    std::string strLine;
    std::map<std::string, int>::iterator it;
    bool bRet = true;

    for (it = mDict.begin(); it != mDict.end(); it++)
    {
        strLine = it->first + strSeparator;
        strLine += IntegerToString(it->second);

        bRet &= Write(strFileName, strLine);
    }
    return bRet;
}

/** 把一个map输出到一个文件中 */
bool FileUtility::DumpMap(std::map<std::string, std::string>& mDict, std::string strFileName, std::string strSeparator)
{
    std::string strLine;
    std::map<std::string, std::string>::iterator it;
    bool bRet = true;

    for (it = mDict.begin(); it != mDict.end(); it++)
    {
        strLine = it->first + strSeparator;
        strLine += it->second;

        bRet &= Write(strFileName, strLine);
    }
    return bRet;
}

/** 遍历一个目录，对目录下所有文件执行某个函数 */
bool FileUtility::DirRoamer(const char* szPath, std::vector<std::string>& vFileName)
{
    // 遍历目录szPath下的所有文件
#ifdef _WIN32
    HANDLE handle;
    WIN32_FIND_DATAA fd;

    char szFile[MAX_FILE_NAME_LENGTH];
    sprintf(szFile,"%s\\*.*",szPath);

    if ((handle = FindFirstFileA(szFile, &fd)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            //处理找到的文件
            if (0 == strcmp(fd.cFileName, ".") || 0 == strcmp(fd.cFileName, ".."))
            {
                continue;
            }
            else if ((FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes) == FILE_ATTRIBUTE_DIRECTORY)
            {
                char szSubPath[MAX_FILE_NAME_LENGTH];
                snprintf(szSubPath, MAX_FILE_NAME_LENGTH, "%s\\%s",szPath, fd.cFileName);
                DirRoamer(szSubPath,  vFileName);
            }
            else
            {
                // 生成文件名
                char szFileName[MAX_FILE_NAME_LENGTH];
                snprintf(szFileName, MAX_FILE_NAME_LENGTH, "%s\\%s", szPath, fd.cFileName);
				vFileName.push_back(std::string(szFileName));
            }
        }
        while (FindNextFileA(handle, &fd));
        FindClose(handle);
    }
#else
    struct dirent entry;
    struct dirent* dirp = NULL;
    DIR* dp;

    if ((dp = opendir(szPath)) == NULL)
    {
        std::cout << "open dir failed : " << szPath << std::endl;
        return false;
    }

    readdir_r(dp, &entry, &dirp);

    while (dirp != NULL)
    {
        //处理找到的文件
        if (0 == strcmp(dirp->d_name, ".") || 0 == strcmp(dirp->d_name, ".."))
        {
        }
        else if (DT_DIR == dirp->d_type)
        {
            char szSubPath[MAX_FILE_NAME_LENGTH];
            snprintf(szSubPath, MAX_FILE_NAME_LENGTH, "%s/%s",szPath, dirp->d_name);

            // 是目录的情况，递归遍历
            DirRoamer(szSubPath,  vFileName);
        }
        else
        {
            char szFileName[MAX_FILE_NAME_LENGTH];
            snprintf(szFileName, MAX_FILE_NAME_LENGTH, "%s/%s", szPath, dirp->d_name);
            vFileName.push_back(std::string(szFileName));
        }
        readdir_r(dp, &entry, &dirp);
    }
    if (closedir(dp) < 0)
    {
        std::cout << "close dir failed" << std::endl;
        return false;
    }
#endif
    return true;
}

#ifndef _WIN32

// 文件系统的设置
void FileUtility::SetFileSystemLimit()
{
    struct rlimit rlim;

    // 设置打开文件数目的限制
    rlim.rlim_cur = MAXFDS;
    rlim.rlim_max = MAXFDS;
    setrlimit(RLIMIT_NOFILE, &rlim);

    // 设置 core dump 文件大小限制
    rlim.rlim_cur = RLIM_INFINITY;
    rlim.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &rlim);
}
#endif

/** 本文件结束 */

