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

enum Score {
    MATE = 20000,
    INFINITE = MATE + 1,
};

// Move directions
enum Dir {
    NORTH,
    SOUTH,
    EAST,
    WEST,
    NORTH_EAST,
    NORTH_WEST,
    SOUTH_EAST,
    SOUTH_WEST
};
// Piece
enum {
    WPAWN,
    WKNIGHT,
    WBISHOP,
    WROOK,
    WQUEEN,
    WKING,
    BPAWN,
    BKNIGHT,
    BBISHOP,
    BROOK,
    BQUEEN,
    BKING
};
// Piece Types
enum {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

enum {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    NO_SQ
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

//returns reversed bitboard (rotate 180 degrees)
inline U64 reverse(U64 bb) {
    bb = (bb & 0x5555555555555555) << 1 | ((bb >> 1) & 0x5555555555555555);
    bb = (bb & 0x3333333333333333) << 2 | ((bb >> 2) & 0x3333333333333333);
    bb = (bb & 0x0f0f0f0f0f0f0f0f) << 4 | ((bb >> 4) & 0x0f0f0f0f0f0f0f0f);
    bb = (bb & 0x00ff00ff00ff00ff) << 8 | ((bb >> 8) & 0x00ff00ff00ff00ff);

    return (bb << 48) | ((bb & 0xffff0000) << 16) | ((bb >> 16) & 0xffff0000) | (bb >> 48);
}

//file masks
static constexpr U64 MASK_FILE[8] = {
    0x101010101010101, 0x202020202020202, 0x404040404040404, 0x808080808080808,
    0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080,
};

//rank masks
static constexpr U64 MASK_RANK[8] = {
    0xff, 0xff00, 0xff0000, 0xff000000,
    0xff00000000, 0xff0000000000, 0xff000000000000, 0xff00000000000000
};

//diagonal masks
static constexpr U64 MASK_DIAGONAL[15] = {
    0x80, 0x8040, 0x804020,
    0x80402010, 0x8040201008, 0x804020100804,
    0x80402010080402, 0x8040201008040201, 0x4020100804020100,
    0x2010080402010000, 0x1008040201000000, 0x804020100000000,
    0x402010000000000, 0x201000000000000, 0x100000000000000,
};

//anti-diagonal masks
static constexpr U64 MASK_ANTI_DIAGONAL[15] = {
    0x1, 0x102, 0x10204,
    0x1020408, 0x102040810, 0x10204081020,
    0x1020408102040, 0x102040810204080, 0x204081020408000,
    0x408102040800000, 0x810204080000000, 0x1020408000000000,
    0x2040800000000000, 0x4080000000000000, 0x8000000000000000
};

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

