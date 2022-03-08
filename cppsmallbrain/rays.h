#include <intrin.h>
#pragma once
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)
#define U64 unsigned long long int


U64 _rays[8][64];

const U64 not_a_file = 0xfefefefefefefefe;

const U64 not_h_file = 0x7f7f7f7f7f7f7f7f;


inline int _bitscanforward(U64 mask) {
    //if (mask == 0) {
    //    return -1;
    //}
    unsigned long index;
    _BitScanForward64(&index, mask);
    return index & 4294967295;
}
inline int _bitscanreverse(U64 mask) {
    //if (mask == 0) {
    //    return -1;
    //}
    unsigned long index;
    _BitScanReverse64(&index, mask);
    return index & 4294967295;

}

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

int _row(int sq) {
    return sq / 8;
}

int _col(int sq) {
    return sq % 8;
}

U64 _eastN(U64 mask, int n) {
    U64 newmask = mask;
    for (int i = 0; i < n; i++) {
        newmask = ((newmask << 1) & (not_a_file));
    }

    return newmask;
}


U64 _westN(U64 mask, int n) {
    U64 newmask = mask;
    for (int i = 0; i < n; i++) {
        newmask = ((newmask >> 1) & (not_h_file));
    }

    return newmask;
}
void init_rays() {
    for (int sq = 0; sq < 64; sq++) {
        // North
        _rays[NORTH][sq] = 0x0101010101010100ULL << sq;

        // South
        _rays[SOUTH][sq] = 0x0080808080808080ULL >> (63 - sq);

        // East
        _rays[EAST][sq] = 2 * ((1ULL << (sq | 7)) - (1ULL << sq));

        // West
        _rays[WEST][sq] = (1ULL << sq) - (1ULL << (sq & 56));

        // North West
        _rays[NORTH_WEST][sq] = _westN(0x102040810204000ULL, 7 - _col(sq)) << (_row(sq) * 8);

        // North East
        _rays[NORTH_EAST][sq] = _eastN(0x8040201008040200ULL, _col(sq)) << (_row(sq) * 8);

        // South West
        _rays[SOUTH_WEST][sq] = _westN(0x40201008040201ULL, 7 - _col(sq)) >> ((7 - _row(sq)) * 8);

        // South East
        _rays[SOUTH_EAST][sq] = _eastN(0x2040810204080ULL, _col(sq)) >> ((7 - _row(sq)) * 8);
    }
}