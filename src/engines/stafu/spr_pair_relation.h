// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Descriptive, bounded SPR image-pair relationships. No frame semantics.
#include "engines/stafu/spr_suffix_compare.h"
#include "engines/stafu/spr_terminal_bits.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
namespace reluva::stafu {
struct SprPairRelation {
    bool valid=false,exact=false;
    size_t prefixBytes=0,suffixBytes=0,sharedSuffixBits=0;
    size_t trimA=0,trimB=0;
    bool trimmedZero=false,fullSmallerSuffix=false;
};
inline SprPairRelation compareSprPair(const uint8_t* a,size_t na,
                                      const uint8_t* b,size_t nb) {
    SprPairRelation r;
    if(!a||!b||!na||!nb||na>std::numeric_limits<size_t>::max()/8||
       nb>std::numeric_limits<size_t>::max()/8)return r;
    r.valid=true;
    const size_t smaller=std::min(na,nb);
    while(r.prefixBytes<smaller&&a[r.prefixBytes]==b[r.prefixBytes])++r.prefixBytes;
    while(r.suffixBytes<smaller&&a[na-r.suffixBytes-1]==b[nb-r.suffixBytes-1])
        ++r.suffixBytes;
    r.exact=na==nb&&r.prefixBytes==na;
    const auto bits=compareSprSuffix(a,na,b,nb,true,4096,true);
    r.sharedSuffixBits=bits.suffixBits;
    r.trimA=bits.trimmedA;r.trimB=bits.trimmedB;
    r.trimmedZero=sprTerminalBitsZero(a,na,r.trimA)&&
                  sprTerminalBitsZero(b,nb,r.trimB);
    // The comparison is capped to 4096 bytes per payload.
    r.fullSmallerSuffix=smaller<=4096&&r.sharedSuffixBits>=smaller*8;
    return r;
}
}
