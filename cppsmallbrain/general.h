#pragma once
#include <intrin.h>
#include <iostream>  
#include <cmath>

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)

#define _Compiletime __forceinline static constexpr
#define U64 unsigned __int64

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
