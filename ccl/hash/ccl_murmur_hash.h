#ifndef CCL_MURMUR_HASH_H
#define CCL_MURMUR_HASH_H

#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef unsigned char ccl_uint8_t;
typedef unsigned int ccl_uint32_t;
typedef unsigned __int64 ccl_uint64_t;
#else
#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
#endif
typedef uint8_t ccl_uint8_t;
typedef uint32_t ccl_uint32_t;
typedef uint64_t ccl_uint64_t;
#endif

#if defined(_MSC_VER)
#define CCL_MURMUR_BIG_CONSTANT(x) (x)
#else
#define CCL_MURMUR_BIG_CONSTANT(x) (x##LLU)
#endif

//-----------------------------------------------------------------------------
// MurmurHash2A, by Austin Appleby

// This is a variant of MurmurHash2 modified to use the Merkle-Damgard 
// construction. Bulk speed should be identical to Murmur2, small-key speed 
// will be 10%-20% slower due to the added overhead at the end of the hash.

// This variant fixes a minor issue where null keys were more likely to
// collide with each other than expected, and also makes the function
// more amenable to incremental implementations.

#define CCL_MURMUR_MMIX(h,k) \
    { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }

static ccl_uint32_t ccl_murmur_hash_2a(const void *key, ccl_uint32_t len, ccl_uint32_t seed)
{
    const ccl_uint32_t m = 0x5bd1e995;
    const int r = 24;
    ccl_uint32_t l = len;
    const unsigned char *data = (const unsigned char*) key;
    ccl_uint32_t h = seed; 
    ccl_uint32_t k;
    ccl_uint32_t t;

    while (len >= 4)
    {
        k = *(const ccl_uint32_t*) data;
        CCL_MURMUR_MMIX(h,k);
        data += 4;
        len -= 4;
    }

    t = 0;

    switch (len)
    {
        case 3: t ^= data[2] << 16;
        case 2: t ^= data[1] << 8;
        case 1: t ^= data[0];
    };

    CCL_MURMUR_MMIX(h,t);
    CCL_MURMUR_MMIX(h,l);

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
}

//-----------------------------------------------------------------------------
// MurmurHash2, 64-bit versions, by Austin Appleby

// The same caveats as 32-bit MurmurHash2 apply here - beware of alignment 
// and endian-ness issues if used across multiple platforms.

// 64-bit hash for 64-bit platforms

static ccl_uint64_t ccl_murmur_hash_64a(const void *key, ccl_uint64_t len, ccl_uint64_t seed)
{
    const ccl_uint64_t m = CCL_MURMUR_BIG_CONSTANT(0xc6a4a7935bd1e995);
    const int r = 47;
    ccl_uint64_t h = seed ^ (len * m);
    const ccl_uint64_t *data = (const ccl_uint64_t*) key;
    const ccl_uint64_t *end = data + (len / 8);
    ccl_uint64_t k;
    const unsigned char *data2;

    while (data != end)
    {
        k = *data++;

        k *= m; 
        k ^= k >> r; 
        k *= m; 
        
        h ^= k;
        h *= m; 
    }

    data2 = (const unsigned char*) data;

    switch (len & 7)
    {
        case 7: h ^= ccl_uint64_t(data2[6]) << 48;
        case 6: h ^= ccl_uint64_t(data2[5]) << 40;
        case 5: h ^= ccl_uint64_t(data2[4]) << 32;
        case 4: h ^= ccl_uint64_t(data2[3]) << 24;
        case 3: h ^= ccl_uint64_t(data2[2]) << 16;
        case 2: h ^= ccl_uint64_t(data2[1]) << 8;
        case 1: h ^= ccl_uint64_t(data2[0]);
            h *= m;
    };
 
    h ^= h >> r;
    h *= m;
    h ^= h >> r;
    return h;
} 

#endif
