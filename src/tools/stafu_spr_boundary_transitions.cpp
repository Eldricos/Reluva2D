// SPDX-License-Identifier: GPL-3.0-or-later
// M255: read-only boundary transitions and signature recurrence in Picard gap one.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
bool bit(const uint8_t* d,size_t i){return ((d[i/8]>>(7-(i%8)))&1u)!=0;}
struct Sample {size_t record,image,offset,size;};
struct Run {size_t start,length;bool equal;};
void inspect(const std::vector<uint8_t>& bytes,const Sample& a,const Sample& b,
             size_t startA,size_t endA,size_t startB,size_t endB){
    if(endA>a.size*8||endB>b.size*8||startA>endA||startB>endB){
        std::puts("SPR_M255_ERROR invalid-gap-coordinates");return;
    }
    const uint8_t* pa=bytes.data()+a.offset;
    const uint8_t* pb=bytes.data()+b.offset;
    const size_t n=std::min(endA-startA,endB-startB);
    // Boundaries are offsets between adjacent bits, inferred from M254's run map.
    const size_t boundaries[]={69,140,141,570,571,705,706,1168};
    const size_t widths[]={8,16,32};
    for(size_t boundary:boundaries){
        if(boundary>n)continue;
        const bool before=boundary>0&&bit(pa,startA+boundary-1)==bit(pb,startB+boundary-1);
        const bool after=boundary<n&&bit(pa,startA+boundary)==bit(pb,startB+boundary);
        std::printf("SPR_M255_BOUNDARY offset=%zu absoluteA=%zu absoluteB=%zu beforeEqual=%d afterEqual=%d\n",
            boundary,startA+boundary,startB+boundary,before?1:0,after?1:0);
        for(size_t width:widths){
            if(boundary<width||boundary+width>n)continue;
            std::string patternA,patternB,match;
            for(size_t i=boundary-width;i<boundary+width;++i){
                bool va=bit(pa,startA+i),vb=bit(pb,startB+i);
                patternA+=va?'1':'0';
                patternB+=vb?'1':'0';
                match+=va==vb?'=':'X';
            }
            // Exact boundary-spanning signature recurrence inside the full image payloads.
            // A signature has 2*width bits; do not count the source occurrence.
            auto occurrences=[&](const uint8_t* p,size_t totalBits,const std::string& pattern,size_t origin){
                size_t count=0;
                for(size_t pos=0;pos+pattern.size()<=totalBits;++pos){
                    if(pos==origin)continue;
                    size_t j=0;
                    for(;j<pattern.size();++j)
                        if(bit(p,pos+j)!=(pattern[j]=='1'))break;
                    if(j==pattern.size())++count;
                }
                return count;
            };
            const size_t originA=startA+boundary-width,originB=startB+boundary-width;
            const size_t repeatA=occurrences(pa,a.size*8,patternA,originA);
            const size_t repeatB=occurrences(pb,b.size*8,patternB,originB);
            const size_t crossA=occurrences(pb,b.size*8,patternA,b.size*8);
            const size_t crossB=occurrences(pa,a.size*8,patternB,a.size*8);
            size_t changed=0;
            for(char c:match)if(c=='X')++changed;
            std::printf("SPR_M255_WINDOW offset=%zu flank=%zu differing=%zu patternA=%s patternB=%s comparison=%s repeatsA=%zu repeatsB=%zu crossAinB=%zu crossBinA=%zu\n",
                boundary,width,changed,patternA.c_str(),patternB.c_str(),match.c_str(),
                repeatA,repeatB,crossA,crossB);
        }
    }
    const size_t regions[][2]={{0,69},{69,140},{141,570},{571,705},{706,1168},{1168,1285}};
    for(size_t i=0;i<6;++i){
        const size_t lo=regions[i][0],hi=regions[i][1];
        size_t diff=0;
        for(size_t j=lo;j<hi&&j<n;++j)
            diff+=bit(pa,startA+j)!=bit(pb,startB+j);
        std::printf("SPR_M255_REGION index=%zu start=%zu end=%zu compared=%zu differing=%zu\n",
            i+1,lo,hi,hi-lo,diff);
    }
    std::puts("SPR_M255_SUMMARY boundaries=8 widths=8,16,32 regions=6");
}
}
int stafuSprM255BoundaryTransitions(const char* installed,const char* cd,const char* optionalName){
    const std::string name=optionalName?optionalName:"BRDGPICA.SPR";
    if(name!="BRDGPICA.SPR"){
        std::fprintf(stderr,"spr-m255-transitions: M255 target is BRDGPICA.SPR\n");return 2;
    }
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"spr-m255-transitions: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"spr-m255-transitions: %s\n",error.c_str());return 1;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){
        std::fprintf(stderr,"spr-m255-transitions: %s\n",error.c_str());return 1;
    }
    std::vector<Sample> samples;
    for(size_t ri=0;ri<index.records().size();++ri){
        size_t ordinal=0;
        for(const auto& e:index.records()[ri].events){
            if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
            reluva::stafu::SprImagePayloadAccessor::View v;
            if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,v)&&
               !(v.marker==0x00030000&&v.size==4))
                samples.push_back({ri,ordinal,v.offset,v.size});
            ++ordinal;
        }
    }
    const Sample* a=nullptr;const Sample* b=nullptr;
    for(const auto& s:samples){
        if(s.record==5&&s.image==1)a=&s;
        if(s.record==5&&s.image==2)b=&s;
    }
    if(!a||!b){std::puts("SPR_M255_ERROR missing-direct-images");return 1;}
    inspect(bytes,*a,*b,3204,4489,3234,4526);
    std::puts("SPR_M255_NOTE MSB-first zero-based payload-relative positions; windows span each observed transition. Recurrences search full direct payloads, excluding self. Boundaries are observational, not codec fields.");
    return 0;
}
