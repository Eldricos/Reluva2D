// SPDX-License-Identifier: GPL-3.0-or-later
// M251: read-only exact-run boundaries and local displacement diagnostics.
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
        std::printf("SPR_BOUNDARY_ERROR resource=%s reason=gap-out-of-range\n",name);return;
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
    const Alignment zero=evaluate(0);
    Alignment best=zero;
    for(int shift=-32;shift<=32;++shift){
        Alignment v=evaluate(shift);
        if(better(v,best))best=v;
    }
    std::printf("SPR_BOUNDARY_GAP resource=%s record=%zu imageA=%zu imageB=%zu startA=%zu endA=%zu startB=%zu endB=%zu bitsA=%zu bitsB=%zu directEqual=%zu directCompared=%zu bestShift=%d bestEqual=%zu bestCompared=%zu\n",
        name,record,imageA,imageB,startA,endA,startB,endB,lenA,lenB,
        zero.equal,zero.compared,best.shift,best.equal,best.compared);
    auto report=[&](const char* mode,const Alignment& alignment){
        std::vector<Run> runs;
        for(size_t i=0;i<alignment.compared;++i){
            const bool eq=bitAt(a,startA+alignment.startA+i)==bitAt(b,startB+alignment.startB+i);
            if(!runs.empty()&&runs.back().equal==eq)++runs.back().length;
            else runs.push_back({i,1,eq});
        }
        std::vector<Run> longRuns;
        for(const auto& r:runs)if(r.equal&&r.length>=32)longRuns.push_back(r);
        std::sort(longRuns.begin(),longRuns.end(),[](const Run& x,const Run& y){
            if(x.length!=y.length)return x.length>y.length;
            return x.start<y.start;
        });
        const size_t count=std::min(size_t(12),longRuns.size());
        std::printf("SPR_BOUNDARY_MAP resource=%s record=%zu imageA=%zu imageB=%zu mode=%s shift=%d compared=%zu equal=%zu qualifyingRuns=%zu reported=%zu minRun=32\n",
            name,record,imageA,imageB,mode,alignment.shift,alignment.compared,
            alignment.equal,longRuns.size(),count);
        for(size_t k=0;k<count;++k){
            const Run& r=longRuns[k];
            const size_t aa=alignment.startA+r.start,bb=alignment.startB+r.start;
            const size_t left=std::min(size_t(16),r.start);
            const size_t right=std::min(size_t(16),alignment.compared-r.start-r.length);
            size_t leftEqual=0,rightEqual=0;
            for(size_t j=0;j<left;++j)
                leftEqual+=bitAt(a,startA+aa-left+j)==bitAt(b,startB+bb-left+j);
            for(size_t j=0;j<right;++j)
                rightEqual+=bitAt(a,startA+aa+r.length+j)==bitAt(b,startB+bb+r.length+j);
            const bool leftMismatch=r.start>0&&
                bitAt(a,startA+aa-1)!=bitAt(b,startB+bb-1);
            const bool rightMismatch=r.start+r.length<alignment.compared&&
                bitAt(a,startA+aa+r.length)!=bitAt(b,startB+bb+r.length);
            std::printf("SPR_BOUNDARY_RUN resource=%s mode=%s rank=%zu offsetA=%zu offsetB=%zu absoluteA=%zu absoluteB=%zu length=%zu leftCompared=%zu leftEqual=%zu rightCompared=%zu rightEqual=%zu leftImmediateMismatch=%d rightImmediateMismatch=%d\n",
                name,mode,k+1,aa,bb,startA+aa,startB+bb,r.length,
                left,leftEqual,right,rightEqual,leftMismatch?1:0,rightMismatch?1:0);
        }
        // Re-sort selected long runs by position to inspect intervening regions.
        longRuns.resize(count);
        std::sort(longRuns.begin(),longRuns.end(),[](const Run& x,const Run& y){return x.start<y.start;});
        for(size_t k=1;k<longRuns.size();++k){
            const Run& prev=longRuns[k-1];const Run& next=longRuns[k];
            const size_t begin=prev.start+prev.length;
            const size_t end=next.start;
            if(end<begin)continue;
            size_t equal=0;
            for(size_t j=begin;j<end;++j)
                equal+=bitAt(a,startA+alignment.startA+j)==
                       bitAt(b,startB+alignment.startB+j);
            const size_t width=end-begin;
            // A short local offset probe is only a candidate, not an edit script.
            size_t bestEqual=0,bestCompared=0;int bestDelta=0;
            for(int delta=-16;delta<=16;++delta){
                const size_t a0=delta<0?static_cast<size_t>(-delta):0;
                const size_t b0=delta>0?static_cast<size_t>(delta):0;
                if(a0>=width||b0>=width)continue;
                const size_t n=std::min(width-a0,width-b0);
                if(n<8)continue;
                size_t matches=0;
                for(size_t j=0;j<n;++j)
                    matches+=bitAt(a,startA+alignment.startA+begin+a0+j)==
                             bitAt(b,startB+alignment.startB+begin+b0+j);
                if(!bestCompared||matches*bestCompared>bestEqual*n||
                   (matches*bestCompared==bestEqual*n&&n>bestCompared)){
                    bestEqual=matches;bestCompared=n;bestDelta=delta;
                }
            }
            std::printf("SPR_BOUNDARY_INTERVAL resource=%s mode=%s from=%zu to=%zu startA=%zu startB=%zu width=%zu directEqual=%zu localBestDelta=%d localBestEqual=%zu localBestCompared=%zu\n",
                name,mode,k,k+1,alignment.startA+begin,alignment.startB+begin,
                width,equal,bestDelta,bestEqual,bestCompared);
        }
    };
    report("direct",zero);
    if(best.shift!=0)report("best-shift",best);
}
}
int stafuSprRunBoundaries(const char* installed,const char* cd,const char* optionalName){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"spr-run-boundaries: %s\n",error.c_str());return 1;
    }
    const std::string name=optionalName?optionalName:"BRDGPICA.SPR";
    if(name!="BRDGPICA.SPR"&&name!="BRDGDATA.SPR"){
        std::fprintf(stderr,"spr-run-boundaries: supported resources: BRDGPICA.SPR, BRDGDATA.SPR\n");return 2;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"spr-run-boundaries: %s\n",error.c_str());return 1;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){
        std::fprintf(stderr,"spr-run-boundaries: %s\n",error.c_str());return 1;
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
            std::printf("SPR_BOUNDARY_ERROR resource=%s record=%zu imageA=%zu imageB=%zu reason=missing-direct-payload\n",
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
    std::printf("SPR_BOUNDARY_SUMMARY resource=%s inspected=%zu expected=%d\n",
        name.c_str(),count,name=="BRDGPICA.SPR"?4:2);
    std::puts("SPR_BOUNDARY_NOTE M249 gap coordinates, MSB-first. Up to 12 exact runs >=32 bits per alignment, with 16-bit boundary contexts. Local interval shifts +/-16 are descriptive; insertions/deletions and codec structure are not inferred.");
    return count==(name=="BRDGPICA.SPR"?4u:2u)?0:1;
}
