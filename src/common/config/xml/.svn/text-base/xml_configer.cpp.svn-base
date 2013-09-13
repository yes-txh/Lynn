/*
 * XML配置文件的解析，完全平台无关
 * @author jerryzhang@tencent.com
 * @since  2006.04.20
 */

#include "common/config/xml/xml_configer.hpp"
#include <assert.h>
#include <math.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <fstream>

#include "common/base/string/string_algorithm.hpp"

namespace {
/** 读文件 */
std::string ReadTextFile(const char *pcFileName)
{
    std::ifstream fs;
    fs.open(pcFileName, std::ios_base::in);

    if (fs.fail())
    {
        return "";
    }

    std::string strDocument, strLine;

    while (getline(fs, strLine))
        strDocument += strLine + "\r\n";

    return strDocument;
}
}

/** 读文件 */
std::string ReadTextFile(const std::string& strFileName)
{
    return ReadTextFile(strFileName.c_str());
}


/* 解析XML文件到一个multimap中 */
bool XMLConfiger::ParseFile(const std::string& strXmlFile)
{
    return ParseFile(strXmlFile.c_str());
}

/* 解析XML文件到一个multimap中 */
bool XMLConfiger::ParseFile(const char* pcXmlFile)
{
    // 读入文件
    std::string strContent = ReadTextFile(pcXmlFile);
    if (strContent.empty())
        return false;
    return ParseXMLString(strContent);
}

/* 对XML文件进行预处理，就是把"&<>'""替换掉 */
std::string XMLConfiger::PreDeal(const std::string& strRawFile)
{
    size_t dwBeginPos;
    size_t dwEndPos;
    std::string strRawXML(strRawFile);
    std::string strRet;
    std::string strTmp;
    do
    {
        dwBeginPos = strRawXML.find("\"");
        if (dwBeginPos != std::string::npos)
        {
            dwEndPos = strRawXML.find("\"", dwBeginPos+1);
            if (dwEndPos != std::string::npos)
            {
                strRet += strRawXML.substr(0, dwBeginPos+1);

                // 把引号中的数据做一些转义
                strTmp = strRawXML.substr(dwBeginPos+1, dwEndPos - dwBeginPos -1);
                strTmp = ReplaceString(strTmp, "&", "*amp;");
                strTmp = ReplaceString(strTmp, "<", "*lt;");
                strTmp = ReplaceString(strTmp, ">", "*gt;");
                strTmp = ReplaceString(strTmp, "<", "*lt;");
                strTmp = ReplaceString(strTmp, "\"", "*quot;");

                strRet += strTmp + std::string("\"");

                strRawXML = strRawXML.substr(dwEndPos+1);
                //strRawXML.erase(0, dwEndPos);
            }
        } // end if
    }
    while (dwBeginPos != std::string::npos && dwEndPos != std::string::npos);

    if (!strRawXML.empty())
        strRet += strRawXML;
    return strRet;
}

/* 把"&<>'""转义后的字符串还原 */
std::string XMLConfiger::UnTransfer(std::string& strAfter)
{
    std::string strTmp(strAfter);

    strTmp = ReplaceString(strTmp, "*amp;", "&");
    strTmp = ReplaceString(strTmp, "*lt;",  "<");
    strTmp = ReplaceString(strTmp, "*gt;",  ">");
    strTmp = ReplaceString(strTmp, "*lt;",  "<");
    strTmp = ReplaceString(strTmp, "*quot;","\"");

    strTmp = ReplaceString(strTmp, "&amp;", "&");
    strTmp = ReplaceString(strTmp, "&lt;",  "<");
    strTmp = ReplaceString(strTmp, "&gt;",  ">");
    strTmp = ReplaceString(strTmp, "&lt;",  "<");
    strTmp = ReplaceString(strTmp, "&quot;","\"");

    return strTmp;
}

/* 得到配置项的所有子节点的个数 */
int  XMLConfiger::GetAllSubElementNum(const std::string& strKey)
{
    std::multimap<std::string, std::string>::iterator it;
    int  dwSubElementNum = 0;
    int  dwKeyLen = (int)strKey.size();

    for (it = m_mConfig.begin(); it != m_mConfig.end(); it++)
    {
        if (it->first.substr(0, dwKeyLen) == strKey)
            dwSubElementNum++;
    }
    return dwSubElementNum;
}

/* 得到配置项的直接子节点 */
int  XMLConfiger::GetDirectSubElement(const std::string& strKey,
                                      std::set<std::string>& sSubElement)
{
    std::multimap<std::string, std::string>::iterator it;
    int  dwSubElementNum = 0;
    int  dwKeyLen = (int)strKey.size();
    int  dwPos;
    std::string strTmp;
    std::set<std::string>::iterator itSet;

    for (it = m_mConfig.begin(); it != m_mConfig.end(); it++)
    {
        if (it->first.substr(0, dwKeyLen) == strKey)
        {
            dwPos = (int)it->first.find(m_strSeparator, dwKeyLen+2);
            if (-1 != dwPos)
            {
                strTmp = it->first.substr(0, dwPos);
                itSet = sSubElement.find(strTmp);
                if (itSet == sSubElement.end())
                {
                    sSubElement.insert(strTmp);
                    dwSubElementNum++;
                }
            }
        }
    }
    return dwSubElementNum;
}

namespace
{

void StringToNumber(const char* str, short &value, char** endpos, int base = 0)
{
    value = (short)strtol(str, endpos, base);
}
void StringToNumber(const char* str, unsigned short &value, char** endpos, int base = 0)
{
    value = (unsigned short)strtoul(str, endpos, base);
}
void StringToNumber(const char* str, int &value, char** endpos, int base = 0)
{
    value = (int)strtol(str, endpos, base);
}
void StringToNumber(const char* str, unsigned int &value, char** endpos, int base = 0)
{
    value = (unsigned int)strtoul(str, endpos, base);
}
void StringToNumber(const char* str, long &value, char** endpos, int base = 0)
{
    value = strtol(str, endpos, base);
}
void StringToNumber(const char* str, unsigned long &value, char** endpos, int base = 0)
{
    value = strtoul(str, endpos, base);
}
void StringToNumber(const char* str, long long &value, char** endpos, int base = 0)
{
#ifdef _MSC_VER
    value = _strtoi64(str, endpos, base);
#else
    value = strtoll(str, endpos, base);
#endif
}

void StringToNumber(const char* str, unsigned long long &value, char** endpos, int base = 0)
{
#ifdef _MSC_VER
    value = _strtoui64(str, endpos, base);
#else
    value = strtoull(str, endpos, base);
#endif
}

void StringToNumber(const char* str, float &value, char** endpos)
{
#ifdef _MSC_VER
    value = (float)strtod(str, endpos);
#else
    value = strtof(str, endpos);
#endif
}

void StringToNumber(const char* str, double &value, char** endpos)
{
    value = strtod(str, endpos);
}

template <typename Type>
void StringToNumber(const char* str, Type& value, char** endpos)
{
    StringToNumber(str, value, &endpos, 0);
}

template <typename Type>
bool StringToNumber(const std::string& str, Type& value, int base)
{
    char* endpos;
    StringToNumber(str.c_str(), value, &endpos, base);
    return *endpos == '\0';
}

template <typename Type>
bool StringToNumber(const std::string& str, Type& value)
{
    char* endpos;
    StringToNumber(str.c_str(), value, &endpos);
    return *endpos == '\0';
}

template <typename Type>
bool GetNumberValue(const XMLConfiger& configer, const std::string& strKey, Type& value)
{
    std::string strValue;
    if (configer.GetValue(strKey, strValue))
    {
        return StringToNumber(strValue, value);
    }
    return false;
}

bool StringToBool(const std::string& str, bool& value)
{
    if (str == "true")
        value = true;
    else if (str == "false")
        value = false;
    else
        return false;
    return true;
}

/* 得到多个整型的配置项的值*/
template <typename Type>
bool GetMultiConfigValue(const std::multimap<std::string, std::string> config, const std::string& strKey, std::vector<Type>& values)
{
    std::multimap<std::string, std::string>::const_iterator it;
    bool bRet = false;

    values.clear();

    for(it = config.begin(); it != config.end(); it++)
    {
        if (it->first == strKey)
        {
            Type value;
            if (StringToNumber(it->second.c_str(), value))
            {
                values.push_back(value);
                bRet = true;
            }
        }
    }
    return bRet;
}

}

/* 得到字符串类型的配置项的值 */
bool XMLConfiger::GetValue(const std::string& strKey, std::string& strValue) const
{
    std::multimap<std::string, std::string>::const_iterator it;
    it = m_mConfig.find(strKey);
    if (it == m_mConfig.end())
    {
        return false;
    }
    else
    {
        strValue = it->second;
        strValue = UnTransfer(strValue);
        return true;
    }
}

/* 得到布尔类型的配置项的值*/
bool XMLConfiger::GetValue(const std::string& strKey, bool& value) const
{
    std::string strValue;
    bool bRet = GetValue(strKey, strValue);
    if (false == bRet)
    {
        return false;
    }
    else
    {
        return StringToBool(strValue, value);
    }
}

/* 得到整数类型的配置项的值*/
bool XMLConfiger::GetValue(const std::string& strKey, int& value) const
{
    return GetNumberValue(*this, strKey, value);
}
bool XMLConfiger::GetValue(const std::string& strKey, unsigned int& value) const
{
    return GetNumberValue(*this, strKey, value);
}
bool XMLConfiger::GetValue(const std::string& strKey, short& value) const
{
    return GetNumberValue(*this, strKey, value);
}
bool XMLConfiger::GetValue(const std::string& strKey, unsigned short& value) const
{
    return GetNumberValue(*this, strKey, value);
}
bool XMLConfiger::GetValue(const std::string& strKey, long& value) const
{
    return GetNumberValue(*this, strKey, value);
}
bool XMLConfiger::GetValue(const std::string& strKey, unsigned long& value) const
{
    return GetNumberValue(*this, strKey, value);
}
bool XMLConfiger::GetValue(const std::string& strKey, long long& value) const
{
    return GetNumberValue(*this, strKey, value);
}
bool XMLConfiger::GetValue(const std::string& strKey, unsigned long long& value) const
{
    return GetNumberValue(*this, strKey, value);
}

/* 得到浮点类型的配置项的值*/
bool XMLConfiger::GetValue(const std::string& strKey, float& value) const
{
    return GetNumberValue(*this, strKey, value);
}

bool XMLConfiger::GetValue(const std::string& strKey, double& value) const
{
    return GetNumberValue(*this, strKey, value);
}

/* 得到多个字符串类型的配置项的值*/
bool XMLConfiger::GetMultiValue(const std::string& strKey, std::vector<std::string>& vStrValue) const
{
    std::multimap<std::string, std::string>::const_iterator it;
    bool bRet = false;

    vStrValue.clear();

    for(it = m_mConfig.begin(); it != m_mConfig.end(); it++)
    {
        if (it->first == strKey)
        {
            vStrValue.push_back(it->second);
            bRet = true;
        }
    }
    return bRet;
}

/* 得到多个bool型的配置项的值*/
bool XMLConfiger::GetMultiValue(const std::string& strKey, std::vector<bool>& values) const
{
    std::multimap<std::string, std::string>::const_iterator it;
    bool bRet = false;

    values.clear();

    for(it = m_mConfig.begin(); it != m_mConfig.end(); it++)
    {
        if (it->first == strKey)
        {
            bool value;
            if (StringToBool(it->second, value))
            {
                values.push_back(value);
                bRet = true;
            }
        }
    }
    return bRet;
}

/* 得到多个整型的配置项的值*/
bool XMLConfiger::GetMultiValue(const std::string& strKey, std::vector<short>& values) const
{
    return GetMultiConfigValue(m_mConfig, strKey, values);
}

bool XMLConfiger::GetMultiValue(const std::string& strKey, std::vector<unsigned short>& values) const
{
    return GetMultiConfigValue(m_mConfig, strKey, values);
}

bool XMLConfiger::GetMultiValue(const std::string& strKey, std::vector<int>& values) const
{
    return GetMultiConfigValue(m_mConfig, strKey, values);
}

bool XMLConfiger::GetMultiValue(const std::string& strKey, std::vector<unsigned int>& values) const
{
    return GetMultiConfigValue(m_mConfig, strKey, values);
}

bool XMLConfiger::GetMultiValue(const std::string& strKey, std::vector<long>& values) const
{
    return GetMultiConfigValue(m_mConfig, strKey, values);
}

bool XMLConfiger::GetMultiValue(const std::string& strKey, std::vector<unsigned long>& values) const
{
    return GetMultiConfigValue(m_mConfig, strKey, values);
}

bool XMLConfiger::GetMultiValue(const std::string& strKey, std::vector<long long>& values) const
{
    return GetMultiConfigValue(m_mConfig, strKey, values);
}

bool XMLConfiger::GetMultiValue(const std::string& strKey, std::vector<unsigned long long>& values) const
{
    return GetMultiConfigValue(m_mConfig, strKey, values);
}

/* 得到多个浮点型的配置项的值*/
bool XMLConfiger::GetMultiValue(const std::string& strKey, std::vector<float>& values) const
{
    return GetMultiConfigValue(m_mConfig, strKey, values);
}

bool XMLConfiger::GetMultiValue(const std::string& strKey, std::vector<double>& values) const
{
    return GetMultiConfigValue(m_mConfig, strKey, values);
}

/* 释放XML结点树中的资源 */
void XMLConfiger::DestructXMLNodeList(struct XMLNode *node)
{
    struct XMLNode *temp;
    while(node!=NULL)
    {
        temp = node->Next;
        if(node->Reserved2!=NULL)
        {
            DestroyHashTree(node->Reserved2);
        }
        free(node);
        node = temp;
    }
}

/* 检查XML文件的有效性 */
int XMLConfiger::ProcessXMLNodeList(struct XMLNode *nodeList)
{
    int RetVal = 0;
    struct XMLNode *current = nodeList;
    struct XMLNode *temp;
    void *TagStack;

    CreateStack(&TagStack);

    while(current!=NULL)
    {
        if(current->StartTag!=0)
        {
            // 开始标记，压栈
            current->Parent = (struct XMLNode *)PeekStack(&TagStack);
            PushStack(&TagStack,current);
        }
        else
        {
            // 遇到结束标记，检查是否是期望的
            temp = (struct XMLNode*)PopStack(&TagStack);
            if(temp!=NULL)
            {
                //
                // Checking to see if this EndElement is correct in scope
                //
                if(temp->NameLength==current->NameLength && memcmp(temp->Name,current->Name,current->NameLength)==0)
                {
                    //
                    // Now that we know this EndElement is correct, set the Peer
                    // pointers of the previous sibling
                    //
                    if(current->Next!=NULL)
                    {
                        if(current->Next->StartTag!=0)
                        {
                            temp->Peer = current->Next;
                        }
                    }
                    temp->ClosingTag = current;
                    current->StartingTag = temp;
                }
                else
                {
                    // 结束标记的顺序不对
                    RetVal = -2;
                    break;
                }
            }
            else
            {
                // 非法的结束标记
                RetVal = -1;
                break;
            }
        }
        current = current->Next;
    }

    // 如果还有元属在栈中, 说明有开始tag没有对应的结束
    if(TagStack!=NULL)
    {
        // 不完整的XML
        RetVal = -3;
        ClearStack(&TagStack);
    }

    return(RetVal);
}

/* 解析一个XML字符串，任何字符串都是没有复制的，都是通过指针访问的 */
struct XMLNode *XMLConfiger::ParseXML(const char *buffer, size_t offset, size_t length)
{
    struct parser_result *xml;
    struct parser_result_field *field;
    struct parser_result *temp;
    struct parser_result *temp2;
    struct parser_result *temp3;
    char* TagName;
    int TagNameLength;
    int StartTag;
    int EmptyTag;
    int i;
    char *TempC;

    struct XMLNode *RetVal = NULL;
    struct XMLNode *current = NULL;
    struct XMLNode *x = NULL;

    char *NSTag;
    int NSTagLength;

    while((buffer[offset] != '<') && (length > 0))
    {
        ++offset;
        --length;
    }

    if(length==0)
    {
        RetVal = (struct XMLNode*)malloc(sizeof(struct XMLNode));
        memset(RetVal,0,sizeof(struct XMLNode));
        return(RetVal);
    }

    //
    // All XML Elements start with a '<' character. If we delineate the string with
    // this character, we can go from there.
    //
    xml = ParseString(buffer,offset,length,"<",1);
    field = xml->FirstResult;
    while(field!=NULL)
    {
        //
        // Ignore the XML declarator
        //
        if(field->datalength !=0 && memcmp(field->data,"?",1)!=0)
        {
            EmptyTag = 0;
            if(memcmp(field->data,"/",1)==0)
            {
                //
                // The first character after the '<' was a '/', so we know this is the
                // EndElement
                //
                StartTag = 0;
                field->data = field->data+1;
                field->datalength -= 1;
                //
                // If we look for the '>' we can find the end of this element
                //
                temp2 = ParseString(field->data,0,field->datalength,">",1);
            }
            else
            {
                //
                // The first character after the '<' was not a '/' so we know this is a
                // StartElement
                //
                StartTag = -1;
                //
                // If we look for the '>' we can find the end of this element
                //
                temp2 = ParseString(field->data,0,field->datalength,">",1);
                if(temp2->FirstResult->datalength>0 && temp2->FirstResult->data[temp2->FirstResult->datalength-1]=='/')
                {
                    //
                    // If this element ended with a '/' this is an EmptyElement
                    //
                    EmptyTag = -1;
                }
            }
            //
            // Parsing on the ' ', we can isolate the Element name from the attributes.
            // The first token, being the element name
            //
            temp = ParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength," ",1);
            //
            // Now that we have the token that contains the element name, we need to parse on the ":"
            // because we need to figure out what the namespace qualifiers are
            //
            temp3 = ParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
            if(temp3->NumResults==1)
            {
                //
                // If there is only one token, there was no namespace prefix.
                // The whole token is the attribute name
                //
                NSTag = NULL;
                NSTagLength = 0;
                TagName = temp3->FirstResult->data;
                TagNameLength = temp3->FirstResult->datalength;
            }
            else
            {
                //
                // The first token is the namespace prefix, the second is the attribute name
                //
                NSTag = temp3->FirstResult->data;
                NSTagLength = temp3->FirstResult->datalength;
                TagName = temp3->FirstResult->NextResult->data;
                TagNameLength = temp3->FirstResult->NextResult->datalength;
            }
            DestructParserResults(temp3);

            //
            // Iterate through the tag name, to figure out what the exact length is, as
            // well as check to see if its an empty element
            //
            for(i=0;i<TagNameLength;++i)
            {
                if( (TagName[i]==' ')||(TagName[i]=='/')||(TagName[i]=='>')||(TagName[i]=='\t')||(TagName[i]=='\r')||(TagName[i]=='\n') )
                {
                    if(i!=0)
                    {
                        if(TagName[i]=='/')
                        {
                            EmptyTag = -1;
                        }
                        TagNameLength = i;
                        break;
                    }
                }
            }

            if(TagNameLength!=0)
            {
                //
                // Instantiate a new XMLNode for this element
                //
                x = (struct XMLNode*)malloc(sizeof(struct XMLNode));
                memset(x,0,sizeof(struct XMLNode));
                x->Name = TagName;
                x->NameLength = TagNameLength;
                x->StartTag = StartTag;
                x->NSTag = NSTag;
                x->NSLength = NSTagLength;

                if(StartTag==0)
                {
                    //
                    // The Reserved field of StartElements point to te first character before
                    // the '<'.
                    //
                    x->Reserved = field->data;
                    do
                    {
                        TempC=(char*)x->Reserved;
                        TempC -= 1;
                        x->Reserved=TempC;

                        ///((char*)x)->Reserved -= 1;
                    }while(*((char*)x->Reserved)=='<');
                }
                else
                {
                    //
                    // The Reserved field of EndElements point to the end of the element
                    //
                    x->Reserved = temp2->LastResult->data;
                }

                if(RetVal==NULL)
                {
                    RetVal = x;
                }
                else
                {
                    current->Next = x;
                }
                current = x;
                if(EmptyTag!=0)
                {
                    //
                    // If this was an empty element, we need to create a bogus EndElement,
                    // just so the tree is consistent. No point in introducing unnecessary complexity
                    //
                    x = (struct XMLNode*)malloc(sizeof(struct XMLNode));
                    memset(x,0,sizeof(struct XMLNode));
                    x->Name = TagName;
                    x->NameLength = TagNameLength;
                    x->NSTag = NSTag;
                    x->NSLength = NSTagLength;

                    x->Reserved = current->Reserved;
                    current->EmptyTag = -1;
                    current->Next = x;
                    current = x;
                }
            }

            DestructParserResults(temp2);
            DestructParserResults(temp);
        }
        field = field->NextResult;
    }

    DestructParserResults(xml);
    return(RetVal);
}



/* 创建一个空的栈 */
void XMLConfiger::CreateStack(void **TheStack)
{
    *TheStack = NULL;
}

/* 把一个元属压入到栈中 */
void XMLConfiger::PushStack(void **TheStack, void *data)
{
    struct XMLStackNode *RetVal = (struct XMLStackNode*)malloc(sizeof(struct XMLStackNode));
    RetVal->Data = data;
    RetVal->Next = (struct XMLStackNode *)*TheStack;
    *TheStack = RetVal;
}

/* 从栈中取出一个元属 */
void *XMLConfiger::PopStack(void **TheStack)
{
    void *RetVal = NULL;
    void *Temp;
    if(*TheStack!=NULL)
    {
        RetVal = ((struct XMLStackNode*)*TheStack)->Data;
        Temp = *TheStack;
        *TheStack = ((struct XMLStackNode*)*TheStack)->Next;
        free(Temp);
    }
    return(RetVal);
}

/* 查看栈顶的那个元属，不取走 */
void *XMLConfiger::PeekStack(void **TheStack)
{
    void *RetVal = NULL;
    if(*TheStack!=NULL)
    {
        RetVal = ((struct XMLStackNode*)*TheStack)->Data;
    }
    return(RetVal);
}

/* 清除栈中所有项 */
void XMLConfiger::ClearStack(void **TheStack)
{
    void *Temp = *TheStack;
    do
    {
        PopStack(&Temp);
    }while(Temp!=NULL);
    *TheStack = NULL;
}

/* 释放hash树资源 */
void XMLConfiger::DestroyHashTree(void *tree)
{
    struct HashNode_Root *r = (struct HashNode_Root*)tree;
    struct XMLHashNode *c = r->Root;
    struct XMLHashNode *n;

    while(c!=NULL)
    {
        // 遍历每个结点，释放所有资源
        n = c->Next;
        free(c->KeyValue);
        free(c);
        c = n;
    }
    free(r);
}

/* 创建一个空的hash树 */
void* XMLConfiger::InitHashTree()
{
    struct HashNode_Root *Root = (struct  HashNode_Root*)malloc(sizeof(struct HashNode_Root));
    struct XMLHashNode *RetVal = (struct XMLHashNode*)malloc(sizeof(struct XMLHashNode));
    memset(RetVal,0,sizeof(struct XMLHashNode));
    Root->Root = RetVal;
    return(Root);
}

/* 一段内存值的比较
 * 检查buffer从offset开始的内存是否和从Delimiter开始的，
 * 长度为DelimiterLength的内存值相同，不同就返回0
 */
bool XMLConfiger::IsDelimiter(
    const char* buffer, size_t offset, size_t buffersize,
    const char* Delimiter, size_t DelimiterLength
)
{
    if(DelimiterLength > buffersize)
        return false;

    for(size_t i = 0; i < DelimiterLength; ++i)
    {
        if(buffer[offset+i] != Delimiter[i])
        {
            return false;
        }
    }
    return true;
}

/* 把一个字符串解析为一个token的链表 */
struct parser_result* XMLConfiger::ParseString(
    const char* buffer, size_t offset, size_t length,
    const char* Delimiter, size_t DelimiterLength
)
{
    struct parser_result* RetVal = (struct parser_result*)malloc(sizeof(struct parser_result));
    size_t i=0;
    const char* Token = NULL;
    int TokenLength = 0;
    struct parser_result_field *p_resultfield;

    RetVal->FirstResult = NULL;
    RetVal->NumResults = 0;

    //
    // By default we will always return at least one token, which will be the
    // entire string if the delimiter is not found.
    //
    // Iterate through the string to find delimiters
    //
    Token = buffer + offset;
    for(i=offset;i<length;++i)
    {
        if(IsDelimiter(buffer,i,length,Delimiter,DelimiterLength))
        {
            //
            // We found a delimiter in the string
            //
            p_resultfield = (struct parser_result_field*)malloc(sizeof(struct parser_result_field));
            p_resultfield->data = (char*)Token;
            p_resultfield->datalength = TokenLength;
            p_resultfield->NextResult = NULL;
            if(RetVal->FirstResult != NULL)
            {
                RetVal->LastResult->NextResult = p_resultfield;
                RetVal->LastResult = p_resultfield;
            }
            else
            {
                RetVal->FirstResult = p_resultfield;
                RetVal->LastResult = p_resultfield;
            }

            //
            // After we populate the values, we advance the token to after the delimiter
            // to prep for the next token
            //
            ++RetVal->NumResults;
            i = i + DelimiterLength -1;
            Token = Token + TokenLength + DelimiterLength;
            TokenLength = 0;
        }
        else
        {
            // 还没有匹配的，所以增加记数
            ++TokenLength;
        }
    }

    //
    // Create a result for the last token, since it won't be caught in the above loop
    // because if there are no more delimiters, than the entire last portion of the string since the
    // last delimiter is the token
    //
    p_resultfield = (struct parser_result_field*)malloc(sizeof(struct parser_result_field));
    p_resultfield->data = (char*)Token;
    p_resultfield->datalength = TokenLength;
    p_resultfield->NextResult = NULL;
    if(RetVal->FirstResult != NULL)
    {
        RetVal->LastResult->NextResult = p_resultfield;
        RetVal->LastResult = p_resultfield;
    }
    else
    {
        RetVal->FirstResult = p_resultfield;
        RetVal->LastResult = p_resultfield;
    }
    ++RetVal->NumResults;

    return(RetVal);
}

/* 释放ParseString解析后的token链表的资源 */
void XMLConfiger::DestructParserResults(struct parser_result *result)
{
    struct parser_result_field *node = result->FirstResult;
    struct parser_result_field *temp;

    while(node!=NULL)
    {
        temp = node->NextResult;
        free(node);
        node = temp;
    }
    free(result);
}


/* 得到XML字符串中第一个tag */
std::string XMLConfiger::GetFirstTag(const std::string& strTag)
{
    //将XML格式的报文体分离
    size_t dwPos = (int)strTag.find(">");
    if (dwPos != std::string::npos)
    {
        return strTag.substr(0, dwPos);
    }

    return "";
}

/* 得到XML字符串中某个tag的value */
std::string XMLConfiger::GetTagValue(const std::string& strTag)
{
    //将XML格式的报文体分离
    size_t dwBegin = strTag.find(">");
    if (dwBegin != std::string::npos)
    {
        int dwEnd;
        dwEnd = (int)strTag.find("<");
        return strTag.substr(dwBegin+1, dwEnd - dwBegin -1);
    }
    else
    {
        return "";
    }
}

/* 把XML的一个路径生成为一个字符串 */
std::string XMLConfiger::GetTagPath(const std::vector<std::string>& vTags) const
{
    std::string str("");
    for (size_t t = 0; t < vTags.size(); t++)
    {
        str += vTags[t];
        if (t != vTags.size()-1)
        {
            str += m_strSeparator;
        }
    }
    return str;
}

/* 将XML文件中的个节点的路径和对应的值放到一个map中 */
int XMLConfiger::GetAllTags(struct XMLNode *nodeList, std::multimap<std::string, std::string>& mXmlTags)
{
    int RetVal = 0;
    struct XMLNode *current = nodeList;
    struct XMLNode *temp;
    void *TagStack;
    std::vector<std::string> vTagPath;

    CreateStack(&TagStack);

    while(current!=NULL)
    {
        if(current->StartTag!=0)
        {
            current->Parent = (struct XMLNode *)PeekStack(&TagStack);
            PushStack(&TagStack,current);

            // 这里需要处理tag中有值的情况
            std::string strTagName = GetFirstTag(current->Name);
            size_t dwPos = (int)strTagName.find("=");
            if (std::string::npos == dwPos)
            {
                StringTrim(&strTagName);
                if (strTagName.substr(strTagName.size()-1, 1) != "/")
                {
                    // 平常的情况
                    vTagPath.push_back(strTagName);
                }
            }
            else
            {
                dwPos = (int)strTagName.find(" ");
                std::string strName  = strTagName.substr(0, dwPos);
                std::string strValue = strTagName.substr(dwPos+1);
                bool bIsCloseTag = false;

                StringTrim(&strValue);

                // 如果是/结束则说明
                if (strValue.substr(strValue.size()-1, 1) == "/")
                {
                    bIsCloseTag = true;
                    strTagName = strName;
                    strValue = strValue.substr(0, strValue.size()-1);
                }

                if (!bIsCloseTag)
                    vTagPath.push_back(strName);

                std::string strTemp(strValue);

                while (strTemp.size() > 3)
                {
                    // 把属性项的名字取出
                    dwPos = (int)strTemp.find("=");
                    if (std::string::npos == dwPos)
                        break;
                    strName = strTemp.substr(0, dwPos);
                    StringTrim(&strName);

                    // 把属性项的值取出
                    size_t dwBegin = (int)strTemp.find('\"', dwPos+1);
                    if (dwBegin == std::string::npos)
                        break;

                    size_t dwEnd  = (int)strTemp.find('\"', dwBegin+1);
                    if (dwEnd == std::string::npos)
                        break;

                    strValue = strTemp.substr(dwBegin+1, dwEnd - dwBegin - 1);
                    StringTrim(&strValue);

                    // 得到剩下的部分
                    strTemp   = strTemp.substr(dwEnd+1);

                    // 把取到的结果插入集合
                    std::string strPath = GetTagPath(vTagPath);
                    if (bIsCloseTag)
                    {
                        strPath += m_strSeparator;
                        strPath += strTagName;
                    }
                    strPath += m_strSeparator;
                    strPath += strName;
                    mXmlTags.insert(make_pair(strPath, UnTransfer(strValue)));
                }
            } // end else
        }
        else
        {
            // 遇到结束标记
            temp = (struct XMLNode*)PopStack(&TagStack);
            if(temp!=NULL)
            {
                //
                // Checking to see if this EndElement is correct in scope
                //
                if(temp->NameLength==current->NameLength && memcmp(temp->Name,current->Name,current->NameLength)==0)
                {
                    if(current->Next!=NULL)
                    {
                        if(current->Next->StartTag!=0)
                        {
                            temp->Peer = current->Next;
                        }
                    }
                    temp->ClosingTag = current;
                    current->StartingTag = temp;

                    std::string strTempName = temp->Name;
                    size_t dwEnd = strTempName.find('>');
                    strTempName = strTempName.substr(0, dwEnd);
                    StringTrim(&strTempName);

                    if (strTempName[strTempName.length() - 1] != '/')
                    {
                        std::string strPath = GetTagPath(vTagPath);

                        std::string strValue = GetTagValue(temp->Name);
                        StringTrim(&strValue);
                        if (!strValue.empty())
                        {
                            strPath = GetTagPath(vTagPath);
                            mXmlTags.insert(make_pair(strPath, UnTransfer(strValue)));
                        }
                        vTagPath.pop_back();
                    } // end if
                    else if (std::string::npos == strTempName.find('='))
                    {
                        std::string strPath = GetTagPath(vTagPath);
                        strPath += m_strSeparator;
                        strPath += strTempName.substr(0, strTempName.size()-1);
                        mXmlTags.insert(make_pair(strPath, std::string("")));
                    }
                }
                else
                {
                    // 非法的结束标记顺序
                    RetVal = -2;
                    break;
                }
            }
            else
            {
                RetVal = -1;
                break;
            }
        }
        current = current->Next;
    }

    if(TagStack!=NULL)
    {
        RetVal = -3;
        ClearStack(&TagStack);
    }

    return(RetVal);
}
/** 删除html或xml格式的注释 <!-- --> */
static std::string StripXmlComments(const std::string& strRawFile)
{
    std::string strNewFile;
    strNewFile.reserve(strRawFile.size());

    const char *ptr = strRawFile.c_str();
    const char *end = ptr + strRawFile.length();

    bool bIsInsideComment = false;
    while(1)
    {
        if(!bIsInsideComment)
        {
            if(ptr  + 3 < end)
            {
                if(*ptr == '<' && *(ptr+1) == '!' && *(ptr+2) =='-' && *(ptr + 3) == '-' )
                {
                    bIsInsideComment = true;
                }
            }
        }
        else
        {
            if(ptr + 2 < end)
            {
                if(*ptr == '-' && *(ptr+1) == '-' && *(ptr+2) == '>' )
                {
                    bIsInsideComment = false;
                    ptr += 3;
                }
            }
        }
        if(ptr == end)
            break;
        if(!bIsInsideComment)
            strNewFile += *ptr;
        ptr++;
    }

    strNewFile.resize(strNewFile.size());

    return strNewFile;
}


bool XMLConfiger::ParseXMLString(const std::string& strContent)
{
    struct XMLNode* pRootNode;
    struct XMLNode* pTempNode;

    // 清上次结果
    m_mConfig.clear();

    // 删除注释
    std::string content = StripXmlComments(strContent);

    // 把保留字符替换为正常的转义字符串
    content = PreDeal(content);

    if (strContent.empty())
        return false;

    pRootNode = ParseXML(content.c_str(), 0, (int)content.size());
    if (NULL == pRootNode)
    {
        return false;
    }
    else
    {
        pTempNode = pRootNode;

        if(ProcessXMLNodeList(pRootNode) != 0)
        {
            DestructXMLNodeList(pRootNode);
            return false;
        }

        GetAllTags(pRootNode, m_mConfig);
        DestructXMLNodeList(pRootNode);
    }
    return true;
}
