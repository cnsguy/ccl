#ifndef CCL_DJB2_HASH
#define CCL_DJB2_HASH

#ifndef __cplusplus
#include <stdlib.h>
#else
#include <cstdlib>
#endif

#include "../ccl_inline.h"

static CCL_SUGGEST_INLINE size_t ccl_djb2_hash(const char *key)
{
    size_t hash = 5381;
    char c;

    while (*key)
    {
        c = *key++;
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

#endif
