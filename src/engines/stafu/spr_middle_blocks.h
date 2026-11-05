// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Bounded, read-only matching blocks inside the non-shared middle of two payloads.
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace reluva::stafu {
struct SprMiddleBlock {size_t a=0,b=0,length=0; int shift=0;};
inline std::vector<SprMiddleBlock> findSprMiddleBlocks(
    const uint8_t* a,size_t na,const uint8_t* b,size_t nb,
    size_t prefix,size_t suffix,int radius=32,size_t minimum=16) {
    std::vector<SprMiddleBlock> out;
    if((!a&&na)||(!b&&nb)||radius<0||radius>4096||
       prefix>na||prefix>nb||suffix>na-prefix||suffix>nb-prefix||minimum==0)
        return out;
    const size_t endA=na-suffix,endB=nb-suffix;
    for(int shift=-radius;shift<=radius;++shift) {
        const size_t beginA=prefix+(shift<0?static_cast<size_t>(-shift):0);
        const size_t beginB=prefix+(shift>0?static_cast<size_t>(shift):0);
        if(beginA>=endA||beginB>=endB)continue;
        const size_t count=std::min(endA-beginA,endB-beginB);
        size_t start=0,run=0;
        for(size_t i=0;i<=count;++i) {
            if(i<count&&a[beginA+i]==b[beginB+i]) {
                if(run==0)start=i;
                ++run;
            } else {
                if(run>=minimum)out.push_back({beginA+start,beginB+start,run,shift});
                run=0;
            }
        }
    }
    std::sort(out.begin(),out.end(),[](const auto& x,const auto& y){
        if(x.length!=y.length)return x.length>y.length;
        if(x.a!=y.a)return x.a<y.a;
        return x.b<y.b;
    });
    return out;
}
}
