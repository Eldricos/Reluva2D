// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Bounded, read-only shifted comparisons of opaque SPR payloads.
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace reluva::stafu {
struct SprShiftMatch {
    size_t startA=0,startB=0,length=0; // length in bytes or bits
};
struct SprShiftResult {
    size_t bytesA=0,bytesB=0;
    SprShiftMatch byteMatch,msbMatch,lsbMatch;
};
inline SprShiftResult surveySprShifts(const uint8_t* a,size_t na,
                                     const uint8_t* b,size_t nb,
                                     size_t maxBytes=4096,size_t maxShiftBytes=256) {
    SprShiftResult out;
    if(!a||!b||!maxBytes)return out;
    out.bytesA=std::min(na,maxBytes);out.bytesB=std::min(nb,maxBytes);
    // Byte offsets are compared at every relative displacement in a bounded range.
    // This is O(maxShift * maxBytes), with no allocations.
    const size_t maxShift=std::min(maxShiftBytes,maxBytes);
    for(size_t shift=0;shift<=maxShift;++shift)for(unsigned direction=0;direction<2;++direction) {
        if(shift==0&&direction==1)continue;
        const size_t ia=direction?shift:0,ib=direction?0:shift;
        if(ia>=out.bytesA||ib>=out.bytesB)continue;
        const size_t n=std::min(out.bytesA-ia,out.bytesB-ib);
        size_t run=0;
        for(size_t k=0;k<n;++k) {
            if(a[ia+k]==b[ib+k])++run;
            else run=0;
            if(run>out.byteMatch.length)
                out.byteMatch={ia+k+1-run,ib+k+1-run,run};
        }
    }
    // Test each bit displacement from -7 to +7 around each byte shift.
    // Bit order changes the interpretation of cross-byte bit runs.
    for(unsigned order=0;order<2;++order) {
        auto bit=[&](const uint8_t* p,size_t pos)->unsigned {
            return (p[pos/8]>>(order==0?7-pos%8:pos%8))&1u;
        };
        SprShiftMatch& best=order==0?out.msbMatch:out.lsbMatch;
        const size_t bitsA=out.bytesA*8,bitsB=out.bytesB*8;
        const size_t shiftLimit=maxShift*8+7;
        for(size_t shift=0;shift<=shiftLimit;++shift)for(unsigned direction=0;direction<2;++direction) {
            if(shift==0&&direction==1)continue;
            const size_t ia=direction?shift:0,ib=direction?0:shift;
            if(ia>=bitsA||ib>=bitsB)continue;
            const size_t n=std::min(bitsA-ia,bitsB-ib);
            size_t run=0;
            for(size_t k=0;k<n;++k) {
                if(bit(a,ia+k)==bit(b,ib+k))++run;
                else run=0;
                if(run>best.length)best={ia+k+1-run,ib+k+1-run,run};
            }
        }
    }
    return out;
}
}
