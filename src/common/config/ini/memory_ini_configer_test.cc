//////////////////////////////////////////////////////////////////////////
// ivanhuang @ 20101106
// unit test
//
// ע�⣡����
// common�Ⲣû�а���gtest�����ͷ�ļ��Ϳ��ļ�,make֮ǰ��ִ�����²���:
// 1.����gtest��tar������ѹ
// 2.����gtestĿ¼,ִ��./configure��make��make install
//////////////////////////////////////////////////////////////////////////

#include "common/config/ini/memory_ini_configer.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <gtest/gtest.h>

using namespace std; // NOLINT // come on, it is only a test, just for ease

#ifdef _WIN32
#define snprintf _snprintf
#endif // _WIN32

// ���ý��ּ�
class MemoryConfigerTest : public testing::Test {
protected:
    virtual void SetUp() {
        // ���������ļ�����
        m_ini_file = new MemoryIniConfiger;
        m_field    = "ivan";
    }

    virtual void TearDown() {
        // ����ʼ�����ͷ��ڴ�
        delete m_ini_file;
        m_ini_file = NULL;
    }

protected:
    string         m_field;
    MemoryIniConfiger *m_ini_file;
};

// ��������
TEST_F(MemoryConfigerTest, Integer) {
    int8_t byte_set = -12;
    ASSERT_TRUE(m_ini_file->SetValue(m_field, "byte", byte_set));
    int8_t byte_get = 0;
    ASSERT_TRUE(m_ini_file->GetValue(m_field, "byte", &byte_get));
    ASSERT_EQ(byte_set, byte_get);

    uint8_t ubyte_set = 12;
    ASSERT_TRUE(m_ini_file->SetValue(m_field, "ubyte", ubyte_set));
    uint8_t ubyte_get = 0;
    ASSERT_TRUE(m_ini_file->GetValue(m_field, "ubyte", &ubyte_get));
    ASSERT_EQ(ubyte_set, ubyte_get);

    int16_t word_set = -129;
    ASSERT_TRUE(m_ini_file->SetValue(m_field, "word", word_set));
    int16_t word_get = 0;
    ASSERT_TRUE(m_ini_file->GetValue(m_field, "word", &word_get));
    ASSERT_EQ(word_set, word_get);

    uint16_t uword_set = 129;
    ASSERT_TRUE(m_ini_file->SetValue(m_field, "uword", uword_set));
    uint16_t uword_get = 0;
    ASSERT_TRUE(m_ini_file->GetValue(m_field, "uword", &uword_get));
    ASSERT_EQ(uword_set, uword_get);

    int32_t double_word_set = -70000;
    ASSERT_TRUE(m_ini_file->SetValue(m_field, "dword", double_word_set));
    int32_t double_word_get = 0;
    ASSERT_TRUE(m_ini_file->GetValue(m_field, "dword", &double_word_get));
    ASSERT_EQ(double_word_set, double_word_get);

    uint32_t udouble_word_set = 70000;
    ASSERT_TRUE(m_ini_file->SetValue(m_field, "udword", udouble_word_set));
    uint32_t udouble_word_get = 0;
    ASSERT_TRUE(m_ini_file->GetValue(m_field, "udword", &udouble_word_get));
    ASSERT_EQ(udouble_word_set, udouble_word_get);

    int64_t quad_word_set = -7000000000;
    ASSERT_TRUE(m_ini_file->SetValue(m_field, "ddword", quad_word_set));
    int64_t quad_word_get = 0;
    ASSERT_TRUE(m_ini_file->GetValue(m_field, "ddword", &quad_word_get));
    ASSERT_EQ(quad_word_set, quad_word_get);

    uint64_t uquad_word_set = 7000000000;
    ASSERT_TRUE(m_ini_file->SetValue(m_field, "uddword", uquad_word_set));
    uint64_t uquad_word_get = 0;
    ASSERT_TRUE(m_ini_file->GetValue(m_field, "uddword", &uquad_word_get));
    ASSERT_EQ(uquad_word_set, uquad_word_get);
}

// ���Ը�����
TEST_F(MemoryConfigerTest, FloatingPoint) {
    double diff = 0.0;

    float single_set = 1.23456f;
    ASSERT_TRUE(m_ini_file->SetValue(m_field, "single", single_set));
    float single_get = 0.0f;
    ASSERT_TRUE(m_ini_file->GetValue(m_field, "single", &single_get));
    diff = single_set - single_get;
    ASSERT_TRUE(diff < 0.000001 && diff > -0.000001);

    double double_set = 1.23456789f;
    ASSERT_TRUE(m_ini_file->SetValue(m_field, "double", double_set));
    double double_get = 0.0;
    ASSERT_TRUE(m_ini_file->GetValue(m_field, "double", &double_get));
    diff = double_set - double_get;
    ASSERT_TRUE(diff < 0.000001 && diff > -0.000001);
}
