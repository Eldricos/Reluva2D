// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Independently recheck matches found by the bounded SPR shift survey.
#include "engines/stafu/spr_shift_survey.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
struct SprVerifiedMatch {
    bool valid=false;
    size_t startA=0,startB=0,length=0; // units are bytes or bits
    size_t extendedLength=0;           // forward extent in complete payloads
    bool truncatedA=false,truncatedB=false; // original match touched survey boundary
};
inline SprVerifiedMatch verifySprMatch(const uint8_t* a,size_t na,
                                      const uint8_t* b,size_t nb,
                                      SprShiftMatch m,unsigned bitOrder,
                                      size_t surveyedA,size_t surveyedB) {
    SprVerifiedMatch r;
    if(!a||!b||bitOrder>2||m.length==0)return r;
    const size_t unit=bitOrder==0?1:8;
    if(m.startA>na*unit||m.startB>nb*unit||
       m.length>na*unit-m.startA||m.length>nb*unit-m.startB)return r;
    auto at=[&](const uint8_t* p,size_t pos)->unsigned {
        if(bitOrder==0)return p[pos];
        return (p[pos/8]>>(bitOrder==1?7-pos%8:pos%8))&1u;
    };
    for(size_t k=0;k<m.length;++k)
        if(at(a,m.startA+k)!=at(b,m.startB+k))return r;
    r.valid=true;r.startA=m.startA;r.startB=m.startB;r.length=m.length;
    r.extendedLength=m.length;
    while(r.startA+r.extendedLength<na*unit &&
          r.startB+r.extendedLength<nb*unit &&
          at(a,r.startA+r.extendedLength)==at(b,r.startB+r.extendedLength))
        ++r.extendedLength;
    r.truncatedA=m.startA+m.length==std::min(na,surveyedA)*unit&&surveyedA<na;
    r.truncatedB=m.startB+m.length==std::min(nb,surveyedB)*unit&&surveyedB<nb;
    return r;
}
}
