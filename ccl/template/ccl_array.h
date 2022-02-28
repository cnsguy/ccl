/*
IN:
 - CCL_ARRAY_NAME: Name of the struct to generate
 - CCL_ARRAY_TYPE: Type of the stored value
 - (optional) CCL_ARRAY_VALUE_COPY_FN: Function for copying values on insert
 - (optional) CCL_ARRAY_VALUE_FREE_FN: Function for freeing values in dtor
 - (optional) CCL_ARRAY_USE_EXISTING_STRUCT: Don't define our own struct, instead just refer to user-defined one
*/

#ifndef CCL_ARRAY_NAME
#error "Please define CCL_ARRAY_NAME before including this file."
#endif

#ifndef CCL_ARRAY_TYPE
#error "Please define CCL_ARRAY_TYPE before including this file."
#endif

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

#define CCL_ARRAY_INIT \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _init)
#define CCL_ARRAY_INIT_EX \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _init_ex)
#define CCL_ARRAY_PREALLOCATE \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _preallocate)
#define CCL_ARRAY_FREE \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _free)
#define CCL_ARRAY_FREE_WITHOUT_DTOR \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _free_without_dtor)
#define CCL_ARRAY_APPEND_BASE \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _append)
#define CCL_ARRAY_INSERT_NO_BOUND_CHECK_BASE \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _insert_no_bound_check)
#define CCL_ARRAY_INSERT_BASE \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _insert)
#define CCL_ARRAY_POP_NO_BOUND_CHECK \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _pop_no_bound_check)
#define CCL_ARRAY_POP \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _pop)
#define CCL_ARRAY_POP_NO_BOUND_CHECK_NO_DTOR \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _pop_no_bound_check_no_dtor)
#define CCL_ARRAY_POP_NO_DTOR \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _pop_no_dtor)
#define CCL_ARRAY_REMOVE_NO_BOUND_CHECK \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _remove_no_bound_check)
#define CCL_ARRAY_REMOVE \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _remove)
#define CCL_ARRAY_REMOVE_NO_BOUND_CHECK_NO_DTOR \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _remove_no_bound_check_no_dtor)
#define CCL_ARRAY_REMOVE_NO_DTOR \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _remove_no_dtor)
#define CCL_ARRAY_MAX_CAPACITY \
    CCL_MACRO_CONCAT(CCL_ARRAY_NAME, _max_capacity)

#ifndef CCL_ARRAY_VALUE_COPY_FN
#define CCL_ARRAY_COPY_VALUE(dst_ptr, src) \
    ((*(dst_ptr)) = (src))
#define CCL_ARRAY_TRY_COPY_VALUE_OTHERWISE(dst_ptr, src) \
    CCL_ARRAY_COPY_VALUE((dst_ptr), (src)); if (0)
#else
#define CCL_ARRAY_COPY_VALUE(dst_ptr, src) \
    CCL_ARRAY_VALUE_COPY_FN((dst_ptr), (src))
#define CCL_ARRAY_TRY_COPY_VALUE_OTHERWISE(dst_ptr, src) \
    CCL_IF_UNLIKELY (!CCL_ARRAY_VALUE_COPY_FN((dst_ptr), (src)))
#endif

#ifdef CCL_ARRAY_VALUE_FREE_FN
#define CCL_ARRAY_FREE_VALUE(self) \
    CCL_ARRAY_VALUE_FREE_FN((self))
#else
#define CCL_ARRAY_FREE_VALUE(self) \
    ;
#endif

#ifndef CCL_ARRAY_USE_EXISTING_STRUCT
struct CCL_ARRAY_NAME
{
    CCL_ARRAY_TYPE *data;
    size_t size;
    size_t capacity;
};
#endif

static const size_t CCL_ARRAY_MAX_CAPACITY =
    SIZE_MAX / sizeof(CCL_ARRAY_TYPE);

static void CCL_ARRAY_INIT(struct CCL_ARRAY_NAME *self)
{
    self->data = NULL;
    self->size = 0;
    self->capacity = 0;
}

static ccl_result CCL_ARRAY_INIT_EX(struct CCL_ARRAY_NAME *self, size_t capacity)
{
    void *ptr;

    if (capacity > CCL_ARRAY_MAX_CAPACITY)
        return CCL_WOULD_OVERFLOW;

    if (capacity == 0)
    {
        CCL_ARRAY_INIT(self);
        return CCL_SUCCESS;
    }

    CCL_IF_UNLIKELY ((ptr = malloc(capacity * sizeof(CCL_ARRAY_TYPE))) == NULL)
        return CCL_ALLOC_FAIL;

    self->data = (CCL_ARRAY_TYPE*) ptr;
    self->size = 0;
    self->capacity = capacity;
    return CCL_SUCCESS;
}

static ccl_result CCL_ARRAY_PREALLOCATE(struct CCL_ARRAY_NAME *self, size_t capacity)
{
    void *new_ptr;

    if (capacity > CCL_ARRAY_MAX_CAPACITY)
        return CCL_WOULD_OVERFLOW;

    /* Handle stuff like array_init(&test, 0); array_preallocate(&test, 0); */
    if (capacity <= self->capacity)
        return CCL_SUCCESS;

    CCL_IF_UNLIKELY ((new_ptr = realloc(self->data, capacity * sizeof(CCL_ARRAY_TYPE))) == NULL)
        return CCL_ALLOC_FAIL;

    self->data = (CCL_ARRAY_TYPE*) new_ptr;
    self->capacity = capacity;
    return CCL_SUCCESS;
}

static void CCL_ARRAY_FREE_WITHOUT_DTOR(struct CCL_ARRAY_NAME *self)
{
    free(self->data);
}

static void CCL_ARRAY_FREE(struct CCL_ARRAY_NAME *self)
{
#ifdef CCL_ARRAY_VALUE_FREE_FN
    size_t i;

    for (i = 0; i < self->size; ++i)
        CCL_ARRAY_VALUE_FREE_FN(&self->data[i]);
#endif

    CCL_ARRAY_FREE_WITHOUT_DTOR(self);
}

#define CCL_ARRAY_CALC_NEW_CAPACITY \
    CCL_MACRO_CONCAT3(_, CCL_ARRAY_NAME, _calc_new_capacity)

static CCL_SUGGEST_INLINE size_t CCL_ARRAY_CALC_NEW_CAPACITY(size_t capacity)
{
    CCL_IF_UNLIKELY (capacity == 0)
        return 1;

    return (CCL_ARRAY_MAX_CAPACITY - capacity) >= capacity?
        capacity * 2 : CCL_ARRAY_MAX_CAPACITY;
}

#define CCL_ARRAY_GROW_CAPACITY \
    CCL_MACRO_CONCAT3(_, CCL_ARRAY_NAME, _grow_capacity)

static ccl_result CCL_ARRAY_GROW_CAPACITY(struct CCL_ARRAY_NAME *self)
{
    CCL_ARRAY_TYPE *new_ptr;
    size_t new_capacity, new_nb;

    CCL_IF_UNLIKELY (self->size == CCL_ARRAY_MAX_CAPACITY)
        return CCL_WOULD_OVERFLOW;

    if (self->size == self->capacity)
    {
        new_capacity = CCL_ARRAY_CALC_NEW_CAPACITY(self->capacity);
        new_nb = new_capacity * sizeof(CCL_ARRAY_TYPE);

        CCL_IF_UNLIKELY ((new_ptr = (CCL_ARRAY_TYPE*) realloc(self->data, new_nb)) == NULL)
            return CCL_ALLOC_FAIL;

        self->data = new_ptr;
        self->capacity = new_capacity;
    }

    return CCL_SUCCESS;
}

/* Insert normally, copying values */
#define CCL_ARRAY_APPEND \
    CCL_ARRAY_APPEND_BASE
#define CCL_ARRAY_INSERT_NO_BOUND_CHECK \
    CCL_ARRAY_INSERT_NO_BOUND_CHECK_BASE
#define CCL_ARRAY_INSERT \
    CCL_ARRAY_INSERT_BASE
#include "private/ccl_array_insert.h"

/* Insert with move */
#define CCL_ARRAY_APPEND \
    CCL_MACRO_CONCAT(CCL_ARRAY_APPEND_BASE, _move)
#define CCL_ARRAY_INSERT_NO_BOUND_CHECK \
    CCL_MACRO_CONCAT(CCL_ARRAY_INSERT_NO_BOUND_CHECK_BASE, _move)
#define CCL_ARRAY_INSERT \
    CCL_MACRO_CONCAT(CCL_ARRAY_INSERT_BASE, _move)
#define CCL_ARRAY_INSERT_MOVE_VALUE
#include "private/ccl_array_insert.h"

static void CCL_ARRAY_POP_NO_BOUND_CHECK(struct CCL_ARRAY_NAME *self)
{
    CCL_ARRAY_FREE_VALUE(&self->data[self->size - 1]);
    self->size--;
}

static ccl_result CCL_ARRAY_POP(struct CCL_ARRAY_NAME *self)
{
    CCL_IF_UNLIKELY (self->size == 0)
        return CCL_INVALID_INDEX;

    CCL_ARRAY_POP_NO_BOUND_CHECK(self);
    return CCL_SUCCESS;
}

static void CCL_ARRAY_REMOVE_NO_BOUND_CHECK(struct CCL_ARRAY_NAME *self, size_t i)
{
    if (i == (self->size - 1))
    {
        CCL_ARRAY_POP_NO_BOUND_CHECK(self);
        return;
    }

    CCL_ARRAY_FREE_VALUE(&self->data[i]);

    memmove(
        &self->data[i],
        &self->data[i + 1],
        (self->size - 1 - i) * sizeof(CCL_ARRAY_TYPE)
    );

    self->size--;
}

static ccl_result CCL_ARRAY_REMOVE(struct CCL_ARRAY_NAME *self, size_t i)
{
    CCL_IF_UNLIKELY (self->size == 0 || i > (self->size - 1))
        return CCL_INVALID_INDEX;

    CCL_ARRAY_REMOVE_NO_BOUND_CHECK(self, i);
    return CCL_SUCCESS;
}

static void CCL_ARRAY_POP_NO_BOUND_CHECK_NO_DTOR(struct CCL_ARRAY_NAME *self)
{
    self->size--;
}

static ccl_result CCL_ARRAY_POP_NO_DTOR(struct CCL_ARRAY_NAME *self)
{
    CCL_IF_UNLIKELY (self->size == 0)
        return CCL_INVALID_INDEX;

    CCL_ARRAY_POP_NO_BOUND_CHECK_NO_DTOR(self);
    return CCL_SUCCESS;
}

static void CCL_ARRAY_REMOVE_NO_BOUND_CHECK_NO_DTOR(struct CCL_ARRAY_NAME *self, size_t i)
{
    if (i == (self->size - 1))
    {
        CCL_ARRAY_POP_NO_BOUND_CHECK_NO_DTOR(self);
        return;
    }

    memmove(
        &self->data[i],
        &self->data[i + 1],
        (self->size - 1 - i) * sizeof(CCL_ARRAY_TYPE)
    );

    self->size--;
}

static ccl_result CCL_ARRAY_REMOVE_NO_DTOR(struct CCL_ARRAY_NAME *self, size_t i)
{
    CCL_IF_UNLIKELY (self->size == 0 || i > (self->size - 1))
        return CCL_INVALID_INDEX;

    CCL_ARRAY_REMOVE_NO_BOUND_CHECK_NO_DTOR(self, i);
    return CCL_SUCCESS;
}

#undef CCL_ARRAY_NAME
#undef CCL_ARRAY_TYPE
#undef CCL_ARRAY_CAPACITY_GROW_FN
#undef CCL_ARRAY_INIT
#undef CCL_ARRAY_FREE
#undef CCL_ARRAY_INSERT
#undef CCL_ARRAY_REMOVE
#undef CCL_ARRAY_APPEND
#undef CCL_ARRAY_POP
#undef CCL_ARRAY_MAX_CAPACITY
#undef CCL_ARRAY_GROW_CAPACITY
#undef CCL_ARRAY_CALC_NEW_CAPACITY
#undef CCL_ARRAY_VALUE_COPY_FN
#undef CCL_ARRAY_VALUE_FREE_FN
#undef CCL_ARRAY_FREE_WITHOUT_DTOR
#undef CCL_ARRAY_COPY_VALUE
#undef CCL_ARRAY_TRY_COPY_VALUE_OTHERWISE
#undef CCL_ARRAY_FREE_VALUE
#undef CCL_ARRAY_USE_EXISTING_STRUCT
