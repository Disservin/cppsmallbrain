#pragma once
#include <intrin.h>

#define U64 unsigned __int64

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