// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Bounded MSB-first bit extraction from opaque SPR payloads; no format interpretation.
#include <cstddef>
#include <cstdint>
#include <limits>
namespace reluva::stafu {
inline bool readSprBits(const uint8_t* bytes,size_t size,size_t bitOffset,
                       unsigned width,uint32_t& value) {
    value=0;
    if(!bytes||width==0||width>32||bitOffset>size*8||
       width>size*8-bitOffset)return false;
    for(unsigned i=0;i<width;++i) {
        const size_t pos=bitOffset+i;
        value=(value<<1)|((bytes[pos/8]>>(7-pos%8))&1u);
    }
    return true;
}
}
