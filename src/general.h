#pragma once
#include <iostream>  
#include <cmath>
#include <string>
#include <bitset>
#include <intrin.h>

//#pragma intrinsic(_BitScanForward)
//#pragma intrinsic(_BitScanReverse)

#define U64 unsigned __int64

enum Score {
    MATE = 20000,
    INFINITE = MATE + 1,
};

struct Move {
    __int8 piece = -1;
    __int8 from_square = -1;
    __int8 to_square = -1;
    __int8 promotion = -1;
};

//Gets the file index of the square where 0 is the a-file
inline __int8 square_file(__int8 sq) {
    return sq & 7;
}

//Gets the rank index of the square where 0 is the first rank."""
inline __int8 square_rank(__int8 sq) {
    return sq >> 3;
}

inline int square_distance(int a, int b) {
    return std::max(std::abs(square_file(a) - square_file(b)), std::abs(square_rank(a) - square_rank(b)));
}

inline bool get_square_color(int square) {
    if ((square % 8) % 2 == (square / 8) % 2) {
        return false;
    }
    else {
        return true;
    }
}

inline bool _test_bit(U64 bit, int sq) {
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

#else
inline __int8 _bitscanforward(U64 mask) {
    unsigned long index;
    _BitScanForward64(&index, mask);
    return index;
}

inline __int8 _bitscanreverse(U64 mask) {
    unsigned long index;
    _BitScanReverse64(&index, mask);
    return index;
}
#endif

//inline int popcount(U64 mask) {
//
//#ifndef defined(_MSC_VER) || defined(__INTEL_COMPILER)
//
//    return (int)_mm_popcnt_u64(mask);
//
//#else
//
//    return __builtin_popcountll(mask);
//
//#endif
//}
inline __int8 popcount(U64 mask) {
    return std::bitset<64>(mask).count();
}	
inline __int8 pop_lsb(U64* mask) {
    const __int8 s = _bitscanforward(*mask);
    //*mask = _blsr_u64(*mask);
    *mask &= *mask - 1;
    return s;
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

