/**
 * 日志类，完全平台无关
 * @author jerryzhang@tencent.com
 * @since  2006.06.08
 */

#include "common/log/log.hpp"

#include <time.h>
#include <errno.h>
#include <string.h>

#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/types.h>

#ifndef _WIN32

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/syscall.h>

// Slackware/2.4.30 kernel, using linuxthreads, doesn't really support __thread keyword,
// will generate runtime error
static bool SupportTls()
{
	char buffer[64];
	confstr(_CS_GNU_LIBPTHREAD_VERSION, buffer, sizeof(buffer));
	return strstr(buffer, "NPTL") != NULL;
}

static inline pid_t gettid()
{
	static bool support_tls = SupportTls();
	if (support_tls)
	{
		static __thread pid_t tid = 0;
		if (tid == 0)
			tid = (pid_t) syscall(SYS_gettid);
		return tid;
	}
	return (pid_t) syscall(SYS_gettid);
}

#else
#include <common/base/common_windows.h>
#include <process.h>
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define getpid() _getpid()
#define gettid() GetCurrentThreadId
#include <io.h>
#define access _access
#define F_OK 0
#define S_ISDIR(f) ((f) & _S_IFDIR)

#endif

namespace twse {
namespace webspider {

static const char* const LogLevelFileNames[] =
{
	"fatal", "error", "warning", "notice", "stat", "debug", "trace"
};

static const char* const LogLevelNames[] =
{
	"FATAL", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG", "TRACE"
};

Log* g_pLog = NULL;

class Log::LogFile
{
public:
	LogFile(
		const std::string& prefix,
		bool has_time,
		unsigned long long max_size,
		int max_count):
		m_Prefix(prefix),
		m_HasTime(has_time),
		m_MaxSize(max_size),
		m_MaxCount(max_count),
		m_fp(NULL),
		m_Size(0),
		m_OpenTime(0),
		m_LastShiftCheckTime(0)
	{
		Open();
	}
	~LogFile()
	{
		fclose(m_fp);
	}
	bool Open()
	{
		Close();

		std::string name;
		m_OpenTime = time(NULL);
		GetNameOf(m_OpenTime, 0, name);

		m_fp = fopen(name.c_str(), "ab");
		if (m_fp)
		{
#ifdef __GNUC__
			m_Size = ftello(m_fp);
#elif defined _MSC_VER
			m_Size = _ftelli64(m_fp);
#endif
			return true;
		}
		return false;
	}
	void Close()
	{
		if (m_fp)
		{
			fclose(m_fp);
			m_fp = NULL;
			m_Size = 0;
		}
	}

	bool CheckShift(bool& date_changed)
	{
		date_changed = false;
		if (m_Size > m_MaxSize)
			return true;

		time_t now = time(NULL);
		if (now - m_LastShiftCheckTime < 1)
			return false;
		m_LastShiftCheckTime = now;

		if (!m_fp)
			return true;

		struct stat st;
	#ifdef _MSC_VER
		int fd = _fileno(m_fp);
	#else
		int fd = fileno(m_fp);
	#endif
		if (fstat(fd, &st) == 0)
		{
			if (st.st_nlink == 0)
				return true;

			if (st.st_size > (long long)m_MaxSize)
				return true;
		}

		if (m_HasTime)
		{
			struct tm tm_old, tm_now;
			tm_old = *localtime(&m_OpenTime);
			tm_now = *localtime(&now);
			if (tm_now.tm_yday != tm_old.tm_yday)
			{
				date_changed = true;
				return true;
			}
		}

		return false;
	}

	bool Shift(bool date_changed)
	{
		if (date_changed || !m_fp)
		{
			return Open();
		}

		Close();

		// remove oldest file
		std::string filename;
		if (m_MaxCount > 1)
		{
			GetNameOf(m_OpenTime, m_MaxCount - 1, filename);
			remove(filename.c_str());
		}

		std::string old_name;
		std::string new_name;
		for(int i = m_MaxCount - 2; i >= 0; i--)
		{
			GetNameOf(m_OpenTime, i, old_name);
			if (access(old_name.c_str(), F_OK) == 0)
			{
				GetNameOf(m_OpenTime, i + 1, new_name);
				rename(old_name.c_str(), new_name.c_str());
			}
		}

		return Open();
	}

	bool TestShift()
	{
		bool date_changed;
		if (CheckShift(date_changed))
		{
			return Shift(date_changed);
		}
		return false;
	}

	bool Write(const void* data, size_t size)
	{
		if (m_fp)
		{
			size_t length = fwrite(data, 1, size, m_fp);
			if (length > 0)
			{
				m_Size += length;
				return true;
			}
		}
		return false;
	}

	bool Write(const char* string)
	{
		return Write(string, strlen(string));
	}

	bool VPrintf(const char* format, va_list va)
	{
		if (m_fp)
		{
			int length = vfprintf(m_fp, format, va);
			if (length >= 0)
			{
				m_Size += length;
				return true;
			}
		}
		return false;
	}

	bool Printf(const char* format, ...)
	{
		va_list va;
		va_start(va, format);
		bool result = VPrintf(format, va);
		va_end(va);
		return result;
	}

	void Flush()
	{
		if (m_fp)
			fflush(m_fp);
	}
private:
	void GetNameOf(time_t time, int sn, std::string& result) const
	{
		result = m_Prefix;
		if (m_HasTime)
		{
			struct tm tm;
		#ifdef _WIN32
			tm = *localtime(&time);
		#else
			localtime_r(&time, &tm);
		#endif

			char    data_str[16];
			snprintf(data_str, sizeof(data_str), "_%04d%02d%02d",
				tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
			result += data_str;
		}
		if (sn != 0)
		{
			char sn_str[12];
			snprintf(sn_str, sizeof(sn_str), "_%d", sn);
			result += sn_str;
		}
		result += ".log";
	}
private:
	std::string m_Prefix;
	bool m_HasTime;
	unsigned long long m_MaxSize;
	int m_MaxCount;
	FILE* m_fp;
	unsigned long long m_Size;
	time_t m_OpenTime;
	time_t m_LastShiftCheckTime;
};

#ifdef _WIN32
const char END_LINE[] = "\r\n";
#else
const char END_LINE[] = "\n";
#endif

const size_t END_LINE_SIZE = sizeof(END_LINE) - 1;

static void VerifyLogDir(const char* path)
{
	struct stat st;
	if (stat(path, &st) == -1)
	{
		int error = errno;

		std::string message = "Log path error: '";
		message += path;
		message += "', ";
		message += strerror(error);

		throw std::runtime_error(message);
	}

	if (!S_ISDIR(st.st_mode))
	{
		std::string message = "Log path error: '";
		message += path;
		message += "', Not a directory";
		throw std::runtime_error(message);
	}
}

Log::Log(
	const char* pcLogPath,
	int dwLevel,
	unsigned long long dwMaxLogSize,
	int dwMaxLogNum,
	const char* BaseFileName,
	bool bMultiLine,
	bool bSingleFile,
	bool bThreadSafe,
	bool bNeedFileAndLine,
	bool bNeedFileNameTime,
	bool bNeedLineTime,
	bool bNeedThreadID,
	bool auto_flush
) :
	m_Path(pcLogPath),
	m_Prefix(BaseFileName ? BaseFileName : ""),
	m_MinLevel(dwLevel),
	m_MaxFileSize(dwMaxLogSize),
	m_MaxFileNumber(dwMaxLogNum)
{
	m_Options.SingleFile(bSingleFile);
	m_Options.ThreadSafe(bThreadSafe);
	m_Options.MultiLine(bMultiLine);
	m_Options.NeedFileAndLine(bNeedFileAndLine);
	m_Options.NeedFileNameTime(bNeedFileNameTime);
	m_Options.NeedLineTime(bNeedLineTime);
	m_Options.NeedThreadID(bNeedThreadID);
	m_Options.Flush(true);

	VerifyLogDir(pcLogPath);
	InitLock();
}

Log::Log()
{
}

Log::Log(
	const char* pcLogPath,
	int dwLevel,
	unsigned long long dwMaxLogSize,
	int dwMaxLogNum,
	const char* BaseFileName,
	const LogOptions& options
) :
	m_Path(pcLogPath),
	m_Prefix(BaseFileName ? BaseFileName : ""),
	m_MinLevel(dwLevel),
	m_MaxFileSize(dwMaxLogSize),
	m_MaxFileNumber(dwMaxLogNum),
	m_Options(options)
{
	VerifyLogDir(pcLogPath);
	InitLock();
}

Log::~Log()
{
	DestoryLock();
};

Log::LogFile& Log::GetLogFile(int level)
{
	if (m_Options.SingleFile())
	{
		if (!m_LogFile.get())
		{
			m_LogFile.reset(
				new LogFile(
					m_Path + "/" + m_Prefix,
					m_Options.NeedFileNameTime(),
					m_MaxFileSize,
					m_MaxFileNumber
				)
			);
		}
		return *m_LogFile;
	}
	else
	{
		std::auto_ptr<LogFile>& log_file_ptr = m_LevelLogFiles[level];
		if (!log_file_ptr.get())
		{
			std::string name;
			name = m_Path;
			name += "/";
			if (!m_Prefix.empty())
			{
				name += m_Prefix;
				name += "_";
			}
			name += LogLevelFileNames[level];

			log_file_ptr.reset(
				new LogFile(
					name,
					m_Options.NeedFileNameTime(),
					m_MaxFileSize,
					m_MaxFileNumber
				)
			);
		}
		return *log_file_ptr;
	}
}

namespace {
class StdoutStream
{
public:
	StdoutStream(FILE* fp)
		: m_fp(fp)
	{
	}
	void Write(const void* data, size_t size)
	{
		fwrite(data, 1, size, m_fp);
	}
	void Write(const char* str)
	{
		fwrite(str, 1, strlen(str), m_fp);
	}
	void VPrintf(const char* format, va_list va)
	{
		vfprintf(m_fp, format, va);
	}
	void Printf(const char* format, ...)
	{
		va_list va;
		va_start(va, format);
		VPrintf(format, va);
		va_end(va);
	}
	void Flush()
	{
		fflush(m_fp);
	}
private:
	FILE* m_fp;
};

template <typename StreamType>
void WriteLogToStream(
	StreamType& stream,
	const LogOptions& options,
	int level,
	const char* file,
	int line,
	const char* format,
	va_list va
	)
{
	if (options.SingleFile())
	{
		const char* level_name = LogLevelNames[level];
		if (options.MultiLine())
		{
			stream.Write("[level] ");
			stream.Write(level_name);
			stream.Write(END_LINE);
		}
		else
		{
			stream.Write(level_name);
			stream.Write(" ");
		}
	}

	if (options.NeedLineTime())
	{
		if (options.MultiLine())
			stream.Write("[time] ");

		struct tm tm;
		int micro_seconds;

#ifdef _WIN32
		time_t now = time(NULL);
		tm = *localtime(&now);
		struct _timeb	tbNow;
		_ftime(&tbNow);
		micro_seconds = tbNow.millitm * 1000;
#else
		struct timeval	tvNow;
		gettimeofday(&tvNow, NULL);
		time_t now = tvNow.tv_sec;
		micro_seconds = (int) tvNow.tv_usec;
		localtime_r(&now, &tm);
#endif

		stream.Printf(
			"%04d-%02d-%02d %02d:%02d:%02d.%06d",
			tm.tm_year + 1900,
			tm.tm_mon + 1,
			tm.tm_mday,
			tm.tm_hour,
			tm.tm_min,
			tm.tm_sec,
			micro_seconds
		);

		if (options.MultiLine())
			stream.Write(END_LINE);
		else
			stream.Write(" ");
	}


	if (options.NeedProcessID())
	{
		if (options.MultiLine())
			stream.Printf("[pid] %d%s", getpid(), END_LINE);
		else
			stream.Printf("%d ", getpid());
	}

	if (options.NeedThreadID())
	{
		if (options.MultiLine())
			stream.Printf("[tid] %d%s", gettid(), END_LINE);
		else
			stream.Printf("%d ", gettid());
	}

	if (options.NeedFileAndLine())
	{
		if (options.MultiLine())
			stream.Printf("[file] %s%s[line] %d%s", file, END_LINE, line, END_LINE);
		else
			stream.Printf("%s:%d ", file, line);
	}

	if (options.MultiLine())
		stream.Write("[info] ");

	stream.VPrintf(format, va);

	stream.Write(END_LINE);
	if (options.Flush())
		stream.Flush();
}
} // end anonymous namespace


void Log::WriteV(int dwLogLevel, const char* pcFileName, int dwLine,const char* fmt, va_list va)
{
	if (dwLogLevel > m_MinLevel)
		return;

	Lock();

	LogFile& log_file = GetLogFile(dwLogLevel);

	log_file.TestShift();

	// 有些库比如 X86-64/Linux 下的 glibc，va_list 不能重复使用，
	// 但是提供了 va_copy 用来拷贝 va_list。
#ifdef va_copy // va_copy 如果定义，必然是宏
	va_list va1, va2;
	va_copy(va1, va);
	va_copy(va2, va);
#else // 假设不支持 va_copy 的环境都能直接拷贝 va_list，通常没问题。
	va_list va1 = va, va2 = va;
#endif

	WriteLogToStream(log_file, m_Options, dwLogLevel, pcFileName, dwLine, fmt, va);

	if (dwLogLevel <= Level_Warning)
	{
		if (m_Options.WriteStdError())
		{
#ifdef __unix__ // ANSI 彩色
			if (dwLogLevel == Level_Warning)
				fprintf(stderr, "\x1B[1;33m"); // 亮黄色
			else if(dwLogLevel == Level_Error)
				fprintf(stderr, "\x1B[0;31m"); // 红色
			else if(dwLogLevel == Level_Fatal)
				fprintf(stderr, "\x1B[1;31m"); // 亮红色
#endif
			fprintf(stderr, "%-8s", LogLevelNames[dwLogLevel]);
			StdoutStream stdout_stream(stderr);
			WriteLogToStream(stdout_stream, m_Options, dwLogLevel, pcFileName, dwLine, fmt, va1);
#ifdef __unix__
			fprintf(stderr, "\x1B[m"); // 恢复默认颜色
#endif
		}
	}
	else
	{
		if (m_Options.WriteStdOut() || dwLogLevel == Level_Verbose)
		{
#ifdef __unix__ // ANSI 彩色
			if (dwLogLevel == Level_Notice)
				fprintf(stdout, "\x1B[1;36m");
			else if (dwLogLevel == Level_Info)
				fprintf(stdout, "\x1B[1;32m"); // 亮绿色
#endif
			fprintf(stdout, "%-8s", LogLevelNames[dwLogLevel]);
			StdoutStream stdout_stream(stdout);
			WriteLogToStream(stdout_stream, m_Options, dwLogLevel, pcFileName, dwLine, fmt, va2);
#ifdef __unix__
			fprintf(stdout, "\x1B[m");
#endif
			if (m_Options.Flush())
				fflush(stdout);
		}
	}

#ifdef va_copy
	va_end(va1);
	va_end(va2);
#endif
	Unlock();
}

void Log::Write(int dwLogLevel, const char* pcFileName, int dwLine, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	WriteV(dwLogLevel, pcFileName, dwLine, fmt, va);
	va_end(va);
}

void Log::WriteLog(int dwLogLevel, const char* pcFileName, int dwLine, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	WriteV(dwLogLevel, pcFileName, dwLine, fmt, va);
	va_end(va);
}

void Log::InitLock()
{
	if (m_Options.ThreadSafe())
	{
#ifndef _WIN32
		pthread_mutex_init(&m_lock, NULL);
#else
		InitializeCriticalSection(&m_lock);
#endif
	}
}

void Log::Lock()
{
	if (m_Options.ThreadSafe())
	{
#ifndef _WIN32
		pthread_mutex_lock(&m_lock);
#else
		EnterCriticalSection(&m_lock);
#endif
	}
}

void Log::Unlock()
{
	if (m_Options.ThreadSafe())
	{
#ifndef _WIN32
		pthread_mutex_unlock(&m_lock);
#else
		LeaveCriticalSection(&m_lock);
#endif
	}
}

void Log::DestoryLock()
{
	if (m_Options.ThreadSafe())
	{
#ifndef _WIN32
		pthread_mutex_destroy(&m_lock);
#else
		DeleteCriticalSection(&m_lock);
#endif
	}
}

} // end namespace webspider
} // end namespace twse
