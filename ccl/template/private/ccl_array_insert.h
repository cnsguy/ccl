#ifdef CCL_ARRAY_INSERT_MOVE_VALUE
#define CCL_ARRAY_INSERT_COPY_VALUE(dst_ptr, src) \
    (*(dst_ptr)) = (src);
#define CCL_ARRAY_INSERT_TRY_INSERT_VALUE_OTHERWISE(dst_ptr, src) \
    (*(dst_ptr)) = (src); if (0)
#define CCL_ARRAY_INSERT_TYPE \
    CCL_ARRAY_TYPE
#else
#define CCL_ARRAY_INSERT_COPY_VALUE(dst_ptr, src) \
    CCL_ARRAY_COPY_VALUE((dst_ptr), (src))
#define CCL_ARRAY_INSERT_TRY_INSERT_VALUE_OTHERWISE(dst_ptr, src) \
    CCL_ARRAY_TRY_COPY_VALUE_OTHERWISE((dst_ptr), (src))
#define CCL_ARRAY_INSERT_TYPE \
    const CCL_ARRAY_TYPE
#endif

static ccl_result CCL_ARRAY_APPEND(struct CCL_ARRAY_NAME *self, CCL_ARRAY_INSERT_TYPE value)
{
    ccl_result res;

    CCL_IF_UNLIKELY ((res = CCL_ARRAY_GROW_CAPACITY(self)) != CCL_SUCCESS)
        return res;

    CCL_ARRAY_INSERT_TRY_INSERT_VALUE_OTHERWISE(&self->data[self->size], value)
        return CCL_VALUE_COPY_FN_FAIL;

    self->size++;
    return CCL_SUCCESS;
}

static ccl_result CCL_ARRAY_INSERT_NO_BOUND_CHECK(struct CCL_ARRAY_NAME *self, size_t i, CCL_ARRAY_INSERT_TYPE value)
{
    ccl_result res;

    if (i == self->size)
        return CCL_ARRAY_APPEND(self, value);

    CCL_IF_UNLIKELY ((res = CCL_ARRAY_GROW_CAPACITY(self)) != CCL_SUCCESS)
        return res;

    memmove(
        &self->data[i + 1],
        &self->data[i],
        (self->size - i) * sizeof(CCL_ARRAY_TYPE)
    );

    CCL_ARRAY_INSERT_TRY_INSERT_VALUE_OTHERWISE(&self->data[i], value)
    {
        /* Should be VERY unlikely so we take this horrible performance impact instead of double copying */
        memmove(
            &self->data[i],
            &self->data[i + 1],
            (self->size - i) * sizeof(CCL_ARRAY_TYPE)
        );
        return CCL_VALUE_COPY_FN_FAIL;
    }

    self->size++;
    return CCL_SUCCESS;
}

static ccl_result CCL_ARRAY_INSERT(struct CCL_ARRAY_NAME *self, size_t i, CCL_ARRAY_INSERT_TYPE value)
{
    CCL_IF_UNLIKELY (i > self->size)
        return CCL_INVALID_INDEX;

    return CCL_ARRAY_INSERT_NO_BOUND_CHECK(self, i, value);
}

#undef CCL_ARRAY_APPEND
#undef CCL_ARRAY_INSERT_NO_BOUND_CHECK
#undef CCL_ARRAY_INSERT
#undef CCL_ARRAY_INSERT_COPY_VALUE
#undef CCL_ARRAY_INSERT_TRY_INSERT_VALUE_OTHERWISE
#undef CCL_ARRAY_INSERT_TYPE
