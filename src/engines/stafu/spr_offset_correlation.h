// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Read-only classification of hypothetical absolute/record-relative offsets.
#include "engines/stafu/spr_sequence_index.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace reluva::stafu {
struct SprOffsetMatch {
    size_t target=0;
    const char* base="absolute";
    const char* region="file";
    const char* boundary="none";
    size_t record=0;
    size_t chunk=0;
};
inline std::vector<SprOffsetMatch> correlateSprOffset(
    uint32_t value,size_t fileSize,const SprSequenceIndex& index,size_t sourceRecord) {
    std::vector<SprOffsetMatch> out;
    auto classify=[&](size_t target,const char* base) {
        if(target>=fileSize)return;
        SprOffsetMatch match;
        match.target=target;match.base=base;
        for(size_t r=0;r<index.records().size();++r) {
            const auto& rec=index.records()[r];
            if(target==rec.start||target==rec.end) {
                match.boundary=target==rec.start?"record-start":"record-end";
                match.record=r;
            }
            if(target<rec.start||target>=rec.end)continue;
            match.region="record";match.record=r;
            for(size_t c=0;c<rec.events.size();++c) {
                const auto& e=rec.events[c];
                if(target==e.chunkOffset)match.boundary="chunk-start";
                else if(target==e.payloadOffset)match.boundary="payload-start";
                else if(e.hasImageHeader&&target==e.imageDataOffset)
                    match.boundary="image-data-start";
                else if(target==e.chunkOffset+e.chunkSize)
                    match.boundary="chunk-end";
                if(target>=e.chunkOffset&&target<e.chunkOffset+e.chunkSize) {
                    match.region="chunk";match.chunk=c;
                    if(target>=e.payloadOffset)match.region="payload";
                    if(e.hasImageHeader&&target>=e.imageDataOffset)
                        match.region="image-data";
                    break;
                }
            }
            break;
        }
        out.push_back(match);
    };
    classify(value,"absolute");
    if(sourceRecord<index.records().size()) {
        const size_t r=sourceRecord;
        const auto& rec=index.records()[r];
        if(value<=rec.end-rec.start) {
            if(rec.start<=fileSize-value) {
                const size_t target=rec.start+value;
                if(target<rec.end)classify(target,"record-relative");
            }
        }
    }
    return out;
}
}
