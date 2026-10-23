// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Experimental bounded run-length interpretations of opaque SPR bits.
// These are hypotheses, not a known SPR decoder or proof of pixel layout.
#include "engines/stafu/spr_bit_reader.h"
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>
namespace reluva::stafu {
enum class SprRunCode { Gamma, Rice };
struct SprRunExperiment {
    bool valid=false,complete=false;
    size_t bitsConsumed=0,runs=0,outputSymbols=0;
    size_t trailingBits=0;
    std::vector<uint8_t> alternatingSymbols;
};
inline SprRunExperiment trySprRunDecode(const uint8_t* data,size_t bytes,
                                       size_t startBit,SprRunCode code,
                                       unsigned riceK=0,size_t outputCap=65536,
                                       size_t maxRuns=4096) {
    SprRunExperiment out;
    if(!data||bytes>std::numeric_limits<size_t>::max()/8||
       startBit>bytes*8||riceK>8||outputCap==0||maxRuns==0)return out;
    const size_t total=bytes*8;
    size_t pos=startBit;
    out.valid=true;
    while(pos<total&&out.runs<maxRuns) {
        const size_t begin=pos;
        size_t zeros=0;
        uint32_t bit=0;
        while(pos<total) {
            if(!readSprBits(data,bytes,pos++,1,bit))return {};
            if(bit)break;
            if(++zeros>20)return {}; // Reject extreme unary prefixes.
        }
        if(!bit) {pos=begin;break;} // Incomplete unary prefix.
        uint32_t tail=0;
        size_t run=0;
        if(code==SprRunCode::Gamma) {
            if(zeros&&(!readSprBits(data,bytes,pos,static_cast<unsigned>(zeros),tail))) {
                pos=begin;break;
            }
            pos+=zeros;
            run=(size_t(1)<<zeros)|tail;
        } else {
            if(riceK&&(!readSprBits(data,bytes,pos,riceK,tail))) {
                pos=begin;break;
            }
            pos+=riceK;
            run=(zeros<<riceK)+tail+1;
        }
        if(run>outputCap-out.outputSymbols)return {};
        out.alternatingSymbols.insert(out.alternatingSymbols.end(),run,
                                      static_cast<uint8_t>(out.runs%2));
        out.outputSymbols+=run;
        ++out.runs;
    }
    out.bitsConsumed=pos-startBit;
    out.trailingBits=total-pos;
    out.complete=out.trailingBits<=7;
    return out;
}
}
