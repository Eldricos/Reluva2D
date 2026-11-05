// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Read-only resolution of observed backward-relative SPR image references.
#include "engines/stafu/spr_image_payload.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace reluva::stafu {
class SprImageReferenceResolver {
public:
    enum class Status {
        NotReference, InvalidSource, Underflow, MissingTarget,
        AmbiguousTarget, MetadataMismatch, Resolved
    };
    struct Result {
        Status status=Status::NotReference;
        SprImagePayloadAccessor::View source{};
        SprImagePayloadAccessor::View target{};
        size_t targetOffset=0;
        size_t targetRecord=0, targetEvent=0;
        bool metadataCompatible=false;
        // Borrowed payload; the caller must retain the original bytes.
        const uint8_t* data=nullptr;
        size_t size=0;
    };
    // Only marker 0x00030000 with exactly four data bytes is treated
    // as a reference. A metadata mismatch still reports its target,
    // but does not expose it as a validated resolved payload.
    static Result resolve(const std::vector<uint8_t>& bytes,
                          const SprSequenceIndex& index,
                          const SprSequenceIndex::Event& source) {
        Result out;
        if(!SprImagePayloadAccessor::get(bytes,source,out.source)) {
            out.status=Status::InvalidSource;
            return out;
        }
        if(out.source.marker!=0x00030000 || out.source.size!=4)
            return out;
        const uint8_t* p=out.source.data;
        const uint32_t distance=uint32_t(p[0])|(uint32_t(p[1])<<8)|
                                (uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);
        if(distance==0 || distance>out.source.offset) {
            out.status=Status::Underflow;
            return out;
        }
        out.targetOffset=out.source.offset-distance;
        size_t matches=0;
        for(size_t r=0;r<index.records().size();++r) {
            const auto& events=index.records()[r].events;
            for(size_t e=0;e<events.size();++e) {
                if(!events[e].hasImageHeader ||
                   events[e].imageDataOffset!=out.targetOffset)continue;
                SprImagePayloadAccessor::View view;
                if(!SprImagePayloadAccessor::get(bytes,events[e],view))continue;
                ++matches;
                out.target=view;
                out.targetRecord=r;
                out.targetEvent=e;
            }
        }
        if(matches==0) {
            out.status=Status::MissingTarget;
            return out;
        }
        if(matches!=1) {
            out.status=Status::AmbiguousTarget;
            return out;
        }
        out.metadataCompatible=source.tag==index.records()[out.targetRecord].events[out.targetEvent].tag &&
            out.source.headerA==out.target.headerA &&
            out.source.headerB==out.target.headerB;
        if(!out.metadataCompatible) {
            out.status=Status::MetadataMismatch;
            return out;
        }
        out.status=Status::Resolved;
        out.data=out.target.data;
        out.size=out.target.size;
        return out;
    }
};
}
