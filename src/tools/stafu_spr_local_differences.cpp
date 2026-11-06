// SPDX-License-Identifier: GPL-3.0-or-later
// M250: read-only localized bit-difference mapping for a verified SPR gap.
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
Alignment score(const uint8_t* a,size_t na,const uint8_t* b,size_t nb,int shift){
    const size_t aa=shift<0?static_cast<size_t>(-shift):0;
    const size_t bb=shift>0?static_cast<size_t>(shift):0;
    Alignment v{shift,aa,bb,0,0};
    if(aa>=na||bb>=nb)return v;
    v.compared=std::min(na-aa,nb-bb);
    for(size_t i=0;i<v.compared;++i)v.equal+=bitAt(a,aa+i)==bitAt(b,bb+i);
    return v;
}
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
        std::printf("SPR_LOCAL_ERROR resource=%s reason=gap-out-of-range\n",name);return;
    }
    const size_t lenA=endA-startA,lenB=endB-startB;
    const uint8_t* pa=a;const uint8_t* pb=b;
    const Alignment direct=score(pa+startA/8,0,pb+startB/8,0,0);
    (void)direct; // Bit positions may not be byte aligned; use absolute offsets below.
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
    const Alignment zero=evaluate(0);
    Alignment best=zero;
    for(int shift=-32;shift<=32;++shift){
        Alignment v=evaluate(shift);
        if(better(v,best))best=v;
    }
    std::printf("SPR_LOCAL_GAP resource=%s record=%zu imageA=%zu imageB=%zu startA=%zu endA=%zu startB=%zu endB=%zu bitsA=%zu bitsB=%zu directEqual=%zu directCompared=%zu bestShift=%d bestEqual=%zu bestCompared=%zu\n",
        name,record,imageA,imageB,startA,endA,startB,endB,lenA,lenB,
        zero.equal,zero.compared,best.shift,best.equal,best.compared);
    auto report=[&](const char* mode,const Alignment& alignment){
        std::vector<Run> runs;
        for(size_t i=0;i<alignment.compared;++i){
            bool equal=bitAt(a,startA+alignment.startA+i)==bitAt(b,startB+alignment.startB+i);
            if(!runs.empty()&&runs.back().equal==equal)++runs.back().length;
            else runs.push_back({i,1,equal});
        }
        size_t equalRuns=0,mismatchRuns=0,longestEqual=0,longestMismatch=0;
        std::vector<Run> mismatches;
        for(const auto& r:runs){
            if(r.equal){++equalRuns;longestEqual=std::max(longestEqual,r.length);}
            else {++mismatchRuns;longestMismatch=std::max(longestMismatch,r.length);mismatches.push_back(r);}
        }
        std::sort(mismatches.begin(),mismatches.end(),[](const Run& x,const Run& y){
            if(x.length!=y.length)return x.length>y.length;return x.start<y.start;
        });
        const size_t mismatchBits=alignment.compared-alignment.equal;
        size_t top1=0,top3=0,top5=0;
        for(size_t i=0;i<mismatches.size();++i){
            if(i<1)top1+=mismatches[i].length;
            if(i<3)top3+=mismatches[i].length;
            if(i<5)top5+=mismatches[i].length;
        }
        std::printf("SPR_LOCAL_MAP resource=%s mode=%s shift=%d compared=%zu equal=%zu mismatches=%zu equalRuns=%zu mismatchRuns=%zu longestEqual=%zu longestMismatch=%zu top1MismatchBits=%zu top3MismatchBits=%zu top5MismatchBits=%zu\n",
            name,mode,alignment.shift,alignment.compared,alignment.equal,mismatchBits,
            equalRuns,mismatchRuns,longestEqual,longestMismatch,top1,top3,top5);
        // Run coordinates are relative to the original gap, not the payload.
        size_t printed=0;
        for(const auto& r:runs){
            if(printed>=48)break;
            if(r.length<8)continue;
            std::printf("SPR_LOCAL_RUN resource=%s mode=%s type=%s offsetA=%zu offsetB=%zu length=%zu\n",
                name,mode,r.equal?"equal":"mismatch",alignment.startA+r.start,
                alignment.startB+r.start,r.length);
            ++printed;
        }
        std::printf("SPR_LOCAL_RUN_NOTE resource=%s mode=%s reported=%zu minLength=8 maxRuns=48\n",
            name,mode,printed);
    };
    report("direct",zero);
    if(best.shift!=0)report("best-shift",best);
}
}
int stafuSprLocalDifferences(const char* installed,const char* cd,const char* optionalName){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"spr-local-differences: %s\n",error.c_str());return 1;
    }
    const std::string name=optionalName?optionalName:"BRDGPICA.SPR";
    if(name!="BRDGPICA.SPR"&&name!="BRDGDATA.SPR"){
        std::fprintf(stderr,"spr-local-differences: supported resources: BRDGPICA.SPR, BRDGDATA.SPR\n");return 2;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"spr-local-differences: %s\n",error.c_str());return 1;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){
        std::fprintf(stderr,"spr-local-differences: %s\n",error.c_str());return 1;
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
            std::printf("SPR_LOCAL_ERROR resource=%s record=%zu imageA=%zu imageB=%zu reason=missing-direct-payload\n",
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
    std::printf("SPR_LOCAL_SUMMARY resource=%s inspected=%zu expected=%d\n",
        name.c_str(),count,name=="BRDGPICA.SPR"?4:2);
    std::puts("SPR_LOCAL_NOTE Coordinates are verified M249 gap coordinates. Direct and best-shift comparisons use full gaps and MSB-first bits. Runs shorter than 8 bits are omitted from detail, not totals. No codec interpretation is implied.");
    return count==(name=="BRDGPICA.SPR"?4u:2u)?0:1;
}
