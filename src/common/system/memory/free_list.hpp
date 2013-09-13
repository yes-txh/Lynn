#ifndef COMMON_SYSTEM_MEMORY_FREE_LIST_HPP
#define COMMON_SYSTEM_MEMORY_FREE_LIST_HPP

#include <stddef.h>
#include <stdio.h>
#include <new>

/// detect whether a type has its own operator delete overloading
template <typename T>
struct HasOperatorDelete
{
private:
    typedef int yes;
    typedef char no;

    template <typename C>
    static yes test(char (*)[sizeof(&C::operator delete)]);

    template <typename C> static no test(...);
public:
    static const bool value = sizeof(test<T>(0)) == sizeof(yes);
};

/// for types without operator delete
template <typename T, bool>
struct CallOperatorDelete
{
    static void Do(void* p)
    {
        ::operator delete(p);
        printf("::operator delete\n");
    }
};

/// for types with operator delete overloading
template <typename T>
struct CallOperatorDelete<T, true>
{
    static void Do(void* p)
    {
        T::operator delete(p);
        printf("T::operator delete\n");
    }
};

template <typename T>
void OperatorDelete(void* p)
{
    CallOperatorDelete<T, HasOperatorDelete<T>::value>::Do(p);
}

/// free list, hold raw memory cache
template <typename T>
class FreeList
{
private:
    struct Node
    {
        Node* next;
    };

    // to chain nodes into single linked list,
    // sizeof(T) must not less than a pointer
    typedef char static_assert_size[sizeof(T) >= sizeof(Node) ? 1 : -1];
public:
    FreeList(): m_head(NULL) {}

    ~FreeList()
    {
        Clear();
    }

    /// allocate memory from free list, lower level interface
    T* Allocate()
    {
        if (m_head)
        {
            T* head = m_head;
            m_head = m_head->next;
            return head;
        }
        return NULL;
    }

    /// release memory into the free list, lower level interface
    void Release(T* p)
    {
        Node* node = reinterpret_cast<Node*>(p);
        node->next = m_head;
        m_head = node;
    }


    /// allocate memory and construct new object
    T* New()
    {
        T* p = Allocate();
        if (p)
        {
            try
            {
                new(p) T();
            }
            catch (...)
            {
                Release(p);
                throw;
            }
        }
        return p;
    }

    /// allocate memory and construct new object
    template <typename A1>
    T* New(A1 a1)
    {
        T* p = Allocate();
        if (p)
        {
            try
            {
                new(p) T(a1);
            }
            catch (...)
            {
                Release(p);
                throw;
            }
        }
        return p;
    }

    /// allocate memory and construct new object
    template <typename A1, typename A2>
    T* New(A1 a1, A2 a2)
    {
        T* p = Allocate();
        if (p)
        {
            try
            {
                new(p) T(a1, a2);
            }
            catch (...)
            {
                Release(p);
                throw;
            }
        }
        return p;
    }

    /// allocate memory and construct new object
    template <typename A1, typename A2, typename A3>
    T* New(A1 a1, A2 a2, A3 a3)
    {
        T* p = Allocate();
        if (p)
        {
            try
            {
                new(p) T(a1, a2, a3);
            }
            catch (...)
            {
                Release(p);
                throw;
            }
        }
        return p;
    }

    /// destroy object and release memory into the free list
    void Delete(T* p)
    {
        if (p)
        {
            p->~T();
            Release(p);
        }
    }

    /// @param freer 释放函数
    void Clear(void (*freer)(void* p) = &OperatorDelete<T>)
    {
        Node* p = m_head;
        while (p)
        {
            void* q = p;
            p = p->next;
            if (freer)
                freer(q);
        }
        m_head = NULL;
    }
private:
    Node* m_head;
};

#endif // COMMON_SYSTEM_MEMORY_FREE_LIST_HPP

