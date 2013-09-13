// public_objs.cpp: define all global objects
// wookin@tencent.com
// 2010-05-27
//////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/baselib/svrpublib/base_config.h"
#include "thirdparty/gflags/gflags.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;

// xfs集群名字
DEFINE_string(xfs_cluster_name,"xaec-unittest", "xfs cluster name");

// for binary log
DEFINE_bool(binary_log, false, "output binary log or not\r\n");
DEFINE_int32(binary_log_size, 10*1024*1024,
             "Maximum file size of each binary log [bytes]\r\n");
DEFINE_int32(binary_log_count, 10, "Maximum log file count\r\n");



_START_XFS_BASE_NAMESPACE_

//
// Public object
//
CAutoLibGlobalVars      g_lib_vars;


#ifdef USING_GOOGLE_MEMPOOL
GoogleMemPool*          g_mempool_obj = NULL;
#else
CMemPoolObj*            g_mempool_obj = NULL;
#endif
BinaryLog_<void>*       g_binary_log = NULL;
CLongConnAuxParameters* g_long_conn_aux_parameters = NULL;


// 设置默认的glog参数
char g_log_module_name[MAX_PATH] = {0};
// glog需要一个全局的文件名
void InitGoogleDefaultLogParam(const char* program) {    

    // init google glog
    // 默认在当前运行目录下生成日志
    GetModuleFileName(NULL, g_log_module_name, sizeof(g_log_module_name));
    if (program == NULL) {
        InitGoogleLogging(g_log_module_name);
    } else {
        InitGoogleLogging(program);
    }

    // linux and windows
    char* p = strrchr(g_log_module_name, '/');
    if (!p)
        p = strrchr(g_log_module_name, '\\');
    if (p)
        *p = 0;
    
    FLAGS_log_dir = g_log_module_name;
    FLAGS_log_symlink = false;  // 不用symlink

#ifdef NDEBUG
#else
    // debug模式下默认输出到屏幕
    FLAGS_stderrthreshold = 0;
#endif //
}

// 初始化svrpublib
void InitBaseLib() {
    ATOM_INT val = InterlockedIncrement(&g_lib_vars.m_num_init_base_lib_count);
    if (val == 1) {

#ifdef USING_GOOGLE_MEMPOOL
        // init baselib
        if (!g_mempool_obj)
            g_mempool_obj = new GoogleMemPool;
#else
        if (!g_mempool_obj)
            g_mempool_obj = new CMemPoolObj;
#endif
        if (!g_binary_log) {
            g_binary_log = new BinaryLog_<void>;
        }
        if (!g_long_conn_aux_parameters)
            g_long_conn_aux_parameters = new CLongConnAuxParameters;
    }
    GetXFSVerAndUUID();
}

// 卸载svrpublib
void ShutdownBaseLib() {
    ATOM_INT val = InterlockedDecrement(&g_lib_vars.m_num_init_base_lib_count);
    if (val == 0) {		
        // shutdown baselib
        delete g_long_conn_aux_parameters;
        g_long_conn_aux_parameters = 0;

        delete g_mempool_obj;
        g_mempool_obj = 0;

        delete g_binary_log;
        g_binary_log = 0;
    }

    //
    // 显示还有多少socket handle在使用中
    //
    ListSocketsInUse();
}

_END_XFS_BASE_NAMESPACE_


