#ifndef CCL_RESULT_H
#define CCL_RESULT_H

enum
{
    CCL_SUCCESS = 0,
    CCL_WOULD_OVERFLOW,
    CCL_ALLOC_FAIL,
    CCL_INVALID_INDEX,
    CCL_VALUE_COPY_FN_FAIL,
    CCL_KEY_COPY_FN_FAIL
};

typedef int ccl_result;

#endif
