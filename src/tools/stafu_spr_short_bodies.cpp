// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only survey of four-byte SPR bodies associated with marker 0x30000.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>
namespace {
constexpr const char* names[]={
    "BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR",
    "BRDGWORF.SPR","brdgdoor.spr","brdgtitl.spr","viewscr.spr"
};
uint32_t le32(const uint8_t* p) {
    return uint32_t(p[0])|(uint32_t(p[1])<<8)|
           (uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);
}
struct Stats {size_t count=0,afterSubstantial=0,afterShort=0,noPrior=0;};
}
int main(int argc,char** argv) {
    if(argc!=3){
        std::cerr<<"Usage: reluva-stafu-spr-short-bodies <installed-folder> <cd-folder>\n";
        return 2;
    }
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
    using Key=std::tuple<std::string,uint32_t,uint32_t>;
    std::map<Key,Stats> frequency;
    size_t unavailable=0,shortCount=0,other30000=0,shortOther=0;
    for(const char* name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)){
            std::cerr<<name<<": "<<error<<"\n";++unavailable;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)){
            std::cerr<<name<<": "<<error<<"\n";++unavailable;continue;
        }
        for(size_t ri=0;ri<index.records().size();++ri) {
            const auto& events=index.records()[ri].events;
            bool havePrior=false;
            size_t priorEvent=0;
            uint32_t priorMarker=0,priorA=0,priorB=0;
            std::string priorTag;
            size_t priorSize=0;
            for(size_t ei=0;ei<events.size();++ei) {
                reluva::stafu::SprImagePayloadAccessor::View v;
                if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,events[ei],v))
                    continue;
                if(v.marker==0x30000u&&v.size!=4)++other30000;
                if(v.marker!=0x30000u&&v.size==4)++shortOther;
                if(v.marker==0x30000u&&v.size==4) {
                    ++shortCount;
                    const uint32_t value=le32(v.data);
                    auto& stat=frequency[{events[ei].tag,v.headerA,value}];
                    ++stat.count;
                    if(!havePrior)++stat.noPrior;
                    else if(priorSize>4)++stat.afterSubstantial;
                    else ++stat.afterShort;
                    size_t emit=0,kram=0,other=0;
                    if(havePrior)for(size_t j=priorEvent+1;j<ei;++j) {
                        if(events[j].tag=="EMIT")++emit;
                        else if(events[j].tag=="KRAM")++kram;
                        else ++other;
                    }
                    std::cout<<"SHORT "<<name<<" record="<<ri+1<<" event="<<ei
                             <<" tag="<<events[ei].tag
                             <<" a="<<v.headerA<<" b="<<v.headerB
                             <<" valueLE32="<<value
                             <<" valueHex=0x"<<std::hex<<std::setw(8)
                             <<std::setfill('0')<<value<<std::dec<<std::setfill(' ')
                             <<" bytes="<<unsigned(v.data[0])<<","
                             <<unsigned(v.data[1])<<","<<unsigned(v.data[2])
                             <<","<<unsigned(v.data[3])
                             <<" priorEvent="<<(havePrior?std::to_string(priorEvent):"none")
                             <<" priorTag="<<(havePrior?priorTag:"none")
                             <<" priorMarker="<<(havePrior?std::to_string(priorMarker):"none")
                             <<" priorBodyBytes="<<(havePrior?std::to_string(priorSize):"none")
                             <<" sameAB="<<(havePrior&&priorA==v.headerA&&priorB==v.headerB?"yes":"no")
                             <<" betweenEMIT="<<emit<<" betweenKRAM="<<kram
                             <<" betweenOther="<<other<<"\n";
                }
                havePrior=true;priorEvent=ei;priorMarker=v.marker;
                priorA=v.headerA;priorB=v.headerB;
                priorTag=events[ei].tag;priorSize=v.size;
            }
        }
    }
    for(const auto& [key,stat]:frequency) {
        const auto& [tag,a,value]=key;
        std::cout<<"FREQUENCY tag="<<tag<<" a="<<a
                 <<" valueLE32="<<value<<" count="<<stat.count
                 <<" afterSubstantial="<<stat.afterSubstantial
                 <<" afterShort="<<stat.afterShort
                 <<" noPrior="<<stat.noPrior<<"\n";
    }
    std::cout<<"SUMMARY marker30000Short="<<shortCount
             <<" marker30000NotFour="<<other30000
             <<" otherMarkerFour="<<shortOther
             <<" distinctTagAValue="<<frequency.size()
             <<" unavailable="<<unavailable<<"\n";
    std::cout<<"Four-byte values are opaque. Prior events are record-local image-like "
                "events; intervening controls are counted without timing assumptions.\n";
    return unavailable?1:0;
}
