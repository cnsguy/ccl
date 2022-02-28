#include "string_map_test.hpp"
#include "ccl/ccl_c_string_entry.h"
#include "ccl/hash/ccl_djb2_hash.h"
#include "ccl/ccl_inline.h"
#include "ccl/ccl_string_entry.h"
#include "ccl/ccl_string.h"
#include "ccl/hash/ccl_murmur_hash.h"
#include "common.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>

#define TRACK_BUCKET_GROWING

static CCL_FORCE_INLINE size_t ccl_string_hash(const struct ccl_string self)
{
    return ccl_murmur_hash_64a(self.data, self.size, 0x123);
}

#define CCL_HASH_MAP_NAME                    string_map
#define CCL_HASH_MAP_KEY_TYPE                struct ccl_string
#define CCL_HASH_MAP_VALUE_TYPE              struct ccl_string
#define CCL_HASH_MAP_KEY_HASH_FN             ccl_string_hash
#define CCL_HASH_MAP_KEY_COMPARE_FN          ccl_string_entry_compare
#define CCL_HASH_MAP_KEY_COPY_FN             ccl_string_entry_copy
#define CCL_HASH_MAP_KEY_FREE_FN             ccl_string_entry_free
#define CCL_HASH_MAP_VALUE_COPY_FN           ccl_string_entry_copy
#define CCL_HASH_MAP_VALUE_COPY_OVERWRITE_FN ccl_string_entry_copy_overwrite
#define CCL_HASH_MAP_VALUE_FREE_FN           ccl_string_entry_free
#include "ccl/template/ccl_hash_map.h"

#define NUM_TO_INSERT 1000000

static inline void generate_random_string(std::string &entry)
{
    char choices[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-_%$\"'!/=()[]";

    for (size_t i = 0; i < 40 + (size_t) (rand() % 50); ++i)
        entry.push_back(choices[rand() % (sizeof choices)]);
}

static void generate_string_vector(std::vector<std::string> &out_vector, std::vector<ccl_string> &out_vector2)
{
    std::string entry;
    struct ccl_string entry2;
    entry.reserve(50);

    for (size_t i = 0; i < NUM_TO_INSERT; ++i)
    {
        generate_random_string(entry);
        out_vector.emplace_back(entry);
        TEST_ASSERT(ccl_string_init_cstr_len(&entry2, entry.c_str(), entry.length()) == CCL_SUCCESS);
        out_vector2.emplace_back(entry2);
        entry.clear();
    }
}

static void unordered_string_map_test_run(const std::vector<std::string> &entries, std::chrono::milliseconds &out_insert_time)
{
    std::unordered_map<std::string, std::string> map;
#ifdef TRACK_BUCKET_GROWING
    std::vector<std::tuple<size_t, size_t>> deltas;
    deltas.reserve(100);
    auto old_bucket_count = map.bucket_count();
#endif
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < NUM_TO_INSERT; ++i)
    {
        const std::string &value = entries[i];
        map[value] = value;

#ifdef TRACK_BUCKET_GROWING
        auto new_bucket_count = map.bucket_count();

        if (new_bucket_count != old_bucket_count)
        {
            deltas.emplace_back(old_bucket_count, new_bucket_count);
            old_bucket_count = new_bucket_count;
        }
#endif
    }

    auto end = std::chrono::high_resolution_clock::now();
    out_insert_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

#ifdef TRACK_BUCKET_GROWING
    for (auto entry : deltas)
    {
        std::cout
            << "unordered_string_map_test_run: "
            << "bucket count: " << std::get<0>(entry) << " -> " << std::get<1>(entry) << '\n';
    }
#endif
}

static void ccl_string_map_test_run(const std::vector<ccl_string> &entries, std::chrono::milliseconds &out_insert_time)
{
    struct string_map map;
    string_map_init(&map);
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < NUM_TO_INSERT; ++i)
        string_map_insert(&map, entries[i], entries[i]);

    auto end = std::chrono::high_resolution_clock::now();
    out_insert_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    string_map_free(&map);
}

void string_map_test_run()
{
    std::chrono::milliseconds ccl_insert_time, unordered_map_insert_time;
    std::vector<std::string> entries;
    std::vector<ccl_string> entries2;

    std::cout << "string_map_test_run: generating random string vector of " << NUM_TO_INSERT << " elems" << std::endl;
    generate_string_vector(entries, entries2);
    std::cout << "string_map_test_run: generating random string vector done" << std::endl;

    ccl_string_map_test_run(entries2, ccl_insert_time);
    std::cout << "string_map_test_run: ccl insert time: " << ccl_insert_time.count() << "ms" << std::endl;

    unordered_string_map_test_run(entries, unordered_map_insert_time);
    std::cout << "string_map_test_run: unordered_map insert time: " << unordered_map_insert_time.count() << "ms" << std::endl;
}
