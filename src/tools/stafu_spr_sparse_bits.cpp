// SPDX-License-Identifier: GPL-3.0-or-later
// M252: read-only sparse bit substitutions in the M251 Picard exact-run cluster.
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
        std::puts("SPR_SPARSE_ERROR invalid-gap-coordinates");return;
    }
    const uint8_t* pa=bytes.data()+a.offset;
    const uint8_t* pb=bytes.data()+b.offset;
    const size_t n=std::min(endA-startA,endB-startB);
    std::vector<Run> runs;
    size_t equal=0;
    for(size_t i=0;i<n;++i){
        const bool eq=bit(pa,startA+i)==bit(pb,startB+i);
        equal+=eq?1u:0u;
        if(!runs.empty()&&runs.back().equal==eq)++runs.back().length;
        else runs.push_back({i,1,eq});
    }
    std::vector<Run> longRuns;
    for(const auto& r:runs)if(r.equal&&r.length>=32)longRuns.push_back(r);
    std::printf("SPR_SPARSE_GAP record=%zu imageA=%zu imageB=%zu startA=%zu startB=%zu compared=%zu equal=%zu longRuns=%zu\n",
        a.record,a.image,b.image,startA,startB,n,equal,longRuns.size());
    for(const auto& r:longRuns)
        std::printf("SPR_SPARSE_EXACT offsetA=%zu offsetB=%zu absoluteA=%zu absoluteB=%zu length=%zu\n",
            r.start,r.start,startA+r.start,startB+r.start,r.length);
    size_t singles=0;
    std::vector<size_t> isolated;
    for(size_t i=1;i<longRuns.size();++i){
        const size_t from=longRuns[i-1].start+longRuns[i-1].length;
        const size_t to=longRuns[i].start;
        const size_t width=to-from;
        std::printf("SPR_SPARSE_INTERVAL fromRun=%zu toRun=%zu startA=%zu startB=%zu width=%zu\n",
            i,i+1,startA+from,startB+from,width);
        if(width!=1)continue;
        const size_t pos=from;
        const unsigned valueA=bit(pa,startA+pos)?1u:0u;
        const unsigned valueB=bit(pb,startB+pos)?1u:0u;
        if(valueA==valueB){
            std::puts("SPR_SPARSE_ERROR expected-single-mismatch");continue;
        }
        isolated.push_back(pos);++singles;
        std::printf("SPR_SPARSE_BIT index=%zu offset=%zu absoluteA=%zu absoluteB=%zu byteA=%zu bitInByteA=%zu byteB=%zu bitInByteB=%zu valueA=%u valueB=%u direction=%s\n",
            singles,pos,startA+pos,startB+pos,(startA+pos)/8,(startA+pos)%8,
            (startB+pos)/8,(startB+pos)%8,valueA,valueB,valueA?"1to0":"0to1");
    }
    for(size_t i=1;i<isolated.size();++i)
        std::printf("SPR_SPARSE_SPACING from=%zu to=%zu distanceBits=%zu\n",
            i,i+1,isolated[i]-isolated[i-1]);
    for(size_t period: {size_t(10),size_t(20),size_t(30),size_t(40)}){
        const size_t first=isolated.empty()?0:isolated.front()%period;
        bool same=!isolated.empty();
        for(const size_t x:isolated)if(x%period!=first)same=false;
        std::printf("SPR_SPARSE_PHASE period=%zu samePhase=%d",period,same?1:0);
        for(size_t i=0;i<isolated.size();++i)
            std::printf(" phase%zu=%zu",i+1,isolated[i]%period);
        std::printf("\n");
    }
    std::printf("SPR_SPARSE_SUMMARY isolated=%zu longRuns=%zu compared=%zu equal=%zu\n",
        isolated.size(),longRuns.size(),n,equal);
}
}
int stafuSprSparseBits(const char* installed,const char* cd,const char* optionalName){
    const std::string name=optionalName?optionalName:"BRDGPICA.SPR";
    if(name!="BRDGPICA.SPR"){
        std::fprintf(stderr,"spr-sparse-bits: M252 target is BRDGPICA.SPR\n");return 2;
    }
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"spr-sparse-bits: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"spr-sparse-bits: %s\n",error.c_str());return 1;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){
        std::fprintf(stderr,"spr-sparse-bits: %s\n",error.c_str());return 1;
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
    if(!a||!b){std::puts("SPR_SPARSE_ERROR missing-direct-images");return 1;}
    inspect(bytes,*a,*b,3204,4489,3234,4526);
    std::puts("SPR_SPARSE_NOTE MSB-first payload-relative coordinates; phase is relative to gap start. Period checks are descriptive only. No codec semantics inferred.");
    return 0;
}
