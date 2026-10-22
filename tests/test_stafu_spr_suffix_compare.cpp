// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_suffix_compare.h"
#include <cassert>
#include <cstdint>
int main() {
    using reluva::stafu::compareSprSuffix;
    const uint8_t a[]={0x11,0x22,0x33,0x44};
    const uint8_t b[]={0xff,0x22,0x33,0x44};
    const auto same=compareSprSuffix(a,4,b,4,true,4,false);
    // The preceding 0x11 and 0xff bytes also share their final bit.
    assert(same.rawSuffixBits==25&&same.suffixBits==25);
    const uint8_t c[]={0x11,0x22,0x33,0x45};
    assert(compareSprSuffix(a,4,c,4,true,4,false).rawSuffixBits==0);
    const uint8_t d[]={0x80};
    const uint8_t e[]={0x40};
    const auto padded=compareSprSuffix(d,1,e,1,true,1,true);
    assert(padded.suffixBits>=1&&padded.trimmedA<8&&padded.trimmedB<8);
    const auto limited=compareSprSuffix(a,4,b,4,true,2,false);
    assert(limited.rawSuffixBits==16);
    assert(compareSprSuffix(nullptr,4,b,4).suffixBits==0);
    assert(compareSprSuffix(a,4,b,4,true,0).suffixBits==0);
    return 0;
}
