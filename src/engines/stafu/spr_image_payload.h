// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Bounded, read-only access to opaque SPR image payloads. No decoding or ownership.
#include "engines/stafu/spr_sequence_index.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace reluva::stafu {
class SprImagePayloadAccessor {
public:
    struct View {
        const uint8_t* data=nullptr;
        size_t size=0;
        uint32_t headerA=0,headerB=0,marker=0;
        size_t offset=0;
    };

    // Views borrow storage from bytes. Keep bytes alive and unchanged while using them.
    // The event must originate from an index built for these exact bytes.
    // Returns false and clears out on non-image events or invalid bounds.
    static bool get(const std::vector<uint8_t>& bytes,
                    const SprSequenceIndex::Event& event,View& out) {
        out={};
        if(!event.hasImageHeader||(event.tag!="PMOC"&&event.tag!="MOCS"))
            return false;
        if(event.chunkSize<20||event.payloadSize<12)return false;
        if(event.chunkOffset>bytes.size()||event.chunkSize>bytes.size()-event.chunkOffset)
            return false;
        if(event.payloadOffset!=event.chunkOffset+8||
           event.payloadSize!=static_cast<size_t>(event.chunkSize)-8)
            return false;
        if(event.imageDataOffset!=event.payloadOffset+12||
           event.imageDataSize!=event.payloadSize-12)
            return false;
        if(event.imageDataOffset>bytes.size()||
           event.imageDataSize>bytes.size()-event.imageDataOffset)
            return false;
        const size_t p=event.chunkOffset;
        if(bytes[p]!='P'&&bytes[p]!='M')return false;
        for(size_t i=0;i<4;++i)
            if(bytes[p+i]!=static_cast<uint8_t>(event.tag[i]))return false;
        if(read32(bytes,p+4)!=event.chunkSize||
           read32(bytes,event.payloadOffset)!=event.headerA||
           read32(bytes,event.payloadOffset+4)!=event.headerB||
           read32(bytes,event.payloadOffset+8)!=event.marker)
            return false;
        out.offset=event.imageDataOffset;
        out.size=event.imageDataSize;
        out.data=bytes.data()+out.offset;
        out.headerA=event.headerA;
        out.headerB=event.headerB;
        out.marker=event.marker;
        return true;
    }
private:
    static uint32_t read32(const std::vector<uint8_t>& b,size_t p) {
        return uint32_t(b[p])|(uint32_t(b[p+1])<<8)|
               (uint32_t(b[p+2])<<16)|(uint32_t(b[p+3])<<24);
    }
};
}
