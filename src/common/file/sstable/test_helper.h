#ifndef SSTABLE_TEST_HELPER_H_
#define SSTABLE_TEST_HELPER_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <set>

#include "common/system/memory/unaligned.hpp"
#include "common/base/stdint.h"
#include "common/base/string/string_algorithm.hpp"

namespace sstable
{
struct TestString
{
    uint32_t len;
    char data[0];
};
typedef TestString key_t;
typedef TestString value_t;

struct TestData
{
    key_t* key;
    value_t* value;
};
typedef TestData data_t;

struct KeyCompare
{
    bool operator()(const data_t& left, const data_t& right)const
    {
        int cmp = CompareByteString(left.key->data, left.key->len, right.key->data, right.key->len);
        return cmp <= 0;
    }
};

typedef std::multiset<data_t, KeyCompare>    test_set_t; // 数据容器
typedef test_set_t::iterator            test_set_it; // 排序数据容器

class TestHelper
{
public:
    TestHelper(void);
public:
    ~TestHelper(void);

public:
    /// @brief Init  init and generate the origin data
    /// @param filename the origin url key file
    /// @param row_num read row number
    /// @param record_num generator origin record num
    /// @param seed random seed
    /// @return 0 if success, -1 if failure
    int  Init(const char *filename, uint32_t row_num,
            uint32_t record_num, unsigned int seed,
            uint32_t key_fix_len = 0, uint32_t value_fix_len = 0);

    void UnInit(void);

    inline test_set_t& GetTestData()
    {
        return m_data;
    }

    void Print();

private:

    int ReadUrlsFromFile(const char *filename, uint32_t row_num);

private:
    uint32_t                    m_min_url_len;
    uint32_t                    m_buffer_len;
    char*                       m_buffer;

    TestString**                m_urls;

public:
    test_set_t                  m_data;
};

} // namespace sstable

#endif // SSTABLE_TEST_HELPER_H_
