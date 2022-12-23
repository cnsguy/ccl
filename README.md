# ccl: a pure C89 header-only container library
Check TODO for stuff still required before 1.0. \
\
Also C++ compatible. \
Currently providing hashmaps, (optionally) safe dynamic arrays and strings. \
Functions which generate strings from primitives (like floats), are also planned for the future. \
Attempts to gracefully handle allocation failures instead of abort()ing.

## How to build tools (tests and benchmark)
CMake, ccl/tools is the source directory. \
You want to build ccl-test with a compiler that supports the gcc-style ASan flag. MSVC ASan support will be added at some point.

## Array usage
To define an int array:
```C
#define CCL_ARRAY_NAME int_array
#define CCL_ARRAY_TYPE int
#include "ccl/template/ccl_array.h"
```

To define a C string array that gets its values free()d on string_array_free() and its values copied on insert:
```C
#define CCL_ARRAY_NAME          string_array
#define CCL_ARRAY_TYPE          char*
#define CCL_ARRAY_VALUE_COPY_FN ccl_c_string_entry_copy /* Wrapper around CCL_STRDUP */
#define CCL_ARRAY_VALUE_FREE_FN ccl_c_string_entry_free /* Wrapper around free() */
#include "ccl/template/ccl_array.h"
```

## Hash map usage
To create an int key/val hash map:
```C
#define CCL_CAST_HASH_TYPE int
#define CCL_CAST_HASH_NAME int_hash
#include "ccl/hash/ccl_cast_hash.h"

#define CCL_HASH_MAP_NAME        int_map
#define CCL_HASH_MAP_KEY_TYPE    int
#define CCL_HASH_MAP_VALUE_TYPE  int
#define CCL_HASH_MAP_KEY_HASH_FN int_hash
#include "ccl/template/ccl_hash_map.h"
```

To create a C string key/val hash map that works as expected:
```C
#define CCL_HASH_MAP_NAME                    string_map
#define CCL_HASH_MAP_KEY_TYPE                char*
#define CCL_HASH_MAP_VALUE_TYPE              char*
#define CCL_HASH_MAP_KEY_HASH_FN             ccl_djb2_hash               /* Use ccl_djb2_hash_data for non-str-based hashing */
#define CCL_HASH_MAP_KEY_COMPARE_FN          ccl_c_string_entry_compare  /* Wrapper around strcmp */
#define CCL_HASH_MAP_KEY_COPY_FN             ccl_c_string_entry_copy     /* Wrapper around CCL_STRDUP */
#define CCL_HASH_MAP_KEY_FREE_FN             ccl_c_string_entry_free     /* Wrapper around free() */
#define CCL_HASH_MAP_VALUE_COPY_FN           ccl_c_string_entry_copy
#define CCL_HASH_MAP_VALUE_COPY_OVERWRITE_FN ccl_c_string_entry_copy_overwrite
#define CCL_HASH_MAP_VALUE_FREE_FN           ccl_c_string_entry_free
#include "ccl/template/ccl_hash_map.h"
```

## Avoiding bounds checking
The following functions can be used to avoid bounds checking when using arrays:
```C
static ccl_result array_name##_insert_no_bound_check(struct name *self, size_t i, value_type value);
static void array_name##_remove_no_bound_check(struct name *self, size_t i);
static void array_name##_pop_no_bound_check(struct name *self);
```

## Avoiding copies in arrays
The following functions can be used to avoid copies, even when copy functions are defined:
```C
static ccl_result array_name##_append_move(struct name *self, value_type value);
static ccl_result array_name##_insert_no_bound_check_move(struct name *self, size_t i, value_type value);
static ccl_result array_name##_insert_move(struct name *self, size_t i, value_type value);
```
These will do a primitive value copy instead. If a move-insert fails, the value NEEDS to be freed on the caller-side.

## Avoiding copies in hash maps
The following functions can be used to avoid copies, even when copy functions are defined:
```C
static ccl_result hash_map_name##_insert_move_key(struct name *self, key_type key, value_type value);
static ccl_result hash_map_name##_insert_move_value(struct name *self, key_type key, value_type value);
static ccl_result hash_map_name##_insert_move_both(struct name *self, key_type key, value_type value);
```
These will do a primitive value copy instead. If a move-insert fails, the key or value NEEDS to be freed on the caller-side. \
You will want to avoid moving keys for overwrite operations, as all it leads to is an extra free() in the insert function.
