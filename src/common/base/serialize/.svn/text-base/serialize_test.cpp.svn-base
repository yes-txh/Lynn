// Copyright (c) 2008, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <list>
#include <set>
#include <sstream>
#include <vector>

#include "common/base/stdext/hash_set.hpp"
#include "common/base/stdext/hash_map.hpp"

#include "common/base/serialize/serialize.hpp"
#include "gtest/gtest.h"

// GLOBAL_NOLINT(runtime/int)

#define DUMPBIN_ADDRESS_WIDTH   5   // '0000 ', 4 characters and 1 space
#define DUMPBIN_BYTE_PER_LINE   16
#define DUMPBIN_WIDTH_PER_BYTE  3   // 'F0 ', 2 characters and 1 space

#ifdef _MSC_VER
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif

void DebugDumpBinary(
    const void*p,
    size_t Size,
    const void* StartingAddress
)
{
    static const char* HexAlphabet = "0123456789ABCDEF";
    const unsigned char* Data = (const unsigned char*)p;
    size_t i;

    if(StartingAddress)
    {
        printf("Starting address: %p\n", StartingAddress);
    }

    for (i=0; i<Size; i+=DUMPBIN_BYTE_PER_LINE)
    {
        int j;
        uint32_t a = (uint32_t)(uintptr_t)StartingAddress+i;

        // DebugView display each DbgPrint output in one line,
        // so I gather them in one line
        char Line[
            DUMPBIN_ADDRESS_WIDTH +  // Address width
            DUMPBIN_BYTE_PER_LINE * (DUMPBIN_WIDTH_PER_BYTE+1) // contents
            +1  // tail NULL
            ];

        // print the low 2 byte of address
        Line[0] = HexAlphabet[(a>>12) & 0x0F];
        Line[1] = HexAlphabet[(a>>8) & 0x0F];
        Line[2] = HexAlphabet[(a>>4) & 0x0F];
        Line[3] = HexAlphabet[(a) & 0x0F];
        Line[4] = ' ';

        for (j=0; j<DUMPBIN_BYTE_PER_LINE; ++j)
        {
            size_t HexPos = DUMPBIN_ADDRESS_WIDTH + j*DUMPBIN_WIDTH_PER_BYTE;
            size_t CharPos =
                DUMPBIN_ADDRESS_WIDTH +
                DUMPBIN_BYTE_PER_LINE*DUMPBIN_WIDTH_PER_BYTE+
                j;

            if (i+j < Size)
            {
                unsigned char c = Data[i+j];

                Line[HexPos+0] = HexAlphabet[c>>4];
                Line[HexPos+1] = HexAlphabet[c&0x0F];
                Line[HexPos+2] = ' ';
                Line[CharPos] = isprint(c) ? c : '.';

                if (j==8)
                    Line[HexPos-1] = '-';
            }
            else
            {
                Line[HexPos+0] = ' ';
                Line[HexPos+1] = ' ';
                Line[HexPos+2] = ' ';
                Line[CharPos] = ' ';
            }
        }
        Line[sizeof(Line)-1] = 0;
        printf("%s\n", Line);
    }
}

template <typename T1, typename T2 = T1>
struct TypeEqual
{
    static void Test(const char* name1, const char* name2)
    {
        std::vector<char> v;
        Serialize::VectorOutputStream vos(v);
        Serialize::BinaryEncoder<Serialize::VectorOutputStream> encoder(vos);

        T1 t1 = (T1)rand() * rand();
        encoder << t1;

        Serialize::VectorInputStream vis(v);
        Serialize::BinaryDecoder<Serialize::VectorInputStream> decoder(vis);
        T2 t2;
        decoder >> t2;

//      EXPECT_EQ((t2, (T2)t1), ", T1=%s:%s, T2=%s:%s", name1, ToString(t1).c_str(), name2, ToString(t2).c_str());
    }
private:
    template <typename T>
    static std::string ToString(const T& value)
    {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

    static std::string ToString(char value)
    {
        return ToString(int(value));
    }

    static std::string ToString(signed char value)
    {
        return ToString(int(value));
    }
    static std::string ToString(unsigned char value)
    {
        return ToString(int(value));
    }
};

#define TEST_TYPE_COMPATIBLE(type) TypeEqual<type>::Test(#type, #type)
#define TEST_TYPE_COMPATIBLE2(type1, type2) TypeEqual<type1, type2>::Test(#type1, #type2)

TEST(Serialize, TypeCompatibles)
{
    srand((unsigned int)time(NULL));
    TEST_TYPE_COMPATIBLE(signed char);
    TEST_TYPE_COMPATIBLE(signed short);
    TEST_TYPE_COMPATIBLE(signed int);
    TEST_TYPE_COMPATIBLE(signed long);
    TEST_TYPE_COMPATIBLE(signed long long);

    TEST_TYPE_COMPATIBLE(unsigned char);
    TEST_TYPE_COMPATIBLE(unsigned short);
    TEST_TYPE_COMPATIBLE(unsigned int);
    TEST_TYPE_COMPATIBLE(unsigned long);
    TEST_TYPE_COMPATIBLE(unsigned long long);

    TEST_TYPE_COMPATIBLE(float);
    TEST_TYPE_COMPATIBLE(double);

    TEST_TYPE_COMPATIBLE2(signed char, signed short);
    TEST_TYPE_COMPATIBLE2(signed char, signed int);
    TEST_TYPE_COMPATIBLE2(signed char, signed long);
    TEST_TYPE_COMPATIBLE2(signed char, signed long long);
    TEST_TYPE_COMPATIBLE2(signed char, unsigned short);
    TEST_TYPE_COMPATIBLE2(signed char, unsigned int);
    TEST_TYPE_COMPATIBLE2(signed char, unsigned long);
    TEST_TYPE_COMPATIBLE2(signed char, unsigned long long);

    TEST_TYPE_COMPATIBLE2(unsigned char, signed short);
    TEST_TYPE_COMPATIBLE2(unsigned char, signed int);
    TEST_TYPE_COMPATIBLE2(unsigned char, signed long);
    TEST_TYPE_COMPATIBLE2(unsigned char, signed long long);
    TEST_TYPE_COMPATIBLE2(unsigned char, unsigned short);
    TEST_TYPE_COMPATIBLE2(unsigned char, unsigned int);
    TEST_TYPE_COMPATIBLE2(unsigned char, unsigned long);
    TEST_TYPE_COMPATIBLE2(unsigned char, unsigned long long);

    TEST_TYPE_COMPATIBLE2(signed short, signed char);
    TEST_TYPE_COMPATIBLE2(signed short, signed int);
    TEST_TYPE_COMPATIBLE2(signed short, signed long);
    TEST_TYPE_COMPATIBLE2(signed short, signed long long);
    TEST_TYPE_COMPATIBLE2(signed short, unsigned short);
    TEST_TYPE_COMPATIBLE2(signed short, unsigned int);
    TEST_TYPE_COMPATIBLE2(signed short, unsigned long);
    TEST_TYPE_COMPATIBLE2(signed short, unsigned long long);

    TEST_TYPE_COMPATIBLE2(unsigned short, signed char);
    TEST_TYPE_COMPATIBLE2(unsigned short, signed int);
    TEST_TYPE_COMPATIBLE2(unsigned short, signed long);
    TEST_TYPE_COMPATIBLE2(unsigned short, signed long long);
    TEST_TYPE_COMPATIBLE2(unsigned short, unsigned short);
    TEST_TYPE_COMPATIBLE2(unsigned short, unsigned int);
    TEST_TYPE_COMPATIBLE2(unsigned short, unsigned long);
    TEST_TYPE_COMPATIBLE2(unsigned short, unsigned long long);

    TEST_TYPE_COMPATIBLE2(int, signed char);
    TEST_TYPE_COMPATIBLE2(int, signed int);
    TEST_TYPE_COMPATIBLE2(int, signed long);
    TEST_TYPE_COMPATIBLE2(int, signed long long);
    TEST_TYPE_COMPATIBLE2(int, unsigned short);
    TEST_TYPE_COMPATIBLE2(int, unsigned int);
    TEST_TYPE_COMPATIBLE2(int, unsigned long);
    TEST_TYPE_COMPATIBLE2(int, unsigned long long);

    TEST_TYPE_COMPATIBLE2(unsigned int, signed char);
    TEST_TYPE_COMPATIBLE2(unsigned int, signed short);
    TEST_TYPE_COMPATIBLE2(unsigned int, signed int);
    TEST_TYPE_COMPATIBLE2(unsigned int, signed long);
    TEST_TYPE_COMPATIBLE2(unsigned int, signed long long);
    TEST_TYPE_COMPATIBLE2(unsigned int, unsigned char);
    TEST_TYPE_COMPATIBLE2(unsigned int, unsigned int);
    TEST_TYPE_COMPATIBLE2(unsigned int, unsigned long);
    TEST_TYPE_COMPATIBLE2(unsigned int, unsigned long long);

    TEST_TYPE_COMPATIBLE2(long, signed char);
    TEST_TYPE_COMPATIBLE2(long, signed int);
    TEST_TYPE_COMPATIBLE2(long, signed long);
    TEST_TYPE_COMPATIBLE2(long, signed long long);
    TEST_TYPE_COMPATIBLE2(long, unsigned short);
    TEST_TYPE_COMPATIBLE2(long, unsigned int);
    TEST_TYPE_COMPATIBLE2(long, unsigned long);
    TEST_TYPE_COMPATIBLE2(long, unsigned long long);

    TEST_TYPE_COMPATIBLE2(unsigned long, signed char);
    TEST_TYPE_COMPATIBLE2(unsigned long, signed int);
    TEST_TYPE_COMPATIBLE2(unsigned long, signed long);
    TEST_TYPE_COMPATIBLE2(unsigned long, signed long long);
    TEST_TYPE_COMPATIBLE2(unsigned long, unsigned short);
    TEST_TYPE_COMPATIBLE2(unsigned long, unsigned int);
    TEST_TYPE_COMPATIBLE2(unsigned long, unsigned long);
    TEST_TYPE_COMPATIBLE2(unsigned long, unsigned long long);

    TEST_TYPE_COMPATIBLE2(long long, signed char);
    TEST_TYPE_COMPATIBLE2(long long, signed int);
    TEST_TYPE_COMPATIBLE2(long long, signed long);
    TEST_TYPE_COMPATIBLE2(long long, signed long long);
    TEST_TYPE_COMPATIBLE2(long long, unsigned short);
    TEST_TYPE_COMPATIBLE2(long long, unsigned int);
    TEST_TYPE_COMPATIBLE2(long long, unsigned long);
    TEST_TYPE_COMPATIBLE2(long long, unsigned long long);

    TEST_TYPE_COMPATIBLE2(unsigned long long, signed char);
    TEST_TYPE_COMPATIBLE2(unsigned long long, signed int);
    TEST_TYPE_COMPATIBLE2(unsigned long long, signed long);
    TEST_TYPE_COMPATIBLE2(unsigned long long, signed long long);
    TEST_TYPE_COMPATIBLE2(unsigned long long, unsigned short);
    TEST_TYPE_COMPATIBLE2(unsigned long long, unsigned int);
    TEST_TYPE_COMPATIBLE2(unsigned long long, unsigned long);
    TEST_TYPE_COMPATIBLE2(unsigned long long, unsigned long long);

    TEST_TYPE_COMPATIBLE2(float, double);
    TEST_TYPE_COMPATIBLE2(double, float);
}

enum TestEnum
{
    enum1,
    enum2
};

SERIALIZE_REGISTER_ENUM(TestEnum)

struct Point
{
    int x;
    int y;
};

SERIALIZE_REGISTER_STRUCT(Point)
    SERIALIZE_MEMBER(x)
    SERIALIZE_MEMBER(y)
SERIALIZE_END_STRUCT()

struct User1
{
    std::string Name;
    bool Good;
    short Age;
    Point Location;
    int Money() const
    {
        return INT_MAX;
    }
    void SetMoney(int&)
    {
    }
    int History[3];
};

SERIALIZE_REGISTER_STRUCT(User1)
    SERIALIZE_MEMBER(Name)
    SERIALIZE_MEMBER(Good)
    SERIALIZE_MEMBER(Age)
    SERIALIZE_MEMBER(Location)
    SERIALIZE_PROPERTY(Money, Money, SetMoney)
    SERIALIZE_MEMBER(History)
SERIALIZE_END_STRUCT(SERIALIZE_IGNORE_OTHER_MEMBERS())

struct User2
{
    std::string Name;
    bool Good;
    int Age;
    Point Location;
    int Money() const
    {
        return INT_MIN;
    }
    void SetMoney(int&)
    {
    }
    std::vector<long long> History;
    int Xxx1;
    Point Xxx2;
};

SERIALIZE_REGISTER_STRUCT(User2)
    SERIALIZE_MEMBER(Name)
    SERIALIZE_MEMBER(Good)
    SERIALIZE_MEMBER(Age)
    SERIALIZE_MEMBER(Location)
    SERIALIZE_PROPERTY(Money, Money, SetMoney)
    SERIALIZE_MEMBER(History)
    SERIALIZE_OPTIONAL_MEMBER(Xxx1, 100)
    SERIALIZE_OPTIONAL_MEMBER(Xxx2, Point())
SERIALIZE_END_STRUCT()

TEST(Serialize, VersionCompatibles)
{
    User2 user2 = { "chen3feng", true, 29, { 640, 480 }};
    user2.History.push_back(1);
    user2.History.push_back(2);
    user2.History.push_back(3);

    User1 user1;

    std::vector<char> v;

    // Low version compatible high version
    {
        Serialize::VectorOutputStream vos(v);
        Serialize::BinaryEncoder<Serialize::VectorOutputStream> encoder(vos);
        encoder << user2;

        Serialize::VectorInputStream vis(v);
        Serialize::BinaryDecoder<Serialize::VectorInputStream> decoder(vis);
        decoder >> user1;

        EXPECT_EQ(user1.Name, user2.Name);
        EXPECT_EQ(user1.Good, user2.Good);
        EXPECT_EQ(user1.Age, user2.Age);
        EXPECT_EQ(user1.Location.x, user2.Location.x);
        EXPECT_EQ(user1.Location.y, user2.Location.y);
        EXPECT_EQ(3U, user2.History.size());
        EXPECT_EQ(user1.History[0], user2.History[0]);
        EXPECT_EQ(user1.History[1], user2.History[1]);
        EXPECT_EQ(user1.History[2], user2.History[2]);
    }

    // High version compatible low version
    {
        Serialize::VectorOutputStream vos(v);
        Serialize::BinaryEncoder<Serialize::VectorOutputStream> encoder(vos);
        encoder << user1;

        Serialize::VectorInputStream vis(v);
        Serialize::BinaryDecoder<Serialize::VectorInputStream> decoder(vis);
        decoder >> user2;

        EXPECT_EQ(user1.Name, user2.Name);
        EXPECT_EQ(user1.Good, user2.Good);
        EXPECT_EQ(user1.Age, user2.Age);
        EXPECT_EQ(user1.Location.x, user2.Location.x);
        EXPECT_EQ(user1.Location.y, user2.Location.y);
        EXPECT_EQ(user2.History.size(), 3U);
        EXPECT_EQ(user1.History[0], user2.History[0]);
        EXPECT_EQ(user1.History[1], user2.History[1]);
        EXPECT_EQ(user1.History[2], user2.History[2]);
        EXPECT_EQ(user2.Xxx1, 100); // default value
        EXPECT_EQ(user2.Xxx2.x, 0);
        EXPECT_EQ(user2.Xxx2.y, 0);
    }
}


struct OptionalArray
{
    unsigned int a[4];
};

SERIALIZE_REGISTER_STRUCT(OptionalArray)
    SERIALIZE_OPTIONAL_MEMBER(a, 0)
SERIALIZE_END_STRUCT()

TEST(Serialize, OptionalArray)
{
    OptionalArray a = {};
    size_t n = Serialize::BinaryEncodedSize(a);
    (void) n;
}

class Abc
{
public:
    Abc():
        a(1), b(2), c(3)
    {
    }
    int A() const
    {
        return a;
    }
    void A(int n)
    {
        a = n;
    }
private:
    int a;
    int b;
    int c;
    SERIALIZE_FRIEND(Abc);
};

SERIALIZE_REGISTER_STRUCT(Abc)
    SERIALIZE_MEMBER(a)
    SERIALIZE_MEMBER(b)
    SERIALIZE_MEMBER(c)
SERIALIZE_END_STRUCT()

class Abcd : public Abc
{
public:
    Abcd():d(4){}
    int d;
};

SERIALIZE_REGISTER_STRUCT(Abcd)
    SERIALIZE_BASE(Abc)
    SERIALIZE_MEMBER(d)
SERIALIZE_END_STRUCT()

TEST(Serialize, AccessAndInherit)
{
    Abc abc;
    Abcd abcd;

    std::vector<char> v;
    Serialize::VectorOutputStream vos(v);
    Serialize::BinaryEncoder<Serialize::VectorOutputStream> encoder(vos);
    encoder << abc;
    encoder << abcd;

    abc.A(10);
    abcd.d = 10;

    Serialize::VectorInputStream vis(v);
    Serialize::BinaryDecoder<Serialize::VectorInputStream> decoder(vis);
    decoder >> abc;
    decoder >> abcd;

    EXPECT_EQ(1, abc.A());
    EXPECT_EQ(4, abcd.d);
}

struct EmptyStruct
{
};
SERIALIZE_REGISTER_STRUCT(EmptyStruct)
SERIALIZE_END_STRUCT()

TEST(Serialize, EmptyStruct)
{
    EmptyStruct e;
    std::vector<char> buffer;
    Serialize::BinaryEncode(e, buffer);
    Serialize::BinaryDecode(buffer, e);
}

namespace TestNamespace
{
    struct A
    {
        int a;
    };
    SERIALIZE_REGISTER_STRUCT(A)
        SERIALIZE_MEMBER(a)
    SERIALIZE_END_STRUCT()

    struct B
    {
        int b;
    };
    SERIALIZE_REGISTER_STRUCT(TestNamespace::B)
        SERIALIZE_MEMBER(b)
    SERIALIZE_END_STRUCT()

    struct C
    {
        int c;
    };
}

namespace Serialize
{
    SERIALIZE_REGISTER_STRUCT(TestNamespace::C)
        SERIALIZE_MEMBER(c)
    SERIALIZE_END_STRUCT()
}

struct TestStringX
{
    char str[100];
};

TEST(Serialize, Namespace)
{
    TestNamespace::A a = { 0 };
    std::vector<char> v;
    Serialize::VectorOutputStream vos(v);
    Serialize::BinaryEncoder<Serialize::VectorOutputStream> encoder(vos);
    encoder << a;

    a.a = 1;
    Serialize::VectorInputStream vis(v);
    Serialize::BinaryDecoder<Serialize::VectorInputStream> decoder(vis);
    decoder >> a;

    EXPECT_EQ(0, a.a);

    Serialize::XmlEncoder<Serialize::VectorOutputStream> xml_encoder(vos);
    xml_encoder << a;
}

struct TestString
{
    char str1[100];
    char* str2;
    char* str3;
    char* str4;
    char* str5;
    size_t str5_length;

    TestString()
    {
        str1[0] = '\0';
        str2 = NULL;
        str3 = NULL;
        str4 = NULL;
        str5 = NULL;
        str5_length = 0;
    }

    ~TestString()
    {
        delete[] str2;
        free(str3);
        free(str4);
        free(str5);
    }
};

SERIALIZE_REGISTER_STRUCT(TestString)
    SERIALIZE_MEMBER_STRING(str1)
    SERIALIZE_MEMBER_STRING_NEW(str2)
    SERIALIZE_MEMBER_STRING_MALLOC(str3)
    SERIALIZE_MEMBER_STRING_REALLOC(str4)
    SERIALIZE_MEMBER_STRING_LENGTH_MALLOC(str5, str5_length)
SERIALIZE_END_STRUCT()

struct TestStringLength
{
    char str[100];
    size_t length;
};
SERIALIZE_REGISTER_STRUCT(TestStringLength)
    SERIALIZE_MEMBER_STRING_LENGTH(str, length)
SERIALIZE_END_STRUCT()

TEST(Serialize, String)
{
    std::vector<char> v;
    {
        TestString s1;
        strcpy(s1.str1, "hello");
        s1.str2 = new char[sizeof("hello2")];
        strcpy(s1.str2, "hello2");
        s1.str3 = strdup("hello3");
        s1.str4 = strdup("hello4");
        s1.str5 = strdup("hello5");
        s1.str5_length = strlen(s1.str5);

        Serialize::VectorOutputStream vos(v);
        Serialize::BinaryEncoder<Serialize::VectorOutputStream> encoder(vos);
        encoder << s1;

        TestString s2;
        Serialize::VectorInputStream vis(v);
        Serialize::BinaryDecoder<Serialize::VectorInputStream> decoder(vis);
        decoder >> s2;

        EXPECT_EQ(0, strcmp(s1.str1, s2.str1));
        EXPECT_EQ(0, strcmp(s1.str2, s2.str2));
        EXPECT_EQ(0, strcmp(s1.str3, s2.str3));
        EXPECT_EQ(0, strcmp(s1.str4, s2.str4));
        EXPECT_EQ(0, strcmp(s1.str5, s2.str5));
    }
    {
        TestStringLength s1 = {"hello", 5};
        Serialize::VectorOutputStream vos(v);
        Serialize::BinaryEncoder<Serialize::VectorOutputStream> encoder(vos);
        encoder << s1;

        TestStringLength s2 = {"world123", 8};
        Serialize::VectorInputStream vis(v);
        Serialize::BinaryDecoder<Serialize::VectorInputStream> decoder(vis);
        decoder >> s2;

        EXPECT_EQ(0, strcmp(s1.str, s2.str));
        EXPECT_EQ(5U, s2.length);
    }
}

struct TestArrayLength
{
    int array[100];
    size_t length;
};
SERIALIZE_REGISTER_STRUCT(TestArrayLength)
    SERIALIZE_MEMBER_ARRAY_LENGTH(array, length)
SERIALIZE_END_STRUCT()

TEST(Serialize, Array)
{
    std::vector<char> v;
    {
        TestArrayLength s1 = {{1, 2, 3, 4, 5}, 5};
        Serialize::VectorOutputStream vos(v);
        Serialize::BinaryEncoder<Serialize::VectorOutputStream> encoder(vos);
        encoder << s1;

        TestArrayLength s2 = {};
        Serialize::VectorInputStream vis(v);
        Serialize::BinaryDecoder<Serialize::VectorInputStream> decoder(vis);
        decoder >> s2;

        EXPECT_EQ(5U, s2.length);
        EXPECT_EQ(0, memcmp(s1.array, s2.array, sizeof(s1.array[0])*s1.length));
    }
}

struct TestUnion
{
    int type;
    union
    {
        long long value1;
        struct
        {
            int value2;
            int value3;
            int value4;
        };
    };
};
SERIALIZE_REGISTER_STRUCT(TestUnion)
    SERIALIZE_UNION_SWITCH(type)
        SERIALIZE_UNION_CASE(1, SERIALIZE_MEMBER(value1))
        SERIALIZE_UNION_CASE(2,
            SERIALIZE_MEMBER(value2)
            SERIALIZE_MEMBER(value3)
        )
        SERIALIZE_UNION_CASE(3)
        SERIALIZE_UNION_NO_DEFAULT()
    SERIALIZE_UNION_END()
SERIALIZE_END_STRUCT()

TEST(Serialize, Union)
{
    std::vector<char> v;
    {
        TestUnion u1 = {1};
        u1.value1 = 1;
        u1.value2 = 2;
        u1.value3 = 3;
        u1.value4 = 4;
        Serialize::VectorOutputStream vos(v);
        Serialize::BinaryEncoder<Serialize::VectorOutputStream> encoder(vos);
        encoder << u1;

        TestUnion u2 = {};
        Serialize::VectorInputStream vis(v);
        Serialize::BinaryDecoder<Serialize::VectorInputStream> decoder(vis);
        decoder >> u2;

        EXPECT_EQ(u2.type, u1.type);
        EXPECT_EQ(u2.value1, u1.value1);
        EXPECT_EQ(u2.value2, u1.value2);
        EXPECT_EQ(u2.value3, u1.value3);
        EXPECT_NE(u2.value4, u1.value4);
//      EXPECT_TRUE(u2.value4 == 0, "u2.value4=%d", u2.value4);
    }
    {
        TestUnion u1 = {2};
        u1.value1 = 0xFFFFFFFFFFFFFFFFLL;

        Serialize::VectorOutputStream vos(v);
        Serialize::BinaryEncoder<Serialize::VectorOutputStream> encoder(vos);
        encoder << u1;

        TestUnion u2 = {};
        Serialize::VectorInputStream vis(v);
        Serialize::BinaryDecoder<Serialize::VectorInputStream> decoder(vis);
        decoder >> u2;

        EXPECT_EQ(u2.type, u1.type);
        EXPECT_EQ(u2.value2, u1.value2);
    }
    {
        TestUnion u = {4};

        Serialize::VectorOutputStream vos(v);
        Serialize::BinaryEncoder<Serialize::VectorOutputStream> encoder(vos);
        bool error_detected = false;
        try
        {
            Encode(encoder, u);
        }
        catch(...)
        {
            error_detected = true;
        }
        EXPECT_TRUE(error_detected);
    }
}

TEST(Serialize, Functions)
{
    int n = 0xdeadbeef;

    std::vector<char> v;
    std::string s;

    Serialize::BinaryEncode(n, v);
    size_t old_size = v.size();

    Serialize::BinaryEncodeAppend(n, v);
    size_t new_size = v.size();

    Serialize::BinaryEncode(n, s);
    old_size = s.size();

    Serialize::BinaryEncodeAppend(n, s);
    new_size = s.size();

    EXPECT_EQ(new_size, 2 * old_size);
    EXPECT_EQ(0, memcmp(v.data(), v.data() + old_size, old_size));

    int n1, n2;
    Serialize::BinaryDecode(v, n1);
    EXPECT_EQ(n1, n);

    Serialize::BinaryDecode(s, n1);
    EXPECT_EQ(n1, n);

    Serialize::BinaryDecode(&v[old_size], old_size, n2);
    EXPECT_EQ(n2, n);

    Serialize::BinaryDecode(v.data() + old_size, old_size, n2);
    EXPECT_EQ(n2, n);
}

TEST(Serialize, JsonEncoder)
{
    User2 user2 = { "chen3feng", true, 29, { 640, 480 }};
    Serialize::JsonDump(user2, stdout, false, false);
    putchar('\n');
    Serialize::JsonDump(user2, stdout);
}

TEST(Serialize, XmlEncoder)
{
    User2 user2 = { "chen3feng", true, 29, { 640, 480 }};
    Serialize::XmlDump(user2, stdout);
}

struct TestUnknownMembers
{
    bool b;
    char ch;
    signed char sch;
    unsigned uch;
    short s;
    unsigned us;
    int i;
    unsigned int ui;
    long l;
    unsigned long ul;
    long long ll;
    unsigned long ull;
    float f;
    double d;
    std::string str;
};

SERIALIZE_REGISTER_STRUCT(TestUnknownMembers)
    SERIALIZE_MEMBER(b)
    SERIALIZE_MEMBER(ch)
    SERIALIZE_MEMBER(sch)
    SERIALIZE_MEMBER(uch)
    SERIALIZE_MEMBER(s)
    SERIALIZE_MEMBER(us)
    SERIALIZE_MEMBER(i)
    SERIALIZE_MEMBER(ui)
    SERIALIZE_MEMBER(l)
    SERIALIZE_MEMBER(ul)
    SERIALIZE_MEMBER(ll)
    SERIALIZE_MEMBER(ull)
    SERIALIZE_MEMBER(f)
    SERIALIZE_MEMBER(d)
    SERIALIZE_MEMBER(str)
SERIALIZE_END_STRUCT()

TEST(Serialize, OtherMembers)
{
    std::vector<char> buffer;

    // serialize
    TestUnknownMembers s1 =
    {
        true,
        'A',
        -1,
        1,
        -2,
        2,
        -4,
        4,
        -4,
        4,
        -8,
        8,
        4.0,
        -8.0,
        "hello"
    };
    Serialize::BinaryEncode(s1, buffer);

    // read to AnyStruct and write back
    Serialize::Struct any;
    EXPECT_TRUE(Serialize::BinaryDecode(buffer, any));
    Serialize::BinaryEncode(any, buffer);

    // unserialize again
    TestUnknownMembers s2 = TestUnknownMembers();
    Serialize::BinaryDecode(buffer, s2);

    // verify result;
    EXPECT_EQ(s2.b, s1.b);
    EXPECT_EQ(s2.ch, s1.ch);
    EXPECT_EQ(s2.sch, s1.sch);
    EXPECT_EQ(s2.uch, s1.uch);
    EXPECT_EQ(s2.s, s1.s);
    EXPECT_EQ(s2.us, s1.us);
    EXPECT_EQ(s2.i, s1.i);
    EXPECT_EQ(s2.ui, s1.ui);
    EXPECT_EQ(s2.l, s1.l);
    EXPECT_EQ(s2.ul, s1.ul);
    EXPECT_EQ(s2.ll, s1.ll);
    EXPECT_EQ(s2.ull, s1.ull);
    EXPECT_EQ(s2.f, s1.f);
    EXPECT_EQ(s2.d, s1.d);
    EXPECT_EQ(s2.str, s1.str);
}

struct TestNull
{
    int a;
    int b;
    int c;
};
SERIALIZE_REGISTER_STRUCT(TestNull)
    SERIALIZE_MEMBER(a)
    SERIALIZE_MEMBER(b)
    SERIALIZE_MEMBER(c)
SERIALIZE_END_STRUCT()

struct TestNull2
{
    int a;
    int c;
};
SERIALIZE_REGISTER_STRUCT(TestNull2)
    SERIALIZE_MEMBER(a)
    SERIALIZE_NULL_MEMBER()
    SERIALIZE_MEMBER(c)
SERIALIZE_END_STRUCT()

TEST(Serialize, NullMember)
{
    TestNull a = { 1, 2, 3 };
    std::vector<char> buffer;
    Serialize::BinaryEncode(a, buffer);

    TestNull2 b = { 0, 0 };
    EXPECT_TRUE(Serialize::BinaryDecode(buffer, b));
    EXPECT_EQ(a.a, b.a);
    EXPECT_EQ(a.c, b.c);
}

#if 0
TEST(Serialize, Performance)
{
    User2 user2 = { "chen3feng", true, 29, { 640, 480 }};

    std::vector<char> v;

    const int OUTTER_LOOP_COUNT = 20000;
    const int INNER_LOOP_COUNT = 1000;
    time_t t0 = time(NULL);
    for (int i = 0; i < OUTTER_LOOP_COUNT; ++i)
    {
        v.clear();
        Serialize::VectorOutputStream vos(v);
        Serialize::BinaryEncoder<Serialize::VectorOutputStream> encoder(vos);
        for (int j = 0; j < INNER_LOOP_COUNT; ++j)
            encoder << user2;
    }
    time_t t = time(NULL) - t0;
    UNIT_TEST_INFO("Encode Time=%ds", int(t));

    t0 = time(NULL);
    for (int i = 0; i < OUTTER_LOOP_COUNT; ++i)
    {
        Serialize::VectorInputStream vis(v);
        Serialize::BinaryDecoder<Serialize::VectorInputStream> decoder(vis);
        for (int j = 0; j < INNER_LOOP_COUNT; ++j)
            decoder >> user2;
    }
    t = time(NULL) - t0;
    UNIT_TEST_INFO("Decode Time=%ds", int(t));
}
#endif

TEST(Serialize, Set)
{
    std::set<std::string> s1, s2;
    s1.insert("hello");
    s1.insert("world");
    std::vector<char> buffer;
    Serialize::BinaryEncode(s1, buffer);
    EXPECT_TRUE(Serialize::BinaryDecode(buffer, s2));
    EXPECT_TRUE(s1 == s2);
}

TEST(Serialize, MultiSet)
{
    std::multiset<std::string> s1, s2;
    s1.insert("hello");
    s1.insert("hello");
    s1.insert("world");
    s1.insert("world");
    std::vector<char> buffer;
    Serialize::BinaryEncode(s1, buffer);
    EXPECT_TRUE(Serialize::BinaryDecode(buffer, s2));
    EXPECT_TRUE(s1 == s2);
}

TEST(Serialize, Map)
{
    std::map<int, std::string> m1, m2;
    m1[1] = "hello";
    m1[2] = "world";
    std::vector<char> buffer;
    Serialize::BinaryEncode(m1, buffer);
    EXPECT_TRUE(Serialize::BinaryDecode(buffer, m2));
    EXPECT_TRUE(m1 == m2);
}

TEST(Serialize, MultiMap)
{
    std::multimap<int, std::string> m1, m2;
    m1.insert(std::make_pair(1, "hello"));
    m1.insert(std::make_pair(1, "hello"));
    m1.insert(std::make_pair(2, "hello"));
    m1.insert(std::make_pair(2, "hello"));
    std::vector<char> buffer;
    Serialize::BinaryEncode(m1, buffer);
    EXPECT_TRUE(Serialize::BinaryDecode(buffer, m2));
    EXPECT_TRUE(m1 == m2);
}

struct MemberMap
{
    std::map<int, std::string> M;
};

SERIALIZE_REGISTER_STRUCT(MemberMap)
    SERIALIZE_MEMBER(M)
SERIALIZE_END_STRUCT()

TEST(Serialize, MemberMap)
{
    MemberMap m;
    EXPECT_TRUE(Serialize::BinaryEncodedSize(m) != 0);
}

TEST(Serialize, Hash)
{
    ext::hash_set<int> hs, hs1;
    hs.insert(1);
    hs.insert(2);

    ext::hash_map<int, std::string> hm, hm1;
    hm[0] = "hello";
    hm[1] = "world";

    ext::hash_multiset<int> hms, hms1;
    hms.insert(1);
    hms.insert(1);
    hms.insert(2);

    ext::hash_multimap<int, std::string> hmm, hmm1;
    hmm.insert(std::make_pair(0, "hello"));
    hmm.insert(std::make_pair(1, "hello"));
    hmm.insert(std::make_pair(0, "world"));

    std::vector<char> buffer;
    Serialize::BinaryEncode(hs, buffer);
    EXPECT_TRUE(Serialize::BinaryDecode(buffer, hs1));
    EXPECT_TRUE(hs1 == hs);

    Serialize::BinaryEncode(hm, buffer);
    EXPECT_TRUE(Serialize::BinaryDecode(buffer, hm1));
    EXPECT_TRUE(hm1 == hm);

    Serialize::BinaryEncode(hms, buffer);
    EXPECT_TRUE(Serialize::BinaryDecode(buffer, hms1));
    EXPECT_TRUE(hms1 == hms);

    Serialize::BinaryEncode(hmm, buffer);
    EXPECT_TRUE(Serialize::BinaryDecode(buffer, hmm1));
    EXPECT_TRUE(hmm1 == hmm);

    Serialize::XmlDump(hs, stdout);

    Serialize::JsonDump(hs, stdout);
    Serialize::JsonDump(hm, stdout);
    Serialize::JsonDump(hms, stdout);
    Serialize::JsonDump(hmm, stdout);

    Serialize::JsonDump(hs, std::cout);
    Serialize::JsonDump(hm, std::cout);
    Serialize::JsonDump(hms, std::cout);
    Serialize::JsonDump(hmm, std::cout);
}

struct Struct1
{
    int a;
};

SERIALIZE_REGISTER_STRUCT(Struct1)
    SERIALIZE_MEMBER(a)
SERIALIZE_END_STRUCT()

struct Struct2
{
    int a;
    int b;
};

SERIALIZE_REGISTER_STRUCT(Struct2)
    SERIALIZE_MEMBER(a)
    SERIALIZE_MEMBER(b)
SERIALIZE_END_STRUCT()

struct NestedStruct1
{
    Struct1 a;
};

struct NestedStruct2
{
    Struct2 a;
};

SERIALIZE_REGISTER_STRUCT(NestedStruct1)
    SERIALIZE_MEMBER(a)
SERIALIZE_END_STRUCT()

SERIALIZE_REGISTER_STRUCT(NestedStruct2)
    SERIALIZE_MEMBER(a)
SERIALIZE_END_STRUCT()

TEST(Serialize, Error)
{
    std::vector<char> buffer;
    Struct1 s1;
    Serialize::BinaryEncode(s1, buffer);

    Struct2 s2;
    Serialize::ErrorStack es;
    EXPECT_EQ(0U, Serialize::BinaryDecode(buffer, s2, &es));
    std::cout << es.ToString();

    {
        NestedStruct1 ns1;
        Serialize::BinaryEncode(ns1, buffer);

        NestedStruct2 ns2;
        EXPECT_EQ(0U, Serialize::BinaryDecode(buffer, ns2, &es));
        std::cout << es.ToString();
    }
}

