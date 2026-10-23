// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Bounded common MSB suffix across an arbitrary sequence of opaque payloads.
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>
namespace reluva::stafu {
struct SprSequencePayload {const uint8_t* data=nullptr;size_t size=0;};
struct SprSequenceSuffix {
    bool valid=false;
    size_t sharedBits=0;
    std::vector<size_t> trims,starts;
};
inline SprSequenceSuffix compareSprSequenceSuffix(
    const std::vector<SprSequencePayload>& inputs,size_t maxBytes=4096) {
    SprSequenceSuffix out;
    if(inputs.size()<2||!maxBytes||maxBytes>std::numeric_limits<size_t>::max()/8)
        return out;
    const size_t n=inputs.size();
    std::vector<size_t> sizes(n),bits(n);
    std::vector<const uint8_t*> tails(n);
    for(size_t i=0;i<n;++i) {
        if(!inputs[i].data||!inputs[i].size||
           inputs[i].size>std::numeric_limits<size_t>::max()/8)return out;
        sizes[i]=std::min(inputs[i].size,maxBytes);
        bits[i]=sizes[i]*8;
        tails[i]=inputs[i].data+inputs[i].size-sizes[i];
    }
    const auto bit=[](const uint8_t* p,size_t pos)->unsigned {
        return (p[pos/8]>>(7-pos%8))&1u;
    };
    const auto padding=[](const uint8_t* p,size_t n,size_t trim)->bool {
        return !trim||(p[n-1]&((1u<<trim)-1u))==0;
    };
    out.valid=true;
    out.trims.assign(n,0);
    // Fix the first stream's terminal alignment; independently choose the
    // best alignment of each other stream against it, then take the minimum.
    for(size_t anchorTrim=0;anchorTrim<8&&anchorTrim<bits[0];++anchorTrim) {
        if(!padding(tails[0],sizes[0],anchorTrim))continue;
        size_t common=bits[0]-anchorTrim;
        std::vector<size_t> candidate(n,0);
        candidate[0]=anchorTrim;
        for(size_t i=1;i<n;++i) {
            size_t best=0,bestTrim=0;
            for(size_t trim=0;trim<8&&trim<bits[i];++trim) {
                if(!padding(tails[i],sizes[i],trim))continue;
                const size_t cap=std::min(bits[0]-anchorTrim,bits[i]-trim);
                size_t matched=0;
                while(matched<cap&&
                      bit(tails[0],bits[0]-anchorTrim-1-matched)==
                      bit(tails[i],bits[i]-trim-1-matched))++matched;
                if(matched>best) {best=matched;bestTrim=trim;}
            }
            candidate[i]=bestTrim;
            common=std::min(common,best);
        }
        if(common>out.sharedBits) {
            out.sharedBits=common;
            out.trims=std::move(candidate);
        }
    }
    out.starts.resize(n);
    for(size_t i=0;i<n;++i)
        out.starts[i]=inputs[i].size*8-out.trims[i]-out.sharedBits;
    return out;
}
}
