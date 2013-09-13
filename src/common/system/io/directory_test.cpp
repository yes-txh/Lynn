#include "common/system/io/directory.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include "common/base/compatible/stdlib.h"
#include "common/system/io/file.hpp"
#include "common/system/io/path.hpp"

using namespace std;
using namespace io;

TEST(DirectoryTest, Ops)
{
    string current_dir = directory::GetCurrentDir();
    ASSERT_EQ(directory::ToUnixFormat(directory::ToWindowsFormat(current_dir)), current_dir);

    string tmp_dir = "_test_dir_";
    ASSERT_FALSE(directory::Exists(tmp_dir));
    directory::Create(tmp_dir);
    ASSERT_TRUE(directory::Exists(tmp_dir));

    string root_dir = "/root";
    ASSERT_TRUE(directory::IsReadable(tmp_dir));
    ASSERT_FALSE(directory::IsReadable(root_dir));
    ASSERT_TRUE(directory::IsWritable(tmp_dir));
    ASSERT_FALSE(directory::IsWritable(root_dir));

    string new_current = current_dir + "/" + tmp_dir;
    ASSERT_FALSE(directory::SetCurrentDir(root_dir));
    directory::SetCurrentDir(new_current);
    ASSERT_EQ(directory::GetCurrentDir(), new_current);
    directory::SetCurrentDir("..");
    ASSERT_EQ(directory::GetCurrentDir(), current_dir);

    directory::Delete(tmp_dir);
    ASSERT_FALSE(directory::Exists(tmp_dir));

    vector<string> files;
    cout << "Files under current dir:\n*****************\n";
    directory::GetFiles(current_dir, &files);
    for (size_t i = 0; i < files.size(); i++)
    {
        cout <<  files[i] << endl;
    }
    cout << "Dirs under current dir:\n*****************\n";
    directory::GetSubDirs(current_dir, &files);
    for (size_t i = 0; i < files.size(); i++)
    {
        cout << files[i] << endl;
    }
    cout << "All files under current dir:\n*****************\n";
    directory::GetAllFiles(current_dir, &files);
    for (size_t i = 0; i < files.size(); i++)
    {
        cout <<  files[i] << endl;
    }
    cout << "All dirs under current dir:\n*****************\n";
    directory::GetAllSubDirs(current_dir, &files);
    for (size_t i = 0; i < files.size(); i++)
    {
        cout << files[i] << endl;
    }
}

TEST(DirectoryIteratorTest, Ops)
{
    string str = ".";
    io::DirectoryIterator iter;

    if (!iter.Open(str))
    {
        cout << "open failed.\n";
        return;
    }

    cout << "All dir and files:\n==============\n";
    while (!iter.IsEnd())
    {
        cout << iter.Name() << endl;
        cout << "FullName: " << iter.FullPath() << endl;
        iter.Next();
    }
    iter.Close();

    cout << "All files:\n==============\n";
    if (!iter.Open(str, DirectoryIterator::FILE))
    {
        cout << "open failed.\n";
        return;
    }
    while (!iter.IsEnd())
    {
        cout << iter.Name() << endl;
        cout << "FullName: " << iter.FullPath() << endl;
        iter.Next();
    }
    iter.Close();
    cout << "All cpps:\n==============\n";

    if (!iter.Open(str, DirectoryIterator::FILE, "*.cpp"))
    {
        cout << "open failed.\n";
        return;
    }
    while (!iter.IsEnd())
    {
        cout << iter.Name() << endl;
        cout << "FullName: " << iter.FullPath() << endl;
        iter.Next();
    }
    iter.Close();
    //directory::RecursiveDelete("test");
}


