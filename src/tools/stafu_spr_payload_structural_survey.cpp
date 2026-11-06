// SPDX-License-Identifier: GPL-3.0-or-later
// M275: read-only SPR payload structural survey; no codec interpretation.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace {
struct Stats {
    size_t count=0,bytes=0,minSize=static_cast<size_t>(-1),maxSize=0;
    size_t prefix4=0,prefix8=0,referenceLike=0;
    void add(const reluva::stafu::SprImagePayloadAccessor::View& v) {
        ++count;bytes+=v.size;minSize=std::min(minSize,v.size);
        maxSize=std::max(maxSize,v.size);
        if(v.size==4)++referenceLike;
    }
};
size_t prefix(const uint8_t* a,size_t na,const uint8_t* b,size_t nb) {
    const size_t limit=std::min(na,nb);
    size_t i=0;while(i<limit&&a[i]==b[i])++i;return i;
}
bool survey(const char* installed,const char* cd,const char* name) {
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"SPR_M275_ERROR %s\n",error.c_str());return false;}
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){std::fprintf(stderr,"SPR_M275_ERROR resource=%s %s\n",name,error.c_str());return false;}
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){std::fprintf(stderr,"SPR_M275_ERROR resource=%s %s\n",name,error.c_str());return false;}
    std::map<uint32_t,Stats> byMarker;
    size_t images=0,invalid=0,neighbors=0,prefix4=0,prefix8=0;
    size_t referenceLike=0,directLike=0,recordsWithImages=0;
    for(size_t ri=0;ri<index.records().size();++ri) {
        size_t ordinal=0,recordImages=0,recordBytes=0,recordRefs=0;
        size_t recordPrefix4=0,recordPrefix8=0;
        reluva::stafu::SprImagePayloadAccessor::View previous;
        bool havePrevious=false;
        for(const auto& event:index.records()[ri].events) {
            if(event.tag!="PMOC"&&event.tag!="MOCS")continue;
            reluva::stafu::SprImagePayloadAccessor::View v;
            if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,event,v)){
                ++invalid;++ordinal;continue;
            }
            ++images;++recordImages;recordBytes+=v.size;
            byMarker[v.marker].add(v);
            const bool ref=(v.marker==0x00030000&&v.size==4);
            if(ref){++referenceLike;++recordRefs;}else ++directLike;
            if(havePrevious) {
                ++neighbors;
                const size_t n=prefix(previous.data,previous.size,v.data,v.size);
                if(n>=4){++prefix4;++recordPrefix4;}
                if(n>=8){++prefix8;++recordPrefix8;}
            }
            previous=v;havePrevious=true;
            ++ordinal;
        }
        if(recordImages) {
            ++recordsWithImages;
            std::printf("SPR_M275_RECORD resource=%s record=%zu images=%zu bytes=%zu referenceLike=%zu neighborPrefix4=%zu neighborPrefix8=%zu\n",
                name,ri,recordImages,recordBytes,recordRefs,recordPrefix4,recordPrefix8);
        }
    }
    for(const auto& [marker,s]:byMarker) {
        std::printf("SPR_M275_MARKER resource=%s marker=0x%08X count=%zu bytes=%zu minBytes=%zu maxBytes=%zu size4=%zu\n",
            name,static_cast<unsigned>(marker),s.count,s.bytes,s.minSize,s.maxSize,s.referenceLike);
    }
    std::printf("SPR_M275_RESOURCE resource=%s records=%zu recordsWithImages=%zu images=%zu referenceLike=%zu otherImages=%zu invalid=%zu neighbors=%zu prefix4=%zu prefix8=%zu\n",
        name,index.records().size(),recordsWithImages,images,referenceLike,directLike,invalid,neighbors,prefix4,prefix8);
    return invalid==0;
}
}
int stafuSprM275PayloadStructuralSurvey(const char* installed,const char* cd,const char*) {
    const char* names[]={"BRDGPICA.SPR","BRDGDATA.SPR","BRDGRIKE.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    for(const char* name:names)if(!survey(installed,cd,name))return 1;
    std::puts("SPR_M275_SUMMARY resources=5");
    std::puts("SPR_M275_NOTE Prefix counts compare consecutive image-like payloads in each record; reference-like means marker 0x00030000 and size 4. No decoding or codec inference.");
    return 0;
}
