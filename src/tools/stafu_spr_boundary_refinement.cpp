// SPDX-License-Identifier: GPL-3.0-or-later
// M265: extend validated anchors and inspect intervening bit intervals.
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
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"SPR_M265_ERROR %s\n",error.c_str());return false;}
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){std::fprintf(stderr,"SPR_M265_ERROR %s\n",error.c_str());return false;}
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){std::fprintf(stderr,"SPR_M265_ERROR %s\n",error.c_str());return false;}
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
        std::vector<Target>{{3,9874,9933,34},{3,9920,9989,388}}:
        std::vector<Target>{{2,10873,10819,308},{2,11255,11204,314},{2,11645,11597,283}};
    struct Extended {size_t gap,a,b,len;};
    std::vector<Extended> extended;
    for(const auto& t:targets){
        const auto& g=gaps[t.gap-1];
        const Sample *sa=nullptr,*sb=nullptr;
        for(const auto& sample:samples){
            if(sample.record==g[0]&&sample.image==g[1])sa=&sample;
            if(sample.record==g[0]&&sample.image==g[2])sb=&sample;
        }
        if(!sa||!sb||t.a<g[3]||t.b<g[5]||
           t.a+t.length>g[4]||t.b+t.length>g[6]){
            std::printf("SPR_M265_ERROR resource=%s gap=%zu invalid-target\n",name.c_str(),t.gap);return false;
        }
        const uint8_t* a=bytes.data()+sa->offset;
        const uint8_t* b=bytes.data()+sb->offset;
        for(size_t k=0;k<t.length;++k)
            if(bitAt(a,t.a+k)!=bitAt(b,t.b+k)){
                std::printf("SPR_M265_ERROR resource=%s gap=%zu anchor-not-exact\n",name.c_str(),t.gap);return false;
            }
        size_t left=0,right=0;
        while(t.a>g[3]+left&&t.b>g[5]+left&&
              bitAt(a,t.a-left-1)==bitAt(b,t.b-left-1))++left;
        while(t.a+t.length+right<g[4]&&t.b+t.length+right<g[6]&&
              bitAt(a,t.a+t.length+right)==bitAt(b,t.b+t.length+right))++right;
        extended.push_back({t.gap,t.a-left,t.b-left,t.length+left+right});
        std::printf("SPR_M265_EXTEND resource=%s gap=%zu seedA=%zu seedB=%zu seedLength=%zu left=%zu right=%zu a=%zu b=%zu length=%zu displacement=%lld\n",
            name.c_str(),t.gap,t.a,t.b,t.length,left,right,t.a-left,t.b-left,
            t.length+left+right,static_cast<long long>(t.b)-static_cast<long long>(t.a));
    }
    for(size_t k=1;k<extended.size();++k){
        const auto& prev=extended[k-1];const auto& next=extended[k];
        if(prev.gap!=next.gap)continue;
        const auto& g=gaps[prev.gap-1];
        const Sample *sa=nullptr,*sb=nullptr;
        for(const auto& sample:samples){
            if(sample.record==g[0]&&sample.image==g[1])sa=&sample;
            if(sample.record==g[0]&&sample.image==g[2])sb=&sample;
        }
        const size_t startA=prev.a+prev.len,startB=prev.b+prev.len;
        if(startA>next.a||startB>next.b){
            std::printf("SPR_M265_ERROR resource=%s gap=%zu overlapping-extended-anchors\n",name.c_str(),prev.gap);return false;
        }
        const size_t na=next.a-startA,nb=next.b-startB;
        const uint8_t* a=bytes.data()+sa->offset;
        const uint8_t* b=bytes.data()+sb->offset;
        std::printf("SPR_M265_INTERVAL resource=%s gap=%zu from=%zu to=%zu a=%zu b=%zu bitsA=%zu bitsB=%zu deltaBits=%lld\n",
            name.c_str(),prev.gap,k,k+1,startA,startB,na,nb,
            static_cast<long long>(nb)-static_cast<long long>(na));
        // Enumerate all maximal exact diagonal runs, with no edit penalties.
        // Only runs at least 12 bits long are printed, bounded to the top 8.
        struct Hit {size_t a,b,len;};
        std::vector<Hit> hits;
        for(size_t ia=0;ia<na;++ia){
            for(size_t ib=0;ib<nb;++ib){
                if(bitAt(a,startA+ia)!=bitAt(b,startB+ib))continue;
                if(ia&&ib&&bitAt(a,startA+ia-1)==bitAt(b,startB+ib-1))continue;
                size_t length=0;
                while(ia+length<na&&ib+length<nb&&
                      bitAt(a,startA+ia+length)==bitAt(b,startB+ib+length))++length;
                if(length>=12)hits.push_back({startA+ia,startB+ib,length});
            }
        }
        std::sort(hits.begin(),hits.end(),[](const Hit& x,const Hit& y){
            if(x.len!=y.len)return x.len>y.len;
            if(x.a!=y.a)return x.a<y.a;
            return x.b<y.b;
        });
        std::printf("SPR_M265_CANDIDATES resource=%s gap=%zu from=%zu to=%zu minRun=12 total=%zu reported=%zu\n",
            name.c_str(),prev.gap,k,k+1,hits.size(),std::min(size_t(8),hits.size()));
        for(size_t hi=0;hi<std::min(size_t(8),hits.size());++hi)
            std::printf("SPR_M265_HIT resource=%s gap=%zu from=%zu to=%zu rank=%zu a=%zu b=%zu length=%zu displacement=%lld\n",
                name.c_str(),prev.gap,k,k+1,hi+1,hits[hi].a,hits[hi].b,hits[hi].len,
                static_cast<long long>(hits[hi].b)-static_cast<long long>(hits[hi].a));
    }
    return true;
}
}
int stafuSprM265BoundaryRefinement(const char* installed,const char* cd,const char*){
    const std::vector<std::vector<size_t>> picard={
        {5,1,2,3204,4489,3234,4526},{5,1,2,4967,5093,5004,5120},
        {5,1,2,9730,10499,9792,10603},{5,3,4,6342,6591,6464,6713}};
    const std::vector<std::vector<size_t>> data={
        {0,0,1,9973,10390,9966,10356},{0,0,1,10781,11976,10747,11952}};
    if(!load(installed,cd,"BRDGPICA.SPR",picard)||
       !load(installed,cd,"BRDGDATA.SPR",data))return 1;
    std::puts("SPR_M265_SUMMARY seeds=5 resources=2");
    std::puts("SPR_M265_NOTE Exact MSB-first extension within original M251 gaps, no edit costs. Interior candidates are maximal diagonal runs >=12 bits, top 8 by length; not codec operations.");
    return 0;
}
