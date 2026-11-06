// SPDX-License-Identifier: GPL-3.0-or-later
// M269: enumerate optimal edit-path counts and forced edges.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <utility>
#include <boost/multiprecision/cpp_int.hpp>

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
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"SPR_M269_ERROR %s\n",error.c_str());return false;}
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){std::fprintf(stderr,"SPR_M269_ERROR %s\n",error.c_str());return false;}
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){std::fprintf(stderr,"SPR_M269_ERROR %s\n",error.c_str());return false;}
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
            std::printf("SPR_M269_ERROR resource=%s interval=%zu invalid-range\n",name.c_str(),ix+1);return false;
        }
        const uint8_t* a=bytes.data()+sa->offset;
        const uint8_t* b=bytes.data()+sb->offset;
        const size_t na=v.a1-v.a0,nb=v.b1-v.b0;
        std::printf("SPR_M269_INTERVAL resource=%s gap=%zu interval=%zu a=%zu bitsA=%zu b=%zu bitsB=%zu deltaBits=%lld\n",
            name.c_str(),v.gap,ix+1,v.a0,na,v.b0,nb,
            static_cast<long long>(nb)-static_cast<long long>(na));
        struct Hit {size_t a,b,len;};
        const std::vector<Hit> hits=name=="BRDGPICA.SPR"?
            std::vector<Hit>{{9908,9977,11}}:
            ix==0?std::vector<Hit>{{11221,11163,13}}:
                  std::vector<Hit>{{11573,11525,13},{11619,11574,13}};
        const size_t scenarios=size_t(1)<<hits.size();
        for(size_t mask=0;mask<scenarios;++mask){
            size_t cursorA=v.a0,cursorB=v.b0,matched=0,segment=0;
            bool valid=true;
            for(size_t h=0;h<=hits.size();++h){
                if(h<hits.size()&&!(mask&(size_t(1)<<h)))continue;
                const size_t nextA=h==hits.size()?v.a1:hits[h].a;
                const size_t nextB=h==hits.size()?v.b1:hits[h].b;
                if(nextA<cursorA||nextB<cursorB){valid=false;break;}
                std::printf("SPR_M269_SEGMENT resource=%s interval=%zu mask=%zu segment=%zu a=%zu b=%zu bitsA=%zu bitsB=%zu delta=%lld\n",
                    name.c_str(),ix+1,mask,++segment,cursorA,cursorB,nextA-cursorA,nextB-cursorB,
                    static_cast<long long>(nextB-cursorB)-static_cast<long long>(nextA-cursorA));
                std::string bitsA,bitsB;
                for(size_t k=cursorA;k<nextA;++k)bitsA.push_back(bitAt(a,k)?'1':'0');
                for(size_t k=cursorB;k<nextB;++k)bitsB.push_back(bitAt(b,k)?'1':'0');
                std::printf("SPR_M269_BITS resource=%s interval=%zu mask=%zu segment=%zu bitsA=%s bitsB=%s\n",
                    name.c_str(),ix+1,mask,segment,bitsA.empty()?"-":bitsA.c_str(),bitsB.empty()?"-":bitsB.c_str());
                using boost::multiprecision::cpp_int;
                const size_t m=bitsA.size(),n=bitsB.size(),width=n+1;
                const auto at=[width](size_t i,size_t j){return i*width+j;};
                std::vector<int> forward((m+1)*width,0),backward((m+1)*width,0);
                std::vector<cpp_int> waysF((m+1)*width),waysB((m+1)*width);
                waysF[0]=1;
                for(size_t i=0;i<=m;++i)for(size_t j=0;j<=n;++j){
                    if(!i&&!j)continue;
                    int best=static_cast<int>(m+n+1);
                    if(i)best=std::min(best,forward[at(i-1,j)]+1);
                    if(j)best=std::min(best,forward[at(i,j-1)]+1);
                    if(i&&j)best=std::min(best,forward[at(i-1,j-1)]+(bitsA[i-1]!=bitsB[j-1]));
                    forward[at(i,j)]=best;
                    if(i&&forward[at(i-1,j)]+1==best)waysF[at(i,j)]+=waysF[at(i-1,j)];
                    if(j&&forward[at(i,j-1)]+1==best)waysF[at(i,j)]+=waysF[at(i,j-1)];
                    if(i&&j&&forward[at(i-1,j-1)]+(bitsA[i-1]!=bitsB[j-1])==best)
                        waysF[at(i,j)]+=waysF[at(i-1,j-1)];
                }
                waysB[at(m,n)]=1;
                for(size_t ii=m+1;ii-->0;)for(size_t jj=n+1;jj-->0;){
                    const size_t i=ii,j=jj;
                    if(i==m&&j==n)continue;
                    int best=static_cast<int>(m+n+1);
                    if(i<m)best=std::min(best,backward[at(i+1,j)]+1);
                    if(j<n)best=std::min(best,backward[at(i,j+1)]+1);
                    if(i<m&&j<n)best=std::min(best,backward[at(i+1,j+1)]+(bitsA[i]!=bitsB[j]));
                    backward[at(i,j)]=best;
                    if(i<m&&backward[at(i+1,j)]+1==best)waysB[at(i,j)]+=waysB[at(i+1,j)];
                    if(j<n&&backward[at(i,j+1)]+1==best)waysB[at(i,j)]+=waysB[at(i,j+1)];
                    if(i<m&&j<n&&backward[at(i+1,j+1)]+(bitsA[i]!=bitsB[j]))
                        if(backward[at(i+1,j+1)]+(bitsA[i]!=bitsB[j])==best)
                            waysB[at(i,j)]+=waysB[at(i+1,j+1)];
                    if(i<m&&j<n&&bitsA[i]==bitsB[j]&&backward[at(i+1,j+1)]==best)
                        waysB[at(i,j)]+=waysB[at(i+1,j+1)];
                }
                const int distance=forward[at(m,n)];
                const cpp_int total=waysF[at(m,n)];
                size_t forcedMatch=0,forcedSub=0,forcedInsert=0,forcedDelete=0;
                size_t optimalEdges=0;
                for(size_t i=0;i<=m;++i)for(size_t j=0;j<=n;++j){
                    const auto check=[&](size_t ni,size_t nj,int cost,const char* kind){
                        if(forward[at(i,j)]+cost+backward[at(ni,nj)]!=distance)return;
                        ++optimalEdges;
                        if(waysF[at(i,j)]*waysB[at(ni,nj)]!=total)return;
                        if(kind[0]=='M')++forcedMatch;
                        else if(kind[0]=='S')++forcedSub;
                        else if(kind[0]=='I')++forcedInsert;
                        else ++forcedDelete;
                        std::printf("SPR_M269_FORCED resource=%s interval=%zu mask=%zu segment=%zu type=%s a=%zu b=%zu\n",
                            name.c_str(),ix+1,mask,segment,kind,cursorA+i,cursorB+j);
                    };
                    if(i<m&&j<n)check(i+1,j+1,bitsA[i]!=bitsB[j],bitsA[i]==bitsB[j]?"MATCH":"SUB");
                    if(i<m)check(i+1,j,1,"DELETE");
                    if(j<n)check(i,j+1,1,"INSERT");
                }
                std::printf("SPR_M269_EDIT resource=%s interval=%zu mask=%zu segment=%zu distance=%d optimalPaths=%s optimalEdges=%zu forcedMatch=%zu forcedSub=%zu forcedInsert=%zu forcedDelete=%zu\n",
                    name.c_str(),ix+1,mask,segment,distance,total.convert_to<std::string>().c_str(),
                    optimalEdges,forcedMatch,forcedSub,forcedInsert,forcedDelete);
                if(h==hits.size())break;
                const auto& hit=hits[h];
                for(size_t bit=0;bit<hit.len;++bit)
                    if(bitAt(a,hit.a+bit)!=bitAt(b,hit.b+bit)){valid=false;break;}
                if(!valid)break;
                std::printf("SPR_M269_CONSTRAINT resource=%s interval=%zu mask=%zu a=%zu b=%zu length=%zu displacement=%lld\n",
                    name.c_str(),ix+1,mask,hit.a,hit.b,hit.len,
                    static_cast<long long>(hit.b)-static_cast<long long>(hit.a));
                matched+=hit.len;
                cursorA=hit.a+hit.len;cursorB=hit.b+hit.len;
            }
            std::printf("SPR_M269_SCENARIO resource=%s interval=%zu mask=%zu matched=%zu remainingA=%zu remainingB=%zu monotonic=%d\n",
                name.c_str(),ix+1,mask,matched,na-matched,nb-matched,valid?1:0);
            if(!valid)return false;
        }
    }
    return true;
}
}
int stafuSprM269EditPathEnumeration(const char* installed,const char* cd,const char*){
    const std::vector<std::vector<size_t>> picard={
        {5,1,2,3204,4489,3234,4526},{5,1,2,4967,5093,5004,5120},
        {5,1,2,9730,10499,9792,10603},{5,3,4,6342,6591,6464,6713}};
    const std::vector<std::vector<size_t>> data={
        {0,0,1,9973,10390,9966,10356},{0,0,1,10781,11976,10747,11952}};
    if(!load(installed,cd,"BRDGPICA.SPR",picard)||
       !load(installed,cd,"BRDGDATA.SPR",data))return 1;
    std::puts("SPR_M269_SUMMARY intervals=3 scenarios=8 resources=2");
    std::puts("SPR_M269_NOTE Unit-cost Levenshtein optimal paths, exact arbitrary-precision counts, mandatory edges shared by all paths; no codec interpretation.");
    return 0;
}
