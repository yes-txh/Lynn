#if !defined(_XFS_METASERVER_HASH_TABLE_H_)
#define _XFS_METASERVER_HASH_TABLE_H_
#include "xfs/baselib/svrpublib/server_publib.h"

//链表有序
template<typename T>
class HashTable
{
public:
    HashTable();
    ~HashTable();

    //系统初始化
    bool Init(uint32_t hash_size = 1048576); 

    //系统反初始化
    void Uninit();

    //查找key
    T* Find(uint64_t  key);

    //删除key,返回key对应的元素
    bool Del(uint64_t key);

    //增加元素t
    bool Add(T*& t);

    uint32_t GetValidCount()
    {
        return m_valid_size;
    }

    bool IsPrime(uint32_t num);

	uint32_t GetMaxListLen() { return m_max_list_len;}
private:

    template<typename N>
    struct TagNode
    {
		uint32_t length;
        N* t;
        TagNode<N> *next;
    };

	uint32_t m_max_list_len;
	
    TagNode<T> **m_hash_nodes;       //hash桶
    uint32_t m_valid_size;          //存储的有效元素数
    uint32_t m_hash_size;           //hash桶尺寸
    bool     m_is_init;             //是否初始化
    CXThreadMutex m_mutex;
};

template<typename T>
HashTable<T>::HashTable():
    m_hash_nodes(0),
    m_valid_size(0),
    m_hash_size(0),
    m_is_init(0)
{
	m_max_list_len = 0;
}

template<typename T>
HashTable<T>::~HashTable()
{
    Uninit();
}

template<typename T>
bool HashTable<T>::Init(uint32_t hash_size)
{
    CXThreadAutoLock locker(&m_mutex);
    if (m_is_init) return false;

    for(uint32_t i = hash_size;;i++)
    {
        if(IsPrime(hash_size+i))
        {
            m_hash_size = hash_size + i;
            break;
        }
    }

    m_hash_nodes = (TagNode<T>**)mempool_NEW_BYTES(sizeof(TagNode<T>*) * m_hash_size);
    if (!m_hash_nodes)
        return false;

    memset(m_hash_nodes, 0, sizeof(TagNode<T> *) * m_hash_size);
    m_is_init = true;
    return true;
}

template<typename T>
void HashTable<T>::Uninit()
{
    CXThreadAutoLock locker(&m_mutex);

    if (!m_is_init)
        return;

    for (uint32_t i = 0; i < m_hash_size; i++)
    {
        TagNode<T> * t = m_hash_nodes[i];

        while (t)
        {
            TagNode<T> * tmp = t;
            t = t->next;
            mempool_DELETE(tmp->t);
            mempool_DELETE(tmp);
        }

        m_hash_nodes[i] = NULL;
    }

    mempool_DELETE(m_hash_nodes);
    m_hash_nodes = NULL;
    m_valid_size = 0;
    m_is_init = false;
}

template<typename T>
bool HashTable<T>::Add(T*& t)
{
    CXThreadAutoLock locker(&m_mutex);

    if (!m_is_init)
        return false;

    uint64_t key = t->GetKey();
    uint32_t hash_pos = uint32_t(key % m_hash_size);

    if (m_hash_nodes[hash_pos] == NULL)
    {
        //链表为空
        TagNode<T>* node = mempool_NEW(TagNode<T>);
        node->t = t;
		node->length=1;
        node->next = NULL;
        m_hash_nodes[hash_pos] = node;
        m_valid_size++;

        t = NULL;
        return true;
    }

    TagNode<T>* node = m_hash_nodes[hash_pos];
    TagNode<T>* pre  = NULL;

    while (node)
    {
        if (node->t->GetKey() == key)
        {
            //重复key
            return false;
        }

        if (node->t->GetKey() > key)
        {
            //没有查找到元素
            break;
        }

        pre = node;
        node = node->next;
    }

    TagNode<T>* tmp_node = mempool_NEW(TagNode<T>);

    if (tmp_node == NULL)
        return false;

    tmp_node->t = t;

    if (pre == NULL)
    {
		tmp_node->length = m_hash_nodes[hash_pos]->length;
        tmp_node->next = m_hash_nodes[hash_pos];
        m_hash_nodes[hash_pos] = tmp_node;
    }
    else
    {
        pre->next = tmp_node;
        tmp_node->next = node;
    }

	m_hash_nodes[hash_pos]->length++;
    m_valid_size++;

	if( m_max_list_len < m_hash_nodes[hash_pos]->length)
		m_max_list_len = m_hash_nodes[hash_pos]->length;

    t = NULL;
    return true;
}

//删除key,返回key对应的元素
template<typename T>
T* HashTable<T>::Find(uint64_t key)
{
    CXThreadAutoLock locker(&m_mutex);

    if (!m_is_init)
        return false;

    uint32_t hash_pos = uint32_t(key % m_hash_size);
    TagNode<T>* node = m_hash_nodes[hash_pos];

    while (node)
    {
        if (node->t->GetKey() == key)
            return node->t;

        if (node->t->GetKey() > key)
            break;

        node = node->next;
    }

    return NULL;
}


//删除key,返回key对应的元素
template<typename T>
bool HashTable<T>::Del(uint64_t key)
{
    CXThreadAutoLock locker(&m_mutex);

    if (!m_is_init)
        return false;

    uint32_t hash_pos = uint32_t(key % m_hash_size);
    TagNode<T>* node = m_hash_nodes[hash_pos];
    TagNode<T>* pre = NULL;

    while (node)
    {
        if (node->t->GetKey() == key)
        {

            //删除
            if (pre)
            {
                pre->next = node->next;
            }
            else
            {
                m_hash_nodes[hash_pos] = node->next;
            }

            mempool_DELETE(node->t);
            mempool_DELETE(node);
            m_valid_size--;

            return true;
        }

        if (node->t->GetKey() > key)
            break;

        pre = node;
        node = node->next;
    }

    return false;
}

template<typename T>
bool HashTable<T>::IsPrime(uint32_t num)
{
    for(uint32_t i=2;i<=num/2;i++)  
    {
        if(num%i==0)   
            return false;  
    }
    return true;
}


#endif //_XFS_METASERVER_HASH_TABLE_H_





