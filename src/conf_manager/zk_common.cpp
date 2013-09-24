#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include <boost/algorithm/string.hpp>

#include "conf_manager/zk_common.h"

using log4cplus::Logger;

static Logger logger = Logger::getInstance("collector");

/*
 * @brief: get all zk nodes from a path.
 *
 * @path: the given path.
 * @ret: the zk nodes vector.
 */
std::vector<std::string> ZookeeperCommon::GetAllNodesFromPath(const std::string &path) {
    std::vector<std::string> items, results;
    std::vector<std::string>::size_type ix = 0;
 
    boost::split(items, path, boost::is_any_of("/"));
    //SplitString(path, "/", &items);

    if (items.size() == 0) {
        return results;
    }
    /// get the first node
    results.push_back("/" + items[0]);
    /// get all remaining nodes, add one '/' one times
    for (ix = 0; ix != items.size() - 1; ++ix) {
        results.push_back(results[ix] + "/" + items[ix + 1]);
    }

    return results;
}

/*
 * @brief: create client with zk using the hostports of cluster.
 *
 * @watchctx_t: the watchctx_t.
 * @ret: the zhandle of this client.
 */
zhandle_t* ZookeeperCommon::createClient(watchctx_t *ctx) {
    return createClient(m_hostports.c_str(), ctx);
}

/*
 * @brief:call zookeeper_init 
 *
 * @watchctx_t: the watchctx_t.
 * @ret: the zhandle of this client.
 */
zhandle_t* ZookeeperCommon::createClient(const char *hp, watchctx_t *ctx) {
    zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
    zoo_deterministic_conn_order(1);
    zhandle_t *zk = zookeeper_init(hp, ValueWatcher, 10000, 0, ctx, 0);

    ctx->zh = zk;
    ctx->WaitForConnected(zk);
    //TODO
    //zoo_add_auth(zk, "digest", "borg:secret", 11, 0, 0);

    return zk;
}

/*
 * @brief:get a zk node according to the given path 
 *
 * @path: the given path string.
 * @ret: the zk node string.
 */
std::string ZookeeperCommon::GetNodeFromPath(const std::string &path) {
    std::string node;
    std::vector<std::string> items;
    boost::split(items, path, boost::is_any_of("/"));
    //SplitString(path, "/", &items);
    if (items.size() != 0) {
        //node = StringTrim(*(--items.end()));
        node = *(--items.end());
        boost::trim(node);
    }
    return node;
}

// ctor of ZookeeperCommon
ZookeeperCommon::ZookeeperCommon() {
}

// dtor of ZookeeperCommon
ZookeeperCommon::~ZookeeperCommon() { 
   if(NULL != m_zk) {
       zookeeper_close(m_zk);
   }
}

/*
 * @brief:get zk server form cluster_name and create connection to zk 
 *
 * @cluster_name: the name of this cluster.
 * @ret: the zk node string.
 */
int ZookeeperCommon::Init(const std::string& cluster_name, const std::string& hostports) {
    m_cluster_name = cluster_name;
    m_zk_prefix = "/lynn/" + cluster_name;

    if (!hostports.empty()) {
        m_hostports = hostports;
    } else {
        m_hostports = FLAGS_zk_server;
    }

    // create connection with zk
    m_zk = createClient(&m_ctx);
    if (!m_zk || !m_ctx.connected) {
        return -1;
    }
    return 0;
}

/*
 * @brief:reconnect zk 
 *
 * @ret: return 0 if success, else return -1 
 */
int ZookeeperCommon::Reconnect() {
    m_zk = createClient(&m_ctx);
    if (!m_zk) {
        return -1;
    }
    return 0;
}

/*
 * @brief:get the zk_prefix of this cluster 
 *
 * @ret: the zk prefix string.
 */
std::string ZookeeperCommon::GetZKPrefix() {
    return m_zk_prefix;
}

/*
 * @brief:get the cluster name of this cluster 
 *
 * @ret: the cluster name string of tborg.
 */
std::string ZookeeperCommon::GetClusterName() {
    return m_cluster_name;
}

/*
 * @brief:check if a path exists in zk
 *
 * @path: the given path string.
 * @ret: if the node exists, return -1. if it doesn't exist or error happens, return -1.
 */
int ZookeeperCommon::CheckPathExist(const std::string& path) {
    /// check if the module node exists
    Stat stat;
    if (NULL == m_zk) {
        LOG4CPLUS_ERROR(logger, "m_zk is null");
        return -1;
    }
    int zrc = zoo_exists(m_zk, const_cast<char*>(path.c_str()), 0, &stat);
    if (zrc != ZOK) {
        LOG4CPLUS_WARN(logger, "zoo_exists failed, the error is .. " << zerror(zrc));
        return -1;
    }

    return 0;
}

/*
 * @brief:check if a path exists in zk with a wacther func
 *
 * @path: the given path string.
 * @watcher_fn: the watcher func.
 * @ret: if nothing wrong happens,return 0.else return -1.
 */
int ZookeeperCommon::WCheckPathExist(const std::string& path, watcher_fn watcher) {
    int zrc = 0;
    if (NULL == m_zk) {
        LOG4CPLUS_ERROR(logger, "m_zk is null");
        return -1;
    }
    Stat stat;
    zrc = zoo_wexists(m_zk, path.c_str(), watcher, NULL, &stat);
    if (zrc != ZOK) {
        LOG4CPLUS_WARN(logger, "zoo_get failed, the return value is " << zerror(zrc));
        return -1;
    }
    return 0;
}

/*
 * @brief:just create a node, but no need to specify its value
 *
 * @node_path: the given path string.
 * @ret: if the node has already exist or created successfully, return0.
 * if not,return -1
 */
int ZookeeperCommon::CreateNode(const std::string& node_path, bool is_temporary) {
    LOG4CPLUS_INFO(logger,  "begin to create a node in zookeeper.");
    LOG4CPLUS_INFO(logger,  "path:" << node_path);

    int zrc = 0;
    int node_flag = 0;
    if (NULL == m_zk) {
        LOG4CPLUS_WARN(logger,  "m_zk is null");
        return -1;
    }
    if (is_temporary) {
        node_flag = ZOO_EPHEMERAL;
    }

    zrc = zoo_create(m_zk, node_path.c_str(), NULL, -1,  &ZOO_OPEN_ACL_UNSAFE,
                     node_flag, const_cast<char*>(node_path.c_str()), node_path.length()+1);

    if (zrc != ZOK && zrc != ZNODEEXISTS) {
        LOG4CPLUS_WARN(logger, "ERROR happens when create the node: " << node_path);
        LOG4CPLUS_WARN(logger, "the error is .. " << zerror(zrc));
        return -1;
    }
    LOG4CPLUS_INFO(logger,  "the node is created successfully.");
    return 0;
}

/*
 * @brief: create a node forcefully, if some of these nodes do not exist,
 * its children can also be created
 *
 * @node_path: the given path string.
 * @ret: if the node has already exist or created successfully, return0.
 * if not,return -1
 */
int ZookeeperCommon::CreateNodeForce(const std::string& node_path) {
    LOG4CPLUS_INFO(logger,  "begin to create a node in zookeeper forcelly.");
    LOG4CPLUS_INFO(logger,  "path: " << node_path);

    std::vector<std::string> nodes;
    nodes = GetAllNodesFromPath(node_path);
    int rc = 0;
    for (std::vector<std::string>::size_type ix = 0;
         ix != nodes.size(); ++ix) {
        // create node in zookeeper for zk_prefix
        // prefix node has no value
        rc = CreateNode(nodes[ix]);
    }

    return 0;
}

/*
 * @brief: create a node, and set its value according to the args(forcefully)
 * 
 * @node_path: the given path string.
 * @value: the value of this node
 * @ret: if the node has already exist or created successfully, return 0.
 * if not,return -1
 */
int ZookeeperCommon::CreateNodeWithValue(const std::string& node_path, const std::string& value, bool is_temporary) {
    LOG4CPLUS_INFO(logger,  "begin to create a node with value in zookeeper.");
    LOG4CPLUS_INFO(logger,  "path: " << node_path);
    LOG4CPLUS_INFO(logger,  "value: " << value);

    int zrc = 0;
    if (NULL == m_zk) {
        LOG4CPLUS_ERROR(logger, "m_zk is null");
        return -1;
    }
    int node_flag = 0;
    if (is_temporary) {
        node_flag = ZOO_EPHEMERAL;
    }

    zrc = zoo_create(m_zk, node_path.c_str(), const_cast<char *>(value.c_str()),
                     value.length(),  &ZOO_OPEN_ACL_UNSAFE, node_flag,
                     const_cast<char *>(node_path.c_str()), node_path.length() + 1);

    if (ZNODEEXISTS == zrc || ZOK == zrc) {
        zrc = SetValueOfNode(node_path, value);
        if (zrc < 0) {
            return -1;
        }
    } else {
        LOG4CPLUS_WARN(logger,  "ERROR happens when create the node: " << node_path);
        LOG4CPLUS_WARN(logger,  "the error is .. " << zerror(zrc));
        return -1;
    }
    LOG4CPLUS_INFO(logger,  "the node with value is created successfully.");
    return 0;
}

/*
 * @brief: set one node's value
 * 
 * @node_path: the given path string.
 * @value: the value of this node
 * @ret: if the node does not exist or set value fails, return -1.
 */
int ZookeeperCommon::SetValueOfNode(const std::string& path, const std::string& value) {
    int zrc = 0;
    if (NULL == m_zk) {
        LOG4CPLUS_WARN(logger,  "m_zk is null");
        return -1;
    }

    zrc = zoo_set(m_zk, path.c_str(), const_cast<char *>(value.c_str()), value.length(), -1);
    if (zrc != ZOK) {
        LOG4CPLUS_WARN(logger,  "data set failed... " << path.c_str() << " " << zerror(zrc));
        return -1;
    }
    return 0;
}

/*
 * @brief: get one node's value
 * 
 * @node_path: the given path string.
 * @value: the returned value of this node
 * @ret: if the node does not exist or get value fails, return -1.
 */
int ZookeeperCommon::GetValueOfNode(const std::string& path, std::string *value) {
    value->clear();
    int zrc = 0;
    if (NULL == m_zk) {
        LOG4CPLUS_ERROR(logger, "m_zk is null");
        return -1;
    }

    char buffer[S_MAX_LEN] = "";
    int blen = S_MAX_LEN;
    Stat stat;
    zrc = zoo_get(m_zk, path.c_str(), 0, buffer, &blen, &stat);
    if (zrc != ZOK) {
        LOG4CPLUS_ERROR(logger, "zoo_get failed, the return value is " << zerror(zrc));
        return -1;
    }
    *value = buffer;
    return 0;
}

/*
 * @brief: get one node's value with a watcher func
 * 
 * @node_path: the given path string.
 * @watcher_fn: watcher func
 * @value: the returned value of this node
 * @ret: if error happens, return -1.
 */
int ZookeeperCommon::WGetValueOfNode(const std::string& path,
        watcher_fn watcher, std::string* value) {
    value->clear();
    int zrc = 0;
    if (NULL == m_zk) {
        LOG4CPLUS_ERROR(logger, "m_zk is null");
        return -1;
    }
    Stat stat;
    char buffer[S_MAX_LEN] = "";
    int blen = S_MAX_LEN;
    zrc = zoo_wget(m_zk, path.c_str(), watcher, NULL, buffer, &blen, &stat);
    if (zrc != ZOK) {
        LOG4CPLUS_ERROR(logger, "zoo_wget failed, the return value is " << zerror(zrc));
        return -1;
    }
    *value = buffer;
    return 0;
}

/*
 * @brief: delete one node,if the node has children, it can not be deleted
 * 
 * @path: the given path string.
 * @ret: if error happens, return -1,else return 0
 */
int ZookeeperCommon::DeleteNode(const std::string& path) {
    int rc = 0;
    if (NULL == m_zk) {
        LOG4CPLUS_ERROR(logger, "m_zk is null");
        return -1;
    }

    rc = zoo_delete(m_zk, const_cast<char*>(path.c_str()), -1);
    if (rc != ZOK && rc != ZNONODE) {
        LOG4CPLUS_ERROR(logger, "zoo_delete failed, the return value is " << zerror(rc));
        return -1;
    }
    return 0;
}

/*
 * @brief: delete one node forcefully,if the node has grandchildren, it can not be deleted
 * 
 * @path: the given path string.
 * @ret: if error happens, return -1,else return 0
 */
int ZookeeperCommon::DeleteNodeForce(const std::string& path) {
    int rc = 0;
    if (m_zk == NULL) {
        LOG4CPLUS_ERROR(logger, "m_zk is null");
        return -1;
    }
    /// check whether the node has grandchildren node
    String_vector children;
    rc = zoo_get_children(m_zk, path.c_str(), 0, &children);
    if (rc == ZNONODE) {
        return 0;
    }
    if (rc != ZOK) {
        LOG4CPLUS_WARN(logger,  "error happens when zoo_get_children: " << zerror(rc));
        return -1;
    }

    /// check if the ndoe can be deleted
    for (int i = 0; i < children.count; ++i) {
        std::string child_path = path + "/" + children.data[i];
        String_vector grand_children;
        int rt = zoo_get_children(m_zk, child_path.c_str(), 0, &grand_children);
        if (ZOK == rt && grand_children.count > 0) {
            LOG4CPLUS_WARN(logger,  "The node " << path << "has grandchilden, it can't be deleted");
            return -1;
        }
    }

    /// delete all children of this node
    for (int i = 0; i < children.count; ++i) {
        std::string child_path = path + "/" + children.data[i];
        int delete_rt = zoo_delete(m_zk, const_cast<char*>(child_path.c_str()), -1);
        if (delete_rt != ZOK) {
            // LOG4CPLUS_WARN(logger,  "zoo_delete failed, the return value is " << zerror(delete_rt));
            continue;
        }
    }
    /// at the end, delete the node itself
    rc = zoo_delete(m_zk, const_cast<char*>(path.c_str()), -1);
    if (rc != ZOK && rc != ZNONODE) {
        // LOG4CPLUS_WARN(logger,  "zoo_delete failed, the return value is " << zerror(rc));
        return -1;
    }
    return 0;
}

/*
 * @brief: delete one node recursively
 * 
 * @path: the given path string.
 * @ret: if error happens, return -1, else return 0
 */
int ZookeeperCommon::DeleteNodeForceRecursively(const std::string& path) {
    int rc = 0;
    if (m_zk == NULL) {
        LOG4CPLUS_ERROR(logger, "m_zk is null");
        return -1;
    }
    /// check whether the node has grandchildren node
    String_vector children;
    rc = zoo_get_children(m_zk, path.c_str(), 0, &children);
    if (rc == ZNONODE) {
        return 0;
    }
    if (rc != ZOK) {
        LOG4CPLUS_WARN(logger,  "error happens when zoo_get_children: " << zerror(rc));
        return -1;
    }

    /// delete all children of this node
    for (int i = 0; i < children.count; ++i) {
        std::string child_path = path + "/" + children.data[i];
        DeleteNodeForceRecursively(child_path);
        int delete_rt = zoo_delete(m_zk, const_cast<char*>(child_path.c_str()), -1);
        if (delete_rt != ZOK) {
            // LOG4CPLUS_WARN(logger,  "zoo_delete failed, the return value is " << zerror(delete_rt));
            continue;
        }
    }
    /// at the end, delete the node itself
    rc = zoo_delete(m_zk, const_cast<char*>(path.c_str()), -1);
    if (rc != ZOK && rc != ZNONODE) {
        // LOG4CPLUS_WARN(logger,  "zoo_delete failed, the return value is " << zerror(rc));
        deallocate_String_vector(&children);
        return -1;
    }
    deallocate_String_vector(&children);
    return 0;
}

/*
 * @brief:get all children of one node 
 * 
 * @path: the given path string.
 * @results: the children(path of one child:value of this child)
 * @ret: if error happens, return -1,else return the children number 
 */
int ZookeeperCommon::GetChildren(const std::string& path,
                                 std::map<std::string,
                                 std::string> *results) {
    int rc = 0;
    results->clear();
    if (NULL == m_zk) {
        LOG4CPLUS_ERROR(logger, "m_zk is null");
        return -1;
    }
    /// check if the module node exists
    Stat stat;
    /// get all nodes and values
    String_vector children;
    rc = zoo_get_children(m_zk, path.c_str(), 0, &children);
    if (rc != ZOK) {
        LOG4CPLUS_WARN(logger,  "error happens when zoo_get_children: " << zerror(rc));
        return -1;
    }
    for (int i = 0; i < children.count; ++i) {
        std::string child_path = path + "/" + children.data[i];
        std::string value;

        char buffer[S_MAX_LEN] = "";
        int blen = sizeof(buffer);
        rc = zoo_get(m_zk, child_path.c_str(), 0, buffer, &blen, &stat);
        if (rc != ZOK) {
            LOG4CPLUS_WARN(logger,  "error happens when zoo_get: " << zerror(rc));
            continue;
        }

        value = buffer;
        results->insert(make_pair(children.data[i], value));
    }
    rc = children.count;
    deallocate_String_vector(&children);
    return rc;
}
