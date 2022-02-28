#include "string_array_test.h"
#include "common.h"
#include "ccl/ccl_c_string_entry.h"
#include <stdio.h>

#define CCL_ARRAY_NAME          string_array
#define CCL_ARRAY_TYPE          char*
#define CCL_ARRAY_VALUE_COPY_FN ccl_c_string_entry_copy
#define CCL_ARRAY_VALUE_FREE_FN ccl_c_string_entry_free
#include "ccl/template/ccl_array.h"

void string_array_test_run(ccl_bool use_ex, size_t ex_init)
{
    struct string_array test;
    size_t i;
    char value[512], *value_copy;

    puts("string_array_test_run()");

    if (!use_ex)
        string_array_init(&test);
    else
        TEST_ASSERT(string_array_init_ex(&test, ex_init) == CCL_SUCCESS);

    TEST_ASSERT(string_array_preallocate(&test, ex_init * 2) == CCL_SUCCESS);

    /* Normal insert / append */
    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(string_array_insert(&test, rand() % (test.size + 1), random_string(value, sizeof value)) == CCL_SUCCESS);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(string_array_insert(&test, test.size + 100, random_string(value, sizeof value)) == CCL_INVALID_INDEX);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(string_array_append(&test, random_string(value, sizeof value)) == CCL_SUCCESS);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(string_array_insert_no_bound_check(&test, rand() % (test.size + 1), random_string(value, sizeof value)) == CCL_SUCCESS);

    /* Move insert / append */
    for (i = 0; i < 5000; ++i)
    {
        value_copy = CCL_STRDUP(random_string(value, sizeof value));
        TEST_ASSERT(string_array_insert_move(&test, rand() % (test.size + 1), value_copy) == CCL_SUCCESS);
    }

    for (i = 0; i < 5000; ++i)
    {
        value_copy = CCL_STRDUP(random_string(value, sizeof value));
        TEST_ASSERT(string_array_append_move(&test, value_copy) == CCL_SUCCESS);
    }

    for (i = 0; i < 5000; ++i)
    {
        value_copy = CCL_STRDUP(random_string(value, sizeof value));
        TEST_ASSERT(string_array_insert_no_bound_check_move(&test, rand() % (test.size + 1), value_copy) == CCL_SUCCESS);
    }

    /* Deleting */
    for (i = 0; i < 2500; ++i)
        TEST_ASSERT(string_array_remove(&test, rand() % test.size) == CCL_SUCCESS);

    for (i = 0; i < 2500; ++i)
        TEST_ASSERT(string_array_pop(&test) == CCL_SUCCESS);

    for (i = 0; i < 2500; ++i)
        string_array_remove_no_bound_check(&test, rand() % test.size);

    for (i = 0; i < 2500; ++i)
        string_array_pop_no_bound_check(&test);

    string_array_free(&test);
    printf("string_array_test_run(%hhu, %zu) completed successfully\n", use_ex, ex_init);
}
