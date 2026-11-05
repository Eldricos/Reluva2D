// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only catalog of observed PMOC/MOCS payload structures; no codec assumptions.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include "engines/stafu/spr_offset_correlation.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <array>
#include <set>
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
void inspect(const char* name,const reluva::stafu::SprSequenceIndex& index) {
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
            const size_t dataLength=header?e.imageDataSize:e.payloadSize;
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
        inspect(name.c_str(),index);
    }
    std::puts("SPR_CATALOG_NOTE marker/dimensions are candidate fields only; imageData excludes 12-byte candidate header.");
    return failures?1:0;
}


// Milestone 219: inspect short image data without assigning control semantics.
int stafuSprShortPayloads(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-short-payloads: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    using Bytes=std::vector<uint8_t>;
    using Group=std::tuple<std::string,uint32_t,uint32_t,uint32_t,size_t>;
    std::map<Group,std::map<Bytes,size_t>> global;
    int failures=0;
    for(const auto& name:names) {
        Bytes bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-short-payloads: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-short-payloads: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        std::map<Group,std::map<Bytes,size_t>> local;
        std::map<Bytes,std::set<size_t>> recordOccurrences;
        size_t shortCount=0;
        std::printf("SPR_SHORT_RESOURCE resource=%s records=%zu\n",name.c_str(),index.records().size());
        for(size_t r=0;r<index.records().size();++r) {
            const auto& events=index.records()[r].events;
            size_t ordinal=0;
            for(size_t i=0;i<events.size();++i) {
                const auto& e=events[i];
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View view;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,view)&&view.size<=4) {
                    const Bytes value(view.data,view.data+view.size);
                    const Group group{e.tag,view.marker,view.headerA,view.headerB,view.size};
                    ++local[group][value];++global[group][value];
                    recordOccurrences[value].insert(r);
                    ++shortCount;
                    std::printf("  SPR_SHORT record=%zu image=%zu chunk=%zu tag=%s marker=0x%08x dims=%u/%u size=%zu bytes=",
                                r,ordinal,i,e.tag.c_str(),view.marker,view.headerA,view.headerB,view.size);
                    for(uint8_t b:value)std::printf("%02x",unsigned(b));
                    if(value.size()==4) {
                        const uint32_t le=uint32_t(value[0])|(uint32_t(value[1])<<8)|
                            (uint32_t(value[2])<<16)|(uint32_t(value[3])<<24);
                        const uint32_t be=(uint32_t(value[0])<<24)|(uint32_t(value[1])<<16)|
                            (uint32_t(value[2])<<8)|uint32_t(value[3]);
                        std::printf(" le32=%u be32=%u",le,be);
                    } else std::printf(" le32=NA be32=NA");
                    std::printf(" prev=%s next=%s offset=0x%zx\n",
                                i?events[i-1].tag.c_str():"START",
                                i+1<events.size()?events[i+1].tag.c_str():"END",view.offset);
                }
                ++ordinal;
            }
        }
        std::printf("SPR_SHORT_SUMMARY resource=%s count=%zu groups=%zu uniqueValues=%zu\n",
                    name.c_str(),shortCount,local.size(),recordOccurrences.size());
        for(const auto& [g,values]:local) {
            const auto& [tag,marker,a,b,length]=g;
            size_t total=0;
            for(const auto& [v,n]:values)total+=n;
            std::printf("  SPR_SHORT_GROUP tag=%s marker=0x%08x dims=%u/%u size=%zu count=%zu unique=%zu\n",
                        tag.c_str(),marker,a,b,length,total,values.size());
            for(const auto& [v,n]:values) {
                std::printf("    VALUE bytes=");
                for(uint8_t byte:v)std::printf("%02x",unsigned(byte));
                std::printf(" count=%zu\n",n);
            }
        }
        for(const auto& [value,records]:recordOccurrences) {
            if(records.size()<2)continue;
            std::printf("  SPR_SHORT_CROSS_RECORD bytes=");
            for(uint8_t byte:value)std::printf("%02x",unsigned(byte));
            std::printf(" recordCount=%zu\n",records.size());
        }
    }
    std::printf("SPR_SHORT_GLOBAL groups=%zu\n",global.size());
    for(const auto& [g,values]:global) {
        const auto& [tag,marker,a,b,length]=g;
        size_t count=0;
        for(const auto& [v,n]:values)count+=n;
        std::printf("  SPR_SHORT_GLOBAL_GROUP tag=%s marker=0x%08x dims=%u/%u size=%zu count=%zu unique=%zu\n",
                    tag.c_str(),marker,a,b,length,count,values.size());
    }
    std::puts("SPR_SHORT_NOTE values are opaque raw image-data bytes; integer interpretations are diagnostic only.");
    return failures?1:0;
}


// Milestone 220: test short four-byte values as hypothetical byte offsets.
int stafuSprOffsetCorrelate(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-offset-correlate: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-offset-correlate: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-offset-correlate: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        size_t total=0,absoluteValid=0,relativeValid=0,boundaryHits=0;
        std::printf("SPR_OFFSET_RESOURCE resource=%s size=%zu records=%zu\n",
                    name.c_str(),bytes.size(),index.records().size());
        for(size_t r=0;r<index.records().size();++r) {
            const auto& events=index.records()[r].events;
            size_t image=0;
            for(size_t c=0;c<events.size();++c) {
                const auto& e=events[c];
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View view;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,view)&&view.size==4) {
                    const auto* b=view.data;
                    const uint32_t le=uint32_t(b[0])|(uint32_t(b[1])<<8)|
                        (uint32_t(b[2])<<16)|(uint32_t(b[3])<<24);
                    const uint32_t be=(uint32_t(b[0])<<24)|(uint32_t(b[1])<<16)|
                        (uint32_t(b[2])<<8)|uint32_t(b[3]);
                    ++total;
                    std::printf("  SPR_OFFSET_SOURCE record=%zu image=%zu chunk=%zu tag=%s marker=0x%08x bytes=%02x%02x%02x%02x le=%u be=%u\n",
                                r,image,c,e.tag.c_str(),view.marker,unsigned(b[0]),unsigned(b[1]),
                                unsigned(b[2]),unsigned(b[3]),le,be);
                    for(const auto& [endian,value]:std::array<std::pair<const char*,uint32_t>,2>{{{"LE",le},{"BE",be}}}) {
                        const auto matches=reluva::stafu::correlateSprOffset(value,bytes.size(),index,r);
                        if(matches.empty())
                            std::printf("    CANDIDATE endian=%s value=%u base=none status=out-of-range\n",endian,value);
                        for(const auto& m:matches) {
                            if(std::string(m.base)=="absolute")++absoluteValid;
                            else ++relativeValid;
                            if(std::string(m.boundary)!="none")++boundaryHits;
                            std::printf("    CANDIDATE endian=%s value=%u base=%s target=0x%zx region=%s boundary=%s record=%zu chunk=%zu\n",
                                        endian,value,m.base,m.target,m.region,m.boundary,m.record,m.chunk);
                        }
                    }
                }
                ++image;
            }
        }
        std::printf("SPR_OFFSET_SUMMARY resource=%s fourByteSources=%zu absoluteInRange=%zu relativeInRange=%zu boundaryHits=%zu\n",
                    name.c_str(),total,absoluteValid,relativeValid,boundaryHits);
    }
    std::puts("SPR_OFFSET_NOTE candidate hits are numerical correlations, not validated pointers or decoded images.");
    return failures?1:0;
}
