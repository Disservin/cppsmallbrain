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
    int piece = -1;
    int from_square = -1;
    int to_square = -1;
    int promotion = -1;
    int null = 0;   // 1 == True 0 == False
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


static const std::string square_to_coordinates[64] = {
"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};

