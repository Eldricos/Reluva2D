// SPDX-License-Identifier: GPL-3.0-or-later
// M276: classify adjacent SPR image payload prefixes without interpreting codec data.
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
struct Counts {
    size_t pairs=0,zero=0,oneTo3=0,fourTo7=0,eightTo15=0,sixteenTo31,thirtyTwoPlus=0;
    size_t maxPrefix=0;
    void add(size_t n) {
        ++pairs;maxPrefix=std::max(maxPrefix,n);
        if(n==0)++zero;
        else if(n<4)++oneTo3;
        else if(n<8)++fourTo7;
        else if(n<16)++eightTo15;
        else if(n<32)++sixteenTo31;
        else ++thirtyTwoPlus;
    }
};
size_t commonPrefix(const reluva::stafu::SprImagePayloadAccessor::View& a,
                    const reluva::stafu::SprImagePayloadAccessor::View& b) {
    size_t i=0,limit=std::min(a.size,b.size);
    while(i<limit&&a.data[i]==b.data[i])++i;
    return i;
}
bool isRef(const reluva::stafu::SprImagePayloadAccessor::View& v) {
    return v.marker==0x00030000&&v.size==4;
}
void report(const char* type,const char* resource,size_t record,
            const char* category,const Counts& s,uint32_t markerA=0,uint32_t markerB=0) {
    if(!s.pairs)return;
    if(std::string(type)=="MARKER")
        std::printf("SPR_M276_MARKER resource=%s markerA=0x%08X markerB=0x%08X ",
                    resource,static_cast<unsigned>(markerA),static_cast<unsigned>(markerB));
    else if(std::string(type)=="RECORD")
        std::printf("SPR_M276_RECORD resource=%s record=%zu category=%s ",resource,record,category);
    else
        std::printf("SPR_M276_RESOURCE resource=%s category=%s ",resource,category);
    std::printf("pairs=%zu prefix0=%zu prefix1to3=%zu prefix4to7=%zu prefix8to15=%zu prefix16to31=%zu prefix32plus=%zu maxPrefix=%zu\n",
        s.pairs,s.zero,s.oneTo3,s.fourTo7,s.eightTo15,s.sixteenTo31,s.thirtyTwoPlus,s.maxPrefix);
}
bool survey(reluva::stafu::ResourceManager& manager,const char* name,
            size_t totals[3],size_t& allInvalid) {
    std::vector<uint8_t> bytes;std::string error;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"SPR_M276_ERROR resource=%s %s\n",name,error.c_str());return false;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){
        std::fprintf(stderr,"SPR_M276_ERROR resource=%s %s\n",name,error.c_str());return false;
    }
    Counts resource[3];
    std::map<std::pair<uint32_t,uint32_t>,Counts> markerPairs;
    size_t invalid=0;
    for(size_t ri=0;ri<index.records().size();++ri) {
        Counts record[3];
        reluva::stafu::SprImagePayloadAccessor::View prev;
        bool havePrev=false;
        for(const auto& event:index.records()[ri].events) {
            if(event.tag!="PMOC"&&event.tag!="MOCS")continue;
            reluva::stafu::SprImagePayloadAccessor::View current;
            if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,event,current)){
                ++invalid;havePrev=false;continue;
            }
            if(havePrev) {
                const bool a=isRef(prev),b=isRef(current);
                const size_t cat=!a&&!b?0:a&&b?2:1;
                const size_t n=commonPrefix(prev,current);
                record[cat].add(n);resource[cat].add(n);
                if(cat==0)markerPairs[{prev.marker,current.marker}].add(n);
            }
            prev=current;havePrev=true;
        }
        const char* cats[]={"direct-direct","mixed","reference-reference"};
        for(size_t c=0;c<3;++c)report("RECORD",name,ri,cats[c],record[c]);
    }
    const char* cats[]={"direct-direct","mixed","reference-reference"};
    for(size_t c=0;c<3;++c) {
        report("RESOURCE",name,0,cats[c],resource[c]);
        totals[c]+=resource[c].pairs;
    }
    for(const auto& kv:markerPairs)
        report("MARKER",name,0,"",kv.second,kv.first.first,kv.first.second);
    allInvalid+=invalid;
    std::printf("SPR_M276_CHECK resource=%s invalid=%zu\n",name,invalid);
    return invalid==0;
}
}
int stafuSprM276DirectPayloadPrefixClassification(const char* installed,const char* cd,const char*) {
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"SPR_M276_ERROR %s\n",error.c_str());return 1;
    }
    const char* names[]={"BRDGPICA.SPR","BRDGDATA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    size_t totals[3]={0,0,0},invalid=0;
    for(const char* name:names)if(!survey(manager,name,totals,invalid))return 1;
    std::printf("SPR_M276_SUMMARY resources=5 directDirect=%zu mixed=%zu referenceReference=%zu totalPairs=%zu invalid=%zu\n",
        totals[0],totals[1],totals[2],totals[0]+totals[1]+totals[2],invalid);
    std::puts("SPR_M276_NOTE Adjacent image-like events within records; marker 0x00030000 and 4 bytes classified reference-like. Prefixes are raw bytes, not decoded pixels. Direct marker-pair counts exclude reference-like events.");
    return 0;
}
