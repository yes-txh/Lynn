// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 06/28/11
// Description:

#include "eventfd.h"
#include "common/base/string/string_number.hpp"
#include "common/system/concurrency/thread.hpp"
#include <stdio.h>

void Producer(int evfd, int loop_count)
{
    for (int i = 0; i < loop_count; ++i)
    {
        eventfd_t l = i;
        eventfd_write(evfd, l);
    }
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <loop_count>.\n", argv[0]);
        return 1;
    }
    unsigned int loop_count;
    if (!StringToNumber(argv[1], &loop_count)) {
        fprintf(stderr, "loop count is not a integer.\n");
        return 1;
    }

    int evfd = eventfd(0, 0);
    if (evfd < 0)
    {
        perror("eventfd");
        return 1;
    }

    Producer(evfd, loop_count);
    close(evfd);
}
