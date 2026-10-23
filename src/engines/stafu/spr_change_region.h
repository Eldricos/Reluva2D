// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Descriptive byte-prefix / shifted-bit-suffix boundaries. No codec semantics.
#include "engines/stafu/spr_pair_relation.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
struct SprChangeRegion {
    bool valid=false;
    size_t prefixBits=0;
    size_t suffixStartA=0,suffixStartB=0;
    size_t unmatchedA=0,unmatchedB=0;
    size_t suffixBits=0;
    size_t trimA=0,trimB=0;
    bool overlapsPrefix=false;
};
inline SprChangeRegion describeSprChangeRegion(
    const uint8_t* a,size_t na,const uint8_t* b,size_t nb) {
    SprChangeRegion out;
    const auto r=compareSprPair(a,na,b,nb);
    if(!r.valid)return out;
    out.valid=true;
    out.prefixBits=r.prefixBytes*8;
    out.suffixStartA=r.suffixStartBitA;
    out.suffixStartB=r.suffixStartBitB;
    out.suffixBits=r.sharedSuffixBits;
    out.trimA=r.trimA;out.trimB=r.trimB;
    out.overlapsPrefix=r.suffixOverlapsPrefix;
    // Saturate when prefix and suffix regions overlap.
    out.unmatchedA=out.suffixStartA>out.prefixBits?
        out.suffixStartA-out.prefixBits:0;
    out.unmatchedB=out.suffixStartB>out.prefixBits?
        out.suffixStartB-out.prefixBits:0;
    return out;
}
}
