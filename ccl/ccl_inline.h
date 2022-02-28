#ifndef CCL_INLINE_H
#define CCL_INLINE_H

#if defined(__cplusplus) || __STDC_VERSION__ >= 199901L
#define CCL_SUGGEST_INLINE inline
#else
#define CCL_SUGGEST_INLINE
#endif

#if defined(_MSC_VER)
#define CCL_FORCE_INLINE CCL_SUGGEST_INLINE __forceinline
#elif defined(__GNUC__)
#define CCL_FORCE_INLINE inline __attribute__((always_inline))
#else
#define CCL_FORCE_INLINE CCL_SUGGEST_INLINE
#endif

#endif
