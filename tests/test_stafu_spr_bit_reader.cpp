// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_bit_reader.h"
#include <cassert>
#include <cstdint>
int main() {
    using reluva::stafu::readSprBits;
    const uint8_t bytes[]={0xb2,0x65,0xf0,0x0f};
    uint32_t v=99;
    assert(readSprBits(bytes,4,0,8,v)&&v==0xb2);
    assert(readSprBits(bytes,4,4,8,v)&&v==0x26);
    assert(readSprBits(bytes,4,7,9,v)&&v==0x65);
    assert(readSprBits(bytes,4,16,16,v)&&v==0xf00f);
    assert(readSprBits(bytes,4,0,32,v)&&v==0xb265f00fu);
    assert(readSprBits(bytes,4,31,1,v)&&v==1);
    assert(!readSprBits(bytes,4,32,1,v)&&v==0);
    assert(!readSprBits(bytes,4,0,0,v));
    assert(!readSprBits(bytes,4,0,33,v));
    assert(!readSprBits(bytes,4,31,2,v));
    assert(!readSprBits(nullptr,4,0,1,v));
    return 0;
}
