// log.inl

inline const char* GetLogLevelName(ENUM_LOG_LEVEL level, bool is_short) {
    switch (level) {
    case INFO:
        return is_short ? "I":"INFO";
    case WARN:
        return is_short ? "W":"WARN";
    case ERR:
        return is_short ? "E":"ERROR";
    case FATAL:
        return is_short ? "F":"FATAL";
    case NO_LOG:
        return is_short ? "N":"NO_LOG";
    default:
        break;
    }
    return "UNKNOWN-LOG-LEVEL";
}

//
//  获取用户名字
//
template<class T>
bool GetUserName(char* buff, T len) {
    if (!buff)
        return false;

#ifdef WIN32
    char* user = getenv("USERNAME");
#else // linux
    char* user = getenv("USER");
#endif //
    if (user)
        safe_snprintf(buff, (int32_t)len, "%s", user);
    else
        safe_snprintf(buff, (int32_t)len, "%s", "invalid-user");
    return true;
}

//
//  获取本机计算机名字
//
inline bool GetLocalHostName(char* buff, int32_t len) {
    if (!buff)
        return false;
    memset(buff, 0, len);

#ifdef WIN32
    DWORD ret_len = len;
    if (!GetComputerNameA((LPTSTR)buff, &ret_len)) {
        safe_snprintf(buff, len, "unknown");
    }
#else // linux
    struct utsname uname_buf;
    if (uname(&uname_buf) == 0)
        safe_snprintf(buff, len, "%s", uname_buf.nodename);
    else
        safe_snprintf(buff, len, "%s", "unknown");
#endif //
    return true;
}

template<class T>
bool GetFileString(T full_path, T* filename) {
    if (!full_path)
        return false;

#ifdef WIN32
    T ptr = reinterpret_cast<T>(strrchr(full_path, '\\'));
#else // linux
    T ptr = reinterpret_cast<T>(strrchr(full_path, '/'));
#endif //

    *filename = ptr ? (ptr+1):full_path;
    return true;
}

//
// LONG : 20100622-162006.4308
// SHORT: 0622 16:20:06.366624
//
template<class T>
bool GetTimeString(bool is_short, struct timeval tv, char* buff, T len) {
    bool b = false;
    if (buff && len >= 23) {
        struct tm t0 = {0};
        time_t tsec = tv.tv_sec;

#ifdef WIN32
        safe_localtime(&tsec, &t0);
#else
        localtime_r(&tsec, &t0);
#endif //
        struct tm* pt = 0;
        pt = &t0;

        if (is_short) // SHORT style
            safe_snprintf(buff, (int32_t)len, "%02d%02d %02d:%02d:%02d.%6d",
                          pt->tm_mon+1,
                          pt->tm_mday,
                          pt->tm_hour,
                          pt->tm_min,
                          pt->tm_sec,
                          (int32_t)tv.tv_usec);
        else        // LONG style
            safe_snprintf(buff, (int32_t)len, "%2d%02d%02d-%02d%02d%02d.%d",
                          pt->tm_year+1900,
                          pt->tm_mon+1,
                          pt->tm_mday,
                          pt->tm_hour,
                          pt->tm_min,
                          pt->tm_sec,
                          (int32_t)tv.tv_usec);
        b = true;
    }
    return b;
}

//
// 动态配置LOG
// T:ENUM_LOG_LEVEL
//
template<class T>void ConfigLOG(T level,
                                bool debug_2_screen,
                                bool debug_2_logfile,
                                bool debug_2_xml,
                                bool flush_2_file) {
    //
    // LogParam* p = __GetLogObj();
    //
    __GetLogLevelVal = level;
    __GetLogSettingDebug2Log = debug_2_logfile;    //  是否输出到日志文件
    __GetLogSettingDeubg2Screen = debug_2_screen;  //  是否输出日志到屏幕
    __GetLogSettingDeubg2XMLFmt = debug_2_xml;     //  是否输出为XML格式文件
    __GetLogSettingFlush2Log = flush_2_file;       //  ? flush to file
    //    immediately
}



//
// <program name>.log.<hostname>.<user name>.<severity level:level-xxx> ***
// 后面部分用于判断是否存在该日志后才决定哪个ID
//
// "<program name>.<hostname>.<user name>.
//  <severity level:level-xxx>.IDxx<循环日志编号>.log"
//
template<typename T>
void GetLogFileNamePart(char* buff, T len) {
    if (!buff || len <= 0)
        return;

    LogParam* log_obj = __GetLogObj();
    if (!log_obj)
        return;

    // build log file name
    // "<program name>.log.<hostname>.<user name>.<severity level:level-xxx>"
    // TestSvrpublib.exe.log.WOOKIN-NB.wookin.level-ALL.ID00.log
    // char log_filename[512] = {0};

    // log file name
    char* ptr_log = buff;
    int32_t free_len = len;
    int32_t n = safe_snprintf(ptr_log, free_len, "%s",
                              log_obj->m_log_module_name);
    ptr_log += n;
    free_len -= n;

    // host name
    char tmp_buff[256] = {0};
    GetLocalHostName(tmp_buff, sizeof(tmp_buff));
    n = safe_snprintf(ptr_log, free_len, ".%s", tmp_buff);
    ptr_log += n;
    free_len -= n;

    // user name
    GetUserName(tmp_buff, sizeof(tmp_buff));
    n = safe_snprintf(ptr_log, free_len, ".%s", tmp_buff);
    ptr_log += n;
    free_len -= n;

    // .log
    n = safe_snprintf(ptr_log, free_len, ".%s", "log");
    ptr_log += n;
    free_len -= n;

    // log level
    n = safe_snprintf(ptr_log, free_len,
                      ".level-%s",
                      GetLogLevelName(__GetLogLevel(), false));
    ptr_log += n;
    free_len -= n;

    // 过滤文件名中非法字符
    char* ptr = {0};
    GetFileString(buff, &ptr);
    while (ptr && *ptr) {
        char val = *ptr;
        if (val == '/' || val == '\\' || val == ':' ||
                val == '*' || val == '?' ||
                val == '"' || val == '<' || val == '>' || val == '|')
            *ptr = '_';
        ptr++;
    }
}

template<typename T>
void PrepareLogFileName(char* buff, T len) {
    if (!buff || len <= 0)
        return;

    LogParam* log_obj = __GetLogObj();
    if (!log_obj)
        return;

    int32_t file_id = 0;
    // get part file name
    GetLogFileNamePart(buff, len);
    if (__GetLogSettingOpenForAppend &&
            log_obj->m_select_file_id_first) {
        for (int32_t i = 0; i < NUM_MAX_LOG_FILES; i++) {
            char tmp_filename[512+1] = {0};
            safe_snprintf(tmp_filename, sizeof(tmp_filename),
                          "%s.ID%02d.log",
                          buff, i);
            FILE* fp = fopen(tmp_filename, "rb");
            // maybe file not exist
            if (!fp) {
                file_id = i;
                break;
            }

            fseek(fp, 0, SEEK_END);
            uint32_t file_len = (uint32_t)ftell(fp);
            if (file_len >= __GetLogEachFileMaxSize()) {
                fclose(fp);
                // find next
            } else {
                fclose(fp);
                // can use this file id
                file_id = i;
                break;
            }
        }
        log_obj->m_select_file_id_first = false;
    } else {
        file_id = log_obj->m_num_logfile_id_count % NUM_MAX_LOG_FILES;
    }

    char tmp_id[32] = {0};
    safe_snprintf(tmp_id, sizeof(tmp_id), ".ID%02d.log", file_id);
    file_id++;
    log_obj->m_num_logfile_id_count = file_id;
    uint32_t valid_len = (uint32_t)strlen(buff);
    safe_snprintf(buff + valid_len, len - valid_len, "%s", tmp_id);
}

//
// svrpublib原有LOG方式
// 支持循环日志
// 支持多线程写一个日志文件(竟锁)
// [IWEFS]mmdd hh:mm:ss.uuuuuu threadid file:line] msg
//
template<class T>
void XprintfStr(ENUM_LOG_LEVEL level, const char* file, T line, char* buff) {
    LogParam* log_obj = __GetLogObj();
    if (!log_obj)
        return;

    // Get file name
    const char* code_filename = NULL;
    GetFileString(file, &code_filename);

    // Get time string
    char time_sz[36] = {0};
    struct timeval tv = {0};
    lite_gettimeofday(&tv, 0);
    GetTimeString(true, tv, time_sz, sizeof(time_sz));

    // Get current thread id
    uint32_t thread_id = 0;
    GetSelfThreadID(&thread_id);

    // Build output format string
    char output_fmt[128];
    int32_t bytes = safe_snprintf(output_fmt, sizeof(output_fmt),
                                  "%s%s %u %s:%u>",
                                  GetLogLevelName(level, true),
                                  time_sz,
                                  thread_id,
                                  code_filename,
                                  line);
    output_fmt[bytes] = 0;

    //
    //  START:print debug info on screen
    //
    if (__GetLogSettingDeubg2Screen ) {
        if (__GetLogSettingDeubg2XMLFmt )
            printf("<Xprintf><![CDATA[\r\n");

        // set color
#ifdef WIN32
        WORD console_old_attributes;
        HANDLE console_handle   = GetStdHandle(STD_OUTPUT_HANDLE);
        bool set_color_ok       = SetLogTextColor(level, console_handle,
                                  &console_old_attributes);
#else
        SetLogTextColor(level);
#endif //

        // output message to screen
        printf("%s %s", output_fmt, buff);

        // use default color
#ifdef WIN32
        if (set_color_ok) {
            RecoverLogTextColor(console_handle, console_old_attributes);
            // TODO(wookin): 需要留意下, GetStdHandle()后，
            //     是否要调用CloseHandle()释放句柄，以免泄漏?
        }
#else
        RecoverLogTextColor(level);
#endif //

        if (__GetLogSettingDeubg2XMLFmt )
            printf("]]></Xprintf>\r\n");
    }

    //
    //  output debug string
    //
#if ((defined(WIN32) || defined(_WINDOWS)))
    OutputDebugString(output_fmt);
    OutputDebugString(" ");
    OutputDebugString(buff);
#endif // _WINDOWS

    //
    //  START:output to log file
    //
    if (!__GetLogSettingDebug2Log)
        return;

    //
    //  Create the empty file
    //  check file size
    //
    if (log_obj->m_fp_logfile &&
            (ftell(log_obj->m_fp_logfile)  >= (long) __GetLogEachFileMaxSize())) {
        fclose(log_obj->m_fp_logfile);
        log_obj->m_fp_logfile = 0;
    }

    if (!log_obj->m_fp_logfile) {
        // build log file name
        // "<program name>.<hostname>.<user name>.
        //  <severity level:level-xxx>.IDxx<循环日志编号>.log"
        // TestSvrpublib.exe.log.WOOKIN-NB.wookin.level-ALL.ID00.log
        char log_filename[512] = {0};

        // get log file name
        int32_t buff_len = (int32_t)sizeof(log_filename);
        PrepareLogFileName(log_filename, buff_len);

        // open log file
        FILE* fp = 0;
        if (__GetLogSettingOpenForAppend) {
            fp = fopen(log_filename, "ab+");
            // 判断文件大小是否大于限制,
            // 如果超出大小则删除文件重新创建一个空文件
            if (fp) {
                fseek(fp, 0, SEEK_END);
                uint32_t file_len = (uint32_t)ftell(fp);
                if (file_len >= __GetLogEachFileMaxSize()) {
                    fclose(fp);
                    fp = fopen(log_filename, "wb+");
                }
            }
        } else
            fp = fopen(log_filename, "wb+");

        if (fp) {
            // setvbuff, 设置缓冲区大小
            if (__GetLogSettingCacheSize)
                setvbuf(fp, log_obj->m_output_vbuff,
                        _IOFBF, __GetLogSettingCacheSize);

            // output head info
            fprintf(fp, "Format = SOSO.LOG\r\n");
            fprintf(fp, "Version = 1.0\r\n");
            char time_long[36] = {0};
            GetTimeString(false, tv, time_long, sizeof(time_long));
            fprintf(fp, "Log file created at:%s\r\n", time_long);
            char host[64] = {0};
            GetLocalHostName(host, sizeof(host));
            fprintf(fp, "Running on machine:%s\r\n", host);
            fprintf(fp, "Log level:%s\r\n",
                    GetLogLevelName(__GetLogLevel(), false));

            fprintf(fp,
                    "Log line format: [IWEFS]mmdd hh:mm:ss."
                    "uuuuuu threadid file:line> msg\r\n");
            fprintf(fp,
                    "----------------------------------------"
                    "-----------------------------\r\n\r\n");
            log_obj->m_fp_logfile = fp;
        }
    }

    if (log_obj->m_fp_logfile) {
        fprintf(log_obj->m_fp_logfile, "%s %s", output_fmt, buff);
        if (__GetLogSettingFlush2Log) {
            fflush(log_obj->m_fp_logfile);
        }
    }
    //  END:output to log file
}

inline void    XSetLogFileName(char* log_filename) {
    if (log_filename) {
        LogParam* p = __GetLogObj();
        __GetLoopLogMutex()->Lock();
        safe_snprintf(p->m_log_module_name,
                      sizeof(p->m_log_module_name),
                      "%s",
                      log_filename);
        if (p->m_fp_logfile) {
            fclose(p->m_fp_logfile);
            p->m_fp_logfile = 0;
        }
        __GetLoopLogMutex()->UnLock();
    }
}

#ifdef WIN32

inline bool SetLogTextColor(ENUM_LOG_LEVEL level, HANDLE console_handle,
                            WORD * console_old_attributes) {
    CONSOLE_SCREEN_BUFFER_INFO console_default_setting;
    BOOL is_ok = FALSE;

    if ((level>INFO)
            && (INVALID_HANDLE_VALUE != console_handle)
            && (NULL != console_handle)
            && (TRUE == (is_ok = GetConsoleScreenBufferInfo(console_handle,
                                 &console_default_setting)))) {
        *console_old_attributes = console_default_setting.wAttributes;

        switch (level) {
        case WARN:
            SetConsoleTextAttribute(console_handle, FOREGROUND_RED |
                                    FOREGROUND_GREEN |
                                    FOREGROUND_INTENSITY);
            break;
        case ERR:
            SetConsoleTextAttribute(console_handle, FOREGROUND_RED |
                                    FOREGROUND_INTENSITY);
            break;
        case FATAL:
            SetConsoleTextAttribute(console_handle, FOREGROUND_RED |
                                    FOREGROUND_INTENSITY);
            break;
        default:
            break;
        }
    }

    return (TRUE == is_ok) ? true : false;
}

inline void RecoverLogTextColor(HANDLE console_handle,
                                WORD console_old_attributes) {
    if ((INVALID_HANDLE_VALUE != console_handle) && (NULL != console_handle)) {
        SetConsoleTextAttribute(console_handle, console_old_attributes);
    }
}

#else

inline void SetLogTextColor(ENUM_LOG_LEVEL level) {
    if (level>INFO) {
        const char* color = "0";
        switch (level) {
        case WARN:
            color = "33";
            break;
        case ERR:
            color = "31";
            break;
        case FATAL:
            color = "31";
            break;
        default:
            break;
        }
        printf("\033[1;%s;40m", color);
    }
}

inline void RecoverLogTextColor(ENUM_LOG_LEVEL level) {
    if (level>INFO) {
        printf("\033[0m");
    }
}


#endif


