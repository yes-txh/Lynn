#include <gtest/gtest.h>
#include "slave/system.h"
#include "slave/container.h"

TEST(Container, ParseCpuTime) 
{
    Container a;
    const char* ss = "user 12\nsystem 4";
    EXPECT_EQ(16, static_cast<int>(a.ParseTime(ss)));
}

int main(int argc, char ** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

