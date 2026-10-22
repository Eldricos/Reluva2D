// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Read-only exploratory MSB-first token repetition; widths/phases are hypotheses.
#include "engines/stafu/spr_bit_reader.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
namespace reluva::stafu {
struct SprPatternRow {
    unsigned width=0, phase=0;
    size_t tokens=0, adjacentEqual=0, distinct=0, mostFrequent=0;
};
inline SprPatternRow surveySprPattern(const uint8_t* data,size_t bytes,
                                     size_t startBit,unsigned width,unsigned phase,
                                     size_t maxTokens=256) {
    SprPatternRow out{width,phase};
    if(!data||width<1||width>16||phase>=width||
       bytes>std::numeric_limits<size_t>::max()/8)return out;
    const size_t total=bytes*8;
    if(startBit>total||phase>total-startBit)return out;
    const size_t first=startBit+phase;
    const size_t count=(total-first)/width;
    const size_t limit=count<maxTokens?count:maxTokens;
    std::array<size_t,65536> histogram{};
    uint32_t prior=0;
    for(size_t i=0;i<limit;++i) {
        uint32_t value=0;
        if(!readSprBits(data,bytes,first+i*width,width,value))break;
        if(out.tokens&&value==prior)++out.adjacentEqual;
        if(histogram[value]++==0)++out.distinct;
        if(histogram[value]>out.mostFrequent)out.mostFrequent=histogram[value];
        prior=value;
        ++out.tokens;
    }
    return out;
}
struct SprRepeatResult {size_t period=0, repeats=0;};
inline SprRepeatResult longestSprRepeat(const uint8_t* data,size_t bytes,
                                       size_t startBit,size_t maxPeriod=64,
                                       size_t maxBits=512) {
    SprRepeatResult best{};
    if(!data||bytes>std::numeric_limits<size_t>::max()/8)return best;
    const size_t total=bytes*8;
    if(startBit>=total)return best;
    const size_t available=total-startBit;
    const size_t limit=available<maxBits?available:maxBits;
    for(size_t period=1;period<=maxPeriod&&period<=limit/2;++period) {
        size_t matched=0;
        while(matched+period<limit) {
            uint32_t a=0,b=0;
            if(!readSprBits(data,bytes,startBit+matched,1,a)||
               !readSprBits(data,bytes,startBit+matched+period,1,b)||a!=b)break;
            ++matched;
        }
        const size_t repeats=(matched+period)/period;
        if(repeats>=2&&(repeats>best.repeats||
           (repeats==best.repeats&&period>best.period)))
            best={period,repeats};
    }
    return best;
}
}
