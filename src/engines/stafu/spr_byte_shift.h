// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Read-only byte alignment survey. Shift means B index minus A index.
// Matching runs are observations, not decoded pixel or codec boundaries.
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace reluva::stafu {
struct SprShiftMatch {
    int shift=0;
    size_t startA=0,startB=0,length=0;
    size_t compared=0,equal=0;
};
inline std::vector<SprShiftMatch> surveySprByteShifts(
    const uint8_t* a,size_t na,const uint8_t* b,size_t nb,int radius=32) {
    std::vector<SprShiftMatch> result;
    if ((!a&&na)||(!b&&nb)||radius<0||radius>4096) return result;
    for(int shift=-radius;shift<=radius;++shift) {
        const size_t startA=shift<0?static_cast<size_t>(-shift):0;
        const size_t startB=shift>0?static_cast<size_t>(shift):0;
        SprShiftMatch m;
        m.shift=shift;
        if(startA>=na||startB>=nb) {result.push_back(m);continue;}
        m.compared=std::min(na-startA,nb-startB);
        size_t runStart=0,run=0;
        for(size_t i=0;i<m.compared;++i) {
            if(a[startA+i]==b[startB+i]) {
                ++m.equal;
                if(run==0)runStart=i;
                ++run;
                if(run>m.length) {
                    m.length=run;
                    m.startA=startA+runStart;
                    m.startB=startB+runStart;
                }
            } else run=0;
        }
        result.push_back(m);
    }
    std::sort(result.begin(),result.end(),[](const auto& x,const auto& y){
        if(x.length!=y.length)return x.length>y.length;
        if(x.equal!=y.equal)return x.equal>y.equal;
        const int ax=x.shift<0?-x.shift:x.shift;
        const int ay=y.shift<0?-y.shift:y.shift;
        return ax!=ay?ax<ay:x.shift<y.shift;
    });
    return result;
}
}
