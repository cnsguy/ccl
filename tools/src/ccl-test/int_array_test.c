#include "int_array_test.h"
#include "common.h"
#include <stdio.h>

#define CCL_ARRAY_NAME int_array
#define CCL_ARRAY_TYPE int
#include "ccl/template/ccl_array.h"

void int_array_test_run(ccl_bool use_ex, size_t ex_init)
{
    struct int_array test;
    size_t i;

    if (!use_ex)
        int_array_init(&test);
    else
        TEST_ASSERT(int_array_init_ex(&test, ex_init) == CCL_SUCCESS);

    TEST_ASSERT(int_array_preallocate(&test, ex_init * 2) == CCL_SUCCESS);

    /* Normal insert / append */
    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(int_array_insert(&test, rand() % (test.size + 1), (int) i) == CCL_SUCCESS);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(int_array_insert(&test, test.size + 100, (int) i) == CCL_INVALID_INDEX);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(int_array_append(&test, (int) i) == CCL_SUCCESS);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(int_array_insert_no_bound_check(&test, rand() % (test.size + 1), (int) i) == CCL_SUCCESS);

    /* Move insert / append */
    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(int_array_insert_move(&test, rand() % (test.size + 1), (int) i) == CCL_SUCCESS);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(int_array_append_move(&test, (int) i) == CCL_SUCCESS);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(int_array_insert_no_bound_check_move(&test, rand() % (test.size + 1), (int) i) == CCL_SUCCESS);

    /* Deleting */
    for (i = 0; i < 2500; ++i)
        TEST_ASSERT(int_array_remove(&test, rand() % test.size) == CCL_SUCCESS);

    for (i = 0; i < 2500; ++i)
        TEST_ASSERT(int_array_pop(&test) == CCL_SUCCESS);

    for (i = 0; i < 2500; ++i)
        int_array_remove_no_bound_check(&test, rand() % test.size);

    for (i = 0; i < 2500; ++i)
        int_array_pop_no_bound_check(&test);

    int_array_free(&test);
    printf("int_array_test_run(%hhu, %zu) completed successfully\n", use_ex, ex_init);
}
