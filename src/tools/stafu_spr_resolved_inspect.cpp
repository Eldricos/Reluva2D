// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 227: inspect resolved SPR image data without decoding or rendering.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_image_reference_resolver.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace {
using Resolver=reluva::stafu::SprImageReferenceResolver;
const char* statusName(Resolver::Status s) {
    switch(s) {
    case Resolver::Status::NotReference:return "direct";
    case Resolver::Status::InvalidSource:return "invalidSource";
    case Resolver::Status::Underflow:return "underflow";
    case Resolver::Status::MissingTarget:return "missing";
    case Resolver::Status::AmbiguousTarget:return "ambiguous";
    case Resolver::Status::MetadataMismatch:return "metadataMismatch";
    case Resolver::Status::Resolved:return "resolved";
    }
    return "unknown";
}
void preview(const uint8_t* data,size_t size) {
    const size_t count=std::min(size,size_t(16));
    for(size_t i=0;i<count;++i)std::printf("%02x",unsigned(data[i]));
    if(size>count)std::printf("...");
}
}
int stafuSprResolvedInspect(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-resolved-inspect: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-resolved-inspect: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-resolved-inspect: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        size_t direct=0,resolved=0,mismatches=0,invalid=0,shortTargets=0;
        std::map<size_t,size_t> referenceCounts;
        std::printf("SPR_INSPECT_RESOURCE resource=%s records=%zu\n",name.c_str(),index.records().size());
        for(size_t ri=0;ri<index.records().size();++ri) {
            size_t image=0;
            for(const auto& event:index.records()[ri].events) {
                if(event.tag!="PMOC"&&event.tag!="MOCS")continue;
                const auto result=Resolver::resolve(bytes,index,event);
                if(result.status==Resolver::Status::NotReference) {
                    ++direct;
                    ++image;
                    continue;
                }
                if(result.status==Resolver::Status::Resolved)++resolved;
                else if(result.status==Resolver::Status::MetadataMismatch)++mismatches;
                else ++invalid;
                if(result.target.data) {
                    ++referenceCounts[result.targetOffset];
                    if(result.target.size<=4)++shortTargets;
                }
                std::printf("SPR_INSPECT_REFERENCE resource=%s record=%zu image=%zu status=%s sourceTag=%s sourceDims=%u/%u sourceMarker=0x%08x sourceOffset=0x%zx sourceSize=%zu",
                            name.c_str(),ri,image,statusName(result.status),event.tag.c_str(),
                            event.headerA,event.headerB,event.marker,result.source.offset,result.source.size);
                if(result.source.data&&result.source.size==4) {
                    const auto* p=result.source.data;
                    const uint32_t distance=uint32_t(p[0])|(uint32_t(p[1])<<8)|
                                            (uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);
                    std::printf(" distance=%u",distance);
                }
                if(result.target.data) {
                    const auto& targetEvent=index.records()[result.targetRecord].events[result.targetEvent];
                    std::printf(" targetRecord=%zu targetTag=%s targetDims=%u/%u targetMarker=0x%08x targetOffset=0x%zx targetSize=%zu targetPrefix=",
                                result.targetRecord,targetEvent.tag.c_str(),result.target.headerA,
                                result.target.headerB,result.target.marker,result.targetOffset,result.target.size);
                    preview(result.target.data,result.target.size);
                }
                std::putchar('\n');
                ++image;
            }
        }
        for(const auto& [offset,count]:referenceCounts) {
            if(count<2)continue;
            std::printf("SPR_INSPECT_REUSED_TARGET resource=%s offset=0x%zx references=%zu\n",
                        name.c_str(),offset,count);
        }
        std::printf("SPR_INSPECT_SUMMARY resource=%s direct=%zu resolved=%zu metadataMismatch=%zu otherInvalid=%zu shortTargetReferences=%zu distinctReferencedTargets=%zu\n",
                    name.c_str(),direct,resolved,mismatches,invalid,shortTargets,referenceCounts.size());
    }
    std::puts("SPR_INSPECT_NOTE raw payload prefixes only; no image decoding, rendering, or modification.");
    return failures?1:0;
}
