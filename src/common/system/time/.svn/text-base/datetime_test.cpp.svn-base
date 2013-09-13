#include "gtest/gtest.h"
#include "common/system/time/datetime.hpp"
#include <locale.h>
#include <iostream>
#include <string>

using namespace std;

const char* g_init_locale = setlocale(LC_ALL, "zh_CN.GBK");

TEST(TimeSpan, TestTimeSpanConstructor)
{
    ASSERT_EQ("00:00:00", TimeSpan().ToString());
    ASSERT_EQ("01:02:03", TimeSpan(1, 2, 3).ToString());
    ASSERT_EQ("4.01:02:03", TimeSpan(4, 1, 2, 3).ToString());
    ASSERT_EQ("4.01:02:03.010000", TimeSpan(4, 1, 2, 3, 10).ToString());
    ASSERT_EQ(TimeSpan(4, 1, 2, 3, 10, 50).ToString() , "4.01:02:03.010050");
    ASSERT_EQ(TimeSpan::FromDays(1).ToString() , "1.00:00:00");
    ASSERT_EQ(TimeSpan::FromHours(2).ToString() , "02:00:00");
    ASSERT_EQ(TimeSpan::FromMinutes(3).ToString() , "00:03:00");
    ASSERT_EQ(TimeSpan::FromSeconds(4).ToString() , "00:00:04");
    ASSERT_EQ(TimeSpan::FromMilliSeconds(5).ToString() , "00:00:00.005000");
    ASSERT_EQ(TimeSpan::FromMicroSeconds(6).ToString() , "00:00:00.000006");
}

TEST(TimeSpan, TestTimeSpanAttributes)
{
    TimeSpan ts(1,2,3,4,5,6);
    ASSERT_EQ(ts.GetDays(),1);
    ASSERT_EQ(ts.GetHours(),2);
    ASSERT_EQ(ts.GetMinutes(),3);
    ASSERT_EQ(ts.GetSeconds(),4);
    ASSERT_EQ(ts.GetMilliSeconds(),5);
    ASSERT_EQ(ts.GetMicroSeconds(),6);

    ASSERT_TRUE(abs(ts.GetTotalDays()-1.085463)<0.000001);
    ASSERT_TRUE(abs(ts.GetTotalHours()-26.0511125017)<0.000001);
    ASSERT_TRUE(abs(ts.GetTotalMinutes()-1563.0667501)<0.000001);
    EXPECT_DOUBLE_EQ(ts.GetTotalSeconds(),93784.005006);
    EXPECT_DOUBLE_EQ(ts.GetTotalMilliSeconds(), 93784005.006);
    EXPECT_DOUBLE_EQ(ts.GetTotalMicroSeconds(),93784005006.0);
}

TEST(TimeSpan, TestTimeSpanOperator)
{
    TimeSpan ts1(1,2,3,4,5,6);
    TimeSpan ts2(7,8,9,10,11,12);

    ASSERT_TRUE(ts2-ts1==TimeSpan(6,6,6,6,6,6));
    ASSERT_TRUE(ts2+ts1==TimeSpan(8,10,12,14,16,18));
    ASSERT_TRUE(-ts1==TimeSpan(-1,-2,-3,-4,-5,-6));
}

TEST(DateTime, TestDateTimeConstructor)
{
    ASSERT_EQ("2010年5月3日 0:00:00 +08:00", DateTime(2010, 5, 3).ToString());
    ASSERT_EQ("2010年5月3日 19:14:10.10001 +08:00", DateTime(2010, 5, 3, 19, 14, 10, 100, 10).ToString());
}

TEST(DateTime, TestDateTimeParse)
{
    DateTime dt ;
    dt = DateTime::Parse("2010年5月3日 15:59:10 +08:00");
    ASSERT_TRUE(dt==DateTime(2010,5,3,15,59,10));
    dt = DateTime::Parse("2010-05-03 15:59");
    ASSERT_TRUE(dt == DateTime(2010,5,3,15,59));
    dt = DateTime::Parse("2010-05-03");
    ASSERT_TRUE(dt.GetDay() == DateTime(2010, 5, 3).GetDay());
    dt = DateTime::Parse("2010年5月");
    ASSERT_EQ(2010, dt.GetYear());
    ASSERT_EQ(5, dt.GetMonth());
    ASSERT_TRUE(DateTime::Parse("15:59:10").GetTime()==DateTime(2010,5,1,15,59,10).GetTime());

    dt = DateTime::Parse("2010年5月3日 15:59:10");
    ASSERT_TRUE(dt == DateTime(2010, 5, 3, 15, 59, 10));
    std::string s = "20100503 15:59:10.010019 +08:00";
    dt = DateTime::Parse(s, "yyyyMMdd HH:mm:ss.FFFFFF zzz");
    // ASSERT_EQ(s, dt.ToString());
}

TEST(DateTime, TestDateTimeOperator)
{
    DateTime dt1(2010,5,3,21,03,10,100,10);
    DateTime dt2(2010,5,3,21,04,25,400,51);
    TimeSpan ts(0,0,1,15,300,41);
    ASSERT_TRUE(dt2-dt1==ts);
    ASSERT_TRUE(dt1+ts==dt2);
    ASSERT_TRUE(dt2-ts==dt1);
}

#define ASSERT_EQ_REVERSE(x, y) ASSERT_EQ(y, x)

TEST(DateTime, TestDateTimeToString)
{
    DateTime dt(2010, 5, 3, 15, 59, 10, 100, 50);
    ASSERT_EQ_REVERSE("2010年5月3日 15:59:10.10005 +08:00", dt.ToString());
    ASSERT_EQ_REVERSE("2010-5-3", dt.ToString("d"));
    ASSERT_EQ_REVERSE(dt.ToString("D"), "2010年5月3日");
    ASSERT_EQ_REVERSE(dt.ToString("f"), "2010年5月3日 15:59");
    ASSERT_EQ_REVERSE(dt.ToString("F"), "2010年5月3日 15:59:10");
    ASSERT_EQ_REVERSE(dt.ToString("g"), "2010-5-3 15:59");
    ASSERT_EQ_REVERSE(dt.ToString("G"), "2010-5-3 15:59:10");
    ASSERT_EQ_REVERSE(dt.ToString("m"), "5月3日");
    ASSERT_EQ_REVERSE(dt.ToString("M"), "5月3日");
    ASSERT_EQ_REVERSE(dt.ToString("o"), "2010-05-03T15:59:10.10005+08");
    ASSERT_EQ_REVERSE(dt.ToString("O"), "2010-05-03T15:59:10.10005+08");
    ASSERT_EQ_REVERSE(dt.ToString("r"), "一, 03 五月 2010 15:59:10 GMT");
    ASSERT_EQ_REVERSE(dt.ToString("R"), "一, 03 五月 2010 15:59:10 GMT");
    ASSERT_EQ_REVERSE(dt.ToString("s"), "2010-05-03T15:59:10");
    ASSERT_EQ_REVERSE(dt.ToString("t"), "15:59");
    ASSERT_EQ_REVERSE(dt.ToString("T"), "15:59:10");
    ASSERT_EQ_REVERSE(dt.ToString("u"), "2010-05-03 07:59:10Z");
    ASSERT_EQ_REVERSE(dt.ToString("U"), "2010年5月3日 7:59:10");
    ASSERT_EQ_REVERSE(dt.ToString("y"), "2010年5月");
    ASSERT_EQ_REVERSE(dt.ToString("Y"), "2010年5月");

    ASSERT_EQ_REVERSE(dt.ToString("%d"), "3");
    ASSERT_EQ_REVERSE(dt.ToString("dd"), "03");
    ASSERT_EQ_REVERSE(dt.ToString("ddd"), "一");
    ASSERT_EQ_REVERSE(dt.ToString("dddd"), "星期一");
    ASSERT_EQ_REVERSE(dt.ToString("%f"), "1");
    ASSERT_EQ_REVERSE(dt.ToString("ff"), "10");
    ASSERT_EQ_REVERSE(dt.ToString("fff"), "100");
    ASSERT_EQ_REVERSE(dt.ToString("ffff"), "1000");
    ASSERT_EQ_REVERSE(dt.ToString("fffff"), "10005");
    ASSERT_EQ_REVERSE(dt.ToString("ffffff"), "100050");
    ASSERT_EQ_REVERSE(dt.ToString("%F"), "1");
    ASSERT_EQ_REVERSE(dt.ToString("FF"), "1");
    ASSERT_EQ_REVERSE(dt.ToString("FFF"), "1");
    ASSERT_EQ_REVERSE(dt.ToString("FFFF"), "1");
    ASSERT_EQ_REVERSE(dt.ToString("FFFFF"), "10005");
    ASSERT_EQ_REVERSE(dt.ToString("FFFFFF"), "10005");
    ASSERT_EQ_REVERSE(dt.ToString("gg"), "公元");
    ASSERT_EQ_REVERSE(dt.ToString("%h"), "3");
    ASSERT_EQ_REVERSE(dt.ToString("hh"), "03");
    ASSERT_EQ_REVERSE(dt.ToString("%H"), "15");
    ASSERT_EQ_REVERSE(dt.ToString("HH"), "15");
    ASSERT_EQ_REVERSE(dt.ToString("%m"), "59");
    ASSERT_EQ_REVERSE(dt.ToString("mm"), "59");
    ASSERT_EQ_REVERSE(dt.ToString("%M"), "5");
    ASSERT_EQ_REVERSE(dt.ToString("MM"), "05");
    ASSERT_EQ_REVERSE(dt.ToString("MMM"), "五月");
    ASSERT_EQ_REVERSE(dt.ToString("MMMM"), "五月");
    ASSERT_EQ_REVERSE(dt.ToString("%s"), "10");
    ASSERT_EQ_REVERSE(dt.ToString("ss"), "10");
    ASSERT_EQ_REVERSE(dt.ToString("%t"), "下");
    ASSERT_EQ_REVERSE(dt.ToString("tt"), "下午");
    ASSERT_EQ_REVERSE(dt.ToString("%y"), "10");
    ASSERT_EQ_REVERSE(dt.ToString("yy"), "10");
    ASSERT_EQ_REVERSE(dt.ToString("yyy"), "2010");
    ASSERT_EQ_REVERSE(dt.ToString("yyyy"), "2010");
    ASSERT_EQ_REVERSE(dt.ToString("yyyyy"), "02010");
    ASSERT_EQ_REVERSE(dt.ToString("yyyyyy"), "02010");
    ASSERT_EQ_REVERSE(dt.ToString("%z"), "+8");
    ASSERT_EQ_REVERSE(dt.ToString("zz"), "+08");
    ASSERT_EQ_REVERSE(dt.ToString("zzz"), "+08:00");
    ASSERT_EQ_REVERSE(dt.ToString("yyyyMM"), "201005");
}

