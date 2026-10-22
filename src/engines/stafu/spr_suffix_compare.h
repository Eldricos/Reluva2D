// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Bounded, read-only end-anchored bit comparisons. No compression semantics.
#include <algorithm>
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
struct SprSuffixResult {
    size_t bits=0;
    size_t trimmedA=0,trimmedB=0;
    size_t suffixBits=0;
    size_t rawSuffixBits=0;
};
inline SprSuffixResult compareSprSuffix(const uint8_t* a,size_t na,
                                       const uint8_t* b,size_t nb,
                                       bool msb=true,size_t maxBytes=4096,
                                       bool testPadding=true) {
    SprSuffixResult result;
    if(!a||!b||!maxBytes)return result;
    const size_t nA=std::min(na,maxBytes),nB=std::min(nb,maxBytes);
    const size_t bitsA=nA*8,bitsB=nB*8;
    result.bits=std::min(bitsA,bitsB);
    // Examine the actual final bytes, not the initial prefix.
    a+=na-nA;b+=nb-nB;
    auto bit=[&](const uint8_t* p,size_t pos)->unsigned {
        return (p[pos/8]>>(msb?7-pos%8:pos%8))&1u;
    };
    auto count=[&](size_t trimA,size_t trimB)->size_t {
        const size_t cap=std::min(bitsA-trimA,bitsB-trimB);
        size_t matched=0;
        while(matched<cap&&bit(a,bitsA-trimA-1-matched)==bit(b,bitsB-trimB-1-matched))
            ++matched;
        return matched;
    };
    result.rawSuffixBits=count(0,0);
    result.suffixBits=result.rawSuffixBits;
    if(testPadding)for(size_t ta=0;ta<8&&ta<bitsA;++ta)
        for(size_t tb=0;tb<8&&tb<bitsB;++tb) {
            const size_t matched=count(ta,tb);
            if(matched>result.suffixBits) {
                result.suffixBits=matched;
                result.trimmedA=ta;result.trimmedB=tb;
            }
        }
    return result;
}
}
