// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Deterministic controls for SPR end-anchored suffix matches. No format claims.
#include "engines/stafu/spr_suffix_compare.h"
#include "engines/stafu/spr_terminal_bits.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace reluva::stafu {
struct SprSuffixControl {
    size_t matchedBits=0;
    size_t trimA=0,trimB=0;
    bool trimmedZero=false;
};
inline SprSuffixControl inspectSprSuffixControl(const uint8_t* a,size_t na,
                                                const uint8_t* b,size_t nb) {
    SprSuffixControl out;
    const auto r=compareSprSuffix(a,na,b,nb,true);
    out.matchedBits=r.suffixBits;out.trimA=r.trimmedA;out.trimB=r.trimmedB;
    out.trimmedZero=sprTerminalBitsZero(a,na,r.trimmedA)&&
                    sprTerminalBitsZero(b,nb,r.trimmedB);
    return out;
}
// Shuffle byte positions deterministically; preserves byte frequency, destroys order.
// Input is copied, never mutated. xorshift32 has no platform-dependent RNG state.
inline std::vector<uint8_t> shuffledSprBytes(const uint8_t* data,size_t n,
                                             uint32_t seed=0x9e3779b9u) {
    if(!data||!n)return {};
    std::vector<uint8_t> result(data,data+n);
    uint32_t state=seed?seed:1u;
    for(size_t i=n-1;i>0;--i) {
        state^=state<<13;state^=state>>17;state^=state<<5;
        const size_t j=static_cast<size_t>(state)%(i+1);
        std::swap(result[i],result[j]);
    }
    return result;
}
}
