// Copyright 2010, Tencent Inc.
// Author: typherque(typherque@tencent.com)
//         Yongqiang Zou(aaronzou@tencent.com)
//
// ��װ��zookeeper���߱��ض�д�����ļ��Ľӿڣ���zookeeper�����Լ�����ʱ����ͨ��flagѡ��
// �ӱ��ض�ȡ������Ϣ, Ϊ��ʹ���������ļ����ݺ�zookeeper��һ�������ڱ��ش���һ����zookeeper
// ͬ�����ļ������磺/zk/xaec/xfs/cluster_xaec-193/master_config/config�ڵ�ͻ��Ӧ�����ص�
// zk#xaec#xfs#cluster_xaec-193#master_config#config.dat�ļ�

#ifndef DISTRIBUTE_LOCK_ZOOKEEPER_WRAPPER_H_
#define DISTRIBUTE_LOCK_ZOOKEEPER_WRAPPER_H_

#ifndef WIN32
#include "common/distribute_lock/distribute_lock.h"
#endif
#include "common/distribute_lock/event_mask.h"
#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "thirdparty/gflags/gflags.h"

DECLARE_bool(zookeeper);

namespace distribute_lock{
class EventListener;
}

// wrap parameters for listen zookeeper node.
struct ZookeeperListenStruct {
ZookeeperListenStruct()
    : event_listener(NULL),
      event_mask(distribute_lock::EVENT_MASK_NONE) {}

distribute_lock::EventListener* event_listener;
std::string interest_cluster;
std::string interest_key;
distribute_lock::ZKEventMask event_mask;
};

// lock-free, the caller ensure the locks.
class ZookeeperWrapper {
public:
    // Zookeeper wrapper ctor allow listener and watch interesting nodes.
    // So that then first time init or re-init for retry can init as the same status.
    explicit ZookeeperWrapper(ZookeeperListenStruct listen_struct = ZookeeperListenStruct());

    ~ZookeeperWrapper();

    bool Init(const std::string& cluster_name);

    void Close();

    // Get the value associate with the key in the cluster_name.
    // If version is Not NULL, will hold the version of the value.
    bool GetValue(const std::string& cluster_name,
        const std::string& key, std::string* value,
        int* version = NULL, uint32_t* error_code = NULL);

    // Set the value associate with the key in the cluster_name.
    // The set success only when the actual version in zookeeper equals with expect_version.
    bool SetValue(const std::string& cluster_name,
        const std::string& key, const std::string& value, int expect_version = -1);

    // Lock on <cluster_name><node>.
    bool Lock(const std::string& cluster_name, const std::string& key, bool blocked_type = true);

    bool CreateNode(
        const std::string& cluster_name,
        const std::string& key,
        distribute_lock::ZKEventMask event_mask = distribute_lock::EVENT_MASK_NONE,
        bool is_temporary = false);

    static bool GetZkNodeName(const std::string& cluster_name,
                              const std::string& key,
                              std::string* config_full_path,
                              uint32_t* error_code = NULL);

    static bool ParseIDCName(const std::string& cluster_name,
                             std::string* idc_name,
                             uint32_t* error_code = NULL);

    void SetClientInvalid();
#ifndef WIN32
    SyncEvent* GetSyncEvent() { return &m_sync_event; }
    void SetZookeeperStatus(int32_t status) { m_zk_status = status; }
    bool IsInitializingZookeeperACL() { return m_is_initializing_zk_acl; }
#endif

private:
    // ���һ������ͬʱ����ʲ�ͬ��cluster, ��Ӧ��zk���ܲ�ͬ������ÿ�ζ�ֱ�Ӵ���cluster_name
    // key��cluster name��ĵ�һ����ʼ����/��ͷ����Ҫ����/cluster_xaec-107/master_config/configʱ��
    // key����/master_config/config
    bool GetValueByZookeeper(const std::string& cluster_name,
                             const std::string& key, std::string* value,
                             int* version = NULL, uint32_t* error_code = NULL);
    bool SetValueByZookeeper(const std::string& cluster_name,
                             const std::string& key, const std::string& value,
                             int expect_version = -1);
    bool LockByZookeeper(const std::string& cluster_name,
                         const std::string& key,
                         bool blocked_type = true);
    bool SetWatcher(const std::string& cluster_name,
                    const std::string& key,
                    distribute_lock::ZKEventMask event_mask);

    bool SetWatcherZookeeper(const std::string& cluster_name,
                             const std::string& key,
                             distribute_lock::ZKEventMask event_mask);
    bool GetValueByLocalFile(const std::string& cluster_name,
                             const std::string& key, std::string* value,
                             uint32_t* error_code = NULL);
    bool SetValueByLocalFile(const std::string& cluster_name,
                             const std::string& key, const std::string& value);

    bool GetConfigFileName(const std::string& cluster_name, const std::string& key,
                           std::string* config_full_path, uint32_t* error_code = NULL);

    bool CreateNodeByZookeeper(
         const std::string& cluster,
         const std::string& key,
         distribute_lock::ZKEventMask event_mask,
         bool is_temporary);

    bool CreateNodeByLocalFile(const std::string& cluster_name, const std::string& key);

    bool ReInitZooKeeperIfNeccessary(const std::string& cluster_name, uint32_t* error_code = NULL);

    bool InitZookeeperAcl(uint32_t* error_code = NULL);

    bool CheckNeedReinit(int32_t error_code);


    // ����ʵ��ReInitZooKeeperIfNeccessary
    bool ImplReInitZooKeeperIfNeccessary(const std::string& cluster_name, uint32_t* error_code);

#ifndef WIN32
    int32_t GetZKHandleState();
    void SetPreviousClientid(zhandle_t* handle);

    distribute_lock::DistributeLock m_dist_lock;
    clientid_t* m_previous_clientid;

    SyncEvent m_sync_event; // to wait the callback
    int32_t volatile m_zk_status;   // to save callback result

#endif
    // members for init with listeners and event mask
    ZookeeperListenStruct m_listen_struct;

    bool volatile m_is_zk_client_valid;
    bool volatile m_is_initializing_zk_acl;
    std::string m_cluster_name;
};

//
// step 1: zookeeper_init
// step 2: zoo_get
// step 3: zookeeper_close
// �����Ӷ�ȡ�ڵ�ֵ,ÿ�λ�ȡ��϶���ر�zookeeper�߳�
// ע��: 1: �ú�����class ZookeeperWrapper�ǳ�ͻ��,��Ҫͬʱʹ��
//       2: ֧�ֶ��̵߳���
//
bool ReadZooKeeperNodeVal(const char* node_path, int32_t recv_timeout,
                          std::string* val);

#endif // _DISTRIBUTE_LOCK_ZOOKEEPER_WRAPPER_H_
