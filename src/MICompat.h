/*
    MI compiler compatibility layer.

    Internal to melonDS core; not part of the MI.Core C ABI.  Macro names are
    compiler-neutral; compiler checks stay inside this header.

    Rules:
    - simple intrinsics use expression/statement macros;
    - intrinsics with out-parameters use static inline wrappers;
    - no AVX/AVX2 requirements.
*/

#ifndef MICOMPAT_H
#define MICOMPAT_H

#if defined(_MSC_VER)
#include <intrin.h>
#endif

/* Force inline. */
#if defined(_MSC_VER)
#define MI_COMPAT_FORCEINLINE __forceinline
#elif defined(__GNUC__)
#define MI_COMPAT_FORCEINLINE __attribute__((always_inline)) inline
#else
#define MI_COMPAT_FORCEINLINE inline
#endif

/* noreturn. */
#if defined(_MSC_VER)
#define MI_COMPAT_NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
#define MI_COMPAT_NORETURN __attribute__((noreturn))
#else
#define MI_COMPAT_NORETURN
#endif

/* Packed structs. */
#if defined(_MSC_VER)
#define MI_COMPAT_PACKED
#define MI_COMPAT_PACK_BEGIN __pragma(pack(push, 1))
#define MI_COMPAT_PACK_END __pragma(pack(pop))
#elif defined(__GNUC__)
#define MI_COMPAT_PACKED __attribute__((packed))
#define MI_COMPAT_PACK_BEGIN
#define MI_COMPAT_PACK_END
#else
#define MI_COMPAT_PACKED
#define MI_COMPAT_PACK_BEGIN
#define MI_COMPAT_PACK_END
#endif

/* Unreachable code. */
#if defined(_MSC_VER)
#define MI_COMPAT_UNREACHABLE() __assume(0)
#elif defined(__GNUC__)
#define MI_COMPAT_UNREACHABLE() __builtin_unreachable()
#else
#define MI_COMPAT_UNREACHABLE() ((void)0)
#endif

/*
    Popcount without POPCNT/SSE4.2: SWAR implementation, x64 baseline.
    Arguments are evaluated once by the wrapper call.
*/
static MI_COMPAT_FORCEINLINE unsigned int mi_compat_popcount32(unsigned int x)
{
    x = x - ((x >> 1) & 0x55555555u);
    x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
    x = (x + (x >> 4)) & 0x0F0F0F0Fu;
    return (x * 0x01010101u) >> 24;
}
#define MI_COMPAT_POPCOUNT32(x) ((int)mi_compat_popcount32((unsigned int)(x)))

/*
    Count trailing zeros.  The caller must pass a nonzero value; the wrappers
    return 32/64 for zero as a defensive value, but callers must not rely on
    that (the GCC/Clang builtins are undefined for zero).
*/
#if defined(_MSC_VER)
static MI_COMPAT_FORCEINLINE unsigned int mi_compat_ctz32(unsigned int x)
{
    unsigned long index;
    _BitScanForward(&index, x);
    return (unsigned int)index;
}
#define MI_COMPAT_CTZ32(x) mi_compat_ctz32((unsigned int)(x))
#elif defined(__GNUC__)
#define MI_COMPAT_CTZ32(x) ((unsigned int)__builtin_ctz((unsigned int)(x)))
#else
static MI_COMPAT_FORCEINLINE unsigned int mi_compat_ctz32(unsigned int x)
{
    unsigned int n = 0;
    if (x == 0) return 32;
    while ((x & 1u) == 0)
    {
        x >>= 1;
        ++n;
    }
    return n;
}
#define MI_COMPAT_CTZ32(x) mi_compat_ctz32((unsigned int)(x))
#endif

#if defined(_MSC_VER) && defined(_M_X64)
static MI_COMPAT_FORCEINLINE unsigned int mi_compat_ctz64(unsigned long long x)
{
    unsigned long index;
    _BitScanForward64(&index, x);
    return (unsigned int)index;
}
#define MI_COMPAT_CTZ64(x) mi_compat_ctz64((unsigned long long)(x))
#elif defined(_MSC_VER)
static MI_COMPAT_FORCEINLINE unsigned int mi_compat_ctz64(unsigned long long x)
{
    unsigned long index;
    unsigned int low = (unsigned int)x;
    if (low != 0)
    {
        _BitScanForward(&index, low);
        return (unsigned int)index;
    }
    _BitScanForward(&index, (unsigned int)(x >> 32));
    return (unsigned int)index + 32;
}
#define MI_COMPAT_CTZ64(x) mi_compat_ctz64((unsigned long long)(x))
#elif defined(__GNUC__)
#define MI_COMPAT_CTZ64(x) ((unsigned int)__builtin_ctzll((unsigned long long)(x)))
#else
static MI_COMPAT_FORCEINLINE unsigned int mi_compat_ctz64(unsigned long long x)
{
    unsigned int n = 0;
    if (x == 0) return 64;
    while ((x & 1ull) == 0)
    {
        x >>= 1;
        ++n;
    }
    return n;
}
#define MI_COMPAT_CTZ64(x) mi_compat_ctz64((unsigned long long)(x))
#endif

/*
    Count leading zeros.  The caller must pass a nonzero value; see the ctz
    note above.
*/
#if defined(_MSC_VER) && defined(_M_X64)
static MI_COMPAT_FORCEINLINE unsigned int mi_compat_clz64(unsigned long long x)
{
    unsigned long index;
    _BitScanReverse64(&index, x);
    return 63u - (unsigned int)index;
}
#define MI_COMPAT_CLZ64(x) mi_compat_clz64((unsigned long long)(x))
#elif defined(_MSC_VER)
static MI_COMPAT_FORCEINLINE unsigned int mi_compat_clz64(unsigned long long x)
{
    unsigned long index;
    unsigned int high = (unsigned int)(x >> 32);
    if (high != 0)
    {
        _BitScanReverse(&index, high);
        return 31u - (unsigned int)index;
    }
    _BitScanReverse(&index, (unsigned int)x);
    return 63u - (unsigned int)index;
}
#define MI_COMPAT_CLZ64(x) mi_compat_clz64((unsigned long long)(x))
#elif defined(__GNUC__)
#define MI_COMPAT_CLZ64(x) ((unsigned int)__builtin_clzll((unsigned long long)(x)))
#else
static MI_COMPAT_FORCEINLINE unsigned int mi_compat_clz64(unsigned long long x)
{
    unsigned int n = 0;
    if (x == 0) return 64;
    while ((x & 0x8000000000000000ull) == 0)
    {
        x <<= 1;
        ++n;
    }
    return n;
}
#define MI_COMPAT_CLZ64(x) mi_compat_clz64((unsigned long long)(x))
#endif

#endif /* MICOMPAT_H */
