// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only candidate-target validation for opaque four-byte SPR image payloads.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace {
struct Boundary {
    size_t offset,record,chunk;
    std::string kind;
};
struct Entry {
    size_t record,image,chunk,chunkOffset,imageOffset;
    uint32_t value,a,b,marker;
    std::string tag;
};
struct Key {
    std::string tag;
    uint32_t a,b;
    int64_t anchor;
    bool operator<(const Key& o) const {
        return std::tie(tag,a,b,anchor)<std::tie(o.tag,o.a,o.b,o.anchor);
    }
};
void classify(size_t target,const std::vector<Boundary>& boundaries,
              const std::vector<reluva::stafu::SprSequenceIndex::Record>& records,
              const char* label) {
    size_t exact=0;
    for(const auto& b:boundaries)if(b.offset==target) {
        if(exact<6)std::printf(" %s=%s:r%zu:c%zu",label,b.kind.c_str(),b.record,b.chunk);
        ++exact;
    }
    if(!exact) {
        size_t closest=static_cast<size_t>(-1);
        const Boundary* best=nullptr;
        for(const auto& b:boundaries) {
            const size_t distance=b.offset>target?b.offset-target:target-b.offset;
            if(distance<closest){closest=distance;best=&b;}
        }
        if(best)std::printf(" %s=nearest:%s:r%zu:c%zu:distance%zu",
                            label,best->kind.c_str(),best->record,best->chunk,closest);
    } else if(exact>6)std::printf(" %s_extra=%zu",label,exact-6);
    for(size_t r=0;r<records.size();++r)
        if(target>=records[r].start&&target<records[r].end) {
            std::printf(" %s_inRecord=%zu",label,r);
            break;
        }
}
}

int stafuSprOffsetTargets(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-offset-targets: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-offset-targets: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-offset-targets: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        const auto& records=index.records();
        std::vector<Boundary> boundaries;
        std::vector<Entry> entries;
        for(size_t r=0;r<records.size();++r) {
            boundaries.push_back({records[r].start,r,0,"recordStart"});
            boundaries.push_back({records[r].end,r,0,"recordEnd"});
            size_t ordinal=0;
            for(size_t c=0;c<records[r].events.size();++c) {
                const auto& e=records[r].events[c];
                boundaries.push_back({e.chunkOffset,r,c,"chunkStart"});
                boundaries.push_back({e.chunkOffset+e.chunkSize,r,c,"chunkEnd"});
                boundaries.push_back({e.payloadOffset,r,c,"payloadStart"});
                if(e.hasImageHeader)boundaries.push_back({e.imageDataOffset,r,c,"imageStart"});
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View view;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,view)&&
                   view.size==4&&view.marker==0x00030000) {
                    const auto* p=view.data;
                    uint32_t value=uint32_t(p[0])|(uint32_t(p[1])<<8)|
                                   (uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);
                    entries.push_back({r,ordinal,c,e.chunkOffset,e.imageDataOffset,
                                       value,view.headerA,view.headerB,view.marker,e.tag});
                }
                ++ordinal;
            }
        }
        std::map<Key,std::vector<size_t>> chunkAnchors,imageAnchors;
        size_t absoluteExact=0,recordExact=0;
        std::printf("SPR_TARGET_RESOURCE resource=%s fileSize=%zu records=%zu values=%zu\n",
                    name.c_str(),bytes.size(),records.size(),entries.size());
        for(const auto& e:entries) {
            const int64_t chunkAnchor=static_cast<int64_t>(e.chunkOffset)-e.value;
            const int64_t imageAnchor=static_cast<int64_t>(e.imageOffset)-e.value;
            chunkAnchors[{e.tag,e.a,e.b,chunkAnchor}].push_back(e.image);
            imageAnchors[{e.tag,e.a,e.b,imageAnchor}].push_back(e.image);
            const size_t recordTarget=records[e.record].start+static_cast<size_t>(e.value);
            auto exact=[&](size_t target) {
                for(const auto& b:boundaries)if(b.offset==target)return true;
                return false;
            };
            if(e.value<bytes.size()&&exact(e.value))++absoluteExact;
            if(e.value<=records[e.record].end-records[e.record].start&&exact(recordTarget))
                ++recordExact;
            std::printf("  SPR_TARGET record=%zu image=%zu chunk=%zu tag=%s dims=%u/%u value=%u chunk=0x%zx imageData=0x%zx chunkMinusValue=%lld imageMinusValue=%lld",
                        e.record,e.image,e.chunk,e.tag.c_str(),e.a,e.b,e.value,
                        e.chunkOffset,e.imageOffset,
                        static_cast<long long>(chunkAnchor),
                        static_cast<long long>(imageAnchor));
            if(e.value<bytes.size())classify(e.value,boundaries,records,"absolute");
            else std::printf(" absolute=outOfFile");
            if(e.value<=records[e.record].end-records[e.record].start)
                classify(recordTarget,boundaries,records,"recordRelative");
            else std::printf(" recordRelative=outsideSourceRecord");
            std::printf("\n");
        }
        auto printAnchors=[&](const char* label,const std::map<Key,std::vector<size_t>>& groups) {
            size_t repeated=0;
            for(const auto& [key,images]:groups) {
                if(images.size()<2)continue;
                ++repeated;
                std::printf("  SPR_TARGET_ANCHOR basis=%s tag=%s dims=%u/%u anchor=%lld count=%zu images=",
                            label,key.tag.c_str(),key.a,key.b,
                            static_cast<long long>(key.anchor),images.size());
                for(size_t i=0;i<images.size();++i)
                    std::printf("%s%zu",i?",":"",images[i]);
                if(key.anchor>=0&&static_cast<uint64_t>(key.anchor)<bytes.size())
                    classify(static_cast<size_t>(key.anchor),boundaries,records,"anchor");
                else std::printf(" anchor=outsideFile");
                std::printf("\n");
            }
            return repeated;
        };
        const size_t repeatedChunk=printAnchors("chunkStart",chunkAnchors);
        const size_t repeatedImage=printAnchors("imageDataStart",imageAnchors);
        std::printf("SPR_TARGET_SUMMARY resource=%s sources=%zu absoluteBoundaryHits=%zu recordBoundaryHits=%zu repeatedChunkAnchors=%zu repeatedImageAnchors=%zu\n",
                    name.c_str(),entries.size(),absoluteExact,recordExact,
                    repeatedChunk,repeatedImage);
    }
    std::puts("SPR_TARGET_NOTE exact matches and stable anchors are structural evidence, not proof of pointer semantics.");
    return failures?1:0;
}
