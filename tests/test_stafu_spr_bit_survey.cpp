// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_bit_survey.h"
#include <cassert>
#include <cstdint>
int main() {
    using reluva::stafu::surveySprBits;
    assert(surveySprBits(nullptr,10).bytesExamined==0);
    const uint8_t bytes[]={0x80,0x00,0xff,0x80};
    auto r=surveySprBits(bytes,4);
    assert(r.bytesExamined==4);
    assert(r.msb.ones==10&&r.lsb.ones==10);
    assert(r.msb.transitions==3);
    assert(r.lsb.transitions==5);
    assert(r.msb.windows12==21&&r.lsb.windows12==21);
    assert(r.msb.repeated12<=r.msb.windows12);
    assert(r.lsb.repeated12<=r.lsb.windows12);
    auto limited=surveySprBits(bytes,4,1);
    assert(limited.bytesExamined==1);
    assert(limited.msb.ones==1&&limited.lsb.ones==1);
    assert(limited.msb.windows12==0&&limited.lsb.windows12==0);
    assert(surveySprBits(bytes,4,0).bytesExamined==0);
    const uint8_t repeating[]={0,0,0,0};
    auto repeated=surveySprBits(repeating,4);
    assert(repeated.msb.repeated12==20&&repeated.lsb.repeated12==20);
    assert(repeated.msb.alignedEightRepeats[0]==3);
    return 0;
}
