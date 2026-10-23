// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// End-anchored three-payload MSB suffix survey. No encoding semantics.
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
namespace reluva::stafu {
struct SprTripleSuffix {
    bool valid=false;
    size_t sharedBits=0;
    size_t trimA=0,trimB=0,trimC=0;
    size_t startA=0,startB=0,startC=0;
    bool trimsZero=false;
};
inline SprTripleSuffix compareSprTripleSuffix(const uint8_t* a,size_t na,
                                              const uint8_t* b,size_t nb,
                                              const uint8_t* c,size_t nc,
                                              size_t maxBytes=4096) {
    SprTripleSuffix out;
    if(!a||!b||!c||!na||!nb||!nc||!maxBytes||
       na>std::numeric_limits<size_t>::max()/8||
       nb>std::numeric_limits<size_t>::max()/8||
       nc>std::numeric_limits<size_t>::max()/8)return out;
    out.valid=true;
    const size_t sa=std::min(na,maxBytes),sb=std::min(nb,maxBytes),
                 sc=std::min(nc,maxBytes);
    const size_t ba=sa*8,bb=sb*8,bc=sc*8;
    const uint8_t* pa=a+na-sa,*pb=b+nb-sb,*pc=c+nc-sc;
    const auto bit=[](const uint8_t* p,size_t pos)->unsigned {
        return (p[pos/8]>>(7-pos%8))&1u;
    };
    const auto zero=[](const uint8_t* p,size_t n,size_t trim)->bool {
        if(!trim)return true;
        return (p[n-1]&((1u<<trim)-1u))==0;
    };
    // Consider only plausible zero-valued terminal padding.
    for(size_t ta=0;ta<8&&ta<ba;++ta) {
        if(!zero(pa,sa,ta))continue;
        for(size_t tb=0;tb<8&&tb<bb;++tb) {
            if(!zero(pb,sb,tb))continue;
            for(size_t tc=0;tc<8&&tc<bc;++tc) {
                if(!zero(pc,sc,tc))continue;
                const size_t cap=std::min({ba-ta,bb-tb,bc-tc});
                size_t n=0;
                while(n<cap) {
                    const unsigned x=bit(pa,ba-ta-1-n);
                    if(x!=bit(pb,bb-tb-1-n)||x!=bit(pc,bc-tc-1-n))break;
                    ++n;
                }
                if(n>out.sharedBits) {
                    out.sharedBits=n;
                    out.trimA=ta;out.trimB=tb;out.trimC=tc;
                }
            }
        }
    }
    out.startA=na*8-out.trimA-out.sharedBits;
    out.startB=nb*8-out.trimB-out.sharedBits;
    out.startC=nc*8-out.trimC-out.sharedBits;
    out.trimsZero=true;
    return out;
}
}
