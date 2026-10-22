// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_shift_verify.h"
#include <cassert>
#include <cstdint>
int main() {
    using namespace reluva::stafu;
    const uint8_t a[]={0x10,0x20,0x30,0x40,0x50,0x60};
    const uint8_t b[]={0xff,0x10,0x20,0x30,0x40,0x50,0x60};
    auto survey=surveySprShifts(a,6,b,7,4,2);
    auto verified=verifySprMatch(a,6,b,7,survey.byteMatch,0,survey.bytesA,survey.bytesB);
    assert(verified.valid&&verified.extendedLength==6);
    assert(!verified.truncatedA&&verified.truncatedB);
    auto bad=verifySprMatch(a,6,b,7,{0,0,2},0,4,4);
    assert(!bad.valid);
    auto outside=verifySprMatch(a,6,b,7,{100,0,2},0,4,4);
    assert(!outside.valid);
    auto msb=verifySprMatch(a,6,b,7,{0,8,48},1,4,4);
    auto lsb=verifySprMatch(a,6,b,7,{0,8,48},2,4,4);
    assert(msb.valid&&msb.extendedLength==48);
    assert(lsb.valid&&lsb.extendedLength==48);
    // One-bit displacement: 0x55 followed by 0x55 in MSB order
    // matches the leading 15 bits of 0x2a,0xaa after shifting by one bit.
    const uint8_t c[]={0x55,0x55};
    const uint8_t d[]={0x2a,0xaa,0x80};
    auto shifted=verifySprMatch(c,2,d,3,{0,1,15},1,2,3);
    assert(shifted.valid&&shifted.extendedLength>=15);
    assert(!verifySprMatch(c,2,d,3,{0,1,15},2,2,3).valid);
    assert(!verifySprMatch(nullptr,2,d,3,{0,0,1},1,2,3).valid);
    return 0;
}
