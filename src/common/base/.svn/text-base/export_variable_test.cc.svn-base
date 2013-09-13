// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/base/export_variable.h"
#include "gtest/gtest.h"

// export global variable
int g_test_count;
EXPORT_VARIABLE(test_count, &g_test_count);

// export global function
int CallCount()
{
    static int count;
    return ++count;
}
EXPORT_VARIABLE(call_count, CallCount);

// scoped export class member
class TestClass
{
public:
    TestClass() : m_reg("xxx", this, &TestClass::Xxx) {}
    std::string Xxx() const
    {
        return "xxx";
    }
private:
    VariableRegister m_reg;
};

int main()
{
    std::cout << "-----------------------------------------\n";
    ExportedVariables::Dump(std::cout);

    std::cout << "-----------------------------------------\n";
    ExportedVariables::Dump(std::cout);

    {
        TestClass t;
        std::cout << "-----------------------------------------\n";
        ExportedVariables::Dump(std::cout);
    }

    std::cout << "-----------------------------------------\n";
    ExportedVariables::Dump(std::cout);
}
