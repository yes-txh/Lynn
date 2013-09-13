//////////////////////////////////////////////////////////////////////////
// request_sink_test.cc
// @brief:     Test class CDualLinkListQueueMgr_T/CLinkListQueueMgr_T/
//                 CLinkedListNodesMgr/TestSmartNodes
// @author:  fatliu@tencent
// @time:      2010-10-18
// @version: 1.0
//////////////////////////////////////////////////////////////////////////

#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

typedef struct Node_ {
    unsigned char   node_state;
    uint32_t        data;
    struct Node_*           next;
    struct Node_*           pre;

    Node_() {
        node_state = NODE_STATE_IN_USE;
        data       = 0;
        next       = NULL;
        pre        = NULL;
    }

    uint32_t GetNodeVal()const {
        return data;
    }
} Node;

#ifdef WIN32
int32_t TestSinkNodes(int32_t argc, char** argv)
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

TEST(TestDualLinkListQueueMgr, Normal) {
    CDualLinkListQueueMgr_T<Node> dual_list;
    bool b = false;
    for (uint32_t u = 0; u < 5; u++) {
        Node* node = new Node();
        node->data = u;
        b = dual_list.AppendNodeAtTail(node);
        CHECK(b);
        delete node;
        node = NULL;
    }

    uint32_t num = dual_list.GetNodesCount();
    CHECK_EQ((uint32_t)5, num);

    Node* head_node = 0;
    b = dual_list.GetNodeOnHead(&head_node);
    CHECK(b);
    CHECK_EQ(static_cast<uint32_t>(0), head_node->data);
    num = dual_list.GetNodesCount();
    CHECK_EQ((uint32_t)4, num);

    b = dual_list.RemoveNode(head_node);
    CHECK(!b);

    b = dual_list.GetNodeOnHead(&head_node);
    CHECK(b);
    CHECK_EQ(static_cast<uint32_t>(1), head_node->data);

    head_node = dual_list.GetHeadNodePtr();
    CHECK(head_node);
    CHECK_EQ(static_cast<uint32_t>(2), head_node->data);
    num = dual_list.GetNodesCount();
    CHECK_EQ((uint32_t)3, num);

    Node* tail_node = 0;
    tail_node = dual_list.GetTailNodePtr();
    CHECK(tail_node);
    CHECK_EQ((uint32_t)4, tail_node->data);

    while ((tail_node = dual_list.GetTailNodePtr()) != NULL) {
        b = dual_list.RemoveNode(tail_node);
        CHECK(b);
    }
}

void RemoveErrNode() {
    CDualLinkListQueueMgr_T<Node> dual_list;
    bool b = false;
    dual_list.SetQueueEmpty();

    // abnormal
    Node* node = new Node();
    node->data = 1;
    b = dual_list.AppendNodeAtTail(node);
    CHECK(b);
    delete node;
    node = NULL;

    // remove不存在的node, abort
    Node* head_node = 0;
    b = dual_list.GetNodeOnHead(&head_node);
    CHECK(b);
    head_node->pre = NULL;
    head_node->next = NULL;
    head_node->node_state = NODE_STATE_IN_FREE_QUEUE;
    b = dual_list.RemoveNode(head_node);
}

TEST(MyDeathTest, RemoveErrNode) {
    EXPECT_DEATH(RemoveErrNode(), "");
}

TEST(TestDualLinkListQueueMgr, Abnormal) {
    CDualLinkListQueueMgr_T<Node> dual_list;
    bool b = false;
    dual_list.SetQueueEmpty();

    // abnormal
    Node* node = new Node();
    node->data = 1;
    b = dual_list.AppendNodeAtTail(node);
    CHECK(b);
    delete node;
    node = NULL;

    // 重复追加同一node, fail
    b = dual_list.AppendNodeAtTail(node);
    CHECK(!b);
}
TEST(TestLinkListQueueMgr, LinkListQueueMgr) {
    CLinkListQueueMgr_T<Node> list;
    bool b = false;
    for (uint32_t u = 0; u < 5; u++) {
        Node* node = new Node();
        node->data = u;
        b = list.AppendNodeAtTail(node);
        CHECK(b);
        delete node;
        node = NULL;
    }

    uint32_t num = list.GetNodesCount();
    CHECK_EQ((uint32_t)5, num);

    Node* head_node = 0;
    b = list.GetNodeOnHead(&head_node);
    CHECK(b);
    CHECK_EQ(static_cast<uint32_t>(0), head_node->data);
    num = list.GetNodesCount();
    CHECK_EQ(static_cast<uint32_t>(4), num);

    b = list.GetNodeOnHead(&head_node);
    CHECK(b);
    CHECK_EQ(static_cast<uint32_t>(1), head_node->data);

    head_node = list.GetHeadNodePtr();
    CHECK(head_node);
    CHECK_EQ(static_cast<uint32_t>(2), head_node->data);
    num = list.GetNodesCount();
    CHECK_EQ((uint32_t)3, num);
}

TEST(TestLinkedListNodesMgr, LinkedListNodesMgr) {
    CLinkedListNodesMgr<Node> list;
    bool b = false;

    b = list.IsQueueEmpty();
    CHECK(b);

    b = list.Init(5);
    CHECK(b);

    for (uint32_t u = 0; u < 3; u++) {
        Node* new_node = 0;
        b = list.GetNodeOnHead(&new_node);
        CHECK(b);
        new_node->data = u;
        b = list.AppendNodeAtTail(new_node);
        CHECK(b);
    }

    uint32_t num = list.GetMaxNodesCount();
    CHECK_EQ((uint32_t)5, num);

    int32_t cur_num = list.GetCurrValidNodes();
    CHECK_EQ(5, cur_num);

    Node* new_node = list.GetNodePtrByIndex(2);
    CHECK_EQ((uint32_t)2, new_node->data);

    uint64_t index = list.GetNodeIndex(new_node);
    CHECK_EQ((uint64_t)2, index);

    // valid node ptr
    b = list.IsValidNodePtr(new_node);
    CHECK(b);

    // invalid node ptr
    new_node -= 3;
    b = list.IsValidNodePtr(new_node);
    CHECK(!b);

    b = list.ReconfigNodes(6);
    CHECK(b);
    num = list.GetMaxNodesCount();
    CHECK_EQ((uint32_t)6, num);

    b = list.IsQueueEmpty();
    CHECK(!b);

    list.Reset();
    b = list.IsQueueEmpty();
    CHECK(!b);

    list.Uninit();
}

TEST(TestSmartNodes, SmartNodes) {
    CSmartNodes<Node, 5> smart_nodes;
    bool b = false;

    for (uint32_t u = 0; u < 3; u++) {
        Node* new_node = smart_nodes.GetNode();
        new_node->data = u;
        smart_nodes.PutNode(new_node);
    }

    int32_t cur_free_nodes = smart_nodes.GetCurrFreeNodes();
    CHECK_EQ(5, cur_free_nodes);

    Node* new_node = smart_nodes.GetNodePtrByIndex(2);
    CHECK_EQ((uint32_t)2, new_node->data);

    uint64_t index = smart_nodes.GetNodeIndex(new_node);
    CHECK_EQ((uint64_t)2, index);

    b = smart_nodes.ReconfigNodes(6);
    CHECK(b);
    cur_free_nodes = smart_nodes.GetCurrFreeNodes();
    CHECK_EQ((uint32_t)6, cur_free_nodes);
}

TEST(TestQuickSort_DESC, QuickSort_DESC) {
    Node node_array[10];
    srand(static_cast<uint32_t>(time(NULL)));
    for (uint32_t u = 0; u < 10; u++) {
        node_array[u].data = safe_rand();
    }
    QuickSort_DESC<Node, uint32_t>(0, 9, node_array);
    for (uint32_t u = 0; u < 9; u++) {
        CHECK_GE(node_array[u].data, node_array[u+1].data);
    }
}

