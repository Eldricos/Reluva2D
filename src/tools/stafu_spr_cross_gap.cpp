// SPDX-License-Identifier: GPL-3.0-or-later
// M256: read-only comparison of mismatch clustering and preserved runs across six gaps.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
bool bitAt(const uint8_t* p,size_t bit){return ((p[bit/8]>>(7-(bit%8)))&1u)!=0;}
struct Sample {size_t record,image,offset,size;};
struct Run {size_t start,length;bool equal;};
struct Alignment {int shift;size_t startA,startB,compared,equal;};
bool better(const Alignment& x,const Alignment& y){
    if(!x.compared)return false;
    if(!y.compared)return true;
    const uint64_t lhs=static_cast<uint64_t>(x.equal)*y.compared;
    const uint64_t rhs=static_cast<uint64_t>(y.equal)*x.compared;
    if(lhs!=rhs)return lhs>rhs;
    if(x.compared!=y.compared)return x.compared>y.compared;
    return (x.shift<0?-x.shift:x.shift)<(y.shift<0?-y.shift:y.shift);
}
void map(const char* name,size_t record,size_t imageA,size_t imageB,
         const uint8_t* a,size_t na,const uint8_t* b,size_t nb,
         size_t startA,size_t endA,size_t startB,size_t endB){
    if(startA>endA||startB>endB||endA>na||endB>nb){
        std::printf("SPR_M256_ERROR resource=%s reason=gap-out-of-range\n",name);return;
    }
    const size_t lenA=endA-startA,lenB=endB-startB;
    auto evaluate=[&](int shift){
        const size_t aa=shift<0?static_cast<size_t>(-shift):0;
        const size_t bb=shift>0?static_cast<size_t>(shift):0;
        Alignment v{shift,aa,bb,0,0};
        if(aa>=lenA||bb>=lenB)return v;
        v.compared=std::min(lenA-aa,lenB-bb);
        for(size_t i=0;i<v.compared;++i)
            v.equal+=bitAt(a,startA+aa+i)==bitAt(b,startB+bb+i);
        return v;
    };
    Alignment direct=evaluate(0),best=direct;
    for(int shift=-32;shift<=32;++shift){
        Alignment candidate=evaluate(shift);
        if(better(candidate,best))best=candidate;
    }
    auto describe=[&](const char* mode,const Alignment& v){
        std::vector<Run> runs;
        size_t mismatchRuns=0,mismatchBits=0,longestExact=0,qualifying=0;
        size_t leadingMismatch=0,trailingMismatch=0;
        size_t quartiles[4]={0,0,0,0};
        for(size_t i=0;i<v.compared;++i){
            bool eq=bitAt(a,startA+v.startA+i)==bitAt(b,startB+v.startB+i);
            if(!eq){++mismatchBits;++quartiles[std::min(size_t(3),i*4/v.compared)];}
            if(!runs.empty()&&runs.back().equal==eq)++runs.back().length;
            else runs.push_back({i,1,eq});
        }
        for(const auto& r:runs){
            if(r.equal){
                longestExact=std::max(longestExact,r.length);
                if(r.length>=32)++qualifying;
            }else ++mismatchRuns;
        }
        if(!runs.empty()&&!runs.front().equal)leadingMismatch=runs.front().length;
        if(!runs.empty()&&!runs.back().equal)trailingMismatch=runs.back().length;
        size_t centralBits=0,centralMismatch=0;
        const size_t centralStart=v.compared/4,centralEnd=v.compared*3/4;
        for(size_t i=centralStart;i<centralEnd;++i){
            ++centralBits;
            centralMismatch+=bitAt(a,startA+v.startA+i)!=bitAt(b,startB+v.startB+i);
        }
        std::printf("SPR_M256_GAP resource=%s record=%zu imageA=%zu imageB=%zu mode=%s shift=%d compared=%zu equal=%zu mismatch=%zu mismatchRuns=%zu longestExact=%zu exactRuns32=%zu leadingMismatchRun=%zu trailingMismatchRun=%zu centralBits=%zu centralMismatch=%zu q1=%zu q2=%zu q3=%zu q4=%zu\n",
            name,record,imageA,imageB,mode,v.shift,v.compared,v.equal,mismatchBits,
            mismatchRuns,longestExact,qualifying,leadingMismatch,trailingMismatch,
            centralBits,centralMismatch,quartiles[0],quartiles[1],quartiles[2],quartiles[3]);
        std::vector<Run> exact;
        for(const auto& r:runs)if(r.equal&&r.length>=32)exact.push_back(r);
        std::sort(exact.begin(),exact.end(),[](const Run& x,const Run& y){
            if(x.length!=y.length)return x.length>y.length;
            return x.start<y.start;
        });
        const size_t shown=std::min(size_t(8),exact.size());
        for(size_t i=0;i<shown;++i){
            const Run& r=exact[i];
            const size_t left=r.start,right=v.compared-r.start-r.length;
            size_t leftDiff=0,rightDiff=0;
            for(size_t j=r.start>32?r.start-32:0;j<r.start;++j)
                leftDiff+=bitAt(a,startA+v.startA+j)!=bitAt(b,startB+v.startB+j);
            for(size_t j=r.start+r.length;j<std::min(v.compared,r.start+r.length+32);++j)
                rightDiff+=bitAt(a,startA+v.startA+j)!=bitAt(b,startB+v.startB+j);
            std::printf("SPR_M256_EXACT resource=%s mode=%s rank=%zu offsetA=%zu offsetB=%zu length=%zu beforeBits=%zu afterBits=%zu leftDiff32=%zu rightDiff32=%zu\n",
                name,mode,i+1,v.startA+r.start,v.startB+r.start,r.length,
                left,right,leftDiff,rightDiff);
        }
    };
    std::printf("SPR_M256_PAIR resource=%s record=%zu imageA=%zu imageB=%zu bitsA=%zu bitsB=%zu bestShift=%d\n",
        name,record,imageA,imageB,lenA,lenB,best.shift);
    describe("direct",direct);
    if(best.shift!=0)describe("best-shift",best);
}
}
int stafuSprM256CrossGap(const char* installed,const char* cd,const char* optionalName){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"spr-m256-cross-gap: %s\n",error.c_str());return 1;
    }
    const std::string name=optionalName?optionalName:"BRDGPICA.SPR";
    if(name!="BRDGPICA.SPR"&&name!="BRDGDATA.SPR"){
        std::fprintf(stderr,"spr-m256-cross-gap: supported resources: BRDGPICA.SPR, BRDGDATA.SPR\n");return 2;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"spr-m256-cross-gap: %s\n",error.c_str());return 1;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){
        std::fprintf(stderr,"spr-m256-cross-gap: %s\n",error.c_str());return 1;
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
    auto find=[&](size_t record,size_t image)->const Sample*{
        for(const auto& s:samples)if(s.record==record&&s.image==image)return &s;
        return nullptr;
    };
    size_t count=0;
    auto inspect=[&](size_t record,size_t imageA,size_t imageB,
                     size_t sa,size_t ea,size_t sb,size_t eb){
        const Sample* a=find(record,imageA);
        const Sample* b=find(record,imageB);
        if(!a||!b){
            std::printf("SPR_M256_ERROR resource=%s record=%zu imageA=%zu imageB=%zu reason=missing-direct-payload\n",
                name.c_str(),record,imageA,imageB);return;
        }
        map(name.c_str(),record,imageA,imageB,bytes.data()+a->offset,a->size*8,
            bytes.data()+b->offset,b->size*8,sa,ea,sb,eb);
        ++count;
    };
    if(name=="BRDGPICA.SPR"){
        // M249 verified interior gap coordinates, not guessed codec boundaries.
        inspect(5,1,2,3204,4489,3234,4526);
        inspect(5,1,2,4967,5093,5004,5120);
        inspect(5,1,2,9730,10499,9792,10603);
        inspect(5,3,4,6342,6591,6464,6713);
    }else{
        inspect(0,0,1,9973,10390,9966,10356);
        inspect(0,0,1,10781,11976,10747,11952);
    }
    std::printf("SPR_M256_SUMMARY resource=%s inspected=%zu expected=%d\n",
        name.c_str(),count,name=="BRDGPICA.SPR"?4:2);
    std::puts("SPR_M256_NOTE M249 gap coordinates, MSB-first. Direct and best +/-32-bit alignment; exact runs >=32 bits, up to 8 reported per alignment. Quartiles are alignment-local; no codec interpretation.");
    return count==(name=="BRDGPICA.SPR"?4u:2u)?0:1;
}
