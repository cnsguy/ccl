#include "int_map_test.h"
#include "common.h"
#include <stdio.h>

#define CCL_HASH_MAP_NAME        int_map
#define CCL_HASH_MAP_KEY_TYPE    int
#define CCL_HASH_MAP_VALUE_TYPE  int
#define CCL_HASH_MAP_KEY_HASH_FN int_hash
#include "ccl/template/ccl_hash_map.h"

static void int_map_print(struct int_map *self)
{
    size_t i, l;

    for (i = 0; i < self->num_buckets; ++i)
    {
        if (!self->buckets[i].is_used)
            continue;

        printf("self->buckets[%zu] = %d\n", i, self->buckets[i].entry.value);

        for (l = 0; l < self->buckets[i].extra_entries.size; ++l)
            printf("self->buckets[%zu].extra[%zu] = %d\n", i, l, self->buckets[i].extra_entries.data[l].value);
    }
}

void int_map_test_run(ccl_bool use_ex, size_t ex_init)
{
    struct int_map map;
    size_t i;
    int *value, tmp;

    puts("int_map_test_run()");

    if (!use_ex)
        TEST_ASSERT(int_map_init(&map) == CCL_SUCCESS);
    else
        TEST_ASSERT(int_map_init_ex(&map, ex_init) == CCL_SUCCESS);

    TEST_ASSERT(int_map_insert(&map, 42, 33) == CCL_SUCCESS);
    TEST_ASSERT(int_map_insert(&map, 70, 77) == CCL_SUCCESS);

    TEST_ASSERT(int_map_get(&map, &value, 42) == CCL_SUCCESS);
    TEST_ASSERT(int_map_get(&map, &value, 70) == CCL_SUCCESS);
    TEST_ASSERT(int_map_get(&map, &value, 101) == CCL_INVALID_INDEX);

    TEST_ASSERT(int_map_erase(&map, 42) == CCL_SUCCESS);
    TEST_ASSERT(int_map_get(&map, &value, 42) == CCL_INVALID_INDEX);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(int_map_insert(&map, rand() % 1000, rand() % 1000) == CCL_SUCCESS);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(int_map_insert_move_key(&map, rand() % 1000, rand() % 1000) == CCL_SUCCESS);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(int_map_insert_move_value(&map, rand() % 1000, rand() % 1000) == CCL_SUCCESS);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(int_map_insert_move_both(&map, rand() % 1000, rand() % 1000) == CCL_SUCCESS);

    for (i = 0; i < 5000; ++i)
    {
        if (int_map_get(&map, &value, rand() % 1000) == CCL_SUCCESS)
            TEST_ASSERT(value != NULL);
    }

    for (i = 0; i < 2500; ++i)
    {
        tmp = rand() % 1000;

        if (int_map_has(&map, tmp))
            TEST_ASSERT(int_map_erase(&map, tmp) == CCL_SUCCESS);
        else
            TEST_ASSERT(int_map_erase(&map, tmp) == CCL_INVALID_INDEX);
    }

    int_map_free(&map);
    int_map_init_ex(&map, 123);

    for (i = 0; i < 1000; ++i)
        TEST_ASSERT(int_map_insert(&map, (int) i, (int) i) == CCL_SUCCESS);

    TEST_ASSERT(map.num_entries == 1000);
    int_map_free(&map);
    printf("int_map_test_run(%hhu, %zu) completed successfully\n", use_ex, ex_init);
}
