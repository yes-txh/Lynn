#include "common/file/sstable/test_helper.h"

namespace sstable
{
TestHelper::TestHelper(void):
                m_min_url_len(0),
                m_buffer_len(0),
                m_buffer(NULL),
                m_urls(NULL)
{
}

TestHelper::~TestHelper(void)
{
    UnInit();
}

int TestHelper::Init(const char *filename, uint32_t row_num,
                     uint32_t record_num, unsigned int seed,
                     uint32_t key_fix_len, uint32_t value_fix_len)
{
    UnInit();

    // set pseudo random seed
    srand(seed);

    // malloc buffer
    m_buffer_len = 1 << 20;
    m_buffer = reinterpret_cast<char*>(malloc(m_buffer_len));

    // read urls from file
    if (ReadUrlsFromFile(filename, row_num << 1) < 0)
    {
        return -2;
    }

    assert(m_min_url_len >= key_fix_len);
    assert(m_min_url_len >= value_fix_len);
    // generate the random record
    for (uint32_t i = 0; i < record_num; i++)
    {
        TestData test_data;

        // random the key
        uint32_t key_index = rand() % row_num;
        test_data.key = m_urls[key_index];
        if (key_fix_len > 0)
        {
            test_data.key->len = key_fix_len;
        }

        // random the value
        uint32_t value_index = rand() % row_num + row_num;
        test_data.value = m_urls[value_index];
        if (value_fix_len)
        {
            test_data.value->len = value_fix_len;
        }
        m_data.insert(test_data);
    }

    return 0;
}

void TestHelper::UnInit()
{
    if (m_buffer != NULL)
    {
        free(m_buffer);
        m_buffer = NULL;
    }
}


int TestHelper::ReadUrlsFromFile(const char *filename, uint32_t row_num)
{
    // read the urls from origin data file
    FILE    *fp = fopen(filename, "r+b");
    if (fp == NULL)
    {
        return -1;
    }

    // the max size of url is 2048
    uint32_t     read_url_num = 0;
    char    read_url_buf[2048];

    char*  cur_position = reinterpret_cast<char*>((m_buffer + sizeof(TestString**) * row_num));
    m_urls = reinterpret_cast<TestString**>(m_buffer);

    m_min_url_len = 1000;
    for (; read_url_num < row_num; read_url_num++)
    {
        char *line = fgets(read_url_buf, sizeof(read_url_buf), fp);
        if (line == NULL)
        {
            break;
        }

        // get the min_url_len for fixed key, value
        uint32_t length = strlen(line) - 1;
        if (m_min_url_len > length)
        {
            m_min_url_len = length;
        }
        
        // generate the TestString
        TestString* m_string = reinterpret_cast<TestString*>(cur_position);
        m_string->len = length;
        memcpy(m_string->data, line, m_string->len);
        cur_position += sizeof(TestString) + m_string->len;

        m_urls[read_url_num] = m_string;
    }

    fclose(fp);
    return 0;
}

void TestHelper::Print()
{
    FILE* fp = fopen("test_helper.dat", "w");
    test_set_it iter;
    for (iter = m_data.begin(); iter != m_data.end(); iter++)
    {
        fprintf(fp, "key :");
        fwrite((*iter).key->data, 1, (*iter).key->len, fp);
        fprintf(fp, "\n");
    }
    fclose(fp);
}
} // end namespace sstable
