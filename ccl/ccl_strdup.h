#ifndef CCL_STRDUP_H
#define CCL_STRDUP_H

#ifndef __cplusplus
#include <string.h>
#else
#include <cstring>
#endif

#ifdef _MSC_VER
#define CCL_STRDUP _strdup
#else
#define CCL_STRDUP strdup
#endif

#endif
