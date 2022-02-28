#ifndef CCL_STRING_ENTRY
#define CCL_STRING_ENTRY

#include "ccl_string.h"
#include "ccl_bool.h"
#include "ccl_inline.h"

#ifndef __cplusplus
#include <string.h>
#else
#include <cstring>
#endif

static CCL_FORCE_INLINE void ccl_string_entry_free(struct ccl_string *self)
{
    ccl_string_free(self);
}

static CCL_FORCE_INLINE ccl_bool ccl_string_entry_compare(const struct ccl_string self, struct ccl_string other)
{
    return ccl_string_compare(&self, &other);
}

static CCL_FORCE_INLINE ccl_bool ccl_string_entry_copy(struct ccl_string *dst, struct ccl_string src)
{
    return ccl_string_duplicate(dst, &src) == CCL_SUCCESS;
}

static CCL_FORCE_INLINE ccl_bool ccl_string_entry_copy_overwrite(struct ccl_string *dst, struct ccl_string src)
{
    char *old_ptr = dst->data;
    int res;

    if ((res = ccl_string_duplicate(dst, &src)) != CCL_SUCCESS)
        return 0;

    free(old_ptr);
    return 1;
}

#endif
