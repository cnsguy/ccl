#ifndef COMMON_HPP
#define COMMON_HPP

#define CCL_CAST_HASH_TYPE int
#define CCL_CAST_HASH_NAME int_hash
#include "ccl/hash/ccl_cast_hash.h"

#include <cstdio>
#include <cstdlib>

static void test_assert_report(int result, const char *expr, const char *file, unsigned int line)
{
    if (!result)
    {
        fprintf(stderr, "TEST_ASSERT failed: %s, file %s, line %u", expr, file, line);
        abort();
    }
}

#define TEST_ASSERT(x) \
    test_assert_report((x), #x, __FILE__, __LINE__)

#endif
