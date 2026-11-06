// SPDX-License-Identifier: GPL-3.0-or-later
// M266: count short exact matches and competing positions.
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
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"SPR_M266_ERROR %s\n",error.c_str());return false;}
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){std::fprintf(stderr,"SPR_M266_ERROR %s\n",error.c_str());return false;}
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){std::fprintf(stderr,"SPR_M266_ERROR %s\n",error.c_str());return false;}
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
    struct Interval {size_t gap,a0,a1,b0,b1;};
    const std::vector<Interval> intervals=name=="BRDGPICA.SPR"?
        std::vector<Interval>{{3,9908,9920,9967,9989}}:
        std::vector<Interval>{{2,11181,11255,11127,11204},{2,11569,11645,11518,11597}};
    for(size_t ix=0;ix<intervals.size();++ix){
        const auto& v=intervals[ix];const auto& g=gaps[v.gap-1];
        const Sample *sa=nullptr,*sb=nullptr;
        for(const auto& sample:samples){
            if(sample.record==g[0]&&sample.image==g[1])sa=&sample;
            if(sample.record==g[0]&&sample.image==g[2])sb=&sample;
        }
        if(!sa||!sb||v.a0>v.a1||v.b0>v.b1||
           v.a1>sa->size*8||v.b1>sb->size*8){
            std::printf("SPR_M266_ERROR resource=%s interval=%zu invalid-range\n",name.c_str(),ix+1);return false;
        }
        const uint8_t* a=bytes.data()+sa->offset;
        const uint8_t* b=bytes.data()+sb->offset;
        const size_t na=v.a1-v.a0,nb=v.b1-v.b0;
        std::printf("SPR_M266_INTERVAL resource=%s gap=%zu interval=%zu a=%zu bitsA=%zu b=%zu bitsB=%zu deltaBits=%lld\n",
            name.c_str(),v.gap,ix+1,v.a0,na,v.b0,nb,
            static_cast<long long>(nb)-static_cast<long long>(na));
        struct Hit {size_t a,b,len;};
        std::vector<Hit> hits;
        const size_t threshold=name=="BRDGPICA.SPR"?size_t(4):size_t(13);
        for(size_t ia=0;ia<na;++ia)
            for(size_t ib=0;ib<nb;++ib){
                if(bitAt(a,v.a0+ia)!=bitAt(b,v.b0+ib))continue;
                if(ia&&ib&&bitAt(a,v.a0+ia-1)==bitAt(b,v.b0+ib-1))continue;
                size_t len=0;
                while(ia+len<na&&ib+len<nb&&
                      bitAt(a,v.a0+ia+len)==bitAt(b,v.b0+ib+len))++len;
                if(len>=threshold)hits.push_back({v.a0+ia,v.b0+ib,len});
            }
        std::sort(hits.begin(),hits.end(),[](const Hit& x,const Hit& y){
            if(x.len!=y.len)return x.len>y.len;
            if(x.a!=y.a)return x.a<y.a;
            return x.b<y.b;
        });
        std::printf("SPR_M266_CANDIDATES resource=%s interval=%zu threshold=%zu total=%zu reported=%zu\n",
            name.c_str(),ix+1,threshold,hits.size(),std::min(size_t(20),hits.size()));
        for(size_t hi=0;hi<std::min(size_t(20),hits.size());++hi){
            const auto& hit=hits[hi];
            size_t countA=0,countB=0,gapA=0,gapB=0,alternatives=0;
            for(size_t pos=0;pos+hit.len<=sa->size*8;++pos){
                bool equal=true;
                for(size_t bit=0;bit<hit.len;++bit)
                    if(bitAt(a,hit.a+bit)!=bitAt(a,pos+bit)){equal=false;break;}
                if(equal){++countA;if(pos>=v.a0&&pos+hit.len<=v.a1)++gapA;}
            }
            for(size_t pos=0;pos+hit.len<=sb->size*8;++pos){
                bool equal=true;
                for(size_t bit=0;bit<hit.len;++bit)
                    if(bitAt(a,hit.a+bit)!=bitAt(b,pos+bit)){equal=false;break;}
                if(equal){++countB;if(pos>=v.b0&&pos+hit.len<=v.b1)++gapB;}
            }
            // Alternative matching pairs inside the two local intervals.
            for(size_t ia=v.a0;ia+hit.len<=v.a1;++ia){
                bool equalA=true;
                for(size_t bit=0;bit<hit.len;++bit)
                    if(bitAt(a,hit.a+bit)!=bitAt(a,ia+bit)){equalA=false;break;}
                if(!equalA)continue;
                for(size_t ib=v.b0;ib+hit.len<=v.b1;++ib){
                    bool equalB=true;
                    for(size_t bit=0;bit<hit.len;++bit)
                        if(bitAt(a,hit.a+bit)!=bitAt(b,ib+bit)){equalB=false;break;}
                    if(equalB&&!(ia==hit.a&&ib==hit.b))++alternatives;
                }
            }
            std::printf("SPR_M266_HIT resource=%s interval=%zu rank=%zu a=%zu b=%zu length=%zu displacement=%lld fullA=%zu fullB=%zu intervalA=%zu intervalB=%zu alternativePairs=%zu uniqueBoth=%d\n",
                name.c_str(),ix+1,hi+1,hit.a,hit.b,hit.len,
                static_cast<long long>(hit.b)-static_cast<long long>(hit.a),
                countA,countB,gapA,gapB,alternatives,countA==1&&countB==1?1:0);
        }
    }
    return true;
}
}
int stafuSprM266ShortMatchSignificance(const char* installed,const char* cd,const char*){
    const std::vector<std::vector<size_t>> picard={
        {5,1,2,3204,4489,3234,4526},{5,1,2,4967,5093,5004,5120},
        {5,1,2,9730,10499,9792,10603},{5,3,4,6342,6591,6464,6713}};
    const std::vector<std::vector<size_t>> data={
        {0,0,1,9973,10390,9966,10356},{0,0,1,10781,11976,10747,11952}};
    if(!load(installed,cd,"BRDGPICA.SPR",picard)||
       !load(installed,cd,"BRDGDATA.SPR",data))return 1;
    std::puts("SPR_M266_SUMMARY intervals=3 resources=2");
    std::puts("SPR_M266_NOTE MSB-first maximal diagonal runs; Picard threshold=4, Data threshold=13. Counts include overlapping occurrences; top 20 hits per interval. Uniqueness is descriptive, not a probability or codec operation.");
    return 0;
}
