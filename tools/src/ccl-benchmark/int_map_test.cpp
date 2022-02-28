#include "int_map_test.hpp"
#include "ccl/hash/ccl_murmur_hash.h"
#include "common.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <chrono>

#define CCL_HASH_MAP_NAME        int_map
#define CCL_HASH_MAP_KEY_TYPE    int
#define CCL_HASH_MAP_VALUE_TYPE  int
#define CCL_HASH_MAP_KEY_HASH_FN int_hash
#include "ccl/template/ccl_hash_map.h"

#define NUM_TO_INSERT 10000000
#define TRACK_BUCKET_GROWING

static void generate_int_vector(std::vector<int> &out_vector)
{
    for (size_t i = 0; i < NUM_TO_INSERT; ++i)
        out_vector.emplace_back(rand());
}

static void unordered_int_map_test_run(const std::vector<int> &entries, std::chrono::milliseconds &out_insert_time)
{
    std::unordered_map<int, int> map;
#ifdef TRACK_BUCKET_GROWING
    std::vector<std::tuple<size_t, size_t>> deltas;
    deltas.reserve(100);
    auto old_bucket_count = map.bucket_count();
#endif
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < NUM_TO_INSERT; ++i)
    {
        int value = entries[i];
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
            << "unordered_int_map_test_run: "
            << "bucket count: " << std::get<0>(entry) << " -> " << std::get<1>(entry) << '\n';
    }
#endif
}

static void ccl_int_map_test_run(const std::vector<int> &entries, std::chrono::milliseconds &out_insert_time)
{
    int_map map;
    TEST_ASSERT(int_map_init(&map) == CCL_SUCCESS);
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < NUM_TO_INSERT; ++i)
    {
        int value = entries[i];
        int_map_insert(&map, value, value);
    }

    auto end = std::chrono::high_resolution_clock::now();
    out_insert_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}

void int_map_test_run()
{
    std::chrono::milliseconds ccl_insert_time, unordered_map_insert_time;
    std::vector<int> entries;

    std::cout << "int_map_test_run: generating random int vector of " << NUM_TO_INSERT << " elems" << std::endl;
    generate_int_vector(entries);
    std::cout << "int_map_test_run: generating random int vector done" << std::endl;

    ccl_int_map_test_run(entries, ccl_insert_time);
    std::cout << "int_map_test_run: ccl insert time: " << ccl_insert_time.count() << "ms" << std::endl;

    unordered_int_map_test_run(entries, unordered_map_insert_time);
    std::cout << "int_map_test_run: unordered_map insert time: " << unordered_map_insert_time.count() << "ms" << std::endl;
}
