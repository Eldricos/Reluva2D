// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only, byte-level comparison of two image-like chunks in one SPR record.
// Record and image ordinals are zero-based. No codec or frame semantics assumed.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <limits>
#include <string>
#include <vector>

namespace {
bool parseOrdinal(const char* text, size_t& value) {
    if (!text || !*text || *text == '-') return false;
    errno = 0;
    char* end = nullptr;
    const unsigned long long n = std::strtoull(text, &end, 10);
    if (errno || !end || *end || n > std::numeric_limits<size_t>::max()) return false;
    value = static_cast<size_t>(n);
    return true;
}
struct Range { size_t begin, end; };
}

int stafuSprPayloadDiff(const char* installed, const char* cd, const char* name,
                        const char* recordArg, const char* firstArg, const char* secondArg) {
    size_t recordId=0, firstId=0, secondId=0;
    if (!parseOrdinal(recordArg,recordId) || !parseOrdinal(firstArg,firstId) ||
        !parseOrdinal(secondArg,secondId) || firstId==secondId) {
        std::fprintf(stderr,"spr-payload-diff: expected nonnegative record and two distinct image ordinals\n");
        return 2;
    }
    reluva::stafu::ResourceManager manager;
    std::string error;
    if (!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-payload-diff: %s\n",error.c_str()); return 1;
    }
    std::vector<uint8_t> bytes;
    if (!manager.read(name,bytes,error)) {
        std::fprintf(stderr,"spr-payload-diff: %s\n",error.c_str()); return 1;
    }
    reluva::stafu::SprSequenceIndex index;
    if (!index.build(bytes,error)) {
        std::fprintf(stderr,"spr-payload-diff: invalid SPR: %s\n",error.c_str()); return 1;
    }
    if (recordId>=index.records().size()) {
        std::fprintf(stderr,"spr-payload-diff: record %zu out of range (records=%zu)\n",
                     recordId,index.records().size()); return 2;
    }
    const auto& events=index.records()[recordId].events;
    struct Image { size_t chunk; reluva::stafu::SprImagePayloadAccessor::View view; std::string tag; };
    std::vector<Image> images;
    for (size_t i=0;i<events.size();++i) {
        reluva::stafu::SprImagePayloadAccessor::View view;
        if (reluva::stafu::SprImagePayloadAccessor::get(bytes,events[i],view) && view.size>0)
            images.push_back({i,view,events[i].tag});
    }
    if (firstId>=images.size() || secondId>=images.size()) {
        std::fprintf(stderr,"spr-payload-diff: image ordinal out of range (images=%zu)\n",images.size());
        return 2;
    }
    const auto& a=images[firstId];
    const auto& b=images[secondId];
    const size_t common=std::min(a.view.size,b.view.size);
    size_t prefix=0;
    while (prefix<common && a.view.data[prefix]==b.view.data[prefix]) ++prefix;
    size_t suffix=0;
    while (suffix<common-prefix &&
           a.view.data[a.view.size-1-suffix]==b.view.data[b.view.size-1-suffix]) ++suffix;
    size_t equal=0,changed=0;
    std::vector<Range> ranges;
    bool inRange=false;
    size_t start=0;
    for (size_t i=0;i<common;++i) {
        const bool different=a.view.data[i]!=b.view.data[i];
        if (different) {
            ++changed;
            if (!inRange) {start=i;inRange=true;}
        } else {
            ++equal;
            if (inRange) {ranges.push_back({start,i});inRange=false;}
        }
    }
    if (inRange) ranges.push_back({start,common});
    std::printf("SPR_PAYLOAD_DIFF resource=%s record=%zu images=%zu,%zu chunks=%zu,%zu tags=%s,%s\n",
                name,recordId,firstId,secondId,a.chunk,b.chunk,a.tag.c_str(),b.tag.c_str());
    std::printf("  offsets=0x%zx,0x%zx lengths=%zu,%zu headers=%u/%u/0x%08x,%u/%u/0x%08x\n",
                a.view.offset,b.view.offset,a.view.size,b.view.size,
                a.view.headerA,a.view.headerB,a.view.marker,b.view.headerA,b.view.headerB,b.view.marker);
    std::printf("  alignedCommon=%zu equal=%zu different=%zu changeRuns=%zu prefix=%zu suffix=%zu trailingA=%zu trailingB=%zu\n",
                common,equal,changed,ranges.size(),prefix,suffix,
                a.view.size-common,b.view.size-common);
    constexpr size_t kMaxRanges=40;
    for (size_t i=0;i<std::min(ranges.size(),kMaxRanges);++i) {
        const auto& r=ranges[i];
        std::printf("  change[%zu] relative=[%zu,%zu) bytes=%zu absoluteA=[0x%zx,0x%zx) absoluteB=[0x%zx,0x%zx)\n",
                    i,r.begin,r.end,r.end-r.begin,
                    a.view.offset+r.begin,a.view.offset+r.end,
                    b.view.offset+r.begin,b.view.offset+r.end);
    }
    if (ranges.size()>kMaxRanges)
        std::printf("  omittedChangeRuns=%zu (first %zu shown)\n",ranges.size()-kMaxRanges,kMaxRanges);
    std::puts("Byte-aligned comparisons only; suffix is aligned to each payload end. "
              "Different lengths can shift bytes, so ranges do not identify decoded pixel changes.");
    std::puts("The 12-byte candidate image header is excluded; no decoding or format semantics inferred.");
    return 0;
}
