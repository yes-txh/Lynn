// Copyright 2010, Tencent Inc.
// Author: typherque (typherque@tencent.com)
//         Yongqiang Zou(aaronzou@tencent.com)

#include "common/distribute_lock/zookeeper_wrapper.h"
#include "common/distribute_lock/zkwrapper_error_code.h"

#include <algorithm>
#include <fstream>

#include "common/base/stdint.h"
#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/base/string/string_algorithm.hpp"
#include "common/system/concurrency/mutex.hpp"

#include "common/system/concurrency/sync_event.hpp"

#ifndef _WIN32
#include "common/distribute_lock/error_code.h"
#include "common/distribute_lock/event_listener.h"
#endif

#include "thirdparty/glog/logging.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE

using namespace distribute_lock;

DEFINE_bool(zookeeper, true, "use zookeeper or local file");
DEFINE_bool(enable_zookeeper_auth, true, "whether to enable zk authentication");
DEFINE_string(zookeeper_auth, "xfs:xfspublic", "username:password to set value of zookeeper.");
DEFINE_int32(zookeeper_timeout, 2000, "timeout for request to zookeeper");

namespace {
static const std::string kZookeeperHost = ".zk.oa.com:2181";
static const int32_t kRetryTimes4zk = 2;
static const uint32_t kMaxPathLength = 4096;
static const std::string kZkPort = ":2181";

Mutex g_zk_wrapper_mutex;
Mutex g_zk_wrapper_read_node_mutex;

void ZookeeperAddAuthCallback(int rc, const void* context) {
#ifndef _WIN32
    MutexLocker global_locker(&g_zk_wrapper_mutex);
    ZookeeperWrapper* wrapper = (ZookeeperWrapper*) context;
    CHECK(wrapper != NULL)
        << "the wrapper in zookeeper callback context must NOT NULL.";
    LOG(INFO) << "set auth call back error code: "
               << std::string(distribute_lock::ErrorString(rc));

    wrapper->SetZookeeperStatus(rc);
    if (wrapper->IsInitializingZookeeperACL()) {
        wrapper->GetSyncEvent()->Set();
    }
#endif
}

}

bool ZookeeperWrapper::InitZookeeperAcl(uint32_t* error_code) {
    if (!FLAGS_zookeeper)
        return true; // local file no need to Set ACL

    if (!FLAGS_enable_zookeeper_auth) {
        VLOG(1) << "Disabled zookeeper authentication, Init ACL OK";
        return true;
    }
    // --zookeeper=true
#ifndef _WIN32
    if (FLAGS_zookeeper_auth.length() == 0) {
        VLOG(1) << "ignore zookeeper ACL, no auth is provided";
        return true;
    }

    m_zk_status = ZSYSTEMERROR;

    m_is_initializing_zk_acl = true;
    m_sync_event.Reset();
    const int32_t add_ret = zoo_add_auth(m_dist_lock.GetZKHandle(), "digest",
                                 FLAGS_zookeeper_auth.c_str(), FLAGS_zookeeper_auth.length(),
                                 ZookeeperAddAuthCallback, this);
    if (add_ret != ZOK) {
        // 调用zoo_add_auth失败
        m_is_initializing_zk_acl = false;

        if (error_code == NULL) {
            LOG(ERROR) << "add auth failed, error_code = "
                       << std::string(distribute_lock::ErrorString(add_ret));
        }
        SET_ERRORCODE(error_code, ERROR_ZKWRAPPER_INIT_ACL_FAIL);
        return false;
    } else {
        bool sync_success = m_sync_event.Wait(FLAGS_zookeeper_timeout + 1000);
        m_is_initializing_zk_acl = false;

        // 等待超时或者失败
        if (!sync_success) {
            if (error_code == NULL) { LOG(ERROR) << "wait add auth operation callback fail.";}
            SET_ERRORCODE(error_code, ERROR_ZKWRAPPER_INIT_ACL_FAIL);
            return false;
        }

        // 等待成功,处理回调时设置的错误码
        if (m_zk_status != ZOK) {
            if (error_code == NULL) {
                LOG(ERROR) << "wait add auth callback ok, add auth failed: "
                           << std::string(distribute_lock::ErrorString(m_zk_status));
            }
            SET_ERRORCODE(error_code, ERROR_ZKWRAPPER_INIT_ACL_FAIL);
            return false;
        }
        if (error_code == NULL){
            VLOG(1) << "Init zookeeper acl OK";
        }
    }

    return true;
#else
    SET_ERRORCODE(error_code, ERROR_ZKWRAPPER_INIT_ACL_FAIL);
    return false;
#endif

}

ZookeeperWrapper::ZookeeperWrapper(ZookeeperListenStruct listen_struct):
#ifndef WIN32
	  m_previous_clientid(0),
#endif
      m_listen_struct(listen_struct),
      m_is_zk_client_valid(false),
      m_is_initializing_zk_acl(false) {
    if (listen_struct.event_listener != NULL) {
        CHECK(listen_struct.interest_cluster != ""
              && listen_struct.interest_key != ""
              && listen_struct.event_mask != distribute_lock::EVENT_MASK_NONE)
            << "When init ZookeeperWrapper with event listener,"
            << " must set interesting cluster, key, and event mask";
    }
}

// 如果一个进程同时会访问不同的cluster, 对应的zk可能不同，所以每次都直接传入cluster_name
bool ZookeeperWrapper::GetValue(const std::string& cluster_name,
                                const std::string& key,
                                std::string* value,
                                int* version,
                                uint32_t* error_code) {
    SET_ERRORCODE(error_code, ERROR_OK);
    if (!FLAGS_zookeeper)
        return GetValueByLocalFile(cluster_name, key, value, error_code);

    // --zookeeper=true
#ifndef _WIN32
    return GetValueByZookeeper(cluster_name, key, value, version, error_code);
#else
    return false;
#endif

}

bool ZookeeperWrapper::SetValue(const std::string& cluster_name,
    const std::string& key, const std::string& value, int expect_version) {
    if (!FLAGS_zookeeper)
        return SetValueByLocalFile(cluster_name, key, value);

    // --zookeeper=true
#ifndef _WIN32
    return SetValueByZookeeper(cluster_name, key, value, expect_version);
#else
    return false;
#endif
}

bool ZookeeperWrapper::CreateNode(
    const std::string& cluster_name,
    const std::string& key,
    ZKEventMask event_mask,
    bool is_temporary) {
    if (!FLAGS_zookeeper) return CreateNodeByLocalFile(cluster_name, key);
#ifndef _WIN32
    return CreateNodeByZookeeper(cluster_name, key, event_mask, is_temporary);
#else
    return false;
#endif
}

bool ZookeeperWrapper::CreateNodeByLocalFile(
    const std::string& cluster_name, const std::string& key) {
    // Do nothing. Because set value will create the file.
    return true;
}

bool ZookeeperWrapper::CreateNodeByZookeeper(
    const std::string& cluster_name,
    const std::string& key,
    ZKEventMask event_mask,
    bool is_temporary) {
#ifndef _WIN32
    uint32_t error_code;

    if (!ReInitZooKeeperIfNeccessary(cluster_name, &error_code)) {
        return false;
    }
    int32_t retry_times = 0;

    std::string key_full_path;
    if (!GetZkNodeName(cluster_name, key, &key_full_path, &error_code)) return false;

    int ret = ZOK;
    while (retry_times < kRetryTimes4zk) {
        retry_times++;

        ret = m_dist_lock.CreateNode(key_full_path, event_mask, is_temporary);
        if (ret == ZOK) return true;
    }

    LOG(ERROR) << "fail to create node " << key_full_path
        << " after retry " << retry_times << " times, " << ErrorString(ret);

    return false;
#else
    return false;
#endif
}

bool ZookeeperWrapper::Init(const std::string& cluster_name) {
    if (!FLAGS_zookeeper)
        return true;

    // --zookeeepr=true
#ifdef _WIN32
    return false;
#else
#ifdef NDEBUG
    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
#endif
    return ReInitZooKeeperIfNeccessary(cluster_name, NULL);
#endif
}

void ZookeeperWrapper::Close() {
#ifndef _WIN32
    m_dist_lock.Close();
    SetClientInvalid();
#endif
}

bool ZookeeperWrapper::Lock(const std::string& cluster_name,
                            const std::string& key,
                            bool blocked_type) {
    if (!FLAGS_zookeeper)
        return true;

    // --zookeeper=true
#ifdef _WIN32
    return false;
#else
    return LockByZookeeper(cluster_name, key, blocked_type);
#endif
}

bool ZookeeperWrapper::SetWatcher(const std::string& cluster_name,
                                  const std::string& key,
                                  distribute_lock::ZKEventMask event_mask) {
    if (!FLAGS_zookeeper)
        return true;

    // --zookeeper=true
#ifdef _WIN32
    return false;
#else
    return SetWatcherZookeeper(cluster_name, key, event_mask);
#endif
}

#ifndef _WIN32
bool ZookeeperWrapper::GetValueByZookeeper(const std::string& cluster_name,
        const std::string& key,
        std::string* value,
        int* version,
        uint32_t* error_code) {
    if (!ReInitZooKeeperIfNeccessary(cluster_name, error_code)) {
        return false;
    }
    int32_t ret = 0;
    int32_t retry_times = 0;
    std::string val;

    std::string key_full_path;
    if (!GetZkNodeName(cluster_name, key, &key_full_path, error_code)) return false;

    while (retry_times < kRetryTimes4zk) {
        ++retry_times;
        ret = m_dist_lock.ZooGet(key_full_path, val, version);
        if (ret == 0) {
            VLOG(1) << "Get Attribute OK: " << val;
            value->assign(val);
            SET_ERRORCODE(error_code, ERR_OK);
            return true;
        }

        SET_ERRORCODE(error_code, ERROR_ZKWRAPPER_ZOOGET_FAIL);
        if (error_code == NULL) {
            LOG(ERROR) << "key_full_path is : " << key_full_path
                << ", m_dist_lock.ZooGet return " << ret << " : "
                << distribute_lock::ErrorString(ret) << "("
                << zerror(GetZKHandleState()) << ")";
        }

        if (CheckNeedReinit(ret)) {
            m_is_zk_client_valid = false; // to force re-init

            if(ReInitZooKeeperIfNeccessary(cluster_name, error_code)) {
                continue;
            } else {
                if (error_code == NULL) {
                    LOG(ERROR) << "Get attribute from zookeeper fail : " << key_full_path
                        << ", reason: " << distribute_lock::ErrorString(ret);
                }
                return false;
            }
        }
    }

    if (error_code == NULL) {
        LOG(ERROR) << "Get attribute from zookeeper fail : " << key_full_path
            << ", reason: " << distribute_lock::ErrorString(ret);
    }
    return false;
}

bool ZookeeperWrapper::SetValueByZookeeper(const std::string& cluster_name,
        const std::string& key,
        const std::string& value,
        int expect_version) {
    if (!ReInitZooKeeperIfNeccessary(cluster_name)) {
        return false;
    }
    int32_t ret = 0;
    int32_t retry_times = 0;
    std::string idc_name;
    if (!ParseIDCName(cluster_name, &idc_name)) {
        return false;
    }

    std::string key_full_path;
    if (!GetZkNodeName(cluster_name, key, &key_full_path)) return false;

    while (retry_times < kRetryTimes4zk) {
        ++retry_times;
        ret = m_dist_lock.ZooSet(key_full_path, value, expect_version);
        if (ret == 0) {
            VLOG(2) << "Set Attribute OK as: " << value;
            return true;
        }

        LOG(ERROR) << "Set attribute from zookeeper fail : " << key_full_path
                   << ", reason: " << distribute_lock::ErrorString(ret);
        if (CheckNeedReinit(ret)) {
            m_is_zk_client_valid = false; // to force re-init

            if(ReInitZooKeeperIfNeccessary(cluster_name)) {
                continue;
            } else {
                return false;
            }
        }
    }


    return false;
}

bool ZookeeperWrapper::LockByZookeeper(const std::string& cluster_name,
                                       const std::string& key,
                                       bool blocked_type) {
   if (!ReInitZooKeeperIfNeccessary(cluster_name)) {
       return false;
   }
   std::string idc_name;
   if (!ParseIDCName(cluster_name, &idc_name)) {
       return false;
   }

   std::string key_full_path;
   if (!GetZkNodeName(cluster_name, key, &key_full_path)) return false;

   int32_t ret = m_dist_lock.Lock(key_full_path, blocked_type);
   if (ret == 0) {
       VLOG(1) << "Lock OK: " << key_full_path;
       return true;
   } else {
       LOG(ERROR) << "Lock fail: " << key_full_path
           << ", reason: " << distribute_lock::ErrorString(ret);
       return false;
   }
}

bool ZookeeperWrapper::SetWatcherZookeeper(const std::string& cluster_name,
                                           const std::string& key,
                                           distribute_lock::ZKEventMask event_mask) {
   if (!m_is_zk_client_valid || (cluster_name != m_cluster_name)) {
       LOG(ERROR) << "Must init distribute_lock before set watcher."
               << " m_is_zk_client_valid=" << m_is_zk_client_valid
               << ", cluster_name=" << cluster_name
               << ", key=" << key;
       return false;
   }

   std::string idc_name;
   if (!ParseIDCName(cluster_name, &idc_name)) {
       return false;
   }

   std::string key_full_path;
   if (!GetZkNodeName(cluster_name, key, &key_full_path)) return false;

   int32_t ret = m_dist_lock.SetWatcher(key_full_path, event_mask);
   if (ret == 0) {
       VLOG(1) << "SetWatcher OK: " << key_full_path << ", event=" << event_mask;
       return true;
   } else {
       LOG(ERROR) << "SetWatcher fail: " << key_full_path
           << ", reason: " << distribute_lock::ErrorString(ret);
       return false;
   }
}
#endif

bool ZookeeperWrapper::GetValueByLocalFile(const std::string& cluster_name,
        const std::string& key,
        std::string* value,
        uint32_t* error_code) {
    std::string config_full_path;
    if (!GetConfigFileName(cluster_name, key, &config_full_path, error_code)) {
        return false;
    }

    std::ifstream in_file(config_full_path.c_str());
    if (!in_file.good()) {
        if (error_code) {
            SET_ERRORCODE(error_code, ERROR_ZKWRAPPER_OPEN_CONFIG_FILE_FAIL);
        } else {
            LOG(ERROR) << "Fail to open file : " << config_full_path;
        }
        return false;
    }
    // 读整个文件内容
    value->assign(std::istreambuf_iterator<char>(in_file.rdbuf()),
                  std::istreambuf_iterator<char>());
    in_file.close();
    return true;
}


bool ZookeeperWrapper::SetValueByLocalFile(const std::string& cluster_name,
        const std::string& key,
        const std::string& value) {
    std::string config_full_path;
    if (!GetConfigFileName(cluster_name, key, &config_full_path)) {
        return false;
    }

    std::ofstream out_file(config_full_path.c_str());
    if (!out_file.good()) {
        LOG(ERROR) << "Fail to open file : " << config_full_path;
        return false;
    }

    std::ostreambuf_iterator<char> out_it(out_file);
    std::copy(value.begin(), value.end(), out_it);

    out_file.close();
    return true;
}

bool ZookeeperWrapper::ParseIDCName(const std::string& cluster_name, std::string* idc_name,
                                    uint32_t* error_code) {
    if (cluster_name.empty() || *(cluster_name.begin()) == '-') {
        if (error_code == NULL) {
            LOG(ERROR) << "invalid cluster name format : " << cluster_name;
        }
        SET_ERRORCODE(error_code, ERROR_ZKWRAPPER_INVALID_CLUSTER_NAME);
        return false;
    }
    // 包括两种形式, 如xaec-web, xaec都可以解析
    std::string::size_type index = cluster_name.find('-');
    idc_name->assign(cluster_name.substr(0, index));
    return true;
}

bool ZookeeperWrapper::ReInitZooKeeperIfNeccessary(const std::string& cluster_name,
                                                   uint32_t* error_code) {
#ifndef _WIN32
    uint32_t max_retry = 3;
    uint32_t try_count =0;
    bool is_ok = false;
    for(; try_count < max_retry; ++try_count) {
        // TODO: (wookin)这个版本显示所有日志
        // is_ok = ImplReInitZooKeeperIfNeccessary(cluster_name, error_code);
        is_ok = ImplReInitZooKeeperIfNeccessary(cluster_name, NULL);
        if (is_ok) {
            break;
        } else {
            if (try_count >= 1 && m_previous_clientid != 0) {
                VLOG(1) << "fail to init zookeeper at try count " << try_count
                    << " for " << cluster_name
                    << " with previous clientid. Maybe clientid has become invalid."
                    << " Reset clientid and try again for a fresh connection.";
                SetPreviousClientid(0);
            }
        }
        LOG(ERROR) << "ImplReInitZooKeeperIfNeccessary( cluster_name = " << cluster_name
            << ") fail, try count: " << (try_count + 1);
    }

    return is_ok;
#else
    return false;
#endif //
}

bool ZookeeperWrapper::ImplReInitZooKeeperIfNeccessary(const std::string& cluster_name,
                                                   uint32_t* error_code) {
#ifndef _WIN32
    // zookeeper client本身有效，且当前请求的集群是就对应当前的zookeeper client
    if (m_is_zk_client_valid && (cluster_name == m_cluster_name)) {
        return true;
    }

    LOG(INFO) << "m_is_zk_client_valid : " << m_is_zk_client_valid << ","
              << "m_cluster_name = " << m_cluster_name << ",cluster_name = " << cluster_name
              << ", m_previous_clientid = "
              << (m_previous_clientid == 0 ? 0 : m_previous_clientid->client_id)
              << ", GetZKHandleState() = " << GetZKHandleState() << ": "
              << zerror(GetZKHandleState());

    // 需要将上次已经连接到zk的client关闭了
    if (m_previous_clientid == 0) {
        // only close session when no previous saved session so that to reuse session.
    }
    m_dist_lock.Close();
    SetClientInvalid();

    // 如果连idc name都获取不到,一定传入了错误的cluster_name
    std::string idc_name;
    if (!ParseIDCName(cluster_name, &idc_name, error_code)) {
        return false;
    }

    std::string zk_server_host = idc_name + kZookeeperHost;
    m_cluster_name = cluster_name;

    if (m_listen_struct.interest_cluster != "") {
        CHECK(m_cluster_name == m_listen_struct.interest_cluster)
            << "Only allow interesting cluster the same as init cluster";
    }

	LOG(INFO) << "m_dist_lock reinit: " << zk_server_host;
    int32_t ret = m_dist_lock.Init(
        m_previous_clientid, // if given 0, issus a fresh connection, or else re-use the clientid.
        zk_server_host,
        m_listen_struct.event_listener,
        FLAGS_zookeeper_timeout);
    if (ret != 0) {
        if (error_code == NULL) {
            LOG(ERROR) << "Initialize zookeeper fail : " << distribute_lock::ErrorString(ret)
                       << " for " << zk_server_host;
        }

        m_dist_lock.Close();
        SET_ERRORCODE(error_code, ERROR_ZKWRAPPER_INIT_ZK_FAIL);
        SetClientInvalid();
        return false;
    }

	LOG(INFO) << "InitZookeeperAcl()";
    bool acl_success = InitZookeeperAcl(error_code);
    if (!acl_success) {
        if (error_code == NULL) {
            LOG(ERROR) << "Initialize zookeeper error due to fail to init acl";
        }
        SET_ERRORCODE(error_code, ERROR_ZKWRAPPER_INIT_ZK_FAIL);
        m_dist_lock.Close(); // 应该判断下是否超时比较好,目前把超时和错误导致的set acl失败都关闭
        SetClientInvalid();

        return false;
    }

    if (m_listen_struct.event_listener == NULL) {
        m_is_zk_client_valid = true;

        SetPreviousClientid(m_dist_lock.GetZKHandle());

        return true;
    }

    m_is_zk_client_valid = true; // to pass SetWatcher
    bool watch_success = SetWatcher(m_listen_struct.interest_cluster,
                                    m_listen_struct.interest_key,
                                    m_listen_struct.event_mask);
    if (!watch_success) {
        if (error_code == NULL) {
            LOG(ERROR) << "Initialize zookeeper event mask fail : "
                       << " for " << zk_server_host << " cluster "
                       << m_listen_struct.interest_cluster << " key "
                       << m_listen_struct.interest_key;
        }
        SET_ERRORCODE(error_code, ERROR_ZKWRAPPER_INIT_ZK_FAIL);
        m_dist_lock.Close(); // 失败也关闭,等待重新设置
        SetClientInvalid();
        return false;
    }

    m_is_zk_client_valid = true;

    SetPreviousClientid(m_dist_lock.GetZKHandle());

    return true;

#else
    return false;
#endif
}

#ifndef _WIN32
int32_t ZookeeperWrapper::GetZKHandleState() {
    zhandle_t* zh = m_dist_lock.GetZKHandle();
    return zh ? (zoo_state(zh) >= 0 ? ZOK : zoo_state(zh)) : (int32_t)(ZBADARGUMENTS);
}

void ZookeeperWrapper::SetPreviousClientid(zhandle_t* handle) {
    if (m_previous_clientid != 0) {
        delete m_previous_clientid;
        m_previous_clientid = 0;
    }
    if (handle == NULL) return;

    const clientid_t* clientid = zoo_client_id(handle);
    m_previous_clientid = new clientid_t(*clientid);
    VLOG(1) << "SetPreviousClientid as " << m_previous_clientid->client_id;
}
#endif //

void ZookeeperWrapper::SetClientInvalid(){
    m_cluster_name = "";
    m_is_zk_client_valid = false;
    LOG(INFO) << "Set client invalid";
}

// 构造配置文件全路径
bool ZookeeperWrapper::GetConfigFileName(const std::string& cluster_name,
        const std::string& key,
        std::string* config_full_path,
        uint32_t* error_code) {
    CHECK_NOTNULL(config_full_path);

    std::string key_full_path;
    if (!GetZkNodeName(cluster_name, key, &key_full_path, error_code)) return false;

    std::string local_file_name = key_full_path + ".dat";
    std::replace(local_file_name.begin(), local_file_name.end(), '/', '#');
    local_file_name.erase(local_file_name.begin());
    char file_name[kMaxPathLength] = {'\0'};
    if (GetModuleFileName(NULL, file_name, kMaxPathLength) <= 0) {
        config_full_path->assign(local_file_name);
        return true;
    }
    char* p = strrchr(file_name, '/');
    if (!p)
        p = strrchr(file_name, '\\');

    if (p) {
        ++p;
        *p = '\0';
        config_full_path->assign(std::string(file_name) + local_file_name);
    } else {
        config_full_path->assign(local_file_name);
    }

    VLOG(1) << "Congfig full path is : " << *config_full_path;
    return true;
}

bool ZookeeperWrapper::GetZkNodeName(const std::string& cluster_name, const std::string& key,
                       std::string* zk_node_name, uint32_t* error_code) {
    CHECK(zk_node_name != NULL) << "Input parameter zk_node_name must NOT NULL";

    std::string idc_name;
    if (!ParseIDCName(cluster_name, &idc_name, error_code)) {
        return false;
    }

    *zk_node_name = "/zk/" + idc_name + "/xfs/cluster_" + cluster_name + key;
    return true;
}

ZookeeperWrapper::~ZookeeperWrapper() {
#ifndef _WIN32
    m_dist_lock.Close();
    m_is_zk_client_valid = false;
    SetPreviousClientid(0);
#endif
}

bool ZookeeperWrapper::CheckNeedReinit(int32_t error_code) {
#ifndef _WIN32
    // We should not retry when ZNOAUTH;
    // but we also should not get ZNOAUTH when we write an authenticated node, randomly.
    // So we have to retry when ZNOAUTH.
    if (GetZKHandleState() == ZOK) {
        if (error_code == ZOPERATIONTIMEOUT || error_code == ZNOTHING) {
            return false;
        }
    }
	return (error_code == ZCONNECTIONLOSS) || (error_code == ZSESSIONEXPIRED)
        || (error_code == ZSESSIONMOVED) || (error_code == ZCLOSING)
        || (error_code == ZINVALIDSTATE)
        || (error_code == ZMARSHALLINGERROR)
        || (error_code == ZNOAUTH); // may retry and fail, so re-init
#else
	return true;
#endif
}


struct SimpleZooEvent{
    SyncEvent ev;
    int state;

    SimpleZooEvent(){
        state = 0;
    }
};

#ifndef _WIN32
void SimpleWatcher(zhandle_t *handle, int type, int state, const char *path, void *context)
{
    SimpleZooEvent* ev = (SimpleZooEvent*) context;

    ///
    std::string node_path = std::string(path);
    LOG(INFO) << " watcher " << TypeString(type) << " event state = "
              << StateString(state) << ", path = " << node_path;

    if( type == ZOO_SESSION_EVENT )
    {
        if( state == ZOO_EXPIRED_SESSION_STATE )
        {
            LOG(INFO) << "ZOO_EXPIRED_SESSION_STATE, path = " << node_path;
            // dist_lock->OnSessionExpired();
        }
        else if (state == ZOO_CONNECTING_STATE)
        {
            LOG(INFO) << "ZOO_CONNECTING_STATE";
        }
        else if (ZOO_CONNECTED_STATE)
        {
            LOG(INFO) << "ZOO_CONNECTED_STATE";
            // dist_lock->GetSyncEvent().Set();
        }

        ev->state = state;
        ev->ev.Set();
    }
    else if (type == ZOO_CHANGED_EVENT) ///<当节点发生改变时
    {
        LOG(INFO) << "ZOO_CHANGED_EVENT, path = " << node_path;
    }
    else if (type == ZOO_DELETED_EVENT) ///<当node被删除的时候
    {
        LOG(INFO) <<  "ZOO_DELETED_EVENT. node = " << node_path;
    }
    else if (type == ZOO_CHILD_EVENT) ///<当一个node的子node发生变化的时候
    {
        LOG(INFO) << "CHILD_EVENT, node = " << node_path << ", state = " << StateString(state);
    }
    else if (type == ZOO_CREATED_EVENT) ///<当不存在的节点被创建的时候
    {
        LOG(INFO) << "ZOO_CREATED_EVENT. node = "  << node_path;
    }
}
#endif // linux

// 如从 /xaec.zk.oa.com/zk/xaec/xfs/cluster_xaec-193/master_config/config
// 取出 xaec.zk.oa.com 和 /zk/xaec/xfs/cluster_xaec-193/master_config/config
bool SplitZkPath(const std::string& full_path, std::string* host, std::string* node_path) {
    CHECK((host != NULL) && (node_path != NULL));
    if (full_path.length() < 3) {
        return false;
    }

    std::string::size_type end_pos = full_path.find('/', 1);
    if (end_pos == std::string::npos) {
        LOG(ERROR) << "invalid full path for zk : " << full_path;
        return false;
    }

    *host = full_path.substr(1, end_pos - 1);
    if (host->length() == 0) {
        return false;
    }
    *host += kZkPort;

    *node_path = full_path.substr(end_pos);
    if (node_path->empty()) {
        return false;
    }

    VLOG(3) << "zk host is : " << *host
        << ", node path is : " << *node_path;
    return true;
}

bool ReadZooKeeperNodeVal(const char* full_path, int32_t recv_timeout,
		         std::string* val){
#ifdef _WIN32
     return false;
#else

    MutexLocker auto_lock(&g_zk_wrapper_read_node_mutex);

#ifdef NDEBUG
    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
#endif
    std::string zk_server_host;
    std::string node_path;
    if (!SplitZkPath(full_path, &zk_server_host, &node_path)) {
        return false;
    }

    VLOG(0) << "zk server host is : " << zk_server_host
            << ", full path is : " << node_path;
    bool is_ok = false;
    SimpleZooEvent ev;
    zoo_deterministic_conn_order(0);   // 随机选择一个zookeeper服务器
    zhandle_t* zhandle = zookeeper_init(zk_server_host.c_str(),
        SimpleWatcher, recv_timeout, 0, &ev, 0);
    ev.ev.Wait(recv_timeout);


    if(zhandle == NULL || ev.state != ZOO_CONNECTED_STATE){
        LOG(INFO) << "zookeeper_init() : " << (zhandle ? "OK " : "FAIL ")
                  << "connect to zookeeper server:" << StateString(ev.state);
        zookeeper_close(zhandle);
        return is_ok;
    }

    // get node
    int32_t max_buff_len = 128 * 1024; // 目前暂定最大128K
    int32_t len = max_buff_len;
    char* buff = new char[len];
    struct Stat stat = {0};
    int32_t ret = zoo_get(zhandle, node_path.c_str(), 0, buff, &len, &stat);
    if (ret == ZOK) {
        if (len >= max_buff_len) {
            LOG(ERROR) << "too long data length, exceed limits, the data length: "
                << max_buff_len;
        }else{
            buff[len] = 0;
            LOG(INFO) << "get data of node: " << node_path << " OK"
                      << "get data: " << buff;
            *val = std::string(buff, len);
            is_ok = true;
        }
    }

    zookeeper_close(zhandle);
    delete []buff;
    return is_ok;
#endif //
}
