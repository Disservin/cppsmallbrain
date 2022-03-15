#pragma once
#include <intrin.h>
#include <iostream>  
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <bitset>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <iterator>
#include <vector>
#include <chrono>
#include <cmath>
#include <intrin.h>
#include <assert.h>

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)

#define _Compiletime __forceinline static constexpr
#define U64 unsigned __int64

struct Move {
    int piece;
    int from_square;
    int to_square;
    int promotion;
    int capture;
    int null;   // 1 == True 0 == False
};

inline int square_file(int sq) {
    //Gets the file index of the square where 0 is the a-file
    return sq & 7;
}
inline int square_rank(int sq) {
    //Gets the rank index of the square where 0 is the first rank."""
    return sq >> 3;
}

inline int square_distance(int a, int b) {
    return std::max(std::abs(square_file(a) - square_file(b)), std::abs(square_rank(a) - square_rank(b)));
}

inline int _test_bit(U64 bit, int sq) {
    __int64 test = bit;
    if (_bittest64(&test, sq)) {
        return true;
    }
    else {
        return false;
    }
}

#if defined(__GNUC__)  // GCC, Clang, ICC
inline int _bitscanreverse(U64 b) {
    return 63 ^ __builtin_clzll(b);
}


inline int _bitscanforward(U64 b) {
    return __builtin_ctzll(b);
}

#elif defined(_MSC_VER)  // MSVC
#ifdef _WIN64  // MSVC, WIN64
inline int _bitscanforward(U64 mask) {
    unsigned long index;
    _BitScanForward64(&index, mask);
    return index;
}

inline int _bitscanreverse(U64 mask) {
    unsigned long index;
    _BitScanReverse64(&index, mask);
    return index;
}
#endif
#endif

inline int popcount(U64 mask) {

#ifndef defined(_MSC_VER) || defined(__INTEL_COMPILER)

    return (int)_mm_popcnt_u64(mask);

#else // Assumed gcc or compatible compiler

    return __builtin_popcountll(mask);

#endif
}