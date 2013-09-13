// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 06/23/11
// Description:

#include "common/system/debug/hex_dump.hpp"
#include "common/file/file.h"
#include "common/file/recordio/recordio.h"

int main(int argc, char** argv) {
    scoped_ptr<File> file(File::Open(argv[1], File::ENUM_FILE_OPEN_MODE_R));
    if (!file)
        return 1;
    RecordReader reader(file.release());
    StringPiece record;
    int number = 0;
    while (reader.ReadRecord(&record)) {
        printf("=======================================================================\n");
        printf("#%d: size = %d\n", number++, (int) record.size());
        printf("-----------------------------------------------------------------------\n");
        HexDump(stdout, record.data(), record.size());
    }
}
