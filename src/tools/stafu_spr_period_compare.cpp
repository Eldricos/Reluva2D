// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 242: read-only comparison of periodic bit patterns across SPR frames.
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
int stafuSprPeriodCompare(const char* installed,const char* cd,const char* optionalName){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"spr-period-compare: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName?std::vector<std::string>{optionalName}:
        std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names){
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)){
            std::fprintf(stderr,"spr-period-compare: %s: %s\n",name.c_str(),error.c_str());++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)){
            std::fprintf(stderr,"spr-period-compare: %s: %s\n",name.c_str(),error.c_str());++failures;continue;
        }
        std::vector<Sample> samples;
        for(size_t r=0;r<index.records().size();++r){
            size_t ordinal=0;
            for(const auto& e:index.records()[r].events){
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View v;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,v)&&
                    !(v.marker==0x00030000&&v.size==4))
                    samples.push_back({r,ordinal,v.offset,v.size});
                ++ordinal;
            }
        }
        size_t regionCount=0,comparedPairs=0,exactPatterns=0,rotatedPatterns=0,detail=0;
        std::vector<Region> prior;
        Sample prev{};bool havePrev=false;
        for(const auto& s:samples){
            const uint8_t* data=bytes.data()+s.offset;
            const auto regions=scan(data,s.size*8);
            regionCount+=regions.size();
            if(!regions.empty()&&detail<50){
                for(const auto& r:regions){
                    if(detail++>=50)break;
                    const auto left=context(data,s.size*8,r.start,true);
                    const auto right=context(data,s.size*8,r.end,false);
                    std::printf("SPR_PATTERN_REGION resource=%s record=%zu image=%zu startBit=%zu endBit=%zu lengthBits=%zu period=%zu pattern=%s phase=%zu left24=%s right24=%s\n",
                        name.c_str(),s.record,s.image,r.start,r.end,r.end-r.start,r.period,
                        r.pattern.c_str(),r.start%8,left.c_str(),right.c_str());
                }
            }
            if(havePrev&&s.record==prev.record){
                // Pair same-period regions by best pattern similarity, breaking ties
                // by similar region length. Each current region is reported at most once.
                for(const auto& r:regions){
                    const Region* best=nullptr;size_t bestDistance=size_t(-1),bestLength=size_t(-1),bestShift=0;
                    for(const auto& old:prior){
                        if(old.period!=r.period)continue;
                        size_t shift=0;
                        const size_t distance=cyclicDistance(old.pattern,r.pattern,shift);
                        const size_t lengthDiff=(old.end-old.start)>(r.end-r.start)
                            ?(old.end-old.start)-(r.end-r.start):(r.end-r.start)-(old.end-old.start);
                        if(distance<bestDistance||(distance==bestDistance&&lengthDiff<bestLength)){
                            best=&old;bestDistance=distance;bestLength=lengthDiff;bestShift=shift;
                        }
                    }
                    if(!best)continue;
                    ++comparedPairs;
                    if(best->pattern==r.pattern)++exactPatterns;
                    if(bestDistance==0)++rotatedPatterns;
                    if(comparedPairs<=80)
                        std::printf("SPR_PATTERN_PAIR resource=%s record=%zu imageA=%zu imageB=%zu period=%zu startA=%zu startB=%zu lengthA=%zu lengthB=%zu bitOffsetDelta=%lld exactPattern=%d rotationEquivalent=%d rotationBits=%zu cyclicHamming=%zu boundaryStartPhaseA=%zu boundaryStartPhaseB=%zu\n",
                            name.c_str(),s.record,prev.image,s.image,r.period,best->start,r.start,
                            best->end-best->start,r.end-r.start,
                            static_cast<long long>(r.start)-static_cast<long long>(best->start),
                            best->pattern==r.pattern?1:0,bestDistance==0?1:0,bestShift,bestDistance,
                            best->start%8,r.start%8);
                }
            }
            prior=regions;prev=s;havePrev=true;
        }
        std::printf("SPR_PATTERN_SUMMARY resource=%s direct=%zu regions=%zu comparedPairs=%zu exactPatterns=%zu rotationEquivalent=%zu detailRegionLimit=50 detailPairLimit=80\n",
            name.c_str(),samples.size(),regionCount,comparedPairs,exactPatterns,rotatedPatterns);
    }
    std::puts("SPR_PATTERN_NOTE Exact bit patterns are MSB-first; rotation equivalence ignores cyclic phase; neighboring comparisons are exploratory and may reuse a prior region; boundary contexts are 24 bits or fewer; no codec semantics inferred.");
    return failures?1:0;
}
