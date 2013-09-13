#include <iostream>
#include <string>
#include <time.h>
#include <vector>
#include <cmath>
#include <gtest/gtest.h>
#include "common/base/string/string_algorithm.hpp"
#include "common/encoding/ascii.hpp"
#include "common/base/compatible/string.h"
#include "common/system/time/timestamp.hpp"

using namespace std;

TEST(String, Util)
{
    ASSERT_TRUE(IsWhiteString(""));
    ASSERT_TRUE(IsWhiteString("\t\r\n "));
    ASSERT_FALSE(IsWhiteString("\t\r\\"));

    ASSERT_TRUE(IsCharInString('\\', "c:\\"));

    ASSERT_TRUE(HasPrefixString("\\.*?xiaokang", "\\."));
    ASSERT_FALSE(HasPrefixString("\\.*?xiaokang", ".*"));
    ASSERT_EQ(RemovePrefixString("\\.*?xiaokang", "\\."), "*?xiaokang");
    ASSERT_EQ(RemovePrefixString("\\.*?xiaokang", ""), "\\.*?xiaokang");
    ASSERT_EQ(RemovePrefixString("\\.*?xiaokang", "\\.*?xiaokang"), "");

    ASSERT_TRUE(HasSuffixString("\\.*?xiaokang", ""));
    ASSERT_TRUE(HasSuffixString("\\.*?xiaokang", "kang"));
    ASSERT_EQ(RemoveSuffixString("\\.*?xiaokang", "kang"), "\\.*?xiao");
    ASSERT_EQ(RemoveSuffixString("\\.*?xiaokang", "n"), "\\.*?xiaokang");
    ASSERT_EQ(RemoveSuffixString("\\.*?xiaokang", "\\.*?xiaokang"), "");
}

TEST(String, StringTrim)
{
    string str = " ab end  ";
    ASSERT_EQ(StringTrimLeft(str), "ab end  ");
    ASSERT_EQ(StringTrimRight(str), " ab end");
    ASSERT_EQ(StringTrim(str), "ab end");

    StringTrimLeft(&str);
    ASSERT_EQ(str, "ab end  ");
    StringTrimRight(&str);
    ASSERT_EQ(str, "ab end");
    str = " ab end  ";
    StringTrim(&str);
    ASSERT_EQ(str, "ab end");
    StringTrim(&str, "ab");
    ASSERT_EQ(str, " end");

    str = "\r\n        \t\t\t\t";
    ASSERT_EQ(StringTrimLeft(str), "");
    ASSERT_EQ(StringTrimRight(str), "");
    ASSERT_EQ(StringTrim(str), "");
}

TEST(String, UpperAndLowerCase)
{
    string str = " abcdefg \\end ";
    string str_temp = str;
    UpperString(&str);
    ASSERT_EQ(str, " ABCDEFG \\END ");
    LowerString(&str);
    ASSERT_EQ(str, str_temp);
}

TEST(String, SplitString)
{
    string str = " ab c  d   efg  end ";
    vector<string> vec;
    SplitString(str, " ", &vec);
    ASSERT_EQ((int)vec.size(), 5);
}

TEST(String, SplitUsingStringDelimiter)
{
    string str = "abc\r\n\r\nbc\raaa\n\n\r\n";
    vector<string> vec;
    SplitStringByDelimiter(str, "\r\n", &vec);
    ASSERT_EQ(2u, vec.size());
    EXPECT_EQ("abc", vec[0]);
    EXPECT_EQ("bc\raaa\n\n", vec[1]);
}

TEST(String, ReplaceString)
{
    string str = " ab c  d   efghjijkkkkjkk//gj\\*&^xyz  end  ";
    ASSERT_EQ(ReplaceString(str, "kk", "oo"), " ab c  d   efghjijookkjkk//gj\\*&^xyz  end  ");
    ASSERT_EQ(ReplaceString(str, "", "xxxx"), " ab c  d   efghjijkkkkjkk//gj\\*&^xyz  end  ");
    ASSERT_EQ(ReplaceAll(str, "kk", "oo"), " ab c  d   efghjijoooojoo//gj\\*&^xyz  end  ");
    ASSERT_EQ(StripString(str, "gx*", 'X'), " ab c  d   efXhjijkkkkjkk//Xj\\X&^Xyz  end  ");
    StripString(&str, "gx*", 'X');
    ASSERT_EQ(str, " ab c  d   efXhjijkkkkjkk//Xj\\X&^Xyz  end  ");
    ASSERT_EQ("A_C___B_H", ReplaceAllChars("A-C++/B.H", "/.-+", '_'));
}

TEST(String, RemoveSubString)
{
    string str = " abcdefghjijkkkkjkk//gj\\*&^xyz";
    ASSERT_EQ(RemoveContinuousBlank(str), " abcdefghjijkkkkjkk//gj\\*&^xyz");
    ASSERT_EQ(RemoveAllSubStrings(str, "j"), " abcdefghikkkkkk//g\\*&^xyz");
    ASSERT_EQ(RemoveAllSubStrings(str, "j", true), " abcdefgh i kkkk kk//g \\*&^xyz");
}

TEST(String, EscapeUnEsacpe)
{
    string str = "0ccc\n\r\bxxx\x05\x06 d001";
    string s1 = CEscapeString(str);
    ASSERT_EQ(s1, "0ccc\\n\\r\\bxxx\\x05\\x06 d001");
    string s2 = CUnescapeString(s1);
    ASSERT_EQ(s2, str);
}

TEST(String, Compare)
{
    string str1 = "abcdefgXXXXX";
    string str2 = "abcdefg";
    string str3 = "abcdefgYYYYY";

    bool inclusive;
    ASSERT_GT(CompareByteString(
            str1.data(), str1.length(),
            str2.data(), str2.length(),
            &inclusive), 0);

    ASSERT_TRUE(inclusive);

    ASSERT_GT(CompareByteString(str1, str2), 0);
    ASSERT_LT(CompareByteString(str2, str1), 0);
    ASSERT_EQ(CompareByteString(str2, str2), 0);
    ASSERT_LT(CompareByteString(str1, str3), 0);
    ASSERT_GT(CompareByteString(str3, str2), 0);

    ASSERT_EQ((int)GetCommonPrefixLength(str1, str3), 7);
}

const char g_s1[] = "http://finance.qq.com/a/20101228/http://finance.qq.com/a/20101228/006335.htm";
const char g_s2[] = "http://finance.qq.com/a/20101228/http://finance.qq.com/a/20101228/006336.htm";

TEST(String, GetCommonPrefixLength)
{
    string str1 = "abcdefgXXXXX";
    string str2 = "abcdefg";
    string str3 = "abcdefgYYYYY";
    ASSERT_EQ(7U, GetCommonPrefixLength(str1, str2));
    ASSERT_EQ(7U, GetCommonPrefixLength(str1, str3));

    const char* volatile s1 = g_s1;
    const char* volatile s2 = g_s2;
    int total = 0;
    for (int i = 0; i < 1000000; ++i)
        total += GetCommonPrefixLength(s1, sizeof(g_s1) -1, s2, sizeof(g_s2) -1);
    volatile int n = total;
    (void) n;
}

TEST(String, ComparePerformance)
{
    const char* volatile s1 = g_s1;
    const char* volatile s2 = g_s2;
    int total = 0;
    for (int i = 0; i < 1000000; ++i)
        total += CompareByteString(s1, sizeof(g_s1) -1, s2, sizeof(g_s2) -1);
    volatile int n = total;
    (void) n;
}

TEST(String, memcmp_Performance)
{
    const char* volatile s1 = g_s1;
    const char* volatile s2 = g_s2;
    int total = 0;
    for (int i = 0; i < 1000000; ++i)
    {
        total += memcmp(s1, s2, sizeof(g_s1) - 1);
    }
    volatile int n = total;
    (void) n;
}

TEST(String, CompareMemory_Performance)
{
    const char* volatile s1 = g_s1;
    const char* volatile s2 = g_s2;
    int total = 0;
    for (int i = 0; i < 1000000; ++i)
    {
        total += CompareMemory(s1, s2, sizeof(g_s1) - 1);
    }
    volatile int n = total;
    (void) n;
}

TEST(String, MemoryEqual_Performance)
{
    const char* volatile s1 = g_s1;
    const char* volatile s2 = g_s2;
    int total = 0;
    for (int i = 0; i < 1000000; ++i)
    {
        total += MemoryEqual(s1, s2, sizeof(g_s1) - 1);
    }
    volatile int n = total;
    (void) n;
}

