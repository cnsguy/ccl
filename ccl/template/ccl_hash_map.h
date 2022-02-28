/*
IN:
 - CCL_HASH_MAP_NAME: Name of the struct to generate
 - CCL_HASH_MAP_KEY_TYPE: Type of the stored key
 - CCL_HASH_MAP_VALUE_TYPE: Type of the stored value
 - CCL_HASH_MAP_KEY_HASH_FN: Function for hashing keys
 - (optional) CCL_HASH_MAP_KEY_COMPARE_FN: Function for comparing keys
 - (optional) CCL_HASH_MAP_KEY_COPY_FN: Function for copying keys on insert
 - (optional) CCL_HASH_MAP_VALUE_COPY_FN: Function for copying values on insert
 - (optional) CCL_HASH_MAP_VALUE_COPY_OVERWRITE_FN: Function for copying a value on insert and freeing the existing one when overwriting - required if a value copy fn is defined
 - (optional) CCL_HASH_MAP_KEY_FREE_FN: Function for freeing keys in dtor
 - (optional) CCL_HASH_MAP_VALUE_FREE_FN: Function for freeing values in dtor
 - (optional) CCL_HASH_MAP_USE_EXISTING_STRUCT: Don't define our own struct, instead just refer to user-defined one
 - (optional) CCL_HASH_MAP_DEFAULT_NUM_BUCKETS
 - (optional) CCL_HASH_MAP_SHOULD_GROW
 - (optional) CCL_HASH_MAP_SHOULD_SHRINK
 - (optional) CCL_HASH_MAP_CALC_GROW
 - (optional) CCL_HASH_MAP_CALC_SHRINK
*/

#ifndef CCL_HASH_MAP_NAME
#error "Please define CCL_HASH_MAP_NAME before including this file."
#endif

#ifndef CCL_HASH_MAP_KEY_TYPE
#error "Please define CCL_HASH_MAP_KEY_TYPE before including this file."
#endif

#ifndef CCL_HASH_MAP_VALUE_TYPE
#error "Please define CCL_HASH_MAP_VALUE_TYPE before including this file."
#endif

#ifndef CCL_HASH_MAP_KEY_HASH_FN
#error "Please define CCL_HASH_MAP_KEY_HASH_FN before including this file."
#endif

#if defined(CCL_HASH_MAP_VALUE_COPY_FN) && !defined(CCL_HASH_MAP_VALUE_COPY_OVERWRITE_FN)
#error "Defining CCL_HASH_MAP_VALUE_COPY_OVERWRITE_FN is required if you define CCL_HASH_MAP_VALUE_COPY_FN."
#endif

#include "../ccl_bool.h"
#include "../ccl_defs.h"
#include "../ccl_likely.h"
#include "../ccl_inline.h"
#include "../ccl_macro_utility.h"
#include "../ccl_result.h"

#ifndef __cplusplus
#include <stdlib.h>
#include <string.h>
#else
#include <cstdlib>
#include <cstring>
#endif

#define CCL_HASH_MAP_ENTRY_NAME \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_NAME, _entry)
#define CCL_HASH_MAP_ENTRY_ARRAY_NAME \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_ENTRY_NAME, _array)
#define CCL_HASH_MAP_BUCKET_NAME \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_NAME, _bucket)
#define CCL_HASH_MAP_INIT \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_NAME, _init)
#define CCL_HASH_MAP_INIT_EX \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_NAME, _init_ex)
#define CCL_HASH_MAP_FREE \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_NAME, _free)
#define CCL_HASH_MAP_FREE_WITHOUT_DTOR \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_NAME, _free_without_dtor)
#define CCL_HASH_MAP_INSERT_BASE \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_NAME, _insert)
#define CCL_HASH_MAP_INSERT_MOVE_BOTH \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_NAME, _insert_move_both)
#define CCL_HASH_MAP_ERASE \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_NAME, _erase)
#define CCL_HASH_MAP_GET \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_NAME, _get)
#define CCL_HASH_MAP_HAS \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_NAME, _has)
#define CCL_HASH_MAP_BUCKET_MAX_CAPACITY \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_BUCKET_NAME, _max_capacity)
#define CCL_HASH_MAP_RESIZE \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_NAME, resize)
#define CCL_HASH_MAP_MAX_NUM_BUCKETS \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_NAME, _max_num_buckets)

#define CCL_HASH_MAP_ENTRY_ARRAY_INIT \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_ENTRY_ARRAY_NAME, _init)
#define CCL_HASH_MAP_ENTRY_ARRAY_FREE \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_ENTRY_ARRAY_NAME, _free)
#define CCL_HASH_MAP_ENTRY_ARRAY_FREE_WITHOUT_DTOR \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_ENTRY_ARRAY_NAME, _free_without_dtor)
#define CCL_HASH_MAP_ENTRY_ARRAY_GROW_CAPACITY \
    CCL_MACRO_CONCAT3(_, CCL_HASH_MAP_ENTRY_ARRAY_NAME, _grow_capacity)
#define CCL_HASH_MAP_ENTRY_ARRAY_REMOVE_NO_BOUND_CHECK \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_ENTRY_ARRAY_NAME, _remove_no_bound_check)
#define CCL_HASH_MAP_ENTRY_ARRAY_REMOVE_NO_BOUND_CHECK_NO_DTOR \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_ENTRY_ARRAY_NAME, _remove_no_bound_check_no_dtor)

#define CCL_HASH_MAP_BUCKET_INIT \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_BUCKET_NAME, _init)
#define CCL_HASH_MAP_BUCKET_FREE \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_BUCKET_NAME, _free)
#define CCL_HASH_MAP_BUCKET_FREE_WITHOUT_DTOR \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_BUCKET_NAME, _free_without_dtor)

#ifndef CCL_HASH_MAP_KEY_COPY_FN
#define CCL_HASH_MAP_COPY_KEY(dst_ptr, src) \
    ((*(dst_ptr)) = (src))
#define CCL_TRY_HASH_MAP_TRY_COPY_KEY_OTHERWISE(dst_ptr, src) \
    CCL_HASH_MAP_COPY_KEY((dst_ptr), (src)); if (0)
#else
#define CCL_HASH_MAP_COPY_KEY(dst_ptr, src) \
    CCL_HASH_MAP_KEY_COPY_FN((dst_ptr), (src))
#define CCL_TRY_HASH_MAP_TRY_COPY_KEY_OTHERWISE(dst_ptr, src) \
    CCL_IF_UNLIKELY (!CCL_HASH_MAP_KEY_COPY_FN((dst_ptr), (src)))
#endif

#ifndef CCL_HASH_MAP_KEY_COMPARE_FN
#define CCL_HASH_MAP_KEY_COMPARE(k1, k2) \
    ((k1) == (k2))
#else
#define CCL_HASH_MAP_KEY_COMPARE(k1, k2) \
    CCL_HASH_MAP_KEY_COMPARE_FN((const CCL_HASH_MAP_KEY_TYPE) (k1), (k2))
#endif

#ifdef CCL_HASH_MAP_KEY_FREE_FN
#define CCL_HASH_MAP_FREE_KEY(self) \
    CCL_HASH_MAP_KEY_FREE_FN((self))
#else
#define CCL_HASH_MAP_FREE_KEY(self) \
    ;
#endif

#ifndef CCL_HASH_MAP_VALUE_COPY_FN
#define CCL_HASH_MAP_COPY_VALUE(dst_ptr, src) \
    ((*(dst_ptr)) = (src))
#define CCL_TRY_HASH_MAP_TRY_COPY_VALUE_OTHERWISE(dst_ptr, src) \
    CCL_HASH_MAP_COPY_VALUE((dst_ptr), (src)); if (0)
#else
#define CCL_HASH_MAP_COPY_VALUE(dst_ptr, src) \
    CCL_HASH_MAP_VALUE_COPY_FN((dst_ptr), (src))
#define CCL_TRY_HASH_MAP_TRY_COPY_VALUE_OTHERWISE(dst_ptr, src) \
    CCL_IF_UNLIKELY (!CCL_HASH_MAP_VALUE_COPY_FN((dst_ptr), (src)))
#endif

#ifndef CCL_HASH_MAP_VALUE_COMPARE_FN
#define CCL_HASH_MAP_VALUE_COMPARE(k1, k2) \
    ((k1) == (k2))
#else
#define CCL_HASH_MAP_VALUE_COMPARE(k1, k2) \
    CCL_HASH_MAP_VALUE_COMPARE_FN((const CCL_HASH_MAP_VALUE_TYPE) (k1), (k2))
#endif

#ifdef CCL_HASH_MAP_VALUE_FREE_FN
#define CCL_HASH_MAP_FREE_VALUE(self) \
    CCL_HASH_MAP_VALUE_FREE_FN((self))
#else
#define CCL_HASH_MAP_FREE_VALUE(self) \
    ;
#endif

struct CCL_HASH_MAP_ENTRY_NAME
{
    CCL_HASH_MAP_KEY_TYPE key;
    CCL_HASH_MAP_VALUE_TYPE value;
};

#if defined(CCL_HASH_MAP_KEY_FREE_FN) || defined(CCL_HASH_MAP_VALUE_FREE_FN)
#define CCL_HASH_MAP_ENTRY_FREE_FN \
    CCL_MACRO_CONCAT3(_, CCL_HASH_MAP_ENTRY_NAME, _free)

static void CCL_HASH_MAP_ENTRY_FREE_FN(struct CCL_HASH_MAP_ENTRY_NAME *self)
{
    CCL_HASH_MAP_FREE_KEY(&self->key);
    CCL_HASH_MAP_FREE_VALUE(&self->value);
}

#define CCL_ARRAY_VALUE_FREE_FN CCL_HASH_MAP_ENTRY_FREE_FN
#endif

#define CCL_ARRAY_NAME CCL_HASH_MAP_ENTRY_ARRAY_NAME
#define CCL_ARRAY_TYPE struct CCL_HASH_MAP_ENTRY_NAME
#include "ccl_array.h"

struct CCL_HASH_MAP_BUCKET_NAME
{
    ccl_bool is_used;
    struct CCL_HASH_MAP_ENTRY_NAME entry;
    struct CCL_HASH_MAP_ENTRY_ARRAY_NAME extra_entries;
};

static void CCL_HASH_MAP_BUCKET_INIT(struct CCL_HASH_MAP_BUCKET_NAME *self)
{
    self->is_used = 0;
    CCL_HASH_MAP_ENTRY_ARRAY_INIT(&self->extra_entries);
}

static void CCL_HASH_MAP_BUCKET_FREE(struct CCL_HASH_MAP_BUCKET_NAME *self)
{
#if defined(CCL_HASH_MAP_KEY_FREE_FN) || defined(CCL_HASH_MAP_VALUE_FREE_FN)
    if (self->is_used)
        CCL_HASH_MAP_ENTRY_FREE_FN(&self->entry);
#endif

    CCL_HASH_MAP_ENTRY_ARRAY_FREE(&self->extra_entries);
}

static void CCL_HASH_MAP_BUCKET_FREE_WITHOUT_DTOR(struct CCL_HASH_MAP_BUCKET_NAME *self)
{
    CCL_HASH_MAP_ENTRY_ARRAY_FREE_WITHOUT_DTOR(&self->extra_entries);
}

#ifndef CCL_HASH_MAP_USE_EXISTING_STRUCT
struct CCL_HASH_MAP_NAME
{
    struct CCL_HASH_MAP_BUCKET_NAME *buckets;
    size_t num_buckets;
    size_t num_entries;
};
#endif

#ifndef CCL_HASH_MAP_DEFAULT_NUM_BUCKETS
#define CCL_HASH_MAP_DEFAULT_NUM_BUCKETS 8
#endif

static const size_t CCL_HASH_MAP_MAX_NUM_BUCKETS =
    SIZE_MAX / sizeof(struct CCL_HASH_MAP_BUCKET_NAME);

static ccl_result CCL_HASH_MAP_INIT_EX(struct CCL_HASH_MAP_NAME *self, size_t num_buckets)
{
    struct CCL_HASH_MAP_BUCKET_NAME *data;
    size_t i;

    CCL_IF_UNLIKELY (num_buckets > CCL_HASH_MAP_MAX_NUM_BUCKETS)
        return CCL_WOULD_OVERFLOW;

    data = (struct CCL_HASH_MAP_BUCKET_NAME*)
        malloc(num_buckets * sizeof(struct CCL_HASH_MAP_BUCKET_NAME));

    CCL_IF_UNLIKELY (data == NULL)
        return CCL_ALLOC_FAIL;

    self->num_buckets = num_buckets;
    self->num_entries = 0;
    self->buckets = data;

    for (i = 0; i < self->num_buckets; ++i)
        CCL_HASH_MAP_BUCKET_INIT(&self->buckets[i]);

    return CCL_SUCCESS;
}

static ccl_result CCL_HASH_MAP_INIT(struct CCL_HASH_MAP_NAME *self)
{
   return CCL_HASH_MAP_INIT_EX(self, CCL_HASH_MAP_DEFAULT_NUM_BUCKETS);
}

static void CCL_HASH_MAP_FREE_WITHOUT_DTOR(struct CCL_HASH_MAP_NAME *self)
{
    size_t i;

    for (i = 0; i < self->num_buckets; ++i)
        CCL_HASH_MAP_BUCKET_FREE_WITHOUT_DTOR(&self->buckets[i]);

    free(self->buckets);
}

static void CCL_HASH_MAP_FREE(struct CCL_HASH_MAP_NAME *self)
{
    size_t i;

    for (i = 0; i < self->num_buckets; ++i)
        CCL_HASH_MAP_BUCKET_FREE(&self->buckets[i]);

    free(self->buckets);
}

#ifndef CCL_HASH_MAP_SHOULD_GROW
#define CCL_HASH_MAP_SHOULD_GROW(num_entries, num_buckets) \
    ((num_entries) == (num_buckets))
#endif

#ifndef CCL_HASH_MAP_CALC_GROW
#define CCL_HASH_MAP_CALC_NEW_NUM_BUCKETS_FOR_GROWING \
    CCL_MACRO_CONCAT3(_, CCL_HASH_MAP_NAME, _calc_new_num_buckets_for_growing)

static CCL_FORCE_INLINE size_t CCL_HASH_MAP_CALC_NEW_NUM_BUCKETS_FOR_GROWING(struct CCL_HASH_MAP_NAME *self)
{
    CCL_IF_UNLIKELY (self->num_buckets == 0)
        return CCL_HASH_MAP_DEFAULT_NUM_BUCKETS;

    CCL_IF_UNLIKELY (self->num_buckets > (CCL_HASH_MAP_MAX_NUM_BUCKETS >> 3))
        return CCL_HASH_MAP_MAX_NUM_BUCKETS;

    return self->num_buckets << 3;
}

#define CCL_HASH_MAP_CALC_GROW(self) \
    CCL_HASH_MAP_CALC_NEW_NUM_BUCKETS_FOR_GROWING((self))
#endif

#ifndef CCL_HASH_MAP_SHOULD_SHRINK
#define CCL_HASH_MAP_SHOULD_SHRINK(num_entries, num_buckets) \
    (0)
#endif

#ifndef CCL_HASH_MAP_CALC_SHRINK
#define CCL_HASH_MAP_CALC_NEW_NUM_BUCKETS_FOR_SHRINKING \
    CCL_MACRO_CONCAT3(_, CCL_HASH_MAP_NAME, _calc_new_num_buckets_for_shrinking)

static CCL_SUGGEST_INLINE size_t CCL_HASH_MAP_CALC_NEW_NUM_BUCKETS_FOR_SHRINKING(struct CCL_HASH_MAP_NAME *self)
{
    if (self->num_buckets <= CCL_HASH_MAP_DEFAULT_NUM_BUCKETS)
        return CCL_HASH_MAP_DEFAULT_NUM_BUCKETS;

    return self->num_buckets / 2;
}

#define CCL_HASH_MAP_CALC_SHRINK(self) \
    CCL_HASH_MAP_CALC_NEW_NUM_BUCKETS_FOR_SHRINKING(self)
#endif

static ccl_result CCL_HASH_MAP_RESIZE(struct CCL_HASH_MAP_NAME *self, size_t new_num_buckets);

/* Normal insert, copy everything */
#define CCL_HASH_MAP_INSERT \
    CCL_HASH_MAP_INSERT_BASE
#include "private/ccl_hash_map_insert.h"

/* Move key only, copy value */
#define CCL_HASH_MAP_INSERT \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_INSERT_BASE, _move_key)
#define CCL_HASH_MAP_INSERT_MOVE_KEY
#include "private/ccl_hash_map_insert.h"

/* Move value only, copy key */
#define CCL_HASH_MAP_INSERT \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_INSERT_BASE, _move_value)
#define CCL_HASH_MAP_INSERT_MOVE_VALUE
#include "private/ccl_hash_map_insert.h"

/* Move everything */
#define CCL_HASH_MAP_INSERT \
    CCL_MACRO_CONCAT(CCL_HASH_MAP_INSERT_BASE, _move_both)
#define CCL_HASH_MAP_INSERT_MOVE_KEY
#define CCL_HASH_MAP_INSERT_MOVE_VALUE
#include "private/ccl_hash_map_insert.h"

static ccl_result CCL_HASH_MAP_RESIZE(struct CCL_HASH_MAP_NAME *self, size_t new_num_buckets)
{
    struct CCL_HASH_MAP_NAME new_map;
    struct CCL_HASH_MAP_BUCKET_NAME *bucket;
    struct CCL_HASH_MAP_ENTRY_NAME *entry;
    ccl_result res;
    size_t i, l;

    if (new_num_buckets == self->num_buckets)
        return CCL_SUCCESS;

    CCL_IF_UNLIKELY ((res = CCL_HASH_MAP_INIT_EX(&new_map, new_num_buckets)) != CCL_SUCCESS)
        return res;

    for (i = 0; i < self->num_buckets; ++i)
    {
        bucket = &self->buckets[i];

        if (!bucket->is_used)
            continue;

        entry = &bucket->entry;
        res = CCL_HASH_MAP_INSERT_MOVE_BOTH(&new_map, entry->key, entry->value);

        CCL_IF_UNLIKELY (res != CCL_SUCCESS)
        {
            CCL_HASH_MAP_FREE_WITHOUT_DTOR(&new_map);
            return res;
        }

        for (l = 0; l < bucket->extra_entries.size; ++l)
        {
            entry = &bucket->extra_entries.data[l];
            res = CCL_HASH_MAP_INSERT_MOVE_BOTH(&new_map, entry->key, entry->value);

            CCL_IF_UNLIKELY (res != CCL_SUCCESS)
            {
                CCL_HASH_MAP_FREE_WITHOUT_DTOR(&new_map);
                return res;
            }
        }
    }

    CCL_HASH_MAP_FREE_WITHOUT_DTOR(self);
    self->buckets = new_map.buckets;
    self->num_entries = new_map.num_entries;
    self->num_buckets = new_map.num_buckets;
    return CCL_SUCCESS;
}

#define CCL_HASH_MAP_ERASE_SUCCESS \
    CCL_MACRO_CONCAT3(_, CCL_HASH_MAP_ERASE, _success)

static ccl_result CCL_FORCE_INLINE CCL_HASH_MAP_ERASE_SUCCESS(struct CCL_HASH_MAP_NAME *self)
{
    self->num_entries--;

    if (CCL_HASH_MAP_SHOULD_SHRINK(self->num_entries, self->num_buckets))
        (void) CCL_HASH_MAP_RESIZE(self, CCL_HASH_MAP_CALC_SHRINK(self));

    return CCL_SUCCESS;
}

static ccl_result CCL_HASH_MAP_ERASE(struct CCL_HASH_MAP_NAME *self, const CCL_HASH_MAP_KEY_TYPE key)
{
    size_t hash = CCL_HASH_MAP_KEY_HASH_FN((const CCL_HASH_MAP_KEY_TYPE) key), i;
    struct CCL_HASH_MAP_BUCKET_NAME *bucket;
    struct CCL_HASH_MAP_ENTRY_NAME *entry = NULL;
    ccl_result res = CCL_INVALID_INDEX;

    CCL_IF_UNLIKELY (self->num_buckets == 0)
        return CCL_INVALID_INDEX;

    bucket = &self->buckets[hash % self->num_buckets];

    if (!bucket->is_used)
        return CCL_INVALID_INDEX;

    if (CCL_HASH_MAP_KEY_COMPARE(bucket->entry.key, key))
    {
        entry = &bucket->entry;

        CCL_HASH_MAP_FREE_KEY(&entry->key);
        CCL_HASH_MAP_FREE_VALUE(&entry->value);

        if (bucket->extra_entries.size > 0)
        {
            bucket->entry.key = bucket->extra_entries.data[0].key;
            bucket->entry.value = bucket->extra_entries.data[0].value;
            CCL_HASH_MAP_ENTRY_ARRAY_REMOVE_NO_BOUND_CHECK_NO_DTOR(&bucket->extra_entries, 0);
        }
        else
            bucket->is_used = 0;

        return CCL_HASH_MAP_ERASE_SUCCESS(self);
    }

    for (i = 0; i < bucket->extra_entries.size; ++i)
    {
        if (CCL_HASH_MAP_KEY_COMPARE(bucket->extra_entries.data[i].key, key))
        {
            CCL_HASH_MAP_ENTRY_ARRAY_REMOVE_NO_BOUND_CHECK(&bucket->extra_entries, i);
            return CCL_HASH_MAP_ERASE_SUCCESS(self);
        }
    }

    return CCL_INVALID_INDEX;
}

static ccl_bool CCL_HASH_MAP_HAS(struct CCL_HASH_MAP_NAME *self, const CCL_HASH_MAP_KEY_TYPE key)
{
    size_t hash = CCL_HASH_MAP_KEY_HASH_FN((const CCL_HASH_MAP_KEY_TYPE) key), i;
    struct CCL_HASH_MAP_BUCKET_NAME *bucket;

    CCL_IF_UNLIKELY (self->num_buckets == 0)
        return 0;

    bucket = &self->buckets[hash % self->num_buckets];

    if (!bucket->is_used)
        return 0;

    if (CCL_HASH_MAP_KEY_COMPARE(bucket->entry.key, key))
        return 1;

    for (i = 0; i < bucket->extra_entries.size; ++i)
    {
        if (CCL_HASH_MAP_KEY_COMPARE(bucket->extra_entries.data[i].key, key))
            return 1;
    }

    return 0;
}

static ccl_result CCL_HASH_MAP_GET(struct CCL_HASH_MAP_NAME *self, CCL_HASH_MAP_VALUE_TYPE **out_value, CCL_HASH_MAP_KEY_TYPE key)
{
    size_t hash = CCL_HASH_MAP_KEY_HASH_FN((const CCL_HASH_MAP_KEY_TYPE) key), i;
    struct CCL_HASH_MAP_BUCKET_NAME *bucket;
    ccl_result res = CCL_INVALID_INDEX;

    CCL_IF_UNLIKELY (self->num_buckets == 0)
        return CCL_INVALID_INDEX;

    bucket = &self->buckets[hash % self->num_buckets];

    if (!bucket->is_used)
        return CCL_INVALID_INDEX;

    if (CCL_HASH_MAP_KEY_COMPARE(bucket->entry.key, key))
    {
        *out_value = &bucket->entry.value;
        return CCL_SUCCESS;
    }

    for (i = 0; i < bucket->extra_entries.size; ++i)
    {
        if (CCL_HASH_MAP_KEY_COMPARE(bucket->extra_entries.data[i].key, key))
        {
            *out_value = &bucket->extra_entries.data[i].value;
            return CCL_SUCCESS;
        }
    }

    return CCL_INVALID_INDEX;
}

#undef CCL_HASH_MAP_NAME
#undef CCL_HASH_MAP_KEY_TYPE
#undef CCL_HASH_MAP_VALUE_TYPE
#undef CCL_HASH_MAP_KEY_HASH_FN
#undef CCL_HASH_MAP_DEFAULT_NUM_BUCKETS
#undef CCL_HASH_MAP_ENTRY_NAME
#undef CCL_HASH_MAP_BUCKET_NAME
#undef CCL_HASH_MAP_INIT
#undef CCL_HASH_MAP_FREE
#undef CCL_HASH_MAP_ERASE
#undef CCL_HASH_MAP_GET
#undef CCL_HASH_MAP_HAS
#undef CCL_HASH_MAP_BUCKET_MAX_CAPACITY
#undef CCL_HASH_MAP_KEY_COMPARE_FN
#undef CCL_HASH_MAP_KEY_COPY_FN
#undef CCL_HASH_MAP_VALUE_COPY_FN
#undef CCL_HASH_MAP_VALUE_COPY_OVERWRITE_FN
#undef CCL_HASH_MAP_KEY_FREE_FN
#undef CCL_HASH_MAP_VALUE_FREE_FN
#undef CCL_HASH_MAP_BUCKET_INIT
#undef CCL_HASH_MAP_BUCKET_INIT_EX
#undef CCL_HASH_MAP_BUCKET_FREE
#undef CCL_HASH_MAP_BUCKET_GROW_CAPACITY
#undef CCL_HASH_MAP_BUCKET_REMOVE
#undef CCL_HASH_MAP_USE_EXISTING_STRUCT
#undef CCL_HASH_MAP_SHOULD_GROW
#undef CCL_HASH_MAP_SHOULD_SHRINK
#undef CCL_HASH_MAP_CALC_GROW
#undef CCL_HASH_MAP_CALC_SHRINK
#undef CCL_HASH_MAP_ERASE_SUCCESS
