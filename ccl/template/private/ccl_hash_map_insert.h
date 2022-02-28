#ifdef CCL_HASH_MAP_INSERT_MOVE_KEY
#define CCL_HASH_MAP_INSERT_COPY_KEY(dst_ptr, src) \
    (*(dst_ptr)) = (src);
#define CCL_HASH_MAP_INSERT_TRY_INSERT_KEY_OTHERWISE(dst_ptr, src) \
    (*(dst_ptr)) = (src); if (0)
#define CCL_HASH_MAP_INSERT_KEY_TYPE \
    CCL_HASH_MAP_KEY_TYPE
#else
#define CCL_HASH_MAP_INSERT_COPY_KEY(dst_ptr, src) \
    CCL_HASH_MAP_COPY_KEY((dst_ptr), (src))
#define CCL_HASH_MAP_INSERT_TRY_INSERT_KEY_OTHERWISE(dst_ptr, src) \
    CCL_TRY_HASH_MAP_TRY_COPY_KEY_OTHERWISE((dst_ptr), (src))
#define CCL_HASH_MAP_INSERT_KEY_TYPE \
    const CCL_HASH_MAP_KEY_TYPE
#endif

#ifdef CCL_HASH_MAP_INSERT_MOVE_VALUE
#define CCL_HASH_MAP_INSERT_COPY_VALUE(dst_ptr, src) \
    (*(dst_ptr)) = (src);
#define CCL_HASH_MAP_INSERT_TRY_INSERT_VALUE_OTHERWISE(dst_ptr, src) \
    (*(dst_ptr)) = (src); if (0)
#define CCL_HASH_MAP_INSERT_VALUE_TYPE \
    CCL_HASH_MAP_VALUE_TYPE
#else
#define CCL_HASH_MAP_INSERT_COPY_VALUE(dst_ptr, src) \
    CCL_HASH_MAP_COPY_VALUE((dst_ptr), (src))
#define CCL_HASH_MAP_INSERT_TRY_INSERT_VALUE_OTHERWISE(dst_ptr, src) \
    CCL_TRY_HASH_MAP_TRY_COPY_VALUE_OTHERWISE((dst_ptr), (src))
#define CCL_HASH_MAP_INSERT_VALUE_TYPE \
    const CCL_HASH_MAP_VALUE_TYPE
#endif

#define CCL_HASH_MAP_INSERT_INTO \
    CCL_MACRO_CONCAT3(_, CCL_HASH_MAP_INSERT, _into)

static ccl_result CCL_FORCE_INLINE CCL_HASH_MAP_INSERT_INTO(struct CCL_HASH_MAP_ENTRY_NAME *self,
                                                            CCL_HASH_MAP_INSERT_KEY_TYPE key,
                                                            CCL_HASH_MAP_INSERT_VALUE_TYPE value)
{
    CCL_HASH_MAP_INSERT_TRY_INSERT_KEY_OTHERWISE(&self->key, key)
        return CCL_KEY_COPY_FN_FAIL;

    CCL_HASH_MAP_INSERT_TRY_INSERT_VALUE_OTHERWISE(&self->value, value)
    {
        CCL_HASH_MAP_FREE_KEY(&self->key);
        return CCL_VALUE_COPY_FN_FAIL;
    }

    return CCL_SUCCESS;
}

#define CCL_HASH_MAP_INSERT_SUCCESS \
    CCL_MACRO_CONCAT3(_, CCL_HASH_MAP_INSERT, _success)

static ccl_result CCL_FORCE_INLINE CCL_HASH_MAP_INSERT_SUCCESS(struct CCL_HASH_MAP_NAME *self)
{
    self->num_entries++;
    return CCL_SUCCESS;
}

static ccl_result CCL_HASH_MAP_INSERT(struct CCL_HASH_MAP_NAME *self, CCL_HASH_MAP_INSERT_KEY_TYPE key, CCL_HASH_MAP_INSERT_VALUE_TYPE value)
{
    size_t hash = CCL_HASH_MAP_KEY_HASH_FN((const CCL_HASH_MAP_KEY_TYPE) key), i;
    struct CCL_HASH_MAP_BUCKET_NAME *bucket;
    struct CCL_HASH_MAP_ENTRY_NAME *entry;
    ccl_result res;

    if (CCL_HASH_MAP_SHOULD_GROW(self->num_entries, self->num_buckets))
        res = CCL_HASH_MAP_RESIZE(self, CCL_HASH_MAP_CALC_GROW(self));

    /* Only report result of resize fail if we don't have enough buckets at all */
    CCL_IF_UNLIKELY (self->num_buckets == 0)
        return res;

    /* XXX This is almost surely impossible */
    CCL_IF_UNLIKELY (self->num_entries == SIZE_MAX)
        return CCL_WOULD_OVERFLOW;

    bucket = &self->buckets[hash % self->num_buckets];

    /* Bucket is empty, simple insert */
    if (!bucket->is_used)
    {
        if ((res = CCL_HASH_MAP_INSERT_INTO(&bucket->entry, key, value)) != CCL_SUCCESS)
            return res;

        bucket->is_used = 1;
        return CCL_HASH_MAP_INSERT_SUCCESS(self);
    }

    /* Bucket isn't empty, have to compare existing values */
    for (i = 0; i < bucket->extra_entries.size; ++i)
    {
        entry = &bucket->extra_entries.data[i];

        if (CCL_HASH_MAP_KEY_COMPARE(entry->key, key))
        {
            /* Key was to be moved, but we didn't need to change the original */
#ifdef CCL_HASH_MAP_INSERT_MOVE_KEY
            CCL_HASH_MAP_FREE_KEY(&key);
#endif
            /* Value was to be moved, free original */
#if defined(CCL_HASH_MAP_INSERT_MOVE_VALUE) || !defined(CCL_HASH_MAP_VALUE_COPY_FN)
            CCL_HASH_MAP_FREE_VALUE(&entry->value);
            entry->value = value;
            return CCL_SUCCESS;
#else
            /* Perform a copy overwrite */
            if (!CCL_HASH_MAP_VALUE_COPY_OVERWRITE_FN(&entry->value, value))
                return CCL_VALUE_COPY_FN_FAIL;

            return CCL_SUCCESS;
#endif
        }
    }

    /* Value was not found, time to insert it into the extra values array */
    CCL_IF_UNLIKELY ((res = CCL_HASH_MAP_ENTRY_ARRAY_GROW_CAPACITY(&bucket->extra_entries)) != CCL_SUCCESS)
        return res;

    CCL_IF_UNLIKELY ((res = CCL_HASH_MAP_INSERT_INTO(&bucket->extra_entries.data[bucket->extra_entries.size], key, value)) != CCL_SUCCESS)
        return res;

    bucket->extra_entries.size++;
    return CCL_HASH_MAP_INSERT_SUCCESS(self);
}

#undef CCL_HASH_MAP_INSERT
#undef CCL_HASH_MAP_INSERT_MOVE_KEY
#undef CCL_HASH_MAP_INSERT_MOVE_VALUE
#undef CCL_HASH_MAP_INSERT_COPY_VALUE
#undef CCL_HASH_MAP_INSERT_TRY_INSERT_VALUE_OTHERWISE
#undef CCL_HASH_MAP_INSERT_COPY_KEY
#undef CCL_HASH_MAP_INSERT_TRY_INSERT_KEY_OTHERWISE
#undef CCL_HASH_MAP_INSERT_VALUE_TYPE
#undef CCL_HASH_MAP_INSERT_KEY_TYPE
#undef CCL_HASH_MAP_INSERT_SUCCESS
#undef CCL_HASH_MAP_INSERT_INTO
