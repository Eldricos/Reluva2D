// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_shift_survey.h"
#include <cassert>
#include <cstdint>
int main() {
    using reluva::stafu::surveySprShifts;
    const uint8_t a[]={0x10,0x21,0x32,0x43,0x54,0x65};
    const uint8_t b[]={0xff,0x10,0x21,0x32,0x43,0x54,0x65};
    auto shifted=surveySprShifts(a,6,b,7,7,2);
    assert(shifted.byteMatch.length==6);
    assert(shifted.byteMatch.startA==0&&shifted.byteMatch.startB==1);
    assert(shifted.msbMatch.length>=48&&shifted.lsbMatch.length>=48);
    auto limited=surveySprShifts(a,6,b,7,3,2);
    assert(limited.bytesA==3&&limited.bytesB==3);
    assert(limited.byteMatch.length==2);
    auto empty=surveySprShifts(nullptr,6,b,7);
    assert(empty.byteMatch.length==0&&empty.msbMatch.length==0);
    auto zero=surveySprShifts(a,6,b,7,0);
    assert(zero.bytesA==0&&zero.lsbMatch.length==0);
    const uint8_t c[]={0x01,0x23,0x45,0x67,0x89};
    const uint8_t d[]={0x00,0x12,0x34,0x56,0x78,0x90};
    auto bits=surveySprShifts(c,5,d,6,6,1);
    assert(bits.msbMatch.length>0&&bits.msbMatch.length<=40);
    return 0;
}
