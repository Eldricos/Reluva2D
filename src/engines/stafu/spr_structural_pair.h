// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Bounded descriptive prefix and repetition measures; no codec assumptions.
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
namespace reluva::stafu {
struct SprStructuralPair {
    bool valid=false;
    size_t commonPrefix=0;
    size_t alignedEqual=0;
    size_t alignedCompared=0;
    size_t repeatAdjacentA=0,repeatAdjacentB=0;
    size_t longestRunA=0,longestRunB=0;
};
inline SprStructuralPair surveySprStructuralPair(
    const uint8_t* a,size_t na,const uint8_t* b,size_t nb,
    size_t limit=4096,size_t skip=0) {
    SprStructuralPair out;
    if(!a||!b||!na||!nb||!limit||skip>=na||skip>=nb)return out;
    const size_t ca=std::min(na-skip,limit),cb=std::min(nb-skip,limit);
    a+=skip;b+=skip;out.valid=true;
    const size_t both=std::min(ca,cb);
    while(out.commonPrefix<both&&a[out.commonPrefix]==b[out.commonPrefix])
        ++out.commonPrefix;
    out.alignedCompared=both;
    for(size_t i=0;i<both;++i)if(a[i]==b[i])++out.alignedEqual;
    const auto measure=[](const uint8_t* p,size_t n,size_t& adjacent,size_t& run) {
        size_t current=1;run=1;
        for(size_t i=1;i<n;++i) {
            if(p[i]==p[i-1]){++adjacent;++current;}
            else current=1;
            run=std::max(run,current);
        }
    };
    measure(a,ca,out.repeatAdjacentA,out.longestRunA);
    measure(b,cb,out.repeatAdjacentB,out.longestRunB);
    return out;
}
}
