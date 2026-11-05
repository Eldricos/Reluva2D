// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only catalog of observed PMOC/MOCS payload structures; no codec assumptions.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace {
struct Key {
    std::string tag;
    uint32_t marker=0,headerA=0,headerB=0;
    bool hasHeader=false;
    bool operator<(const Key& o) const {
        return std::tie(tag,hasHeader,marker,headerA,headerB) <
               std::tie(o.tag,o.hasHeader,o.marker,o.headerA,o.headerB);
    }
};
struct Stats {
    size_t count=0,minData=0,maxData=0,small=0;
};
void inspect(const char* name,const std::vector<uint8_t>& bytes) {
    reluva::stafu::SprSequenceIndex index;
    std::string error;
    if(!index.build(bytes,error)) {
        std::printf("SPR_CATALOG_ERROR resource=%s reason=%s\n",name,error.c_str());
        return;
    }
    std::map<Key,Stats> groups;
    size_t count=0,shortChunks=0;
    std::printf("SPR_CATALOG resource=%s records=%zu\n",name,index.records().size());
    for(size_t record=0;record<index.records().size();++record) {
        const auto& events=index.records()[record].events;
        size_t ordinal=0;
        for(size_t i=0;i<events.size();++i) {
            const auto& e=events[i];
            if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
            const bool header=e.hasImageHeader;
            const size_t dataLength=header?e.imageDataSize:0;
            Key key{e.tag,header?e.marker:0,header?e.headerA:0,
                    header?e.headerB:0,header};
            auto& stats=groups[key];
            if(!stats.count)stats.minData=dataLength;
            ++stats.count;
            stats.minData=std::min(stats.minData,dataLength);
            stats.maxData=std::max(stats.maxData,dataLength);
            if(header&&dataLength<=4)++stats.small;
            ++count;
            if(!header)++shortChunks;
            const char* prev=i?events[i-1].tag.c_str():"START";
            const char* next=i+1<events.size()?events[i+1].tag.c_str():"END";
            if(header) {
                std::printf("  SPR_ITEM record=%zu image=%zu chunk=%zu tag=%s marker=0x%08x "
                            "dimensionsCandidate=%u/%u chunkPayload=%zu imageData=%zu "
                            "prev=%s next=%s offset=0x%zx\n",
                            record,ordinal,i,e.tag.c_str(),e.marker,e.headerA,e.headerB,
                            e.payloadSize,dataLength,prev,next,e.chunkOffset);
            } else {
                std::printf("  SPR_ITEM record=%zu image=%zu chunk=%zu tag=%s "
                            "marker=NONE candidateHeader=NO chunkPayload=%zu "
                            "prev=%s next=%s offset=0x%zx\n",
                            record,ordinal,i,e.tag.c_str(),e.payloadSize,prev,next,e.chunkOffset);
            }
            ++ordinal;
        }
    }
    std::printf("SPR_CATALOG_SUMMARY resource=%s imageLikeChunks=%zu withoutCandidateHeader=%zu groups=%zu\n",
                name,count,shortChunks,groups.size());
    for(const auto& [key,s]:groups) {
        if(key.hasHeader)
            std::printf("  SPR_GROUP tag=%s marker=0x%08x dimensionsCandidate=%u/%u "
                        "count=%zu imageDataMin=%zu imageDataMax=%zu dataLe4=%zu\n",
                        key.tag.c_str(),key.marker,key.headerA,key.headerB,
                        s.count,s.minData,s.maxData,s.small);
        else
            std::printf("  SPR_GROUP tag=%s marker=NONE candidateHeader=NO count=%zu "
                        "chunkPayloadMin=%zu chunkPayloadMax=%zu\n",
                        key.tag.c_str(),s.count,s.minData,s.maxData);
    }
}
}
int stafuSprPayloadCatalog(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-payload-catalog: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-payload-catalog: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-payload-catalog: %s: invalid SPR: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        inspect(name.c_str(),bytes);
    }
    std::puts("SPR_CATALOG_NOTE marker/dimensions are candidate fields only; imageData excludes 12-byte candidate header.");
    return failures?1:0;
}
