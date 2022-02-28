#ifndef CCL_MACRO_UTILITY_H
#define CCL_MACRO_UTILITY_H

#define CCL_MACRO_CONCAT_PASTER(x, y) \
    x##y

#define CCL_MACRO_CONCAT(x, y) \
    CCL_MACRO_CONCAT_PASTER(x, y)

#define CCL_MACRO_CONCAT_PASTER3(x, y, z) \
    x##y##z

#define CCL_MACRO_CONCAT3(x, y, z) \
    CCL_MACRO_CONCAT_PASTER3(x, y, z)

#define CCL_ARRAY_NELEMS(x) \
    (sizeof ((x)) / sizeof ((x)[0]))

#endif
