// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Bounded, allocation-free hypothesis tests. These are NOT SPR decoders.
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
enum class SprCandidate { PairCountValue, PackBits, HighBitRuns };
struct SprCandidateResult { bool valid=false; bool complete=false; uint64_t output=0; };
inline SprCandidateResult evaluateSprCandidate(const uint8_t* p, size_t n,
                                                uint64_t limit, SprCandidate kind) {
    SprCandidateResult r;
    if (!p && n) return r;
    size_t i=0;
    uint64_t produced=0;
    while (i<n) {
        uint64_t add=0;
        if (kind==SprCandidate::PairCountValue) {
            if (n-i<2) return r;
            add=p[i]; i+=2;
        } else if (kind==SprCandidate::PackBits) {
            const int8_t control=static_cast<int8_t>(p[i++]);
            if (control>=0) {
                add=uint64_t(control)+1;
                if (add>n-i) return r;
                i+=static_cast<size_t>(add);
            } else if (control!=-128) {
                add=uint64_t(1-int(control));
                if (i==n) return r;
                ++i;
            }
        } else {
            const uint8_t control=p[i++];
            if (control&0x80) {
                add=uint64_t(control&0x7f)+1;
                if (i==n) return r;
                ++i;
            } else {
                add=uint64_t(control)+1;
                if (add>n-i) return r;
                i+=static_cast<size_t>(add);
            }
        }
        if (add>limit-produced) return r;
        produced+=add;
    }
    r.valid=true; r.complete=true; r.output=produced;
    return r;
}
}
