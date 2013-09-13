//////////////////////////////////////////////////////////////////////////
// my_simple_http.h
// @brief:   完成页面显示基本信息和数据的simple http
// @author:  jackyzhao@tencent
// @time:    2010-8-27
// @version: 1.0
//////////////////////////////////////////////////////////////////////////

#if !defined(XFS_NODE_SERVER_MY_SIMPLE_HTTP_H_)
#define XFS_NODE_SERVER_MY_SIMPLE_HTTP_H_

class CMySimpleHttpThread: public CBaseHttpProcThread
{
public:
    CMySimpleHttpThread() {}
    virtual ~CMySimpleHttpThread() {}

    // 用户可以继承这个函数实现具体响应
    virtual bool OnUserHTTPRequest(const BufferV* ptr_received_buff, CHttpBuff* ptr_http_response);


private:
    ///////////////////////////////////////////////////////////////////////////
    // 功能描述: 添加页面底部index的函数;
    // 输入参数:
    //           ptr_http_response, 响应的CHttpBuff;
    // 返回值:   无;
    void SetBottomLink(CHttpBuff* ptr_http_response);



    ///////////////////////////////////////////////////////////////////////////
    // 功能描述: 显示已使用节点页面的函数;
    // 输入参数:
    //           ptr_http_response, 响应的CHttpBuff;
    // 返回值:   无;
    void DealGetUsedNodeReq(CHttpBuff* ptr_http_response);
};

#endif // !defined(XFS_NODE_SERVER_MY_SIMPLE_HTTP_H_)

