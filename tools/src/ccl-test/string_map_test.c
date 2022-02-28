#include "string_map_test.h"
#include "common.h"
#include "ccl/ccl_c_string_entry.h"
#include "ccl/hash/ccl_djb2_hash.h"
#include <stdio.h>

#define CCL_HASH_MAP_NAME                    string_map
#define CCL_HASH_MAP_KEY_TYPE                char*
#define CCL_HASH_MAP_VALUE_TYPE              char*
#define CCL_HASH_MAP_KEY_HASH_FN             ccl_djb2_hash
#define CCL_HASH_MAP_KEY_COMPARE_FN          ccl_c_string_entry_compare
#define CCL_HASH_MAP_KEY_COPY_FN             ccl_c_string_entry_copy
#define CCL_HASH_MAP_KEY_FREE_FN             ccl_c_string_entry_free
#define CCL_HASH_MAP_VALUE_COPY_FN           ccl_c_string_entry_copy
#define CCL_HASH_MAP_VALUE_COPY_OVERWRITE_FN ccl_c_string_entry_copy_overwrite
#define CCL_HASH_MAP_VALUE_FREE_FN           ccl_c_string_entry_free
#include "ccl/template/ccl_hash_map.h"

static void string_map_print(struct string_map *self)
{
    size_t i, l;

    puts("===========[map content]============");

    for (i = 0; i < self->num_buckets; ++i)
    {
        if (!self->buckets[i].is_used)
            continue;

        printf("self->buckets[%zu] = %s\n", i, self->buckets[i].entry.value);

        for (l = 0; l < self->buckets[i].extra_entries.size; ++l)
            printf("self->buckets[%zu].extra[%zu] = %s\n", i, l, self->buckets[i].extra_entries.data[l].value);
    }

    puts("====================================");
}

void string_map_test_run(ccl_bool use_ex, size_t ex_init)
{
    struct string_map map;
    char **result;
    char key[512], value[512], *key_copy, *value_copy;
    size_t i;

    puts("string_map_test_run()");

    if (!use_ex)
        TEST_ASSERT(string_map_init(&map) == CCL_SUCCESS);
    else
        TEST_ASSERT(string_map_init_ex(&map, ex_init) == CCL_SUCCESS);

    TEST_ASSERT(string_map_insert(&map, "test1", "hello") == CCL_SUCCESS);
    TEST_ASSERT(string_map_insert(&map, "test2", "lol") == CCL_SUCCESS);

    TEST_ASSERT(string_map_get(&map, &result, "test1") == CCL_SUCCESS);
    TEST_ASSERT(string_map_get(&map, &result, "test2") == CCL_SUCCESS);
    TEST_ASSERT(string_map_get(&map, &result, "test3") == CCL_INVALID_INDEX);

    TEST_ASSERT(string_map_erase(&map, "test1") == CCL_SUCCESS);
    TEST_ASSERT(string_map_erase(&map, "test2") == CCL_SUCCESS);
    TEST_ASSERT(string_map_erase(&map, "test3") == CCL_INVALID_INDEX);

    for (i = 0; i < 5000; ++i)
        TEST_ASSERT(string_map_insert(&map, random_string(key, sizeof key), random_string(value, sizeof value)) == CCL_SUCCESS);

    for (i = 0; i < 5000; ++i)
    {
        key_copy = CCL_STRDUP(random_string(key, sizeof key));
        TEST_ASSERT(key_copy != NULL);
        TEST_ASSERT(string_map_insert_move_key(&map, key_copy, random_string(value, sizeof value)) == CCL_SUCCESS);
    }

    for (i = 0; i < 5000; ++i)
    {
        value_copy = CCL_STRDUP(random_string(value, sizeof value));
        TEST_ASSERT(value_copy != NULL);
        TEST_ASSERT(string_map_insert_move_value(&map, random_string(key, sizeof key), value_copy) == CCL_SUCCESS);
    }

    for (i = 0; i < 5000; ++i)
    {
        key_copy = CCL_STRDUP(random_string(key, sizeof key));
        value_copy = CCL_STRDUP(random_string(value, sizeof value));
        TEST_ASSERT(key_copy != NULL);
        TEST_ASSERT(value_copy != NULL);
        TEST_ASSERT(string_map_insert_move_both(&map, key_copy, value_copy) == CCL_SUCCESS);
    }

    for (i = 0; i < 5000; ++i)
    {
        if (string_map_get(&map, &result, random_string(key, sizeof key)) == CCL_SUCCESS)
            TEST_ASSERT(result != NULL);
    }

    for (i = 0; i < 2500; ++i)
    {
        (void) random_string(key, sizeof key);

        if (string_map_has(&map, key))
            TEST_ASSERT(string_map_erase(&map, key) == CCL_SUCCESS);
        else
            TEST_ASSERT(string_map_erase(&map, key) == CCL_INVALID_INDEX);
    }

    string_map_free(&map);
    printf("string_map_test_run(%hhu, %zu) completed successfully\n", use_ex, ex_init);
}
