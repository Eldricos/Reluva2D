// SPDX-License-Identifier: GPL-3.0-or-later
// M257: read-only flank and transition comparison around longest exact run in six gaps.
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
        std::printf("SPR_M257_ERROR resource=%s reason=gap-out-of-range\n",name);return;
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
    std::vector<Run> runs;
    for(size_t i=0;i<best.compared;++i){
        const bool eq=bitAt(a,startA+best.startA+i)==bitAt(b,startB+best.startB+i);
        if(!runs.empty()&&runs.back().equal==eq)++runs.back().length;
        else runs.push_back({i,1,eq});
    }
    Run longest{0,0,true};
    for(const Run& r:runs)
        if(r.equal&&r.length>longest.length)longest=r;
    std::printf("SPR_M257_PAIR resource=%s record=%zu imageA=%zu imageB=%zu bestShift=%d compared=%zu exactStart=%zu exactLength=%zu absoluteA=%zu absoluteB=%zu\n",
        name,record,imageA,imageB,best.shift,best.compared,longest.start,longest.length,
        startA+best.startA+longest.start,startB+best.startB+longest.start);
    if(longest.length<32){
        std::printf("SPR_M257_ERROR resource=%s reason=no-exact-run-32\n",name);
        return;
    }
    for(const size_t width:{size_t(32),size_t(64)}){
        for(int side=0;side<2;++side){
            const size_t available=side==0?longest.start:
                best.compared-longest.start-longest.length;
            const size_t take=std::min(width,available);
            const size_t offset=side==0?longest.start-take:
                longest.start+longest.length;
            std::string patternA,patternB,comparison;
            size_t mismatches=0;
            for(size_t j=0;j<take;++j){
                const bool ba=bitAt(a,startA+best.startA+offset+j);
                const bool bb=bitAt(b,startB+best.startB+offset+j);
                patternA+=ba?'1':'0';
                patternB+=bb?'1':'0';
                comparison+=ba==bb?'=':'X';
                mismatches+=ba!=bb;
            }
            std::printf("SPR_M257_FLANK resource=%s record=%zu imageA=%zu imageB=%zu shift=%d side=%s requested=%zu available=%zu compared=%zu mismatch=%zu patternA=%s patternB=%s comparison=%s\n",
                name,record,imageA,imageB,best.shift,side==0?"left":"right",
                width,available,take,mismatches,patternA.c_str(),patternB.c_str(),comparison.c_str());
        }
    }
    for(int side=0;side<2;++side){
        const size_t boundary=side==0?longest.start:longest.start+longest.length;
        const size_t begin=boundary>32?boundary-32:0;
        const size_t end=std::min(best.compared,boundary+32);
        std::string patternA,patternB;
        for(size_t j=begin;j<end;++j){
            patternA+=bitAt(a,startA+best.startA+j)?'1':'0';
            patternB+=bitAt(b,startB+best.startB+j)?'1':'0';
        }
        std::printf("SPR_M257_TRANSITION resource=%s record=%zu imageA=%zu imageB=%zu shift=%d side=%s offset=%zu leftBits=%zu rightBits=%zu patternA=%s patternB=%s\n",
            name,record,imageA,imageB,best.shift,side==0?"start":"end",
            boundary,boundary-begin,end-boundary,patternA.c_str(),patternB.c_str());
    }
}
}
int stafuSprM257RunFlanks(const char* installed,const char* cd,const char* optionalName){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"spr-m257-run-flanks: %s\n",error.c_str());return 1;
    }
    const std::string name=optionalName?optionalName:"BRDGPICA.SPR";
    if(name!="BRDGPICA.SPR"&&name!="BRDGDATA.SPR"){
        std::fprintf(stderr,"spr-m257-run-flanks: supported resources: BRDGPICA.SPR, BRDGDATA.SPR\n");return 2;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"spr-m257-run-flanks: %s\n",error.c_str());return 1;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){
        std::fprintf(stderr,"spr-m257-run-flanks: %s\n",error.c_str());return 1;
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
            std::printf("SPR_M257_ERROR resource=%s record=%zu imageA=%zu imageB=%zu reason=missing-direct-payload\n",
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
    std::printf("SPR_M257_SUMMARY resource=%s inspected=%zu expected=%d\n",
        name.c_str(),count,name=="BRDGPICA.SPR"?4:2);
    std::puts("SPR_M257_NOTE M249 gap coordinates, MSB-first. Best +/-32-bit alignment uses M256 ranking. Flanks clipped to gap bounds; exact-run boundaries are observational, not codec fields.");
    return count==(name=="BRDGPICA.SPR"?4u:2u)?0:1;
}
