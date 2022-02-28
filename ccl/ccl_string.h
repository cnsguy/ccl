#ifndef CCL_STRING_H
#define CCL_STRING_H

#ifndef __cplusplus
#include <stdlib.h>
#else
#include <cstdlib>
#endif

#include "ccl_defs.h"
#include "ccl_result.h"
#include "ccl_likely.h"
#include "ccl_inline.h"
#include "ccl_bool.h"
#include "ccl_strdup.h"

struct ccl_string
{
    char *data;
    size_t size;
    size_t capacity;
};

static const size_t ccl_string_max_capacity =
    SIZE_MAX;

static ccl_result ccl_string_init_empty(struct ccl_string *self)
{
    char *ptr;

    if ((ptr = CCL_STRDUP("\0")) == NULL)
        return CCL_ALLOC_FAIL;

    self->data = ptr;
    self->size = 1;
    self->capacity = 1;
    return CCL_SUCCESS;
}

static ccl_result ccl_string_init_empty_ex(struct ccl_string *self, size_t capacity)
{
    void *ptr;

    if (capacity > ccl_string_max_capacity)
        return CCL_WOULD_OVERFLOW;

    if (capacity <= 1)
    {
        ccl_string_init_empty(self);
        return CCL_SUCCESS;
    }

    CCL_IF_UNLIKELY ((ptr = malloc(capacity)) == NULL)
        return CCL_ALLOC_FAIL;

    self->data = (char*) ptr;
    self->data[0] = '\0';
    self->size = 0;
    self->capacity = capacity;
    return CCL_SUCCESS;
}

static ccl_result ccl_string_init_cstr_len(struct ccl_string *self, const char *init_str, size_t init_str_len)
{
    CCL_IF_UNLIKELY (init_str_len == SIZE_MAX)
        return CCL_WOULD_OVERFLOW;

    ccl_string_init_empty_ex(self, init_str_len + 1);
    memcpy(self->data, init_str, init_str_len + 1);
    self->size = init_str_len + 1;
    self->capacity = init_str_len + 1;
    return CCL_SUCCESS;
}

static ccl_result ccl_string_init_cstr(struct ccl_string *self, const char *init_str)
{
    return ccl_string_init_cstr_len(self, init_str, strlen(init_str));
}

static ccl_result ccl_string_preallocate(struct ccl_string *self, size_t capacity)
{
    void *new_ptr;

    if (capacity > ccl_string_max_capacity)
        return CCL_WOULD_OVERFLOW;

    /* Handle stuff like ccl_string_init(&test, 0); ccl_string_preallocate(&test, 0); */
    if (capacity <= self->capacity)
        return CCL_SUCCESS;

    CCL_IF_UNLIKELY ((new_ptr = realloc(self->data, capacity)) == NULL)
        return CCL_ALLOC_FAIL;

    self->data = (char*) new_ptr;
    self->capacity = capacity;
    return CCL_SUCCESS;
}

static void ccl_string_free(struct ccl_string *self)
{
    free(self->data);
}

static CCL_SUGGEST_INLINE size_t _ccl_string_calc_new_capacity(size_t capacity)
{
    CCL_IF_UNLIKELY (capacity == 0)
        return 1;

    return (ccl_string_max_capacity - capacity) >= capacity?
        capacity * 2 : ccl_string_max_capacity;
}

static ccl_result _ccl_string_grow_capacity(struct ccl_string *self)
{
    char *new_ptr;
    size_t new_capacity, new_nb;

    CCL_IF_UNLIKELY (self->size == ccl_string_max_capacity)
        return CCL_WOULD_OVERFLOW;

    if (self->size == self->capacity)
    {
        new_capacity = _ccl_string_calc_new_capacity(self->capacity);
        new_nb = new_capacity;

        CCL_IF_UNLIKELY ((new_ptr = (char*) realloc(self->data, new_nb)) == NULL)
            return CCL_ALLOC_FAIL;

        self->data = new_ptr;
        self->capacity = new_capacity;
    }

    return CCL_SUCCESS;
}

static ccl_result ccl_string_append_ch(struct ccl_string *self, char chr)
{
    ccl_result res;

    CCL_IF_UNLIKELY((res = _ccl_string_grow_capacity(self)) != CCL_SUCCESS)
        return res;

    self->data[self->size] = chr;
    self->size++;
    return CCL_SUCCESS;
}

static ccl_result ccl_string_insert_ch_no_bound_check(struct ccl_string *self, size_t i, char chr)
{
    ccl_result res;

    if (i == self->size)
        return ccl_string_append_ch(self, chr);

    CCL_IF_UNLIKELY ((res = _ccl_string_grow_capacity(self)) != CCL_SUCCESS)
        return res;

    memmove(
        &self->data[i + 1],
        &self->data[i],
        (self->size - i)
    );

    self->data[i] = chr;
    self->size++;
    return CCL_SUCCESS;
}

static ccl_result ccl_string_insert_ch(struct ccl_string *self, size_t i, char chr)
{
    CCL_IF_UNLIKELY (i > self->size)
        return CCL_INVALID_INDEX;

    return ccl_string_insert_ch_no_bound_check(self, i, chr);
}

static void ccl_string_pop_no_bound_check(struct ccl_string *self)
{
    self->size--;
}

static ccl_result ccl_string_pop(struct ccl_string *self)
{
    CCL_IF_UNLIKELY (self->size == 0)
        return CCL_INVALID_INDEX;

    ccl_string_pop_no_bound_check(self);
    return CCL_SUCCESS;
}

static void ccl_string_remove_no_bound_check(struct ccl_string *self, size_t i)
{
    if (i == (self->size - 1))
    {
        ccl_string_pop_no_bound_check(self);
        return;
    }

    memmove(
        &self->data[i],
        &self->data[i + 1],
        (self->size - 1 - i)
    );

    self->size--;
}

static ccl_result ccl_string_remove(struct ccl_string *self, size_t i)
{
    CCL_IF_UNLIKELY (self->size == 0 || i > (self->size - 1))
        return CCL_INVALID_INDEX;

    ccl_string_remove_no_bound_check(self, i);
    return CCL_SUCCESS;
}

static CCL_FORCE_INLINE ccl_bool ccl_string_compare(const struct ccl_string *self, const struct ccl_string *other)
{
    return (self->size == other->size) && strcmp(self->data, other->data) == 0;
}

static ccl_result ccl_string_duplicate(struct ccl_string *self, const struct ccl_string *source)
{
    ccl_result res;

    CCL_IF_UNLIKELY ((res = ccl_string_init_empty_ex(self, source->size)) != CCL_SUCCESS)
        return res;

    memcpy(self->data, source->data, source->size);
    self->size = source->size;
    self->capacity = source->capacity; /* XXX */
    return CCL_SUCCESS;
}

#endif
