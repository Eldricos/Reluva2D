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
    // Nonoverlapping byte-aligned prefix/suffix partition; middle may be empty.
    size_t middleA=0,middleB=0;
    // MSB-first terminal suffix can start mid-byte; only count if nonoverlapping.
    size_t suffixStartBitA=0,suffixStartBitB=0;
    bool suffixOverlapsPrefix=false;
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
    // Prevent double-counting identical bytes as both prefix and suffix.
    const size_t usableSuffix=std::min({r.suffixBytes,na-r.prefixBytes,nb-r.prefixBytes});
    r.suffixBytes=usableSuffix;
    r.middleA=na-r.prefixBytes-r.suffixBytes;
    r.middleB=nb-r.prefixBytes-r.suffixBytes;
    const auto bits=compareSprSuffix(a,na,b,nb,true,4096,true);
    r.sharedSuffixBits=bits.suffixBits;
    r.trimA=bits.trimmedA;r.trimB=bits.trimmedB;
    r.suffixStartBitA=na*8-r.trimA-r.sharedSuffixBits;
    r.suffixStartBitB=nb*8-r.trimB-r.sharedSuffixBits;
    r.suffixOverlapsPrefix=r.suffixStartBitA<r.prefixBytes*8||
                           r.suffixStartBitB<r.prefixBytes*8;
    r.trimmedZero=sprTerminalBitsZero(a,na,r.trimA)&&
                  sprTerminalBitsZero(b,nb,r.trimB);
    // The comparison is capped to 4096 bytes per payload.
    r.fullSmallerSuffix=smaller<=4096&&r.sharedSuffixBits>=smaller*8;
    return r;
}
}
