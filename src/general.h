#pragma once
#include <iostream>  
#include <cmath>
#include <string>
#include <bitset>
#include <intrin.h>
#include <ostream>

//#pragma intrinsic(_BitScanForward)
//#pragma intrinsic(_BitScanReverse)

#define U64 unsigned __int64

static constexpr int8_t late_move_pruning_margins[4] = { 4, 8, 12, 24 };

enum Score {
    MATE = 20000,
    INFINITE = MATE + 1,
};

struct Move {
    int8_t  piece = -1;
    int8_t  from_square = -1;
    int8_t  to_square = -1;
    int8_t  promotion = -1;
    int8_t  capture = -1;
};

//Gets the file index of the square where 0 is the a-file
inline int8_t square_file(int8_t sq) {
    return sq & 7;
}

//Gets the rank index of the square where 0 is the first rank."""
inline int8_t square_rank(int8_t sq) {
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
    return std::bitset<64>(bit).test(sq);
}

#if defined(__GNUC__)  // GCC, Clang, ICC
inline int _bitscanreverse(U64 b) {
    return 63 ^ __builtin_clzll(b);
}


inline int _bitscanforward(U64 b) {
    return __builtin_ctzll(b);
}

#else
inline uint8_t _bitscanforward(U64 mask) {
    unsigned long index;
    _BitScanForward64(&index, mask);
    return (uint8_t) index;
}

inline uint8_t _bitscanreverse(U64 mask) {
    unsigned long index;
    _BitScanReverse64(&index, mask);
    return (uint8_t) index;
}
#endif

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
inline uint8_t popcount(U64 mask) {

    return (uint8_t)_mm_popcnt_u64(mask);
}
#else
inline uint8_t popcount(U64 mask) {
    return (uint8_t)__builtin_popcountll(mask);
};
#endif


inline int8_t pop_lsb(U64& mask) {
    int8_t s = _bitscanforward(mask);
    mask = _blsr_u64(mask);
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

