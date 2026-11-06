// SPDX-License-Identifier: GPL-3.0-or-later
// M277: inspect raw direct payload prefixes in record 2; no codec assumptions.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
using View=reluva::stafu::SprImagePayloadAccessor::View;
struct Image {
    size_t ordinal=0;
    uint32_t marker=0;
    size_t offset=0;
    std::vector<uint8_t> data;
};
struct Resource {
    std::string name;
    std::vector<Image> direct;
    size_t images=0,references=0;
};
bool referenceLike(const View& v){return v.marker==0x00030000&&v.size==4;}
size_t prefix(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b){
    size_t i=0,limit=std::min(a.size(),b.size());
    while(i<limit&&a[i]==b[i])++i;
    return i;
}
bool load(reluva::stafu::ResourceManager& manager,const char* name,Resource& out) {
    std::vector<uint8_t> bytes;std::string error;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"SPR_M277_ERROR resource=%s %s\n",name,error.c_str());return false;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){
        std::fprintf(stderr,"SPR_M277_ERROR resource=%s %s\n",name,error.c_str());return false;
    }
    if(index.records().size()<=2){
        std::fprintf(stderr,"SPR_M277_ERROR resource=%s missing record=2\n",name);return false;
    }
    out.name=name;
    size_t ordinal=0;
    for(const auto& event:index.records()[2].events){
        if(event.tag!="PMOC"&&event.tag!="MOCS")continue;
        View v;
        if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,event,v)){
            std::fprintf(stderr,"SPR_M277_ERROR resource=%s record=2 ordinal=%zu invalid payload\n",name,ordinal);
            return false;
        }
        ++out.images;
        if(referenceLike(v))++out.references;
        else {
            Image im;
            im.ordinal=ordinal;im.marker=v.marker;im.offset=v.offset;
            im.data.assign(v.data,v.data+v.size);
            out.direct.push_back(std::move(im));
        }
        ++ordinal;
    }
    std::printf("SPR_M277_RECORD resource=%s record=2 images=%zu references=%zu direct=%zu\n",
        name,out.images,out.references,out.direct.size());
    for(const auto& im:out.direct)
        std::printf("SPR_M277_IMAGE resource=%s record=2 ordinal=%zu marker=0x%08X payloadBytes=%zu offset=%zu\n",
            name,im.ordinal,static_cast<unsigned>(im.marker),im.data.size(),im.offset);
    size_t adjacent=0,long32=0;
    for(size_t i=1;i<out.direct.size();++i){
        const auto& a=out.direct[i-1];const auto& b=out.direct[i];
        // Only original neighboring image events, not adjacent after filtering references.
        if(b.ordinal!=a.ordinal+1)continue;
        const size_t n=prefix(a.data,b.data);
        const size_t limit=std::min(a.data.size(),b.data.size());
        ++adjacent;if(n>=32)++long32;
        std::printf("SPR_M277_PAIR resource=%s record=2 ordinalA=%zu ordinalB=%zu markerA=0x%08X markerB=0x%08X bytesA=%zu bytesB=%zu prefix=%zu firstDifference=%s",
            name,a.ordinal,b.ordinal,static_cast<unsigned>(a.marker),static_cast<unsigned>(b.marker),
            a.data.size(),b.data.size(),n,n<limit?"byte":"end");
        if(n<limit)std::printf(" differingOffset=%zu byteA=0x%02X byteB=0x%02X",
            n,static_cast<unsigned>(a.data[n]),static_cast<unsigned>(b.data[n]));
        else std::printf(" matchingThrough=%zu",n);
        std::puts("");
    }
    std::printf("SPR_M277_RESOURCE resource=%s adjacentDirectPairs=%zu prefix32plus=%zu\n",
        name,adjacent,long32);
    return true;
}
}
int stafuSprM277Record2CommonPrefixStructure(const char* installed,const char* cd,const char*) {
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"SPR_M277_ERROR %s\n",error.c_str());return 1;
    }
    const char* names[]={"BRDGPICA.SPR","BRDGDATA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    std::vector<Resource> resources(5);
    for(size_t i=0;i<5;++i)if(!load(manager,names[i],resources[i]))return 1;
    size_t comparisons=0,identical=0;
    // Cross-character comparison at matching original record-2 image ordinal.
    for(size_t i=0;i<resources.size();++i)
        for(size_t j=i+1;j<resources.size();++j)
            for(const auto& a:resources[i].direct)
                for(const auto& b:resources[j].direct){
                    if(a.ordinal!=b.ordinal)continue;
                    const size_t n=prefix(a.data,b.data);
                    const bool equal=a.data.size()==b.data.size()&&n==a.data.size();
                    ++comparisons;if(equal)++identical;
                    std::printf("SPR_M277_CROSS resourceA=%s resourceB=%s ordinal=%zu markerA=0x%08X markerB=0x%08X bytesA=%zu bytesB=%zu prefix=%zu identical=%d\n",
                        resources[i].name.c_str(),resources[j].name.c_str(),a.ordinal,
                        static_cast<unsigned>(a.marker),static_cast<unsigned>(b.marker),
                        a.data.size(),b.data.size(),n,equal?1:0);
                }
    std::printf("SPR_M277_SUMMARY resources=5 crossComparisons=%zu identicalCrossPayloads=%zu\n",comparisons,identical);
    std::puts("SPR_M277_NOTE Prefixes compare raw stored bytes, not decoded pixels. firstDifference=end indicates a complete prefix up to the shorter payload. Cross-resource comparisons require matching image ordinals.");
    return 0;
}
