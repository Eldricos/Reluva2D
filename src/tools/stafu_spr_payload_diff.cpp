// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only, byte-level comparison of two image-like chunks in one SPR record.
// Record and image ordinals are zero-based. No codec or frame semantics assumed.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include "engines/stafu/spr_byte_shift.h"
#include "engines/stafu/spr_middle_blocks.h"
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
    const auto shifts=reluva::stafu::surveySprByteShifts(
        a.view.data,a.view.size,b.view.data,b.view.size,32);
    std::puts("  SHIFT_SURVEY shift=B_index-A_index; ranked by longest exact matching run:");
    for(size_t i=0;i<std::min<size_t>(shifts.size(),8);++i) {
        const auto& m=shifts[i];
        std::printf("  shift[%zu] delta=%+d longest=%zu A=[%zu,%zu) B=[%zu,%zu) equal=%zu/%zu\n",
                    i,m.shift,m.length,m.startA,m.startA+m.length,
                    m.startB,m.startB+m.length,m.equal,m.compared);
    }
    if(prefix && suffix) {
        const size_t endA=a.view.size-suffix,endB=b.view.size-suffix;
        std::printf("  END_ALIGNED_RESYNC prefixEnds=%zu,%zu suffixStarts=%zu,%zu "
                    "netShift=%lld middleLengths=%zu,%zu\n",
                    prefix,prefix,endA,endB,
                    static_cast<long long>(endB)-static_cast<long long>(endA),
                    endA>=prefix?endA-prefix:0,endB>=prefix?endB-prefix:0);
    }
    const auto blocks=reluva::stafu::findSprMiddleBlocks(
        a.view.data,a.view.size,b.view.data,b.view.size,prefix,suffix,32,16);
    std::printf("  MIDDLE_BLOCKS minLength=16 shiftRange=[-32,+32] total=%zu\\n",blocks.size());
    for(size_t i=0;i<std::min<size_t>(blocks.size(),12);++i) {
        const auto& m=blocks[i];
        std::printf("  block[%zu] delta=%+d A=[%zu,%zu) B=[%zu,%zu) length=%zu\\n",
                    i,m.shift,m.a,m.a+m.length,m.b,m.b+m.length,m.length);
    }
    auto boundaryDump=[](const char* label,const uint8_t* data,size_t length,size_t center) {
        const size_t begin=center>32?center-32:0;
        const size_t end=std::min(length,center+(length-center>=32?32:length-center));
        std::printf("  BOUNDARY %s center=%zu range=[%zu,%zu)\\n",label,center,begin,end);
        for(size_t p=begin;p<end;p+=16) {
            const size_t stop=std::min(end,p+16);
            std::printf("    %06zx:",p);
            for(size_t j=p;j<stop;++j)std::printf(" %02x",unsigned(data[j]));
            std::putchar('\\n');
        }
    };
    boundaryDump("prefix-end A",a.view.data,a.view.size,prefix);
    boundaryDump("prefix-end B",b.view.data,b.view.size,prefix);
    if(suffix) {
        boundaryDump("suffix-start A",a.view.data,a.view.size,a.view.size-suffix);
        boundaryDump("suffix-start B",b.view.data,b.view.size,b.view.size-suffix);
    }
    std::puts("Shift survey tests fixed offsets only ([-32,+32]); a long match is "
              "not proof of a single insertion/deletion or a codec boundary.");
    std::puts("Byte-aligned comparisons only; suffix is aligned to each payload end. "
              "Different lengths can shift bytes, so ranges do not identify decoded pixel changes.");
    std::puts("The 12-byte candidate image header is excluded; no decoding or format semantics inferred.");
    return 0;
}
