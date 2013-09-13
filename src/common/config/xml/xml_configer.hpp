/**
 * XML�����ļ��Ľ�������ȫƽ̨�޹�
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

/** �����ǽ���XML�ļ����õ���һЩ���ݽṹ */
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

    /** ����XML�ļ���һ��multimap�� */
    bool ParseFile(const char* pcXmlFile);

    /** ����XML�ļ���һ��multimap�� */
    bool ParseFile(const std::string& strXmlFile);
    bool ParseXMLString(const std::string& strContent);

    /** ������뵽��XML�ļ�������ʱ��*/
    std::string ToString();

    /** �õ�������������ӽڵ�ĸ��� */
    int  GetAllSubElementNum(const std::string& strKey);

    /** �õ��������ֱ���ӽڵ� */
    int  GetDirectSubElement(const std::string& strKey,
                             std::set<std::string>& sSubElement);

    /** �õ��ַ������͵��������ֵ */
    bool GetValue(const std::string& strKey, std::string& strValue) const;

    /** �õ��������͵��������ֵ*/
    bool GetValue(const std::string& strKey, int& value) const;
    bool GetValue(const std::string& strKey, unsigned int& value) const;
    bool GetValue(const std::string& strKey, short& value) const;
    bool GetValue(const std::string& strKey, unsigned short& value) const;
    bool GetValue(const std::string& strKey, long& value) const;
    bool GetValue(const std::string& strKey, unsigned long& value) const;
    bool GetValue(const std::string& strKey, long long& value) const;
    bool GetValue(const std::string& strKey, unsigned long long& value) const;

    /** �õ�ĳ���������͵��������ֵ*/
    bool GetValue(const std::string& strKey, bool& value) const;

    /** �õ�ĳ���������͵��������ֵ*/
    bool GetValue(const std::string& strKey, float& value) const;
    bool GetValue(const std::string& strKey, double& value) const;

    /** �õ�����ַ������͵��������ֵ*/
    bool GetMultiValue(const std::string& strKey, std::vector<std::string>& vStrValue) const;

    /** �õ�������͵��������ֵ*/
    bool GetMultiValue(const std::string& strKey, std::vector<short>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<unsigned short>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<int>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<unsigned int>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<long>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<unsigned long>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<long long>& values) const;
    bool GetMultiValue(const std::string& strKey, std::vector<unsigned long long>& values) const;

    /** �õ����bool�͵��������ֵ*/
    bool GetMultiValue(const std::string& strKey, std::vector<bool>& values) const;

    /** �õ���������͵��������ֵ*/
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
    /* ����XML�Ĺ������õ��Ķ�ջ */
    void CreateStack(void **TheStack);

    /* ��һ��Ԫ��ѹ�뵽ջ�� */
    void PushStack(void **TheStack, void *data);

    /* ��ջ��ȡ��һ��Ԫ�� */
    void *PopStack(void **TheStack);

    /* �鿴ջ�����Ǹ�Ԫ������ȡ�� */
    void *PeekStack(void **TheStack);

    /* ���ջ�������� */
    void ClearStack(void **TheStack);

    /* XML �������� */
    struct XMLNode *ParseXML(const char *buffer, size_t offset, size_t length);

    /* ���XML�ļ�����Ч�� */
    int ProcessXMLNodeList(struct XMLNode *nodeList);

    /* �ͷ�XML������е���Դ */
    void DestructXMLNodeList(struct XMLNode *node);

    /* ����һ���յ�hash�� */
    void* InitHashTree();

    /* �ͷ�hash����Դ */
    void DestroyHashTree(void *tree);

    /* ��һ���ַ�������Ϊһ��token������ */
    struct parser_result* ParseString(
        const char* buffer, size_t offset, size_t length,
        const char* Delimiter, size_t DelimiterLength);

    /* һ���ڴ�ֵ�ıȽ�
     * ���buffer��offset��ʼ���ڴ��Ƿ�ʹ�Delimiter��ʼ�ģ�
     * ����ΪDelimiterLength���ڴ�ֵ��ͬ����ͬ�ͷ���0
     */
    bool IsDelimiter(
        const char* buffer, size_t offset, size_t buffersize,
        const char* Delimiter, size_t DelimiterLength);

    /* �ͷ�ParseString�������token�������Դ */
    void DestructParserResults(struct parser_result *result);

    /* �õ�XML�ַ����е�һ��tag */
    static std::string GetFirstTag(const std::string& strTag);

    /* �õ�XML�ַ�����ĳ��tag��value */
    static std::string GetTagValue(const std::string& strTag);

    /* ��XML��һ��·������Ϊһ���ַ��� */
    std::string GetTagPath(const std::vector<std::string>& vTags) const;

    /* ��XML�ļ��еĸ��ڵ��·���Ͷ�Ӧ��ֵ�ŵ�һ��map�� */
    int GetAllTags(struct XMLNode *nodeList, std::multimap<std::string, std::string>& mXmlTags);

    /* ��XML�ļ�����Ԥ�������ǰ�"&<>'""�滻�� */
    std::string PreDeal(const std::string& strRawFile);

    /* ��"&<>'""ת�����ַ�����ԭ */
    static std::string UnTransfer(std::string& strAfter);

private:
    /* ���е��������ֵ*/
    std::multimap<std::string, std::string> m_mConfig;

    /* ������֮��ֽ��ַ���*/
    std::string m_strSeparator;
};

#endif // COMMON_CONFOG_XML_XML_CONFIGER_HPP
