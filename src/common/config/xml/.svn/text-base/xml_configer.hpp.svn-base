/**
 * XML配置文件的解析，完全平台无关
 * @author jerryzhang@tencent.com
 * @since  2006.04.20
 */

#ifndef COMMON_CONFOG_XML_XML_CONFIGER_HPP
#define COMMON_CONFOG_XML_XML_CONFIGER_HPP

#include <stddef.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <stdexcept>

/** 下面是解析XML文件中用到的一些数据结构 */
struct parser_result_field
{
    char* data;
    int datalength;
    struct parser_result_field *NextResult;
};

struct parser_result
{
    struct parser_result_field *FirstResult;
    struct parser_result_field *LastResult;
    int NumResults;
};

struct XMLNode
{
    char* Name;
    int NameLength;

    char* NSTag;
    int NSLength;
    int StartTag;
    int EmptyTag;

    void *Reserved;
    void *Reserved2;
    struct XMLNode *Next;
    struct XMLNode *Parent;
    struct XMLNode *Peer;
    struct XMLNode *ClosingTag;
    struct XMLNode *StartingTag;
};

struct XMLStackNode
{
    void *Data;
    struct XMLStackNode *Next;
};


struct HashNode_Root
{
    struct XMLHashNode *Root;
};

struct XMLHashNode
{
    struct XMLHashNode *Next;
    struct XMLHashNode *Prev;
    int KeyHash;
    char *KeyValue;
    int KeyLength;
    void *Data;
};

#ifndef DEPRECATED_SYMBOL
#if defined __GNUC__
#define DEPRECATED_SYMBOL __attribute__((deprecated))
#elif defined _MSC_VER
#define DEPRECATED_SYMBOL __declspec(deprecated)
#else
#define DEPRECATED_SYMBOL
#endif
#endif

class XMLConfiger
{
public:
    XMLConfiger(){m_strSeparator = "&";};
    XMLConfiger(const std::string& strSeparator){m_strSeparator = strSeparator;};

    /** 解析XML文件到一个multimap中 */
    bool ParseFile(const char* pcXmlFile);

    /** 解析XML文件到一个multimap中 */
    bool ParseFile(const std::string& strXmlFile);
    bool ParseXMLString(const std::string& strContent);

    /** 输出读入到的XML文件，调试时用*/
    std::string ToString();

    /** 得到配置项的所有子节点的个数 */
    int  GetAllSubElementNum(const std::string& strKey);

    /** 得到配置项的直接子节点 */
    int  GetDirectSubElement(const std::string& strKey,
                             std::set<std::string>& sSubElement);

    /** 得到字符串类型的配置项的值 */
    bool GetValue(const std::string& strKey, std::string& strValue) const;

    /** 得到整数类型的配置项的值*/
    bool GetValue(const std::string& strKey, int& value) const;
    bool GetValue(const std::string& strKey, unsigned int& value) const;
    bool GetValue(const std::string& strKey, short& value) const;
    bool GetValue(const std::string& strKey, unsigned short& value) const;
    bool GetValue(const std::string& strKey, long& value) const;
    bool GetValue(const std::string& strKey, unsigned long& value) const;
    bool GetValue(const std::string& strKey, long long& value) const;
    bool GetValue(const std::string& strKey, unsigned long long& value) const;

    /** 得到某个布尔类型的配置项的值*/
    bool GetValue(const std::string& strKey, bool& value) const;

    /** 得到某个浮点类型的配置项的值*/
    bool GetValue(const std::string& strKey, float& value) const;
    bool GetValue(const std::string& strKey, double& value) const;

    /** 得到多个字符串类型的配置项的值*/
    bool GetMultiValue(const std::string& strKey, std::vector<std::string>& vStrValue) const;

    /** 得到多个整型的配置项的值*/
    bool GetMultiValue(const std::string& strKey, std::vector<short>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<unsigned short>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<int>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<unsigned int>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<long>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<unsigned long>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<long long>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<unsigned long long>& values) const;

    /** 得到多个bool型的配置项的值*/
    bool GetMultiValue(const std::string& strKey, std::vector<bool>& values) const;

    /** 得到多个浮点型的配置项的值*/
    bool GetMultiValue(const std::string& strKey, std::vector<float>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<double>& values) const;

    template <typename T>
    T GetValue(const std::string& name) const
    {
        T result;
        if (GetValue(name, result))
            return result;
        else
            throw std::runtime_error("Can't read configure item: " + name);
    }

private:
    /* 解析XML的过程中用到的堆栈 */
    void CreateStack(void **TheStack);

    /* 把一个元属压入到栈中 */
    void PushStack(void **TheStack, void *data);

    /* 从栈中取出一个元属 */
    void *PopStack(void **TheStack);

    /* 查看栈顶的那个元属，不取走 */
    void *PeekStack(void **TheStack);

    /* 清除栈中所有项 */
    void ClearStack(void **TheStack);

    /* XML 解析方法 */
    struct XMLNode *ParseXML(const char *buffer, size_t offset, size_t length);

    /* 检查XML文件的有效性 */
    int ProcessXMLNodeList(struct XMLNode *nodeList);

    /* 释放XML结点树中的资源 */
    void DestructXMLNodeList(struct XMLNode *node);

    /* 创建一个空的hash树 */
    void* InitHashTree();

    /* 释放hash树资源 */
    void DestroyHashTree(void *tree);

    /* 把一个字符串解析为一个token的链表 */
    struct parser_result* ParseString(
        const char* buffer, size_t offset, size_t length,
        const char* Delimiter, size_t DelimiterLength);

    /* 一段内存值的比较
     * 检查buffer从offset开始的内存是否和从Delimiter开始的，
     * 长度为DelimiterLength的内存值相同，不同就返回0
     */
    bool IsDelimiter(
        const char* buffer, size_t offset, size_t buffersize,
        const char* Delimiter, size_t DelimiterLength);

    /* 释放ParseString解析后的token链表的资源 */
    void DestructParserResults(struct parser_result *result);

    /* 得到XML字符串中第一个tag */
    static std::string GetFirstTag(const std::string& strTag);

    /* 得到XML字符串中某个tag的value */
    static std::string GetTagValue(const std::string& strTag);

    /* 把XML的一个路径生成为一个字符串 */
    std::string GetTagPath(const std::vector<std::string>& vTags) const;

    /* 将XML文件中的个节点的路径和对应的值放到一个map中 */
    int GetAllTags(struct XMLNode *nodeList, std::multimap<std::string, std::string>& mXmlTags);

    /* 对XML文件进行预处理，就是把"&<>'""替换掉 */
    std::string PreDeal(const std::string& strRawFile);

    /* 把"&<>'""转义后的字符串还原 */
    static std::string UnTransfer(std::string& strAfter);

private:
    /* 所有的配置项的值*/
    std::multimap<std::string, std::string> m_mConfig;

    /* 配置项之间分界字符串*/
    std::string m_strSeparator;
};

#endif // COMMON_CONFOG_XML_XML_CONFIGER_HPP
