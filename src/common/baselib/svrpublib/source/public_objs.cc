// public_objs.cpp: define all global objects
// wookin@tencent.com
// 2010-05-27
//////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/baselib/svrpublib/base_config.h"
#include "thirdparty/gflags/gflags.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;

// xfs��Ⱥ����
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


// ����Ĭ�ϵ�glog����
char g_log_module_name[MAX_PATH] = {0};
// glog��Ҫһ��ȫ�ֵ��ļ���
void InitGoogleDefaultLogParam(const char* program) {    

    // init google glog
    // Ĭ���ڵ�ǰ����Ŀ¼��������־
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
    FLAGS_log_symlink = false;  // ����symlink

#ifdef NDEBUG
#else
    // debugģʽ��Ĭ���������Ļ
    FLAGS_stderrthreshold = 0;
#endif //
}

// ��ʼ��svrpublib
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

// ж��svrpublib
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
    // ��ʾ���ж���socket handle��ʹ����
    //
    ListSocketsInUse();
}

_END_XFS_BASE_NAMESPACE_


