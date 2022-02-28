#include "int_string_map_test.h"
#include "common.h"
#include "ccl/ccl_c_string_entry.h"
#include <stdio.h>

#define CCL_HASH_MAP_NAME                    int_string_map
#define CCL_HASH_MAP_KEY_TYPE                int
#define CCL_HASH_MAP_VALUE_TYPE              char*
#define CCL_HASH_MAP_KEY_HASH_FN             int_hash
#define CCL_HASH_MAP_VALUE_COPY_FN           ccl_c_string_entry_copy
#define CCL_HASH_MAP_VALUE_COPY_OVERWRITE_FN ccl_c_string_entry_copy_overwrite
#define CCL_HASH_MAP_VALUE_FREE_FN           ccl_c_string_entry_free
#include "ccl/template/ccl_hash_map.h"

void int_string_map_test_run(ccl_bool use_ex, size_t ex_init)
{
    struct int_string_map map;
    char **result;
    char value[512], *value_copy;
    int tmp;
    size_t i;

    puts("int_string_map_test_run()");

    if (!use_ex)
        TEST_ASSERT(int_string_map_init(&map) == CCL_SUCCESS);
    else
        TEST_ASSERT(int_string_map_init_ex(&map, ex_init) == CCL_SUCCESS);

    TEST_ASSERT(int_string_map_insert(&map, 42, "test1") == CCL_SUCCESS);
    TEST_ASSERT(int_string_map_insert(&map, 70, "test2") == CCL_SUCCESS);

    TEST_ASSERT(int_string_map_get(&map, &result, 42) == CCL_SUCCESS);
    TEST_ASSERT(int_string_map_get(&map, &result, 70) == CCL_SUCCESS);
    TEST_ASSERT(int_string_map_get(&map, &result, 101) == CCL_INVALID_INDEX);

    TEST_ASSERT(int_string_map_erase(&map, 42) == CCL_SUCCESS);
    TEST_ASSERT(int_string_map_erase(&map, 70) == CCL_SUCCESS);
    TEST_ASSERT(int_string_map_erase(&map, 101) == CCL_INVALID_INDEX);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(int_string_map_insert(&map, rand() % 1000, random_string(value, sizeof value)) == CCL_SUCCESS);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(int_string_map_insert_move_key(&map, rand() % 1000, random_string(value, sizeof value)) == CCL_SUCCESS);

    for (i = 0; i < 5000; ++i)
    {
        value_copy = CCL_STRDUP(random_string(value, sizeof value));
        TEST_ASSERT(value_copy != NULL);
        TEST_ASSERT(int_string_map_insert_move_value(&map, rand() % 1000, value_copy) == CCL_SUCCESS);
    }

    for (i = 0; i < 5000; ++i)
    {
        value_copy = CCL_STRDUP(random_string(value, sizeof value));
        TEST_ASSERT(value_copy != NULL);
        TEST_ASSERT(int_string_map_insert_move_both(&map, rand() % 1000, value_copy) == CCL_SUCCESS);
    }

    for (i = 0; i < 5000; ++i)
    {
        if (int_string_map_get(&map, &result, rand() % 1000) == CCL_SUCCESS)
            TEST_ASSERT(result != NULL);
    }

    for (i = 0; i < 2500; ++i)
    {
        tmp = rand() % 1000;

        if (int_string_map_has(&map, tmp))
            TEST_ASSERT(int_string_map_erase(&map, tmp) == CCL_SUCCESS);
        else
            TEST_ASSERT(int_string_map_erase(&map, tmp) == CCL_INVALID_INDEX);
    }

    int_string_map_free(&map);
    printf("int_string_map_test_run(%hhu, %zu) completed successfully\n", use_ex, ex_init);
}
