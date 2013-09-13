// Copyright 2011, Tencent Inc.
// Author: Kypo Yin(kypoyin@tencent.com)
#include "common/base/global_function_register.h"
#include "gtest/gtest.h"

typedef std::string (*GetNameFunctionPtr)();

GLOBAL_REGISTER_DEFINE_REGISTRY(GetName, GetNameFunctionPtr)

static std::string GetName() {
    return "GlobalFunction";
}
GLOBAL_FUNCTION_REGISTER(GetName, GlobalFunction, GetName);

class RecordStream {
public:
    static std::string GetName() {
        return "RecordStream";
    }
};
GLOBAL_FUNCTION_REGISTER(GetName, RecordStream, RecordStream::GetName);


class TextStream:public RecordStream {
public:
    static std::string GetName() {
        return "TextStream";
    }
};
GLOBAL_FUNCTION_REGISTER(GetName, TextStream, TextStream::GetName);

class RecordIOStream:public RecordStream {
public:
    static std::string GetName() {
        return "RecordIOStream";
    }
};
GLOBAL_FUNCTION_REGISTER(GetName, RecordIOStream, RecordIOStream::GetName);

class SSTableStream:public RecordStream {
public:
    static std::string GetName() {
        return "SSTableStream";
    }
};

GLOBAL_FUNCTION_REGISTER(GetName, SSTableStream, SSTableStream::GetName);

TEST(GlobalFunctionRegister, GetFunction) {
    GetNameFunctionPtr func = NULL;

    func = GET_GLOBAL_FUNCTION(GetName, "GlobalFunction");
    ASSERT_TRUE(func != NULL);
    EXPECT_EQ(func(), std::string("GlobalFunction"));

    func = GET_GLOBAL_FUNCTION(GetName, "GlobalFunction ");
    ASSERT_TRUE(func == NULL);

    func = GET_GLOBAL_FUNCTION(GetName, "RecordStream");
    ASSERT_TRUE(func != NULL);
    EXPECT_EQ(func(), std::string("RecordStream"));

    func = GET_GLOBAL_FUNCTION(GetName, "TextStream");
    ASSERT_TRUE(func != NULL);
    EXPECT_EQ(func(), std::string("TextStream"));

    func = GET_GLOBAL_FUNCTION(GetName, "RecordIOStream");
    ASSERT_TRUE(func != NULL);
    EXPECT_EQ(func(), std::string("RecordIOStream"));

    func = GET_GLOBAL_FUNCTION(GetName, "SSTableStream");
    ASSERT_TRUE(func != NULL);
    EXPECT_EQ(func(), std::string("SSTableStream"));
}



