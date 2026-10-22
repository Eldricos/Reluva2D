// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_cross_bit_compare.h"
#include <cassert>
#include <cstdint>
int main() {
    using reluva::stafu::compareSprBits;
    const uint8_t a[]={0x12,0x34,0x56,0x78,0x9a,0xbc,0xde,0xf0};
    auto same=compareSprBits(a,8,a,8);
    assert(same.bytesA==8&&same.bytesB==8&&same.commonPrefix==8);
    assert(same.msb.size()==24&&same.lsb.size()==24);
    for(const auto& t:same.msb)assert(t.tokens==t.matching);
    for(const auto& t:same.lsb)assert(t.tokens==t.matching);
    const uint8_t b[]={0x12,0x34,0xff,0x78,0x9a,0xbc,0xde,0xf0};
    auto changed=compareSprBits(a,8,b,8);
    assert(changed.commonPrefix==2);
    assert(changed.msb[0].matching==7);
    assert(changed.lsb[0].matching==7);
    auto limited=compareSprBits(a,8,b,8,2);
    assert(limited.bytesA==2&&limited.bytesB==2&&limited.commonPrefix==2);
    assert(limited.msb[0].tokens==2);
    auto empty=compareSprBits(nullptr,8,b,8);
    assert(empty.msb.empty()&&empty.bytesA==0);
    auto zero=compareSprBits(a,8,b,8,0);
    assert(zero.msb.empty());
    return 0;
}
