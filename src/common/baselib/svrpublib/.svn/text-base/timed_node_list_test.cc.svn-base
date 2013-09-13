//////////////////////////////////////////////////////////////////////////
// timed_node_list_test.cc
// @brief:     Test class CTimedLoopList_T/CNewTimedLoopList_T
// @author:  fatliu@tencent
// @time:     2010-10-18
// @version: 1.0
//////////////////////////////////////////////////////////////////////////

#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

class MyCallBack : ITimeoutCallback<uint32_t> {
public:
    MyCallBack():times(0) {
    }

    bool OnTimeoutCallback(TimedBuffNode<uint32>* node) {
        node->fd = (SOCKET)times;
        printf("my call back class, u = [%d]..\r\n", node->fd);
        times++;
        return true;
    }
    uint32_t times;
};

class MyNewCallBack : INewTimeoutCallback<uint32_t> {
public:
    explicit MyNewCallBack(uint32_t u):times(u) {
    }

    bool OnTimeoutCallback(uint32_t u) {
        u = times;
        printf("my call back class, u = [%d]..\r\n", u);
        times++;
        return true;
    }
    uint32_t times;
};

#ifdef WIN32
int32_t TestTimedNodeList(int32_t argc, char** argv)
#else
int32_t main(int32_t argc, char** argv)
#endif
{
#ifndef WIN32
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, true);

    AutoBaseLib auto_baselib;
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
#else
    return 0;
#endif
}

TEST(CTimedLoopList, TimedLoopList) {
    bool b = false;
    MyCallBack cb;

    CTimedLoopList_T<uint32_t> list;
    uint16_t max_nodes = 5;
    uint16_t node_buff_len = 256;
    EPOLLHANDLE epoll_handle = 0;
    uint16_t timeout = 1;

    b = list.Init(max_nodes,
                  node_buff_len,
                  epoll_handle,
                  timeout,
                  (ITimeoutCallback<uint32_t>*)&cb);
    CHECK(b);

    uint16_t node_data_buff_len = list.GetNodeDataBuffLen();
    CHECK_EQ(node_buff_len, node_data_buff_len);

    TimedBuffNode<uint32_t>* node = 0;
    uint16_t max_buff_len = 0;
    b = list.GetNextEmptyNodePtr(&node, &max_buff_len);
    CHECK(b);
    CHECK_EQ(max_buff_len, node_buff_len);

    // test set data to next node finished
    b = list.SetDataToNextNodeFinished(node);
    CHECK(b);

    // use up all nodes
    for (int32_t i = 0; i < 3; i++) {
        TimedBuffNode<uint32_t>* new_node = 0;
        b = list.GetNextEmptyNodePtr(&new_node, &max_buff_len);
        CHECK_EQ(max_buff_len, node_buff_len);
        CHECK(b);
        b = list.SetDataToNextNodeFinished(new_node);
        CHECK(b);
    }

    // no node to write
    TimedBuffNode<uint32_t>* bad_node = 0;
    b = list.GetNextEmptyNodePtr(&bad_node, &max_buff_len);
    CHECK(!b);
    CHECK_EQ(max_buff_len, node_buff_len);

    b = list.SetDataToNextNodeFinished(bad_node);
    CHECK(!b);

    // m_node_head.valid is always false
    b = list.IsBufferReady();
    CHECK(b);

    // timeout...
    XSleep(1500);
    // call 3 times of MyCallBack
    list.CheckTimeOut();

    TimedBuffNode<uint32_t>* listen_node;
    list.GetListenNode(&listen_node, &max_buff_len);
    CHECK_EQ(max_buff_len, node_buff_len);

    list.IndicateCloseSocket(node, (int32_t)0);
    CHECK_EQ((uint32_t)node->fd, INVALID_SOCKET);

    list.RemoveHandleFromEpollSet(node);
    CHECK_EQ((uint32_t)node->fd, INVALID_SOCKET);

    list.Uninit();
}

TEST(CNewTimedLoopList, NewTimedLoopList) {
    bool b = false;
    MyNewCallBack cb(0);

    CNewTimedLoopList_T<uint32_t> list;
    uint16_t max_nodes = 5;
    uint16_t timeout = 1;
    b = list.Init(max_nodes, timeout, (INewTimeoutCallback<uint32_t>*)&cb);
    CHECK(b);

    // test get next empty node
    NewTimedNode<uint32_t>* node = 0;
    b = list.GetNextEmptyNodePtr(&node);
    CHECK(b);
    CHECK(!node->valid);

    // test set data to next node finished
    b = list.SetDataToNextNodeFinished(node);
    CHECK(b);
    CHECK(node->valid);

    // test set node invalid
    list.SetNodeInvalid(node);
    CHECK(!node->valid);

    // use up all nodes
    for (int32_t i = 0; i < 3; i++) {
        NewTimedNode<uint32_t>* new_node = 0;
        b = list.GetNextEmptyNodePtr(&new_node);
        CHECK(b);
        b = list.SetDataToNextNodeFinished(new_node);
        CHECK(b);
        CHECK(new_node->valid);
    }

    // no node to write
    NewTimedNode<uint32_t>* bad_node = 0;
    b = list.GetNextEmptyNodePtr(&bad_node);
    CHECK(!b);

    b = list.SetDataToNextNodeFinished(bad_node);
    CHECK(!b);

    // m_node_head.valid is always false
    b = list.IsBufferReady();
    CHECK(b);

    // timeout...
    XSleep(1500);
    // call 3 times of MyCallBack
    list.CheckTimeOut();

    list.Uninit();
}
