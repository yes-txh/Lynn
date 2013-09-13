#include "common/config/ini/ini_file.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "gtest/gtest.h"

using namespace std; // NOLINT // come on, it is only a test, just for ease

#ifdef _WIN32
#define snprintf _snprintf
#endif // _WIN32

// 设置脚手架
class IniFileTest : public testing::Test
{
protected:
    virtual void SetUp()
    {
        // 设置fack的配置文件
        m_path = "in.ini";

        ofstream file;
        file.open(m_path.c_str(), ios::out);
        assert(!file.fail());

        file << "# this is header commont" << endl
            << "[address]" << endl
            << "ip=172.0.0.1" << endl
            << "; address commont" << endl
            << "port=1024" << endl
            << "[mingong]" << endl
            << "; mingong commont" << endl
            << "name=ivanhuang" << endl
            << "career=it" << endl
            << "money=0" << endl
            << "score=100.0" << endl;

        file.close();

        // 分配配置文件对象
        m_ini_file = new IniFile(m_path);
    }

    virtual void TearDown()
    {
        // 反初始化并释放内存
       delete m_ini_file;
       m_ini_file = NULL;
    }

protected:
    string  m_path;
    IniFile *m_ini_file;
};

TEST_F(IniFileTest, LoadIniFile)
{
    ASSERT_TRUE(m_ini_file->ReadFile());
}

TEST_F(IniFileTest, StartUpParam)
{
    ASSERT_TRUE(m_ini_file->ReadFile());
    ASSERT_STREQ("in.ini", (m_ini_file->Path()).c_str());
    ASSERT_TRUE(m_ini_file->IgnoreCase());
}

TEST_F(IniFileTest, DumpIniFile)
{
    ASSERT_TRUE(m_ini_file->ReadFile());
    m_ini_file->SetPath("out.ini");
    ASSERT_TRUE(m_ini_file->WriteFile(true));
}

TEST_F(IniFileTest, Key)
{
    ASSERT_TRUE(m_ini_file->ReadFile());

    int key_id = m_ini_file->FindKey("address");
    ASSERT_NE(kIDNotFound, key_id);
    ASSERT_STREQ("address", (m_ini_file->KeyName(key_id)).c_str());

    key_id = m_ini_file->FindKey("mingong");
    ASSERT_NE(kIDNotFound, key_id);
    ASSERT_STREQ("mingong", (m_ini_file->KeyName(key_id)).c_str());

    ASSERT_EQ(2u, m_ini_file->NumKeys());
}

TEST_F(IniFileTest, GetValue)
{
    ASSERT_TRUE(m_ini_file->ReadFile());

    string ip = m_ini_file->GetValue("address", "ip");
    EXPECT_STREQ("172.0.0.1", ip.c_str());

    int port = m_ini_file->GetValueI("address", "port");
    EXPECT_EQ(1024, port);

    string name = m_ini_file->GetValue("mingong", "name");
    EXPECT_STREQ("ivanhuang", name.c_str());

    string career = m_ini_file->GetValue("mingong", "career");
    EXPECT_STREQ("it", career.c_str());

    int money = m_ini_file->GetValueI("mingong", "money");
    EXPECT_EQ(0, money);

    double score = m_ini_file->GetValueF("mingong", "score");
    double diff = score - 100.0;
    EXPECT_TRUE(diff < 0.0001 && diff > -0.0001);
}
