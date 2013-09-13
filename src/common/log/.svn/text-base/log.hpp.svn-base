/**
 * 日志类，完全平台无关
 * @author jerryzhang@tencent.com
 * @since  2006.06.08
 */

#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <string.h>
#include <string>
#include <memory>
#include <stdarg.h>

#ifdef _WIN32
#include <common/base/common_windows.h>
#else
#include <pthread.h>
#endif

#ifdef __GNUC__
#define gcc_attribute(x) __attribute__(x)
#else
#define gcc_attribute(x)
#endif

namespace twse {
namespace webspider {

struct LogOptions
{
public:
	LogOptions():
		m_MultiLine(true),
		m_SingleFile(false),
		m_ThreadSafe(true),
		m_NeedFileAndLine(true),
		m_NeedFileNameTime(true),
		m_NeedLineTime(true),
		m_NeedProcessID(false),
		m_NeedThreadID(true),
		m_Flush(true),
		m_WriteStdOut(false),
		m_WriteStdError(false)
	{
	}
public:
	bool MultiLine() const          { return m_MultiLine; }
	bool SingleFile() const         { return m_SingleFile; }
	bool ThreadSafe() const         { return m_ThreadSafe; }
	bool NeedFileAndLine() const    { return m_NeedFileAndLine; }
	bool NeedFileNameTime() const   { return m_NeedFileNameTime; }
	bool NeedLineTime() const       { return m_NeedLineTime; }
	bool NeedProcessID() const       { return m_NeedProcessID; }
	bool NeedThreadID() const       { return m_NeedThreadID; }
	bool Flush() const              { return m_Flush; }
	bool WriteStdOut() const              { return m_WriteStdOut; }
	bool WriteStdError() const              { return m_WriteStdError; }

	LogOptions& MultiLine(bool value)       { m_MultiLine = value; return *this; }
	LogOptions& SingleFile(bool value)      { m_SingleFile = value; return *this; }
	LogOptions& ThreadSafe(bool value)         { m_ThreadSafe = value; return *this; }
	LogOptions& NeedFileAndLine(bool value) { m_NeedFileAndLine = value; return *this; }
	LogOptions& NeedFileNameTime(bool value) { m_NeedFileNameTime = value; return *this; }
	LogOptions& NeedLineTime(bool value)    { m_NeedLineTime = value; return *this; }
	LogOptions& NeedProcessID(bool value)    { m_NeedProcessID = value; return *this; }
	LogOptions& NeedThreadID(bool value)    { m_NeedThreadID = value; return *this; }
	LogOptions& Flush(bool value)    { m_Flush = value; return *this; }
	LogOptions& WriteStdError(bool value)    { m_WriteStdError = value; return *this; }
	LogOptions& WriteStdOut(bool value)    { m_WriteStdOut = value; return *this; }
public:
	bool m_MultiLine;
	bool m_SingleFile;
	bool m_ThreadSafe;
	bool m_NeedFileAndLine;
	bool m_NeedFileNameTime;
	bool m_NeedLineTime;
	bool m_NeedProcessID;
	bool m_NeedThreadID;
	bool m_Flush;
	bool m_WriteStdOut;
	bool m_WriteStdError;
};

class Log
{
public:
	enum Level
	{
		Level_Fatal      = 0,		/* 致命错误， 最高级别的日志 */
		Level_Error      = 1,		/* 一般错误 */
		Level_Warning    = 2,		/* 警告条件，对可能出现问题的情况进行警告 */
		Level_Notice     = 3,		/* 正常但又重要的条件，用于提醒 */
		Level_Info       = 4,		/* 一般输出信息 */
		Level_Debug      = 5,		/* 调试信息 */
		Level_Verbose    = 6,		/* 最详细的输出，同时打印屏幕 */
		Level_Max
	};

protected:
	Log();
public:
	Log(
		const char* path,
		int level,
		unsigned long long max_file_size,
		int max_file_count,
		const char* base_name = NULL,
		bool multiline = true,
		bool single_file = false,
		bool thread_safe = true,
		bool log_source_line = true,
		bool filename_time = true,
		bool log_line_time = true,
		bool log_thread_id = true,
		bool auto_flush = true
	);

	Log(
		const char* pcLogPath,
		int level,
		unsigned long long max_file_size,
		int max_file_count,
		const char* base_filename,
		const LogOptions& options
	);

	virtual ~Log();

	virtual void WriteV(
		int level,
		const char* filename,
		int line,
		const char* format,
		va_list
	) gcc_attribute((__format__ (__printf__, 5, 0))) ;

	void Write(
		int level,
		const char* filename,
		int line,
		const char* format,
		...
	) gcc_attribute((__format__ (__printf__, 5, 6)));

	/// 兼容性的旧名字，新代码请使用 Write
	void WriteLog(
		int level,
		const char* filename,
		int line,
		const char* format,
		...
	) gcc_attribute((__format__ (__printf__, 5, 6))) gcc_attribute((deprecated));

	int GetMinLevel() const
	{
		return m_MinLevel;
	}

	bool SetMinLevel(int level)
	{
		if (level >= Level_Fatal && level <= Level_Max)
		{
			m_MinLevel = level;
			return true;
		}
		return false;
	}
private:
	void InitLock();
	void Lock();
	void Unlock();
	void DestoryLock();

private:
	class LogFile;
	LogFile& GetLogFile(int dwLogLevel);
private:
	std::string m_Path;
	std::string m_Prefix;
	int m_MinLevel;
	unsigned long long m_MaxFileSize;
	int m_MaxFileNumber;
	LogOptions m_Options;

#ifndef _WIN32
	pthread_mutex_t m_lock;
#else
	CRITICAL_SECTION m_lock;
#endif
	std::auto_ptr<LogFile> m_LogFile;
	std::auto_ptr<LogFile> m_LevelLogFiles[Level_Max + 1];
};

/**
 * 用于不想输出 Log 的场合
 */
class NullLog : public Log
{
public:
	virtual void WriteV(
		int level,
		const char* filename,
		int line,
		const char* format,
		va_list
	) gcc_attribute((__format__ (__printf__, 5, 0)));
};

inline void NullLog::WriteV(
	int level,
	const char* filename,
	int line,
	const char* format,
	va_list
	)
{
}

extern Log* g_pLog;

} // end namespace webspider
} // end namespace twse

using ::twse::webspider::LogOptions;
using ::twse::webspider::Log;
using ::twse::webspider::NullLog;

#define INIT_LOG ::twse::webspider::g_pLog = new Log
#define LOG ::twse::webspider::g_pLog->Write
#define FATAL_LEVEL   ::twse::webspider::Log::Level_Fatal, __FILE__, __LINE__
#define ERROR_LEVEL   ::twse::webspider::Log::Level_Error, __FILE__, __LINE__
#define WARNING_LEVEL    ::twse::webspider::Log::Level_Warning, __FILE__, __LINE__
#define NOTICE_LEVEL    ::twse::webspider::Log::Level_Notice, __FILE__, __LINE__
#define INFO_LEVEL    ::twse::webspider::Log::Level_Info, __FILE__, __LINE__
#define DEBUG_LEVEL   ::twse::webspider::Log::Level_Debug, __FILE__, __LINE__
#define VERBOSE_LEVEL ::twse::webspider::Log::Level_Verbose, __FILE__, __LINE__

#endif

