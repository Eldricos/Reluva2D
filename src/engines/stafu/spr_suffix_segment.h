// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Greedy contiguous segmentation by common terminal MSB bits.
// A segment is maximal from its starting event; no frame semantics.
#include "engines/stafu/spr_sequence_suffix.h"
#include <cstddef>
#include <vector>
namespace reluva::stafu {
struct SprSuffixSegment {size_t first=0,last=0,commonBits=0;};
inline std::vector<SprSuffixSegment> segmentSprSuffix(
    const std::vector<SprSequencePayload>& inputs,size_t threshold=128,
    size_t maxBytes=4096) {
    std::vector<SprSuffixSegment> result;
    for(size_t start=0;start<inputs.size();) {
        size_t end=start;
        size_t shared=0;
        for(size_t next=start+1;next<inputs.size();++next) {
            const std::vector<SprSequencePayload> group(
                inputs.begin()+start,inputs.begin()+next+1);
            const auto match=compareSprSequenceSuffix(group,maxBytes);
            if(!match.valid||match.sharedBits<threshold)break;
            end=next;
            shared=match.sharedBits;
        }
        result.push_back({start,end,shared});
        start=end+1;
    }
    return result;
}
}
