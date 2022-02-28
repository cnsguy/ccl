#ifndef CCL_C_STRING_ENTRY_UTILITY
#define CCL_C_STRING_ENTRY_UTILITY

#include "ccl_bool.h"
#include "ccl_strdup.h"
#include "ccl_inline.h"

#ifndef __cplusplus
#include <stdlib.h>
#include <string.h>
#else
#include <cstdlib>
#include <cstring>
#endif

static CCL_FORCE_INLINE void ccl_c_string_entry_free(char **p)
{
    free(*p);
}

static CCL_FORCE_INLINE ccl_bool ccl_c_string_entry_compare(const char *p1, const char *p2)
{
    return strcmp(p1, p2) == 0;
}

static CCL_FORCE_INLINE ccl_bool ccl_c_string_entry_copy(char **dst, const char *src)
{
    char *cpy;

    if ((cpy = CCL_STRDUP(src)) == NULL)
        return 0;

    *dst = cpy;
    return 1;
}

static CCL_FORCE_INLINE ccl_bool ccl_c_string_entry_copy_overwrite(char **dst, const char *src)
{
    char *cpy;

    if ((cpy = CCL_STRDUP(src)) == NULL)
        return 0;

    free(*dst);
    *dst = cpy;
    return 1;
}

#endif
