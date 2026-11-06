// SPDX-License-Identifier: GPL-3.0-or-later
// M254: read-only classification of every mismatching bit in Picard gap one.
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
        std::puts("SPR_M254_ERROR invalid-gap-coordinates");return;
    }
    const uint8_t* pa=bytes.data()+a.offset;
    const uint8_t* pb=bytes.data()+b.offset;
    const size_t n=std::min(endA-startA,endB-startB);
    std::vector<Run> runs;
    size_t mismatches=0;
    for(size_t i=0;i<n;++i){
        bool equal=bit(pa,startA+i)==bit(pb,startB+i);
        if(!equal)++mismatches;
        if(!runs.empty()&&runs.back().equal==equal)++runs.back().length;
        else runs.push_back({i,1,equal});
    }
    size_t mismatchRuns=0,singles=0,multi=0,longest=0,firstMismatch=n,lastMismatch=0;
    size_t histogram[6]={0,0,0,0,0,0};
    size_t prior=n,distMin=n,distMax=0,distTotal=0,distCount=0;
    size_t quarter[4]={0,0,0,0};
    for(const auto& r:runs){
        if(r.equal)continue;
        ++mismatchRuns;
        longest=std::max(longest,r.length);
        if(r.length==1)++singles;else ++multi;
        ++histogram[std::min(r.length,size_t(6))-1];
        firstMismatch=std::min(firstMismatch,r.start);
        lastMismatch=std::max(lastMismatch,r.start+r.length-1);
        for(size_t j=0;j<r.length;++j){
            const size_t pos=r.start+j;
            ++quarter[std::min(size_t(3),pos*4/n)];
            if(prior!=n){
                const size_t d=pos-prior;
                distMin=std::min(distMin,d);
                distMax=std::max(distMax,d);
                distTotal+=d;++distCount;
            }
            prior=pos;
        }
        size_t left=0,right=0;
        if(r.start>0){
            for(size_t k=r.start;k>0&&bit(pa,startA+k-1)==bit(pb,startB+k-1);--k)++left;
        }
        for(size_t k=r.start+r.length;k<n&&bit(pa,startA+k)==bit(pb,startB+k);++k)++right;
        std::printf("SPR_M254_RUN index=%zu offset=%zu length=%zu absoluteA=%zu absoluteB=%zu precedingEqual=%zu followingEqual=%zu\n",
            mismatchRuns,r.start,r.length,startA+r.start,startB+r.start,left,right);
    }
    std::printf("SPR_M254_SUMMARY compared=%zu mismatches=%zu matching=%zu mismatchRuns=%zu singleRuns=%zu multiRuns=%zu longestMismatchRun=%zu firstMismatch=%zu lastMismatch=%zu\n",
        n,mismatches,n-mismatches,mismatchRuns,singles,multi,longest,
        firstMismatch==n?0:firstMismatch,lastMismatch);
    std::printf("SPR_M254_HISTOGRAM len1=%zu len2=%zu len3=%zu len4=%zu len5=%zu len6plus=%zu\n",
        histogram[0],histogram[1],histogram[2],histogram[3],histogram[4],histogram[5]);
    std::printf("SPR_M254_QUARTERS q1=%zu q2=%zu q3=%zu q4=%zu\n",
        quarter[0],quarter[1],quarter[2],quarter[3]);
    std::printf("SPR_M254_SPACING count=%zu min=%zu max=%zu sum=%zu mean=%.3f definition=consecutive-mismatching-bits\n",
        distCount,distCount?distMin:0,distMax,distTotal,
        distCount?double(distTotal)/double(distCount):0.0);
    size_t targetSingles=0;
    for(size_t t:{size_t(140),size_t(570),size_t(705)}){
        bool isolated=t>0&&t+1<n&&
            bit(pa,startA+t)!=bit(pb,startB+t)&&
            bit(pa,startA+t-1)==bit(pb,startB+t-1)&&
            bit(pa,startA+t+1)==bit(pb,startB+t+1);
        targetSingles+=isolated?1u:0u;
        std::printf("SPR_M254_REFERENCE offset=%zu isolated=%d\n",t,isolated?1:0);
    }
    std::printf("SPR_M254_CHECK referenceSingles=%zu expected=3\n",targetSingles);
}
}
int stafuSprM254GapClassification(const char* installed,const char* cd,const char* optionalName){
    const std::string name=optionalName?optionalName:"BRDGPICA.SPR";
    if(name!="BRDGPICA.SPR"){
        std::fprintf(stderr,"spr-m254-gap: M254 target is BRDGPICA.SPR\n");return 2;
    }
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"spr-m254-gap: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"spr-m254-gap: %s\n",error.c_str());return 1;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){
        std::fprintf(stderr,"spr-m254-gap: %s\n",error.c_str());return 1;
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
    if(!a||!b){std::puts("SPR_M254_ERROR missing-direct-images");return 1;}
    inspect(bytes,*a,*b,3204,4489,3234,4526);
    std::puts("SPR_M254_NOTE MSB-first, zero-based bit positions relative to each image payload; mismatch spacing includes adjacent bits in multi-bit runs. No codec semantics inferred.");
    return 0;
}
