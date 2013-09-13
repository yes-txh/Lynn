
#include <string>
#include "thirdparty/glog/logging.h"
#include "thirdparty/glog/raw_logging.h"
#include "common/system/time/time_utils.hpp"
#include "common/base/string/string_number.hpp"
#include "common/distribute_lock/pub_func.h"
#include "common/distribute_lock/error_code.h"

namespace distribute_lock
{
DistributeLock::DistributeLock()
{
    m_handle      = NULL;
    m_listener    = NULL;
    m_buff        = new char[s_buff_len];
    m_is_wait_init = false;
}

DistributeLock::~DistributeLock()
{
    Close();
    if (m_buff != NULL) {
        delete []m_buff; m_buff = NULL;
    }
}

void DistributeLock::Close()
{
//    MutexLocker locker(&m_mutex);
    if (m_handle != NULL) {
        zookeeper_close(m_handle);
        m_handle = NULL;
    }
}

int DistributeLock::Init(
        const std::string& host,
        EventListener* listener,
        int time_out,
        std::string log_path)
{
    return Init(NULL, host, listener, time_out, log_path);
}

int DistributeLock::Init(clientid_t* clientid,
        const std::string& host,
        EventListener* listener,
        int time_out,
        std::string log_path)
{
    MutexLocker locker(&m_mutex);
    zoo_set_debug_level(ZooLogLevel(0));

    if (log_path.empty()) {
        log_path = std::string("./");
    }

    m_host = host;
    zoo_deterministic_conn_order(0);   // ���ѡ��һ��zookeeper������
    VLOG(1) << "begin to init zk";
    m_is_wait_init = true;
    m_sync_event.Reset();

    m_handle = zookeeper_init(m_host.c_str(), Watcher, 0.8*time_out, clientid, this, 0);

    if (!m_handle) {
        m_is_wait_init = false;
        LOG(ERROR) << "zookeeper init failed. errno = " << errno << ", error code = " <<
                strerror(errno);
        return -1;
    }
    VLOG(1) << "wait for init process finished";

    bool ret = m_sync_event.Wait(time_out);
    m_is_wait_init = false;
    if (!ret) // time_out
    {
        VLOG(1) << "init failed. time out";
        return ZOPERATIONTIMEOUT;
    }

    int init_state = zoo_state(m_handle);
    if (init_state == ZOO_EXPIRED_SESSION_STATE) {
        VLOG(1) << "init failed with ZOO_EXPIRED_SESSION_STATE";
        return init_state;
    }
    CHECK(init_state == ZOO_CONNECTED_STATE)
        << "When Init wait OK, must with ZOO_CONNECTED_STATE";

    const clientid_t* cid = zoo_client_id(m_handle);
    int64_t session = cid->client_id;
    char prefix[30];
#if defined(__x86_64__)
    snprintf(prefix, sizeof(prefix), "%016lx", session);
#else
    snprintf(prefix, sizeof(prefix), "%016llx", session);
#endif
    m_session_str = std::string(prefix, strlen(prefix));

    if (listener != NULL) {
        m_listener = listener;
    }
    int t_out = zoo_recv_timeout(m_handle);
    LOG(INFO) << "zookeeper init success. session = " << m_session_str << " time_out = " << t_out;

    return 0;
}

int DistributeLock::CreateNode(const std::string& path, ZKEventMask event_mask, bool is_temporary)
{

    return CreateNodeWithVal(path, event_mask, NULL, -1, is_temporary);
}

int DistributeLock::CreateNodeWithVal(const std::string& path, ZKEventMask event_mask,
        const char* val_buff, int len, bool is_temporary)
{
    int node_mask;
    MutexLocker locker(&m_mutex);
    if (is_temporary)
    {
        node_mask = ZOO_EPHEMERAL;
    }
    else node_mask = 0;
    int ret = zoo_create(
            m_handle,                   //  zookeeper���
            path.c_str(),               //  node������
            val_buff,                   //  д��node�е�����
            len,                        //  д��node�е����ݵĳ���
            &ZOO_OPEN_ACL_UNSAFE,       //  ACL
            node_mask,                  //  �ڵ�����
            m_buff,
            s_max_len
           );
    if (ret == ZOK ||  ret == ZNODEEXISTS)
    {
        SetWatcher(path, event_mask);
        ret = ZOK;
    }
    else
    {
        LOG(ERROR) << "create node failed. error code = " << ErrorString(ret);
    }
    return ret;
}

int DistributeLock::MkDir(const std::string& dir, ZKEventMask event_mask)
{
    // �������������Խڵ�
    return CreateNode(dir, event_mask);
}

int DistributeLock::Lock(const std::string& node, bool blocked_type)
{
    MutexLocker locker(&m_mutex);
    std::string node_path = m_host + node;
    // test whether the node exists
    std::string path = node + "/zoo_attr_lock"; // ��lock�����´����ӽڵ�
    int exists =  zoo_exists(m_handle, path.c_str(), 0, NULL);
    int ret, idx, retry_cnt = 0;
    bool locked = false;

    while ((exists == ZCONNECTIONLOSS || exists == ZNONODE) && retry_cnt < s_retry) {
        retry_cnt++;
        if (exists == ZCONNECTIONLOSS) { // retry
            exists = zoo_exists(m_handle, node.c_str(), 0, NULL);
        }
        else if (exists == ZNONODE) { // lockӦ���������ڵ�
            ret = zoo_create(m_handle, path.c_str(), NULL, -1, &ZOO_OPEN_ACL_UNSAFE,
                    0, m_buff, s_max_len);
            if (ret != ZOK && ret != ZNODEEXISTS) {
                VLOG(3) <<  "create lock node failed, node =  " << node_path << ". error code = " <<
                    ErrorString(ret);
                return ret;
            }
        }
    }

    if (exists == ZCONNECTIONLOSS)
    {
        VLOG(3) << "cann't access the server.";
        return exists;
    }

    //  ���lock���ӽڵ�
    struct String_vector  vectorst = {0, NULL};
    struct String_vector  vec_children = {0, NULL};
    struct Stat stat;
    do
    {
        MutexLocker locker(&m_mutex);
        ret = zoo_get_children(m_handle, path.c_str(), 0, &vectorst);
        if (ret != ZOK)
        {
            VLOG(3) << "get children operation failed. error code = " << ErrorString(ret);
            return ret;
        }
        locked = false;
        char *lock_id;
        std::string lock_full_path;
        lock_id = NULL;
        //  �Ҹ�session��Ӧ��lock-node
        for (idx = 0; idx < vectorst.count; ++idx) {
            char * child = vectorst.data[idx];
            if (strncmp(m_session_str.data(), child, m_session_str.length()) == 0) {
                lock_id = strdup(child);
                break;
            }
        }
        if (lock_id == NULL) { // û�д���
            char buf[s_max_len], retbuf[s_max_len];
            snprintf(buf, s_max_len, "%s/%s-", path.c_str(), m_session_str.c_str());
            ret = zoo_create(m_handle, buf, NULL, -1, &ZOO_OPEN_ACL_UNSAFE,
                    ZOO_EPHEMERAL | ZOO_SEQUENCE, retbuf, s_max_len);
            if (ret != ZOK) {
                VLOG(3) << "can't create zoo node " << std::string(buf) << ". error code = " <<
                        ErrorString(ret);
                deallocate_String_vector(&vectorst); // �ͷŵ���Դ
                return ret;
            }
            char *name = strrchr(retbuf, '/');
            if (name == NULL) {
                lock_id = NULL;
            }
            else lock_id = strdup(name + 1); // session_id-ephernum
        }
        deallocate_String_vector(&vectorst); // �ͷŵ���Դ

        if (lock_id != NULL) {
            ret = zoo_get_children(m_handle, path.c_str(), 0, &vec_children);
            if (ret != ZOK) {
                VLOG(3) << "get children failed. error_code = " << ErrorString(ret);
                deallocate_String_vector(&vec_children); // �ͷŵ���Դ
                return ret;
            }
            // �Ժ��ӽڵ�����
            SortChildren(&vec_children);
            //  ��������predecessor
            char *less_than_me = NULL;
            for (idx = 0; idx < vec_children.count; ++idx) {
                if (strcmp(vec_children.data[idx], lock_id) == 0) {
                    break;
                }
                less_than_me = vec_children.data[idx];
            }
            // �ͷ�vectorst, vec_children
            if (less_than_me != NULL) { // ��ǰ��
                // ���ö�ǰ���ļ���
                int flen = path.length() + strlen(less_than_me) + 2;
                char last_child[flen];
                sprintf(last_child, "%s/%s", path.c_str(), less_than_me);

                ret = zoo_wexists(m_handle, last_child, LockWatcher, this , NULL);
                if (ret != ZOK) { // �����������watcher, Ӧ��give up
                    VLOG(3) << "watch predecessor failed. error_code = " <<
                        ErrorString(ret) << ", path = " << std::string(last_child);
                    return ret; // �п���session �Ѿ�ʧЧ
                }
                else
                {
                    VLOG(3) << "watch at predecessor " << std::string(last_child);
                    if (blocked_type) {
                        // Ӧ�õȴ�, ȥ�ȴ��õ���
                        m_wait_lock.Wait();
                    }
                    else
                    {
                        deallocate_String_vector(&vec_children);
                        return -1;                          // û��lock��
                    }
                }
            }
            else // �����
            {
                locked = true;
                VLOG(3) << "get the lock : " << std::string(lock_id);
                std::string lock_full_path = node + "/zoo_attr_lock/" + std::string(lock_id);
                VLOG(3) << "lock full path " << lock_full_path;
                // ���ö����Ĺ�ע
                ret = zoo_exists(m_handle, lock_full_path.data(), 1, &stat);
                if (ret != ZOK) {
                    VLOG(3) << "listen on the lock  failed. node = " << lock_full_path <<
                        "error code = " << ErrorString(ret);
                    return ret;
                }
            }
            deallocate_String_vector(&vec_children);
        }
    }while(locked == false);
    return ZOK;
}


bool DistributeLock::IsLocked(const std::string& node)
{
    MutexLocker locker(&m_mutex);
    struct String_vector vec = {0, NULL};
    bool is_locked = false;
    std::string lock_path = node + std::string("/zoo_attr_lock");
    // �ж�lock_path�Ƿ���Ч
    int ret = zoo_get_children(m_handle, lock_path.data(), 1, &vec);
    if (ret == ZOK)
    {
        if (vec.count > 0)
        {
            //  ˵����lock�ļ��� ��lock�ϵ�
            SortChildren(&vec);
            //  �������ļ�
            struct Stat stat;
            std::string lock_file = lock_path + "/" + std::string(vec.data[0]);
            ret = zoo_exists(m_handle, lock_file.data(), 1, &stat);
            if (ret != ZOK)
            {
                VLOG(3) << "set lock exist watch failed. node = " << lock_path << ", error code = " <<
                     ErrorString(ret);
                is_locked = false;
            }
            else
            {
                is_locked = true;
                VLOG(3) << "set lock exist watcher success. node = " << lock_path;
            }
        }
        else
        {
            VLOG(3) << node << " is not locked.";
            is_locked = false;
        }
    }
    else VLOG(3) <<  "get node's lock children failed. error code = " << ErrorString(ret);
    deallocate_String_vector(&vec);
    return is_locked;
}

int DistributeLock::SetAttr(const std::string& node, const std::string& attr,
        const std::string& value, int version, ZKEventMask event_mask)
{
    MutexLocker locker(&m_mutex);
    std::string node_path = m_host  + node;
    std::string attr_path = node + "/zoo_attr_" + attr;
    int ret = zoo_exists(m_handle, attr_path.c_str(), 0, NULL);
    if (ret == ZOK) {
        ret = zoo_set(m_handle, attr_path.c_str(), value.data(), value.length(), version);
        if (ret == ZOK) {
            //  ���ü�����
            VLOG(3) << "node " << node << " set attr " << attr << " success";
            SetWatcher(attr_path, event_mask);
        }
        else
        {
            VLOG(3) << "set attr val failed. node = " << node_path <<
                ", attr = " << attr << ", error_code = " << ErrorString(ret);
        }
        return ret;
    }
    else if (ret == ZNONODE) {
        ret = zoo_create(m_handle, attr_path.c_str(), value.data(), value.length(),
                &ZOO_OPEN_ACL_UNSAFE, 0, m_buff, s_max_len);
        if (ret == ZOK || ret == ZNODEEXISTS) {
            SetWatcher(attr_path, event_mask);
        }
        else
        {
            VLOG(3) << "set attr val failed. node = " << node_path <<
                ", attr = " << attr << ", error code = " << ErrorString(ret);
        }
        return ret;
    }
    return ret;
}

int DistributeLock::GetAttr(const std::string& node, const std::string& attr,
        std::string& value, int *version)
{
    MutexLocker locker(&m_mutex);
    std::string node_path = m_host + node;
    value = "";
    std::string path = node + "/zoo_attr_" + attr;
    int ret;
    if (attr == "lock") { // �����lock����,���ж��Ƿ��к��ӽ��
        struct String_vector children_path;
        ret = zoo_get_children(m_handle, path.c_str(), 0, &children_path);
        if (ret == ZOK && children_path.count > 0) {
            value = std::string("1");
        }
        else value = std::string("0");
        deallocate_String_vector(&children_path);
        return ret;
    }
    int len = s_buff_len;
    struct Stat stat;
    ret = zoo_get(m_handle, path.data(), 0, m_buff, &len, &stat);
    if (ret == ZOK) {
        if (len > s_buff_len) {
            VLOG(3) <<  "too long data length, exceed limits";
            return -1;
        }
        value = std::string(m_buff, len);
        VLOG(3) << "get attr success. node = " << node_path << ", val's length = " << value.length();
    }
    if (version != NULL)
    {
        *version = stat.version;
    }
    return ret;
}

int DistributeLock::Unlink(const std::string& node)
{
    MutexLocker locker(&m_mutex);
    struct String_vector children_path;
    std::string node_path = m_host + node;
    int ret;
    ret = zoo_get_children(m_handle, node.c_str(), 0, &children_path);
    if (ret == ZOK ) { // ��������ɹ�
        for (int i = 0; i < children_path.count; ++i) {
            Unlink(std::string(node + "/" + std::string(children_path.data[i])));
        }
        ret = zoo_delete(m_handle, node.c_str(), -1);
        if (ret != ZOK) {
            VLOG(3) << "unlink node failed. " << node_path << ", error code = " <<
                    ErrorString(ret);
        }
        else VLOG(3) << "unlink node success. node = " << node_path;
    }
    else
    {
        VLOG(3) << "unlink node failed. node = " << node_path << ", error code = " <<
                ErrorString(ret);
    }
    return ret;
}


int DistributeLock::SetWatcher(const std::string& node, ZKEventMask event_mask)
{
    MutexLocker locker(&m_mutex);
    std::string node_path = m_host + node;
    int len = s_max_len;
    struct Stat stat;
    struct String_vector vec = {0, NULL};
    int ret = 0;
    if (event_mask & EVENT_MASK_ATTR_CHANGED)
    {
        deallocate_String_vector(&vec);
        ret = zoo_get_children(m_handle, node.c_str(), 0, &vec);
        if (ret != ZOK)
        {
            LOG(ERROR) << "get child failed. ret = " << ErrorString(ret);
            return ret;
        }
        for (int i = 0; i < vec.count; ++i)
        {
            std::string attr_node = node + "/" + std::string(vec.data[i]);
            if (IsAttrNode(attr_node)) {
                //  ֻҪ�����Խڵ����ù�ע�¼�
                ret = zoo_get(m_handle, attr_node.c_str(), 1, m_buff, &len, &stat);
                if (ret != ZOK)
                {
                    LOG(ERROR) << "zoo get failed. ret = " << ErrorString(ret);
                    return ret;
                }
            }
        }
        VLOG(3) << "set attr-changed watcher success. node = " << node_path;
        deallocate_String_vector(&vec);
    }
    if (event_mask & EVENT_MASK_CHILD_CHANGED) { // ���������0, ��Ҫ������ӦZOO_CHILD_EVENT
        ret = zoo_get_children(m_handle, node.data(), 1, &vec);
        VLOG(3) << "set watcher child add: node = " << node_path;
        deallocate_String_vector(&vec);
    }
    if (event_mask & EVENT_MASK_DATA_CHANGED) {
        ret = zoo_get(m_handle, node.c_str(), 1, m_buff, &len, &stat);
        if (ret != ZOK)
        {
            VLOG(3) << "set data change watch on node failed. error code = " <<
                ErrorString(ret);
            return -1;
        }
        else
        {
            VLOG(3) << "set data change watch success. node = " << node_path;
        }
    }
    if ((event_mask & EVENT_MASK_LOCK_ACQUIRED) || (event_mask & EVENT_MASK_LOCK_RELEASE)) {
        std::string lock_path = node + "/zoo_attr_lock";
        ret = Exists(lock_path);
        if (ret == ZNONODE) {
            ret = CreateNode(lock_path);
            if (ret == ZOK) {
                ret = zoo_get_children(m_handle, lock_path.data(), 1, &vec);
                if (ret != ZOK)
                {
                    LOG(ERROR) << "get children failed. ret = " << ErrorString(ret);
                    return ret;
                }
                VLOG(3) << "set child watch on node success. node = " << lock_path;
                deallocate_String_vector(&vec);
            }
            else VLOG(3) << "create node failed. node = " << lock_path << ", error code = " <<
                    ErrorString(ret);
        }
        else if (ret == ZOK)
        {
            if (event_mask & EVENT_MASK_LOCK_ACQUIRED)
            {
                ret = zoo_get_children(m_handle, lock_path.data(), 1, &vec);
                if (ret != ZOK)
                {
                    LOG(ERROR) << "get child failed. ret = " << ErrorString(ret);
                    return ret;
                }
                deallocate_String_vector(&vec);
                VLOG(3) << "set lock acquired watch on node successi. node = " << lock_path;
            }
            if (event_mask & EVENT_MASK_LOCK_RELEASE) {
                ret = zoo_get_children(m_handle, lock_path.c_str(), 1, &vec);
                SortChildren(&vec);
                if (vec.count > 0) {
                    std::string path = lock_path + "/" + std::string(vec.data[0]);
                    ret = zoo_exists(m_handle, path.c_str(), 1, &stat);
                    if (ret != ZOK)
                    {
                        LOG(ERROR) << "zoo exist set lock failed. path = " << path;
                        return ret;
                    }
                    VLOG(3) << "set watcher on node success. node =  " << path;
                }
                deallocate_String_vector(&vec);
            }
        }
        else return ret;
    }
    return  ZOK;
}

void DistributeLock::OnNodeChanged(const std::string& node, int state)
{
    MutexLocker locker(&m_mutex);
    if (m_handle == NULL) return;
    // ���������¼�
    SetWatcher(node, EVENT_MASK_DATA_CHANGED);
    VLOG(3) << "node data change, ndoe = " << node;
    if (IsAttrNode(node)) { // ֻ�����Խڵ�ĸı����Ȥ
        std::string parent_node, attr_node;
        GetPathName(node, parent_node);
        GetBaseName(node, attr_node);
        if (IsAttrNode(attr_node)) {
            GetPureAttrName(attr_node);
        }

        if (m_listener != NULL)
        {
            VLOG(3) << "report attr change, node = " << attr_node;
            m_listener->AttrChange(parent_node, attr_node);
        }
    }
}

void DistributeLock::OnNodeDeleted(const std::string& node, int state)
{
    MutexLocker locker(&m_mutex);
    if (m_handle == NULL) return;
    size_t pos = node.find("zoo_attr_lock/"); // ������zoo_attr_lock�ĺ��ӽڵ�
    if (pos != std::string::npos) {
        std::string preces_node = node.substr(0, pos - 1);
        VLOG(3) <<  "node release lock. node = " << preces_node;
        if (m_listener != NULL) {
            m_listener->LockReleased(preces_node);
        }
    }
    else
    {
        if (m_listener != NULL)
        {
            VLOG(3) << "report node delete. node = " << node;
            m_listener->NodeDel(node);
        }
    }
    VLOG(3) << "node deleted. node = " << node;
}

void DistributeLock::OnChildNode(const std::string& node, int state)
{
    MutexLocker locker(&m_mutex);
    if (m_handle == NULL) return;
    struct String_vector  vec = {0, NULL};
    struct Stat stat;
    // �������ú��ӽ�����
    int ret = zoo_get_children(m_handle, node.data(), 1, &vec);
    int len = s_max_len;
    if (ret == ZOK)
    {
        VLOG(3) << "set children watch on node " << node << " success. vec_count = " << vec.count;
    }
    else
    {
        VLOG(3) << "set children watch on node " << node << " failed. error code = " <<
            ErrorString(ret);
    }

    if (IsAttrNode(node)) // ֻ����lock���Խ��
    {
        //  �����ӽ��ɾҲ�ᴥ�����¼�
        if (vec.count > 0) // �к��ӽ��, ���Ǽ���, ��ֱ���ϱ�������
        {
            // ���ö����ļ��� (����������±�lock���Σ�����ϱ�����)
            std::string lock_path = node + "/" + std::string(vec.data[0]);
            ret = zoo_exists(m_handle, lock_path.c_str(), 1, &stat);
            if (ret != ZOK)
            {
                LOG(ERROR) << "zoo exist set lock failed. ret = " << ErrorString(ret);
                return;
            }
            VLOG(3) << "set lock exist watcher on node " << lock_path;
            // �ϱ�������¼�(�������lock)
            std::string path;
            GetPathName(node, path);
            DELT_FLAG flag = GetProcessedFlag(path);
            if (flag != kNotDelt) return;
            // mark
            SetProcessed(path);
            if (m_listener != NULL)
            {
                VLOG(3) << "report lock acquired. path = " << path;
                m_listener->LockAcquired(path);
            }
        }
        else
            VLOG(3) << "node = " << node << ", maybe lose lock";
    }
    else //node ���������Խ��
    {
        for (int i = 0; i < vec.count; ++i)
        {
            len = s_max_len;
            std::string path = node + "/" + std::string(vec.data[i]);
            DELT_FLAG flag = GetProcessedFlag(path);
            VLOG(3) << "deal child add. node = " << path << " val_len = " << len;
            if (flag == kNotDelt) // ���û�д��ϱ��
            {
                if (!IsAttrNode(path)) // �¼ӽ����Ľڵ㣬Ҫ�����亢�ӽڵ�
                {
                    SetWatcher(path, EVENT_MASK_CHILD_CHANGED);
                    VLOG(3) << "set child watch on node " << path;
                    bool is_locked = IsLocked(path); // �����ü���(�ж���û������)
                    if (is_locked )
                    {
                        // set flag on node (��Ϊ�������)
                        SetProcessed(path);
                        if (m_listener != NULL)
                        {
                            VLOG(3) << "report lock acquired. node = " << path;
                            m_listener->LockAcquired(path);  // �����ϱ�
                        }
                    }
                }
                else // �¼ӽ��������Խڵ� Ҫ����lock���ĺ��ӽ��
                {
                    if (IsLockAttr(path)) // �����ǰ�������Խ��, �ж��Ƿ��к���
                    {
                        // ���ú��Ӽ����¼�
                        SetWatcher(path, EVENT_MASK_CHILD_CHANGED);

                        std::string rep_path = "";
                        GetPathName(path, rep_path);
                        DELT_FLAG tmp_flag = GetProcessedFlag(rep_path);
                        if (tmp_flag == kNotDelt)
                        {
                            // ���û�д���
                            bool is_locked = IsLocked(rep_path);
                            if (!is_locked) continue;
                            // set flag on node (��Ϊ�������)
                            SetProcessed(rep_path);
                            //  �Ҹ�session��Ӧ��lock-node
                            if (m_listener != NULL)
                            {
                                VLOG(3) << "report lock acquire, node = " << rep_path;
                                m_listener->LockAcquired(rep_path);
                            }
                        }
                    }
                    else
                    {
                        // �����������Խ��ĺ��ӽ���¼�
                    }
                }
            }
            else
            {
                VLOG(3) << " node " << path << " has been delt. ";
            }
        }
    }
    //  �ͷ���Դ
    deallocate_String_vector(&vec);
}

void DistributeLock::OnNodeCreate(const std::string& path, int state)
{
    //reserved
    VLOG(3) <<  "create node " << path;
    MutexLocker locker(&m_mutex);
    if (m_handle == NULL) return;
    if (m_listener != NULL)
    {
        VLOG(3) << "report node create. node = " << path;
        m_listener->NodeCreate(path);
    }
}

void DistributeLock::OnSessionExpired()
{
    VLOG(3) << "session expired";
    if (m_handle == NULL) return;
    if (m_listener != NULL)
    {
        VLOG(3) << "report session expired. session_id = " << m_session_str;
        m_listener->SessionExpired();
    }
}

int DistributeLock::GetBaseName(const std::string& src, std::string &dst)
{
    int pos = src.rfind('/');
    if (pos < 0) {
        return -1;
    }
    dst     = src.substr(pos + 1, src.length() - pos - 1);
    return 0;
}

int DistributeLock::GetPathName(const std::string& src, std::string &dst)
{
    int pos = src.rfind('/');
    if (pos < 0) {
        return -1;
    }
    dst     = src.substr(0, pos);
    return 0;
}


bool DistributeLock::IsAttrNode(const std::string& node)
{
    size_t pos = node.find("zoo_attr_");
    if (pos == std::string::npos) {
        return false;
    }
    return true;
}
void DistributeLock::GetPureAttrName(std::string& node)
{
    std::string str = std::string("zoo_attr_");
    size_t pos = node.find(str);
    node = node.substr(pos+str.length(), node.length() - str.length() - pos);
}

int DistributeLock::Exists(const std::string& node)
{
    struct Stat stat;
    int ret = zoo_exists(m_handle, node.c_str(), 0, &stat);
    return ret;
}

int DistributeLock::UnLock(const std::string& node)
{
    //  �жϸýڵ��Ƿ�ӵ����
    std::string lock_path = node + "/zoo_attr_lock";
    struct String_vector vec = {0, NULL};
    int ret = zoo_get_children(m_handle, lock_path.c_str(), 0, &vec);
    if (ret == ZNONODE) {
        VLOG(3) <<  "the node has not been locked.";
    }
    else if (ret == ZOK  && vec.count != 0) {
        int idx;
        // ����lock��sessionid���Լ���id��������
        // ������������
        SortChildren(&vec);
        const char* prefix = m_session_str.c_str();
        //  �Ҹ�session��Ӧ��lock-node
        for (idx = 0; idx < vec.count; ++idx) {
            char * child = vec.data[idx];
            if (strncmp(prefix, child, strlen(prefix)) == 0) {
                break;
            }
        }
        if (idx != 0) {
            VLOG(3) << "you don't own the lock";
            ret = ZNOAUTH;            // ����û��Ȩ��
        }
        else
        {
            //  �ͷŵ����ڵ�
            lock_path = lock_path + "/" + std::string(vec.data[0]);
            ret = zoo_delete(m_handle, lock_path.c_str(), -1);
            if (ret == ZOK) {
                VLOG(3) << "release lock node success. node = " << lock_path;
            }
            else
            {
                VLOG(3) << "release lock node failed. node = " << lock_path << ", error code = " <<
                        ErrorString(ret);
            }
        }
    }
    else
    {
        VLOG(3) << "unlock node = " << lock_path << " failed. error code = " <<
                ErrorString(ret);
    }
    deallocate_String_vector(&vec);
    return ret;
}

int DistributeLock::ListNode(const std::string& dir, std::vector<std::string>& node)
{
    node.clear();
    struct String_vector children_path = {0, NULL};
    int ret = zoo_get_children(m_handle, dir.c_str(), 0, &children_path);
    if (ret == ZOK) {
        for (int i = 0; i < children_path.count; ++i) {
            std::string child = std::string(children_path.data[i]);
            if (!IsAttrNode(child)) {
                node.push_back(child);
            }
        }
    }
    else VLOG(3) << "get children failed. node = " << dir << ", error code = " <<
        ErrorString(ret);
    deallocate_String_vector(&children_path);
    return ret;

}

int DistributeLock::ListAttr(const std::string& dir, std::vector<struct Attr>& attr)
{
    attr.clear();
    struct String_vector children_path = {0, NULL};
    int ret = zoo_get_children(m_handle, dir.data(), 0, &children_path);
    if (ret == ZOK) {
        for (int i = 0; i < children_path.count; ++i) {
            std::string child = std::string(children_path.data[i]);
            if (IsAttrNode(child)) {
                Attr tmp_attr;
                GetPureAttrName(child);
                tmp_attr.name = child;
                ret = GetAttr(dir, child, tmp_attr.val);
                if (ret != ZOK)
                {
                    VLOG(3) << "node get attr failed. node = " << dir <<", attr = " << child <<
                        "error code = " << tmp_attr.val;
                }
                attr.push_back(tmp_attr);
            }
        }
    }
    else VLOG(3) << "get children failed. node = " << dir << ", error code = " <<
        ErrorString(ret);
    deallocate_String_vector(&children_path);
    return ret;
}

int DistributeLock::IsLockAttr(const std::string& node)
{
    std::string dst;
    if (GetBaseName(node, dst) != 0) {
        return -1;
    }
    if (dst == "zoo_attr_lock") {
        return 1;
    }
    return 0;
}

/// @brief ���ý���ֵ
///
/// @param node - �ڵ�����
/// @param value - ���õ�ֵ
/// @param version - �汾��
///
/// @retval 0 - �ɹ� ���� - ʧ��
int DistributeLock::ZooSet(const std::string& node, const std::string& value, int version)
{
    MutexLocker locker(&m_mutex);
    int ret = zoo_set(m_handle, node.c_str(), value.data(), value.length(), version);
    return ret;
}

/// @brief ��ý���ֵ
///
/// @param node - �������
/// @param value - ��õ�ֵ
/// @param version - ��õİ汾��
///
/// @retval 0 - �ɹ� ���� - ʧ��
int DistributeLock::ZooGet(const std::string& node, std::string& value, int* version)
{
    MutexLocker locker(&m_mutex);
    int len = s_buff_len;
    struct Stat stat;
    int ret = zoo_get(m_handle, node.c_str(), 0, m_buff, &len, &stat);
    if (ret == ZOK) {
        if (len > s_buff_len) {
            VLOG(3) << "too long data length, exceed limits";
            return -1;
        }
        if (len < 0) return -1;
        value = std::string(m_buff, len);
        if (version != NULL)
        {
            *version = stat.version;
        }
    }
    return ret;
}

/// @brief ���ñ�ǣ����Ϊsession id
/// @param node - ����Ľ��
void DistributeLock::SetProcessed(const std::string& node)
{
    SetAttr(node, "delt", m_session_str);
}

/// @brief �ж��Ƿ����
/// @param node - ���жϵĽ��
DELT_FLAG DistributeLock::GetProcessedFlag(const std::string& node)
{
    std::string value = "";
    int ret = GetAttr(node, "delt", value);
    if (ret == ZOK)
    {
        if (value == m_session_str) return kDelt;
        else return kNotDelt;
    }
    else if (ret == ZNONODE)
    {
        VLOG(3) << "haven't create attr node";
        return kNotDelt;
    }
    else
    {
        LOG(ERROR) << "get delt flag failed. error_code = " << ErrorString(ret);
        return kError;
    }
}
}


