// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2011-06-12 20:26:58
// Description:

#include "common/system/debug/hex_dump.hpp"
#include "gtest/gtest.h"

int main()
{
static const char text[] =
    "#include <common/system/debug/hex_dump.hpp>\n"
    "#include <gtest/gtest.h>\n"
    "\n"
    "int main()\n"
    "{\n"
    "    HexDump((void*) &main, 64, (void*)main);\n"
    "}\n";

    HexDump(stdout, text, sizeof(text));
}
