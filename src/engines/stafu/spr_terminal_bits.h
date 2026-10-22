// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Inspect 0..7 terminal bits without assuming that they are padding.
#include <cstddef>
#include <cstdint>
#include <limits>
namespace reluva::stafu {
inline bool sprTerminalBitsZero(const uint8_t* data,size_t bytes,size_t trim) {
    if(!data||trim>7||bytes>std::numeric_limits<size_t>::max()/8||
       trim>bytes*8)return false;
    for(size_t i=0;i<trim;++i)
        if((data[bytes-1-i/8]>>(i%8))&1u)return false;
    return true;
}
}
