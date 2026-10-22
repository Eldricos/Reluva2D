// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Cross-payload bit comparisons are observations, not image decoding.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vector>

namespace reluva::stafu {
struct SprCrossBitResult {
    struct Trial {
        unsigned width=0,phase=0;
        size_t tokens=0,matching=0,baselineMatching=0;
    };
    size_t bytesA=0,bytesB=0,commonPrefix=0;
    std::vector<Trial> msb,lsb;
};
inline SprCrossBitResult compareSprBits(const uint8_t* a,size_t na,
                                       const uint8_t* b,size_t nb,
                                       size_t maxBytes=4096) {
    SprCrossBitResult out;
    if(!a||!b||!maxBytes)return out;
    out.bytesA=std::min(na,maxBytes);
    out.bytesB=std::min(nb,maxBytes);
    const size_t common=std::min(out.bytesA,out.bytesB);
    while(out.commonPrefix<common&&a[out.commonPrefix]==b[out.commonPrefix])
        ++out.commonPrefix;
    // Fixed-seed Fisher-Yates shuffle of B's bytes: preserves byte frequencies,
    // destroys positional correspondence. Baseline is a control, not a p-value.
    std::vector<uint8_t> shuffled(b,b+out.bytesB);
    uint32_t seed=0x9e3779b9u;
    for(size_t i=shuffled.size();i>1;--i) {
        seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
        std::swap(shuffled[i-1],shuffled[seed%i]);
    }
    for(unsigned mode=0;mode<2;++mode) {
        auto& trials=mode==0?out.msb:out.lsb;
        auto bit=[&](const uint8_t* p,size_t pos)->unsigned {
            return (p[pos/8]>>(mode==0?7-pos%8:pos%8))&1u;
        };
        for(unsigned width:{8u,12u,16u})for(unsigned phase=0;phase<8;++phase) {
            SprCrossBitResult::Trial t;
            t.width=width;t.phase=phase;
            const size_t bits=common*8;
            for(size_t start=phase;start+width<=bits;start+=width) {
                uint32_t va=0,vb=0,control=0;
                for(unsigned k=0;k<width;++k) {
                    va=(va<<1)|bit(a,start+k);
                    vb=(vb<<1)|bit(b,start+k);
                    control=(control<<1)|bit(shuffled.data(),start+k);
                }
                ++t.tokens;
                t.matching+=va==vb;
                t.baselineMatching+=va==control;
            }
            trials.push_back(t);
        }
    }
    return out;
}
}
