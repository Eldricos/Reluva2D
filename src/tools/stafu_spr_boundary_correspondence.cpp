// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 243: read-only boundary-anchored, ordered SPR region correspondence.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
struct Sample {size_t record,image,offset,size;};
struct Region {size_t start,end,period;std::string pattern;};
bool bit(const uint8_t* d,size_t i){return (d[i/8]>>(7-i%8))&1u;}
std::string pattern(const uint8_t* d,size_t start,size_t period){
    std::string s; s.reserve(period);
    for(size_t j=0;j<period;++j)s.push_back(bit(d,start+j)?'1':'0');
    return s;
}
size_t primitive(const uint8_t* d,size_t a,size_t b){
    for(size_t p=1;p<=40&&2*p<=b-a;++p){
        bool equal=true;
        for(size_t i=a;i+p<b;++i)if(bit(d,i)!=bit(d,i+p)){equal=false;break;}
        if(equal)return p;
    }
    return 0;
}
std::vector<Region> scan(const uint8_t* d,size_t bits){
    std::vector<Region> candidates;
    for(size_t p: {size_t(10),size_t(20),size_t(30),size_t(40)}){
        size_t i=0;
        while(i+p<bits){
            if(bit(d,i)!=bit(d,i+p)){++i;continue;}
            const size_t a=i;
            do{++i;}while(i+p<bits&&bit(d,i)==bit(d,i+p));
            const size_t b=i+p;
            if(b-a<80||b-a<3*p)continue;
            const size_t actual=primitive(d,a,b);
            if(actual==0)continue;
            candidates.push_back({a,b,actual,pattern(d,a,actual)});
        }
    }
    std::sort(candidates.begin(),candidates.end(),[](const Region& a,const Region& b){
        if(a.end-a.start!=b.end-b.start)return a.end-a.start>b.end-b.start;
        return a.start<b.start;
    });
    std::vector<Region> result;
    for(const auto& c:candidates){
        bool duplicate=false;
        for(const auto& r:result){
            const size_t lo=std::max(c.start,r.start),hi=std::min(c.end,r.end);
            if(hi>lo&&(hi-lo)*2>=std::min(c.end-c.start,r.end-r.start)){
                duplicate=true;break;
            }
        }
        if(!duplicate)result.push_back(c);
    }
    std::sort(result.begin(),result.end(),[](const Region& a,const Region& b){return a.start<b.start;});
    return result;
}
size_t hamming(const std::string& a,const std::string& b){
    if(a.size()!=b.size())return size_t(-1);
    size_t n=0;for(size_t i=0;i<a.size();++i)n+=a[i]!=b[i];return n;
}
size_t cyclicDistance(const std::string& a,const std::string& b,size_t& shift){
    if(a.size()!=b.size())return size_t(-1);
    size_t best=a.size()+1;shift=0;
    for(size_t s=0;s<a.size();++s){
        size_t n=0;
        for(size_t i=0;i<a.size();++i)n+=a[(i+s)%a.size()]!=b[i];
        if(n<best){best=n;shift=s;}
    }
    return best;
}
std::string context(const uint8_t* d,size_t bits,size_t pos,bool before){
    std::string s;
    const size_t a=before?(pos>24?pos-24:0):pos;
    const size_t b=before?pos:std::min(bits,pos+24);
    for(size_t i=a;i<b;++i)s.push_back(bit(d,i)?'1':'0');
    return s;
}
}

struct Anchored {Region region;std::string left,right;};
int stafuSprBoundaryCorrespondence(const char* installed,const char* cd,const char* optionalName){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"spr-boundary-correspondence: %s\n",error.c_str());return 1;}
    const std::vector<std::string> names=optionalName?std::vector<std::string>{optionalName}:
        std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names){
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)){std::fprintf(stderr,"spr-boundary-correspondence: %s: %s\n",name.c_str(),error.c_str());++failures;continue;}
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)){std::fprintf(stderr,"spr-boundary-correspondence: %s: %s\n",name.c_str(),error.c_str());++failures;continue;}
        std::vector<Sample> samples;
        for(size_t ri=0;ri<index.records().size();++ri){
            size_t ordinal=0;
            for(const auto& e:index.records()[ri].events){
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View v;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,v)&&!(v.marker==0x00030000&&v.size==4))
                    samples.push_back({ri,ordinal,v.offset,v.size});
                ++ordinal;
            }
        }
        size_t totalRegions=0,pairs=0,matched=0,ambiguous=0,unmatchedOld=0,unmatchedNew=0,consistent=0,detail=0;
        std::vector<Anchored> prior;Sample prev{};bool havePrev=false;
        for(const auto& s:samples){
            const uint8_t* data=bytes.data()+s.offset;
            const auto raw=scan(data,s.size*8);
            std::vector<Anchored> current;current.reserve(raw.size());
            for(const auto& r:raw)current.push_back({r,context(data,s.size*8,r.start,true),context(data,s.size*8,r.end,false)});
            totalRegions+=current.size();
            if(havePrev&&s.record==prev.record){
                ++pairs;
                // A candidate requires both full 24-bit boundaries, same primitive period,
                // and a cyclically identical pattern. Partial edge contexts are excluded.
                std::vector<std::vector<size_t>> candidates(current.size());
                std::vector<size_t> oldCounts(prior.size(),0);
                for(size_t j=0;j<current.size();++j){
                    const auto& b=current[j];
                    if(b.left.size()!=24||b.right.size()!=24)continue;
                    for(size_t i=0;i<prior.size();++i){
                        const auto& a=prior[i];
                        if(a.left.size()!=24||a.right.size()!=24||a.region.period!=b.region.period||
                           a.left!=b.left||a.right!=b.right)continue;
                        size_t shift=0;
                        if(cyclicDistance(a.region.pattern,b.region.pattern,shift)!=0)continue;
                        candidates[j].push_back(i);++oldCounts[i];
                    }
                }
                // One-to-one, order-preserving correspondence: only uniquely anchored
                // candidates on both sides can be accepted, in increasing offset order.
                size_t lastOld=0;bool hasLast=false;bool haveDelta=false;long long lastDelta=0;
                size_t localMatches=0,localAmbiguous=0,localUnmatchedNew=0;
                std::vector<bool> used(prior.size(),false);
                for(size_t j=0;j<current.size();++j){
                    const auto& c=candidates[j];
                    if(c.size()!=1||(c.size()==1&&oldCounts[c[0]]!=1)){
                        if(!c.empty()){++ambiguous;++localAmbiguous;}
                        else {++unmatchedNew;++localUnmatchedNew;}
                        continue;
                    }
                    const size_t i=c[0];
                    if(used[i]||(hasLast&&i<=lastOld)){
                        ++ambiguous;++localAmbiguous;continue;
                    }
                    used[i]=true;lastOld=i;hasLast=true;
                    const long long delta=static_cast<long long>(current[j].region.start)-static_cast<long long>(prior[i].region.start);
                    const bool stable=haveDelta&&delta==lastDelta;
                    if(stable)++consistent;
                    lastDelta=delta;haveDelta=true;++matched;++localMatches;
                    if(detail++<80)std::printf("SPR_ANCHOR_MATCH resource=%s record=%zu imageA=%zu imageB=%zu startA=%zu startB=%zu lengthA=%zu lengthB=%zu period=%zu deltaBits=%lld sameDeltaAsPrevious=%d left24=%s right24=%s\n",
                        name.c_str(),s.record,prev.image,s.image,prior[i].region.start,current[j].region.start,
                        prior[i].region.end-prior[i].region.start,current[j].region.end-current[j].region.start,
                        current[j].region.period,delta,stable?1:0,current[j].left.c_str(),current[j].right.c_str());
                }
                size_t localUnmatchedOld=0;
                for(bool x:used)if(!x){++unmatchedOld;++localUnmatchedOld;}
                if(detail++<80)std::printf("SPR_ANCHOR_PAIR resource=%s record=%zu imageA=%zu imageB=%zu regionsA=%zu regionsB=%zu uniqueOrderedMatches=%zu ambiguousNew=%zu unmatchedA=%zu unmatchedB=%zu\n",
                    name.c_str(),s.record,prev.image,s.image,prior.size(),current.size(),localMatches,localAmbiguous,localUnmatchedOld,localUnmatchedNew);
            }
            prior=std::move(current);prev=s;havePrev=true;
        }
        std::printf("SPR_ANCHOR_SUMMARY resource=%s direct=%zu regions=%zu neighboringPairs=%zu uniqueOrderedMatches=%zu ambiguousNew=%zu unmatchedA=%zu unmatchedB=%zu consecutiveSameDelta=%zu detailLimit=80\n",
            name.c_str(),samples.size(),totalRegions,pairs,matched,ambiguous,unmatchedOld,unmatchedNew,consistent);
    }
    std::puts("SPR_ANCHOR_NOTE Strict 24-bit left/right context and cyclic pattern equality; ambiguous and crossing candidates are not assigned; same-delta compares consecutive accepted matches only; no codec semantics inferred.");
    return failures?1:0;
}
