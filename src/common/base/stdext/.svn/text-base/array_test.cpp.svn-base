#include "common/base/stdext/array.hpp"
#include <gtest/gtest.h>

template< class T >
void BadValue(const T&)
{
}

template<class T>
void  RunTests()
{
    typedef stdext::array<T, 0>  test_type;

    //  Test value and aggegrate initialization
    test_type                 test_case  =  {};
    const stdext::array<T, 0>  const_test_case = test_type();

    test_case.assign(T());

    //  front/back and operator[] must compile, but calling them is undefined
    //  Likewise, all tests below should evaluate to false, avoiding undefined behaviour
    if (!test_case.empty()) {
        BadValue(test_case.front());
    }

    if (!const_test_case.empty()) {
        BadValue(const_test_case.back());
    }

    if (test_case.size() > 0) {
        BadValue(test_case[ 0 ]);
    }

    if (const_test_case.max_size() > 0) {
        BadValue(const_test_case[ 0 ]);
    }

    //  Assert requirements of TR1 6.2.2.4
    ASSERT_EQ(test_case.begin(), test_case.end()) << "Not an empty range";
    ASSERT_EQ(const_test_case.begin(), const_test_case.end()) << "Not an empty range";

    ASSERT_EQ(const_test_case.begin(), const_test_case.begin())
        << "iterators for different containers are not distinct";

    if (test_case.data() == const_test_case.data()) {
        //  Value of data is unspecified in TR1, so no requirement this test pass or fail
        //  However, it must compile!
    }


    //  Check can safely use all iterator types with std algorithms
    std::for_each(test_case.begin(), test_case.end(), BadValue<T>);
    std::for_each(test_case.rbegin(), test_case.rend(), BadValue<T>);
    std::for_each(const_test_case.begin(), const_test_case.end(), BadValue<T>);
    std::for_each(const_test_case.rbegin(), const_test_case.rend(), BadValue<T>);

    //  Check swap is well formed
    std::swap(test_case, test_case);

    //  Check assigment operator and overloads are well formed
    test_case = const_test_case;

    //  Confirm at() throws the std lib defined exception
    EXPECT_THROW(test_case.at(0), std::out_of_range);
    EXPECT_THROW(const_test_case.at(0), std::out_of_range);
}

TEST(EmptyArray, Bool)
{
    RunTests< bool >();
}

TEST(EmptyArray, PVoid)
{
    RunTests<void*>();
}

TEST(EmptyArray, LongDouble)
{
    RunTests< long double >();
}

TEST(EmptyArray, String)
{
    RunTests< std::string >();
}

TEST(Array, Size)
{
    stdext::array<int, 1> a1 = {{1}};
    EXPECT_EQ(1U, a1.size());
    EXPECT_FALSE(a1.empty());
}
