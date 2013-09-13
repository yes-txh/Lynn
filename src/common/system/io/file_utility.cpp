#ifndef _CRT_NONSTDC_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

/**
 * �ļ���FileUtility.cpp
 * ���ܣ����õ��ļ����ߺ���
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

// linux��windows���ļ�API��ͬ
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

/** ���ļ� */
std::string FileUtility::ReadTextFile(const char *pcFileName)
{
    std::string strFileName(pcFileName);
    StringTrim(&strFileName);

    // ���ȼ���ļ��Ƿ��ж�Ȩ��
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

/** ���ļ� */
std::string FileUtility::ReadTextFile(std::string strFileName)
{
    return ReadTextFile(strFileName.c_str());
}

/** ���ܣ����������ļ�, ���ʺϾ޴����Ҫ�ֶζ�ȡ���ļ� */
bool FileUtility::ReadBinFile(char* pcContent, int& dwReadLen, const char *pcFileName)
{
    if (NULL == pcContent || NULL == pcFileName)
        return false;

    dwReadLen = 0;

    // ȡ���ļ��Ĵ�С
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

    // ������ѭ����������Ϊ�����ļ���ʱ��ú������ܱ��ź��ж�
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

/** ���ܣ�д�������ļ� */
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

/** ���ܣ�׷�ӷ�ʽд�ı��ļ���֧�ֿɱ������ʹ������printf
 *   ˵����ֻ������дС������(< 16K)
 *    ʾ����FileUtility::WriteFile("/data/1.dat", "%s\t%d\n", szName, dwIndex);
 */
bool FileUtility::WriteFile(const char *pcFileName, const char* fmt, ...)
{
    // ���ȼ���ļ��Ƿ���дȨ��
    // if (!IsFileWritable(pcFileName))
    //  return false;

    FILE *fp = fopen(pcFileName, "at");
    if (fp != NULL)
    {
        char szBuf [1024*64];
        int dwPos = 0;

        memset(szBuf, 0, sizeof(szBuf));

        // ��������д��������
        va_list ap;
        va_start(ap, fmt);
#ifndef _WIN32
        vsnprintf(szBuf + dwPos, sizeof(szBuf) - dwPos, fmt, ap);
#else
        _vsnprintf(szBuf + dwPos, sizeof(szBuf) - dwPos, fmt, ap);
#endif
        va_end(ap);

        // �ѻ�������д�뵽�ļ�
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

/** ���ܣ�׷�ӷ�ʽ��һ���ַ���д���ļ� */
bool FileUtility::Write(std::string& strFileName, std::string& strValue, bool bPrint)
{
    if (bPrint)
        std::cout << strValue << std::endl;
    return WriteFile(strFileName.c_str(), "%s", strValue.c_str());
}

/** ���ܣ�׷�ӷ�ʽ��һ������д���ļ� */
bool FileUtility::Write(std::string& strFileName, int dwValue, bool bPrint)
{
    if (bPrint)
        std::cout << dwValue << std::endl;
    return WriteFile(strFileName.c_str(), "%d", dwValue);
}

/** ���ܣ�׷�ӷ�ʽ��һ������д���ļ� */
bool FileUtility::Write(std::string& strFileName, double lfValue, bool bPrint)
{
    if (bPrint)
        std::cout << lfValue << std::endl;
    return WriteFile(strFileName.c_str(), "%lf", lfValue);
}

/** ���ܣ������ļ� */
bool FileUtility::Copy(const char *pcDestFile, const char* pcSrcFile, char bFlags)
{
#ifdef _WIN32
    if (bFlags == FILECOPY_OVERWRITE_ON)
    {
        // ����д
        if (TRUE != (CopyFileA(pcSrcFile, pcDestFile , false)))
            return false;
    }
    else if (bFlags == FILECOPY_OVERWRITE_OFF)
    {
        // ������д
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

    // ������д��ʱ����Ҫ���ԭ�ļ��Ƿ����
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
        // Դ�ļ�������
        return false;
    }

    if ((pDestFile = fopen(pcDestFile, "w+")) == NULL)
    {
        // ��Ŀ���ļ�ʧ��
        fclose(pSrcFile);
        return false;
    }

    while ((dwRet = fread(szBuf, 1, sizeof(szBuf), pSrcFile)) > 0)
    {
        // �Ѷ���д�����ļ�
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

/** ���ܣ�ɾ���ļ� */
bool FileUtility::Delete(const char* pcFileName)
{
    if (!IsFileExist(pcFileName))
        return false;
    return remove(pcFileName) == 0;
}

/** ���ܣ��ļ����� */
bool FileUtility::Rename(const char *pcNewFile, const char* pcOldFile)
{
    if (!IsFileExist(pcOldFile))
        return false;
    return rename(pcOldFile, pcNewFile) == 0;

}

/** ���ܣ��ж��ļ��Ƿ���� */
bool FileUtility::IsFileExist(const char *pcFileName)
{
    return IsFile(pcFileName);
    // return (access(pcFileName, 0)) != -1;
}

/** ���ܣ�һ���ַ����ǲ���һ���ļ��� */
bool FileUtility::IsFile(const char *pcFileName)
{
    struct stat buf;
    if (!(stat(pcFileName, &buf) == 0))
        return false;
    return (buf.st_mode & S_IFREG) != 0;
}

/** ���ܣ��ж��ļ��Ƿ�ɶ� */
bool FileUtility::IsFileReadable(const char *pcFileName)
{
    if (!IsFileExist(pcFileName))
        return false;
    return access(pcFileName, R_OK) == 0;
}

/** ���ܣ��ж��ļ��Ƿ��д */
bool FileUtility::IsFileWritable(const char *pcFileName)
{
    if (!IsFileExist(pcFileName))
        return false;
    return access(pcFileName, W_OK) == 0;
}

/** ���ܣ�ȡ���ļ���С */
long long FileUtility::GetFileSize(const char *pcFileName)
{
    struct stat stFileStat;
    int dwRet = stat(pcFileName, &stFileStat);
    if (dwRet < 0)
        return -1;
    else
        return stFileStat.st_size;
}

/** ���ܣ�ȡ���ļ�������ʱ�� */
std::string FileUtility::GetFileAccessTime(const char *pcFileName)
{
    struct stat stFileStat;
    int dwRet = stat(pcFileName, &stFileStat);
    if (dwRet < 0)
        return std::string("");
    else
        return std::string(ctime(&stFileStat.st_atime));
}

/** ���ܣ�ȡ���ļ�����ʱ�� */
std::string FileUtility::GetFileCreateTime(const char *pcFileName)
{
    struct stat stFileStat;
    int dwRet = stat(pcFileName, &stFileStat);
    if (dwRet < 0)
        return std::string("");
    else
        return std::string(ctime(&stFileStat.st_ctime));
}

/** ���ܣ�ȡ���ļ�����޸�ʱ�� */
std::string FileUtility::GetFileLastModifyTime(const char *pcFileName)
{
    struct stat stFileStat;
    int dwRet = stat(pcFileName, &stFileStat);
    if (dwRet < 0)
        return std::string("");
    else
        return std::string(ctime(&stFileStat.st_mtime));
}

/** ���ܣ����������ļ�·������ȡ���ļ��� */
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

/** ���ܣ����ļ�����ȡ����չ�� */
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

/** ���ܣ���url���ļ���չ���ж��Ƿ���html�ļ� */
bool FileUtility::IsHtmlUrl(std::string& strFileName)
{
    //////////////////////////////////////////////////////////////////////////
    // ��ʼ��Url�ļ����Ͳ��ձ�;
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

    // ��ȡ����չ��
    std::string strFileExtName = ExtractFileExtName(strFileName);
    if (strFileExtName.empty())
        return true;

    if (strFileExtName == "torrent")
    {
        return false;
    }
    // ��չ�����ȴ���5����ôӦ����url�Ķ�̬��������
    if (strFileExtName.size() > 5)
        return true;

    strFileExtName = LowerString(strFileExtName.c_str());

    // ���������������������жϣ�����html�ļ�����չ����һ�����⼸��
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

    // �����Ĳ���html������
    for (int i = 0; i < dwInValidExtNameCnt; i++)
    {
        if (strFileExtName == szInValidExtName[i])
        {
            return false;
        }
    }

    // �󲿷ֲ�����ȷ�Ķ���Ϊ��html�ļ�
    return true;
}

/** ���ܣ�����ļ�����Ŀ¼�Ĳ�� */
int FileUtility::GetFilePathLevel(const char *pcFilePath)
{
    const char *pStr;
    int    dwLevel = 0;

    pStr = pcFilePath + strlen(pcFilePath) - 1;
    for (; pStr > pcFilePath; pStr--)
    {
        // ��Ҫ�Ѷ��������
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

/** ���ܣ��ж�Ŀ¼�Ƿ���� */
bool FileUtility::IsDirExist(const char *pcDirName)
{
    return IsDir(pcDirName);
}

/** ���ܣ�һ���ַ����ǲ���һ��Ŀ¼�� */
bool FileUtility::IsDir(const char *pcDirName)
{
    struct stat buf;
    if (!(stat(pcDirName, &buf) == 0))
        return false;
    return (buf.st_mode & S_IFDIR) != 0;
}

/** ���ܣ��ж�Ŀ¼�Ƿ�ɶ� */
bool FileUtility::IsDirReadable(const char *pcDirName)
{
    if (!IsDirExist(pcDirName))
        return false;
    return access(pcDirName, R_OK) == 0;
}

/** ���ܣ��ж�Ŀ¼�Ƿ��д */
bool FileUtility::IsDirWritable(const char *pcDirName)
{
    if (!IsDirExist(pcDirName))
        return false;
    return access(pcDirName, W_OK) == 0;
}

/** ���ܣ�����һ��Ŀ¼ */
bool FileUtility::CreateDir(const char *pcDirName)
{
#ifdef _WIN32
    return (_mkdir(pcDirName) == 0);
#else
    return (mkdir(pcDirName, 0777) == 0);
#endif
}

/** ���ܣ�ɾ��һ��Ŀ¼ */
bool FileUtility::DeleteDir(const char *pcDirName)
{
    // ���ȼ��Ŀ¼�Ƿ����
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

/** ���ܣ����õ�ǰĿ¼ */
bool FileUtility::SetCurrentDir(const char *pcDirName)
{
    // ���ȼ��Ŀ¼�Ƿ����
    if (!IsDirExist(pcDirName))
        return false;

#ifdef _WIN32
    // ���ط�0�ǳɹ�
    return (SetCurrentDirectoryA(pcDirName) != 0);
#else
    // ����0�ǳɹ�
    return (chdir(pcDirName) == 0);
#endif
}

/** ���ܣ���õ�ǰĿ¼ */
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

/** ���ܣ����ļ�·����ʽ�� linuxתΪwindows */
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

/** ��windows�ļ�·���ָ����滻��linux��ʽ�ģ����ѽ�β����ķָ���ȥ�� */
void FileUtility::NormalizePath(char* pcFilePath)
{
    size_t  dwLen = strlen( pcFilePath );
    char *cTmp;

    // windows����β�����
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

/** ����һ���ʵ�, ��ʽҪ����ÿ��һ�� */
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

/** ��һ��set�����һ���ļ��� */
bool FileUtility::DumpSet(std::set<std::string>& sDict, std::string strFileName)
{
    std::string strLine;
    std::set<std::string>::iterator it;
    bool bRet = true;

    for (it = sDict.begin(); it != sDict.end(); it++)
        bRet &= WriteFile(strFileName.c_str(), "%s", (*it).c_str());
    return bRet;
}

/** ���ļ����ص�һ��vector, ��ʽҪ����ÿ��һ�� */
bool FileUtility::Load2Vector(std::vector<std::string>& vDict, const char* pcFileName)
{
    // ���ﲻ����Load2Set��Ϊ�˱����ļ���ԭ��˳��
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

/** ���ļ��м���һ��map, ��ʽҪ����ÿ�ж���<key = value> */
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

/** ���ļ��м���һ��map, ��ʽҪ����ÿ�ж���<key = value> */
bool FileUtility::Load2Map(std::map<std::string, int>& mDict, const char* pcFileName, std::string strSeparator)
{
    // ���ȵ�������ĺ��������ݵ��뵽��ʱmap��
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

/** ���ļ��м���һ��map, ��ʽҪ����ÿ�ж���<key = value> */
bool FileUtility::Load2Map(std::map<std::string, double>& mDict, const char* pcFileName, std::string strSeparator)
{
    // ���ȵ�������ĺ��������ݵ��뵽��ʱmap��
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

/** ��һ��map�����һ���ļ��� */
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

/** ��һ��map�����һ���ļ��� */
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

/** ����һ��Ŀ¼����Ŀ¼�������ļ�ִ��ĳ������ */
bool FileUtility::DirRoamer(const char* szPath, std::vector<std::string>& vFileName)
{
    // ����Ŀ¼szPath�µ������ļ�
#ifdef _WIN32
    HANDLE handle;
    WIN32_FIND_DATAA fd;

    char szFile[MAX_FILE_NAME_LENGTH];
    sprintf(szFile,"%s\\*.*",szPath);

    if ((handle = FindFirstFileA(szFile, &fd)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            //�����ҵ����ļ�
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
                // �����ļ���
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
        //�����ҵ����ļ�
        if (0 == strcmp(dirp->d_name, ".") || 0 == strcmp(dirp->d_name, ".."))
        {
        }
        else if (DT_DIR == dirp->d_type)
        {
            char szSubPath[MAX_FILE_NAME_LENGTH];
            snprintf(szSubPath, MAX_FILE_NAME_LENGTH, "%s/%s",szPath, dirp->d_name);

            // ��Ŀ¼��������ݹ����
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

// �ļ�ϵͳ������
void FileUtility::SetFileSystemLimit()
{
    struct rlimit rlim;

    // ���ô��ļ���Ŀ������
    rlim.rlim_cur = MAXFDS;
    rlim.rlim_max = MAXFDS;
    setrlimit(RLIMIT_NOFILE, &rlim);

    // ���� core dump �ļ���С����
    rlim.rlim_cur = RLIM_INFINITY;
    rlim.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &rlim);
}
#endif

/** ���ļ����� */

