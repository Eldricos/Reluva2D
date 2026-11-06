// SPDX-License-Identifier: GPL-3.0-or-later
// M253: read-only context and recurrence comparison around three Picard substitutions.
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
        std::puts("SPR_CONTEXT_ERROR invalid-gap-coordinates");return;
    }
    const uint8_t* pa=bytes.data()+a.offset;
    const uint8_t* pb=bytes.data()+b.offset;
    const size_t n=std::min(endA-startA,endB-startB);
    const size_t targets[]={140,570,705};
    size_t checked=0;
    for(size_t t:targets){
        if(t>=n||bit(pa,startA+t)==bit(pb,startB+t)){
            std::printf("SPR_CONTEXT_ERROR offset=%zu reason=not-a-mismatch\n",t);continue;
        }
        ++checked;
        std::printf("SPR_CONTEXT_BIT offset=%zu absoluteA=%zu absoluteB=%zu valueA=%u valueB=%u\n",
            t,startA+t,startB+t,bit(pa,startA+t)?1u:0u,bit(pb,startB+t)?1u:0u);
        for(size_t width:{size_t(8),size_t(16),size_t(32),size_t(64)}){
            // Symmetric context on each side; center substitution excluded.
            if(startA+t<width||startB+t<width||
               startA+t+width>=a.size*8||startB+t+width>=b.size*8){
                std::printf("SPR_CONTEXT_WINDOW offset=%zu flank=%zu status=out-of-range\n",t,width);
                continue;
            }
            size_t left=0,right=0;
            for(size_t j=1;j<=width;++j){
                left+=bit(pa,startA+t-j)==bit(pb,startB+t-j);
                right+=bit(pa,startA+t+j)==bit(pb,startB+t+j);
            }
            // Recurrences are exact centered contexts within each image payload.
            // Search both orientations; exclude the center occurrence.
            size_t matchesA=0,matchesB=0,crossAtoB=0,crossBtoA=0;
            size_t firstA=0,firstB=0,firstCrossAB=0,firstCrossBA=0;
            auto sameContext=[&](const uint8_t* x,size_t cx,const uint8_t* y,size_t cy){
                for(size_t j=1;j<=width;++j)
                    if(bit(x,cx-j)!=bit(y,cy-j)||bit(x,cx+j)!=bit(y,cy+j))
                        return false;
                return true;
            };
            const size_t centerA=startA+t,centerB=startB+t;
            for(size_t pos=width;pos+width<a.size*8;++pos){
                if(pos!=centerA&&sameContext(pa,centerA,pa,pos)){
                    if(!matchesA)firstA=pos;
                    ++matchesA;
                }
                if(sameContext(pb,centerB,pa,pos)){
                    if(!crossBtoA)firstCrossBA=pos;
                    ++crossBtoA;
                }
            }
            for(size_t pos=width;pos+width<b.size*8;++pos){
                if(pos!=centerB&&sameContext(pb,centerB,pb,pos)){
                    if(!matchesB)firstB=pos;
                    ++matchesB;
                }
                if(sameContext(pa,centerA,pb,pos)){
                    if(!crossAtoB)firstCrossAB=pos;
                    ++crossAtoB;
                }
            }
            std::printf("SPR_CONTEXT_WINDOW offset=%zu flank=%zu leftEqual=%zu rightEqual=%zu contextBits=%zu recurrenceA=%zu recurrenceB=%zu crossAtoB=%zu crossBtoA=%zu firstA=%zu firstB=%zu firstCrossAB=%zu firstCrossBA=%zu\n",
                t,width,left,right,2*width,matchesA,matchesB,crossAtoB,crossBtoA,
                firstA,firstB,firstCrossAB,firstCrossBA);
        }
    }
    std::printf("SPR_CONTEXT_SUMMARY checked=%zu expected=3\n",checked);
}
}
int stafuSprM253ContextRecurrence(const char* installed,const char* cd,const char* optionalName){
    const std::string name=optionalName?optionalName:"BRDGPICA.SPR";
    if(name!="BRDGPICA.SPR"){
        std::fprintf(stderr,"spr-substitution-context: M252 target is BRDGPICA.SPR\n");return 2;
    }
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"spr-substitution-context: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"spr-substitution-context: %s\n",error.c_str());return 1;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){
        std::fprintf(stderr,"spr-substitution-context: %s\n",error.c_str());return 1;
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
    if(!a||!b){std::puts("SPR_CONTEXT_ERROR missing-direct-images");return 1;}
    inspect(bytes,*a,*b,3204,4489,3234,4526);
    std::puts("SPR_CONTEXT_NOTE MSB-first payload-relative coordinates. Flank widths are per side; central changed bit excluded from recurrence matching. Cross matches may include the corresponding image position. No codec semantics inferred.");
    return 0;
}
