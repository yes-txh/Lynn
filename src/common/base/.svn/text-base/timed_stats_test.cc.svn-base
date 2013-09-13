// Copyright (c) 2010, Tencent Inc.
// All rights reserved.

#include <iostream>
#include "common/base/timed_stats.hpp"
#include "common/system/concurrency/thread.hpp"
#include "gtest/gtest.h"

using namespace std;

TEST(Stats, AccStat)
{
    TimedStats<int> stat1;
    TimedStats<double> stat2;

    for (int i = 0; i < 3601; i += 2)
    {
        stat1.AddCount(i);
        stat1.AddCount(i);
    }
    ostringstream os1;
    stat1.Print(os1);
    cout << os1.str();

    for (int i = 0; i < 10; i++)
    {
        stat2.AddValue(i, i);
    }
    ostringstream os2;
    stat2.Print(os2);
    cout << os2.str();
}
