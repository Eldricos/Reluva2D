// SPDX-License-Identifier: GPL-3.0-or-later
// M264: validate exact bit anchors without alignment scoring.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <utility>

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

struct Boundary {
    std::string resource;
    size_t gap,record,imageA,imageB;
    int shift;
    size_t position,runLength;
    bool isStart;
    std::string a,b;
};
bool load(const char* installed,const char* cd,const std::string& name,
          const std::vector<std::vector<size_t>>& gaps){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"SPR_M264_ERROR %s\n",error.c_str());return false;}
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){std::fprintf(stderr,"SPR_M264_ERROR %s\n",error.c_str());return false;}
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){std::fprintf(stderr,"SPR_M264_ERROR %s\n",error.c_str());return false;}
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
    struct Target {size_t gap,a,b,length;};
    const std::vector<Target> targets=name=="BRDGPICA.SPR"?
        std::vector<Target>{{3,9920,9989,388}}:
        std::vector<Target>{{2,10873,10819,308},{2,11255,11204,314},{2,11645,11597,283}};
    for(const auto& target:targets){
        const auto& g=gaps[target.gap-1];
        const Sample *sa=nullptr,*sb=nullptr;
        for(const auto& sample:samples){
            if(sample.record==g[0]&&sample.image==g[1])sa=&sample;
            if(sample.record==g[0]&&sample.image==g[2])sb=&sample;
        }
        if(!sa||!sb||target.a+target.length>sa->size*8||
           target.b+target.length>sb->size*8){
            std::printf("SPR_M264_ERROR resource=%s gap=%zu invalid-target\n",name.c_str(),target.gap);
            return false;
        }
        const uint8_t* a=bytes.data()+sa->offset;
        const uint8_t* b=bytes.data()+sb->offset;
        size_t verified=0;
        for(size_t k=0;k<target.length;++k)
            verified+=bitAt(a,target.a+k)==bitAt(b,target.b+k);
        std::printf("SPR_M264_ANCHOR resource=%s gap=%zu a=%zu b=%zu length=%zu verified=%zu exact=%d\n",
            name.c_str(),target.gap,target.a,target.b,target.length,verified,
            verified==target.length?1:0);
        if(verified!=target.length)return false;
        // Deterministic windows: start, quarter, midpoint, three-quarters, end.
        for(size_t width:{size_t(8),size_t(12),size_t(16),size_t(24),size_t(32)}){
            if(width>target.length)continue;
            const size_t extent=target.length-width;
            const size_t positions[]={0,extent/4,extent/2,(extent*3)/4,extent};
            for(size_t pi=0;pi<5;++pi){
                const size_t local=positions[pi];
                const size_t posA=target.a+local,posB=target.b+local;
                auto equalWindow=[&](const uint8_t* left,size_t leftPos,
                                     const uint8_t* right,size_t rightPos){
                    for(size_t bit=0;bit<width;++bit)
                        if(bitAt(left,leftPos+bit)!=bitAt(right,rightPos+bit))return false;
                    return true;
                };
                size_t countA=0,countB=0,countGapA=0,countGapB=0;
                // Count every occurrence in the two complete payloads, not
                // only the selected gap; overlapping occurrences are included.
                for(size_t at=0;at+width<=sa->size*8;++at)
                    if(equalWindow(a,posA,a,at))++countA;
                for(size_t at=0;at+width<=sb->size*8;++at)
                    if(equalWindow(a,posA,b,at))++countB;
                for(size_t at=g[3];at+width<=g[4];++at)
                    if(equalWindow(a,posA,a,at))++countGapA;
                for(size_t at=g[5];at+width<=g[6];++at)
                    if(equalWindow(a,posA,b,at))++countGapB;
                std::printf("SPR_M264_WINDOW resource=%s gap=%zu anchorA=%zu anchorB=%zu width=%zu sample=%zu a=%zu b=%zu fullA=%zu fullB=%zu gapA=%zu gapB=%zu uniqueBoth=%d\n",
                    name.c_str(),target.gap,target.a,target.b,width,pi+1,posA,posB,
                    countA,countB,countGapA,countGapB,countA==1&&countB==1?1:0);
            }
        }
    }
    return true;
}
}
int stafuSprM264StableAnchors(const char* installed,const char* cd,const char*){
    const std::vector<std::vector<size_t>> picard={
        {5,1,2,3204,4489,3234,4526},{5,1,2,4967,5093,5004,5120},
        {5,1,2,9730,10499,9792,10603},{5,3,4,6342,6591,6464,6713}};
    const std::vector<std::vector<size_t>> data={
        {0,0,1,9973,10390,9966,10356},{0,0,1,10781,11976,10747,11952}};
    if(!load(installed,cd,"BRDGPICA.SPR",picard)||
       !load(installed,cd,"BRDGDATA.SPR",data))return 1;
    std::puts("SPR_M264_SUMMARY anchors=4 widths=5 samplesPerWidth=5");
    std::puts("SPR_M264_NOTE Exact MSB-first windows, complete-payload and gap frequencies; overlapping hits included. Anchor coordinates from M260. No statistical chance probability or codec semantics inferred.");
    return 0;
}
