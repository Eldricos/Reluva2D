// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Descriptive boundaries for verified MSB-first matches; no format semantics.
#include "engines/stafu/spr_shift_verify.h"
#include <cstddef>
#include <cstdint>
#include <limits>
namespace reluva::stafu {
struct SprBoundaryReport {
    bool valid=false;
    size_t startA=0,startB=0,endA=0,endB=0;
    size_t leadingA=0,leadingB=0,trailingA=0,trailingB=0;
    bool reachesEndA=false,reachesEndB=false;
    bool priorBitsEqual=false,hasPriorBits=false;
    bool nextBitsEqual=false,hasNextBits=false;
};
inline SprBoundaryReport describeSprBoundary(const uint8_t* a,size_t na,
                                             const uint8_t* b,size_t nb,
                                             const SprVerifiedMatch& match) {
    SprBoundaryReport r;
    if(!a||!b||!match.valid||na>std::numeric_limits<size_t>::max()/8||
       nb>std::numeric_limits<size_t>::max()/8)return r;
    const size_t ba=na*8,bb=nb*8;
    if(match.startA>ba||match.startB>bb||
       match.extendedLength>ba-match.startA||
       match.extendedLength>bb-match.startB)return r;
    r.valid=true;r.startA=match.startA;r.startB=match.startB;
    r.endA=r.startA+match.extendedLength;r.endB=r.startB+match.extendedLength;
    r.leadingA=r.startA;r.leadingB=r.startB;
    r.trailingA=ba-r.endA;r.trailingB=bb-r.endB;
    r.reachesEndA=r.trailingA==0;r.reachesEndB=r.trailingB==0;
    const auto bit=[](const uint8_t* p,size_t pos) {
        return (p[pos/8]>>(7-pos%8))&1u;
    };
    r.hasPriorBits=r.startA>0&&r.startB>0;
    if(r.hasPriorBits)r.priorBitsEqual=bit(a,r.startA-1)==bit(b,r.startB-1);
    r.hasNextBits=r.endA<ba&&r.endB<bb;
    if(r.hasNextBits)r.nextBitsEqual=bit(a,r.endA)==bit(b,r.endB);
    return r;
}
}
