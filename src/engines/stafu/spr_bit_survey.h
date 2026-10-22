// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Bounded bit-pattern observations only; not a decoder or compression identifier.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace reluva::stafu {
struct SprBitSurvey {
    struct Orientation {
        size_t ones=0,transitions=0;
        size_t repeated12=0,windows12=0;
        std::array<size_t,8> alignedEightRepeats{};
    };
    size_t bytesExamined=0;
    Orientation msb,lsb;
};
inline SprBitSurvey surveySprBits(const uint8_t* data,size_t length,
                                  size_t maxBytes=4096) {
    SprBitSurvey result;
    if(!data||!maxBytes)return result;
    result.bytesExamined=std::min(length,maxBytes);
    const size_t bitCount=result.bytesExamined*8;
    for(unsigned mode=0;mode<2;++mode) {
        auto& out=mode==0?result.msb:result.lsb;
        auto bit=[&](size_t position)->unsigned {
            const uint8_t b=data[position/8];
            return (b>>(mode==0?7-(position%8):position%8))&1u;
        };
        for(size_t i=0;i<bitCount;++i) {
            out.ones+=bit(i);
            if(i&&bit(i)!=bit(i-1))++out.transitions;
        }
        // 12-bit sliding windows: counts repeat occurrences after the first.
        std::array<uint32_t,4096> seen{};
        unsigned value=0;
        for(size_t i=0;i<bitCount;++i) {
            value=((value<<1)|bit(i))&4095u;
            if(i>=11) {
                ++out.windows12;
                if(seen[value]++)++out.repeated12;
            }
        }
        // 8-bit tokens for all eight possible bit alignments.
        for(size_t phase=0;phase<8;++phase) {
            std::array<bool,256> visited{};
            for(size_t i=phase;i+8<=bitCount;i+=8) {
                unsigned token=0;
                for(size_t k=0;k<8;++k)token=(token<<1)|bit(i+k);
                if(visited[token])++out.alignedEightRepeats[phase];
                visited[token]=true;
            }
        }
    }
    return result;
}
}
