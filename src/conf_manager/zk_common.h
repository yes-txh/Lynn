#ifndef CONF_MANAGER_ZK_COMMON_H
#define CONF_MANAGER_ZK_COMMON_H

#include <string>
#include <map>
#include <vector>
#include <list>
#include <zookeeper/zookeeper.h>
#include "conf_manager/watcher.h"

class ZookeeperCommon {
 public:
    /// host and ports info of zk servers
    std::string m_hostports;

    /*
     * @brief: create client with zk using the hostports of cluster.
     *
     * @watchctx_t: the watchctx_t.
     * @ret: the zhandle of this client.
     */
    zhandle_t* createClient(watchctx_t *ctx);

    /*
     * @brief:call zookeeper_init 
     *
     * @watchctx_t: the watchctx_t.
     * @ret: the zhandle of this client.
     */
    zhandle_t* createClient(const char *hp, watchctx_t *ctx);

 public:
    static const int S_MAX_LEN = 1024;

    ZookeeperCommon();
    ~ZookeeperCommon();

    /*
     * @brief: get zk server form cluster_name and create connection to zk 
     *
     * @cluster_name: the name of this cluster.
     * @ret: the zk node string.
     */
    int Init(const std::string& cluster_name, const std::string& hostports);

    /*
     * @brief:get the zk_prefix of this cluster 
     *
     * @ret: the zk prefix string.
     */
    std::string GetZKPrefix();

    /*
     * @brief:get the cluster name of this cluster 
     *
     * @ret: the cluster name string of tborg.
     */
    std::string GetClusterName();
    /*
     * @brief: get all zk nodes from a path.
     *
     * @path: the given path.
     * @ret: the zk nodes vector.
     */
    std::vector<std::string> GetAllNodesFromPath(const std::string &path);

    /*
     * @brief: create a node forcefully, if some of these nodes do not exist,
     * its children can also be created
     *
     * @node_path: the given path string.
     * @ret: if the node has already exist or created successfully, return0.
     * if not,return -1
     */
    int CreateNodeForce(const std::string& node_path);

    /*
     * @brief:get a zk node according to the given path 
     *
     * @path: the given path string.
     * @ret: the zk node string.
     */
    std::string GetNodeFromPath(const std::string &path);
    /// if the m_zk is invalid, reconnect zk
    int Reconnect();

    /*
     * @brief:just create a node, but no need to specify its value
     *
     * @node_path: the given path string.
     * @ret: if the node has already exist or created successfully, return0.
     * if not,return -1
     */
    int CreateNode(const std::string& node_path, bool is_temporary = false);

    /*
     * @brief:check if a path exists in zk
     *
     * @path: the given path string.
     * @ret: if the node exists, return -1. if it doesn't exist or error happens, return -1.
     */
    int CheckPathExist(const std::string& path);

    /*
     * @brief:check if a path exists in zk
     *
     * @path: the given path string.
     * @ret: if the node exists, return -1. if it doesn't exist or error happens, return -1.
     */
    int WCheckPathExist(const std::string& path,  watcher_fn watcher);

    /*
     * @brief: create a node, and set its value according to the args(forcefully)
     * 
     * @node_path: the given path string.
     * @value: the value of this node
     * @ret: if the node has already exist or created successfully, return 0.
     * if not,return -1
     */
    int CreateNodeWithValue(const std::string& node_path, const std::string& value, bool is_temporary = false);

    /*
     * @brief: set one node's value
     * 
     * @node_path: the given path string.
     * @value: the value of this node
     * @ret: if the node does not exist or set value fails, return -1.
     */
    int SetValueOfNode(const std::string& path, const std::string& value);

    /*
     * @brief: get one node's value
     * 
     * @node_path: the given path string.
     * @value: the returned value of this node
     * @ret: if the node does not exist or get value fails, return -1.
     */
    int GetValueOfNode(const std::string& path, std::string *value);

    /*
     * @brief: delete one node,if the node has children, it can not be deleted
     * 
     * @path: the given path string.
     * @ret: if error happens, return -1,else return 0
     */
    int DeleteNode(const std::string& path);

    /*
     * @brief: delete one node forcefully,if the node has grandchildren, it can not be deleted
     * 
     * @path: the given path string.
     * @ret: if error happens, return -1,else return 0
     */
    int GetChildren(const std::string& path, std::map<std::string, std::string> *results);

    /*
     * @brief: get one node's value with a watcher func
     * 
     * @node_path: the given path string.
     * @watcher_fn: watcher func
     * @value: the returned value of this node
     * @ret: if error happens, return -1.
     */
    int WGetValueOfNode(const std::string& path, watcher_fn watcher, std::string* value);

    /*
     * @brief: delete one node forcefully,if the node has grandchildren, it can not be deleted
     * 
     * @path: the given path string.
     * @ret: if error happens, return -1,else return 0
     */
    int DeleteNodeForce(const std::string& path);
    /*
     * @brief: delete one node recursively
     * 
     * @path: the given path string.
     * @ret: if error happens, return -1, else return 0
     */
    int DeleteNodeForceRecursively(const std::string& path);
 private:
    /// prefix for cluster in zk
    /// e.g.
    std::string m_zk_prefix;
    /// cluster name
    std::string m_cluster_name;
    /// zk handler
    zhandle_t* m_zk;
    watchctx_t m_ctx;
};
#endif // TBORG_CONF_MANAGER_ZK_COMMON_H
