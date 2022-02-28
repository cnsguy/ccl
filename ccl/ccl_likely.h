#ifndef CCL_LIKELY_H
#define CCL_LIKELY_H

#if __cplusplus >= 202002L
#define CCL_IF_LIKELY(x)   if ((x)) [[likely]]
#define CCL_IF_UNLIKELY(x) if ((x)) [[unlikely]]
#elif defined(__GNUC__)
#define CCL_IF_LIKELY(x)   if (__builtin_expect(!!(x), 1))
#define CCL_IF_UNLIKELY(x) if (__builtin_expect(!!(x), 0))
#else
#define CCL_IF_LIKELY(x)   if ((x))
#define CCL_IF_UNLIKELY(x) if ((x))
#endif

#endif
