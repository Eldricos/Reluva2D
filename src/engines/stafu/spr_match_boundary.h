// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Inspect bits adjacent to a verified shifted match, without reading out of bounds.
#include "engines/stafu/spr_shift_verify.h"
#include <cstddef>
#include <cstdint>
#include <string>
namespace reluva::stafu {
inline std::string sprBitWindow(const uint8_t* bytes,size_t size,
                               size_t center,unsigned order,size_t radius=16) {
    if(!bytes||order>1||center>size*8)return {};
    const size_t begin=center>radius?center-radius:0;
    const size_t end=center<=size*8&&size*8-center<radius?size*8:center+radius;
    std::string out;
    for(size_t i=begin;i<end;++i) {
        if(i==center)out.push_back('|');
        out.push_back(char('0'+((bytes[i/8]>>(order==0?7-i%8:i%8))&1u)));
    }
    if(center==end)out.push_back('|');
    return out;
}
}
