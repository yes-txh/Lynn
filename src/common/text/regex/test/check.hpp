#ifndef CHECH_HPP
#define CHECH_HPP

#include <stdio.h>
#include <stdlib.h>

// CHECK dies with a fatal error if condition is not true.  It is *not*
// controlled by NDEBUG, so the check will be executed regardless of
// compilation mode.  Therefore, it is safe to do things like:
//    CHECK(fp->Write(x) == 4)
#define CHECK(condition) do {                           \
    if (!(condition)) {                                 \
        fprintf(stderr, "%s:%d: Check failed: %s\n",    \
                __FILE__, __LINE__, #condition);        \
        exit(1);                                        \
    }                                                   \
} while (0)

// Dies with a fatal error if the two values are not equal.
#define CHECK_EQ(a, b)  do {                                      \
    if ( (a) != (b) ) {                                           \
        fprintf(stderr, "%s:%d: Check failed because %s != %s\n", \
                __FILE__, __LINE__, #a, #b);                      \
        exit(1);                                                  \
    }                                                             \
} while (0)

#endif // CHECH_HPP
