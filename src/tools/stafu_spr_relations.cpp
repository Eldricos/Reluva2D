// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only, record-local PMOC/MOCS pair relationship survey.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include "engines/stafu/spr_pair_relation.h"
#include <array>
#include <iostream>
#include <string>
#include <vector>
namespace {
constexpr const char* resources[]={
    "BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR",
    "BRDGWORF.SPR","brdgdoor.spr","brdgtitl.spr","viewscr.spr"
};
struct Counts {
    size_t pairs=0,headerEqual=0,exact=0,longSuffix=0,zeroTrimLong=0,fullSmaller=0;
    size_t smallMiddle=0,bitOverlap=0;
};
void tally(Counts& c,const reluva::stafu::SprPairRelation& r,bool headers) {
    ++c.pairs;
    if(headers)++c.headerEqual;
    if(r.exact)++c.exact;
    if(r.sharedSuffixBits>=128) {
        ++c.longSuffix;
        if(r.trimmedZero)++c.zeroTrimLong;
    }
    if(r.fullSmallerSuffix)++c.fullSmaller;
    if(r.middleA<=32&&r.middleB<=32)++c.smallMiddle;
    if(r.suffixOverlapsPrefix)++c.bitOverlap;
}
void summary(const char* label,const Counts& c) {
    std::cout<<"SUMMARY "<<label<<" pairs="<<c.pairs<<" sameHeader="<<c.headerEqual
             <<" exact="<<c.exact<<" suffix128="<<c.longSuffix
             <<" zeroTrimSuffix128="<<c.zeroTrimLong
             <<" fullSmallerSuffix="<<c.fullSmaller
             <<" middleBothAtMost32="<<c.smallMiddle
             <<" bitSuffixOverlapsPrefix="<<c.bitOverlap<<"\n";
}
}
int main(int argc,char** argv) {
    if(argc!=3) {
        std::cerr<<"Usage: reluva-stafu-spr-relations <installed-folder> <cd-folder>\n";
        return 2;
    }
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(argv[1],argv[2],error)) {
        std::cerr<<"Resource manager: "<<error<<"\n";return 1;
    }
    Counts all,pmocPmoc,pmocMocs,mocsMocs,mocsPmoc;
    size_t unavailable=0;
    std::cout<<"Record-local SPR payload relations. No frame/delta semantics implied.\n";
    for(const char* name:resources) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::cerr<<name<<" unavailable: "<<error<<"\n";++unavailable;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::cerr<<name<<" parse error: "<<error<<"\n";++unavailable;continue;
        }
        for(size_t ri=0;ri<index.records().size();++ri) {
            const auto& events=index.records()[ri].events;
            reluva::stafu::SprImagePayloadAccessor::View prior,now;
            size_t priorEvent=0;
            bool have=false;
            for(size_t ei=0;ei<events.size();++ei) {
                if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,events[ei],now))
                    continue;
                if(have&&prior.size>=64&&now.size>=64) {
                    const auto r=reluva::stafu::compareSprPair(prior.data,prior.size,
                                                                now.data,now.size);
                    const bool headers=prior.headerA==now.headerA&&
                                       prior.headerB==now.headerB&&prior.marker==now.marker;
                    Counts* group=nullptr;
                    const auto& a=events[priorEvent].tag;
                    const auto& b=events[ei].tag;
                    if(a=="PMOC"&&b=="PMOC")group=&pmocPmoc;
                    else if(a=="PMOC"&&b=="MOCS")group=&pmocMocs;
                    else if(a=="MOCS"&&b=="MOCS")group=&mocsMocs;
                    else group=&mocsPmoc;
                    tally(all,r,headers);tally(*group,r,headers);
                    std::cout<<name<<" record="<<ri+1<<" events="<<priorEvent<<"->"<<ei
                             <<" tags="<<a<<"->"<<b
                             <<" bytes="<<prior.size<<","<<now.size
                             <<" headerEqual="<<(headers?"yes":"no")
                             <<" exact="<<(r.exact?"yes":"no")
                             <<" prefixBytes="<<r.prefixBytes
                             <<" suffixBytes="<<r.suffixBytes
                             <<" middleBytes="<<r.middleA<<","<<r.middleB
                             <<" bitSuffixStarts="<<r.suffixStartBitA<<","<<r.suffixStartBitB
                             <<" bitSuffixOverlapsPrefix="<<(r.suffixOverlapsPrefix?"yes":"no")
                             <<" msbSuffixBits="<<r.sharedSuffixBits
                             <<" trims="<<r.trimA<<","<<r.trimB
                             <<" trimZero="<<(r.trimmedZero?"yes":"no")
                             <<" fullSmallerSuffix="<<(r.fullSmallerSuffix?"yes":"no")
                             <<"\n";
                }
                prior=now;priorEvent=ei;have=true;
            }
        }
    }
    summary("ALL",all);summary("PMOC->PMOC",pmocPmoc);
    summary("PMOC->MOCS",pmocMocs);summary("MOCS->MOCS",mocsMocs);
    summary("MOCS->PMOC",mocsPmoc);
    std::cout<<"Substantial pairs only (both >=64 bytes). MSB suffix scan capped at 4096 bytes.\n";
    std::cout<<"middleBytes is the unmatched byte region after disjoint prefix/suffix removal.\n";
    std::cout<<"Bit suffix boundaries use the capped 4096-byte end-anchored comparison.\n";
    std::cout<<"A shared suffix or equal header is not proof of complete or delta frames.\n";
    return unavailable?1:0;
}
