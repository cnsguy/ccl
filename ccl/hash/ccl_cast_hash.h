#ifndef __cplusplus
#include <stddef.h>
#else
#include <cstddef>
#endif

#include "../ccl_inline.h"

#ifndef CCL_CAST_HASH_NAME
#error "Please define CCL_CAST_HASH_NAME before including this file."
#endif

#ifndef CCL_CAST_HASH_TYPE
#error "Please define CCL_CAST_HASH_TYPE before including this file."
#endif

static CCL_FORCE_INLINE size_t CCL_CAST_HASH_NAME(CCL_CAST_HASH_TYPE key)
{
    return (size_t) key;
}

#undef CCL_CAST_HASH_NAME
#undef CCL_CAST_HASH_TYPE
