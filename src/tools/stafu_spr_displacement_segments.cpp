// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 244: read-only SPR anchor displacement segmentation.
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
struct Match {size_t a,b,len,period;long long delta;};
void emitSegment(const std::string& name,size_t record,size_t imageA,size_t imageB,
                 const std::vector<Match>& matches,size_t first,size_t end,size_t tolerance,
                 size_t& segments,size_t& exact,size_t& near,size_t& single,size_t& detail){
    if(first>=end)return;
    ++segments;
    const size_t count=end-first;
    if(count==1)++single;
    long long lo=matches[first].delta,hi=lo;
    for(size_t i=first+1;i<end;++i){
        lo=std::min(lo,matches[i].delta);hi=std::max(hi,matches[i].delta);
    }
    if(count>=2){if(lo==hi)++exact;else ++near;}
    if(detail++<100)
        std::printf("SPR_SEGMENT resource=%s record=%zu imageA=%zu imageB=%zu anchors=%zu startA=%zu endA=%zu startB=%zu endB=%zu deltaMin=%lld deltaMax=%lld deltaRange=%lld toleranceBits=%zu classification=%s\n",
            name.c_str(),record,imageA,imageB,count,matches[first].a,
            matches[end-1].a+matches[end-1].len,matches[first].b,
            matches[end-1].b+matches[end-1].len,lo,hi,hi-lo,tolerance,
            count==1?"singleton":lo==hi?"exact":"near");
}
int stafuSprDisplacementSegments(const char* installed,const char* cd,const char* optionalName){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"spr-displacement-segments: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName?std::vector<std::string>{optionalName}:
        std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names){
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)){std::fprintf(stderr,"spr-displacement-segments: %s: %s\n",name.c_str(),error.c_str());++failures;continue;}
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)){std::fprintf(stderr,"spr-displacement-segments: %s: %s\n",name.c_str(),error.c_str());++failures;continue;}
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
        size_t pairs=0,anchors=0,segments=0,exact=0,near=0,single=0,detail=0,ambiguous=0;
        std::vector<Anchored> prior;Sample prev{};bool havePrev=false;
        for(const auto& s:samples){
            const uint8_t* data=bytes.data()+s.offset;
            const auto regions=scan(data,s.size*8);
            std::vector<Anchored> current;
            for(const auto& r:regions)
                current.push_back({r,context(data,s.size*8,r.start,true),context(data,s.size*8,r.end,false)});
            if(havePrev&&s.record==prev.record){
                ++pairs;
                std::vector<std::vector<size_t>> candidates(current.size());
                std::vector<size_t> oldCounts(prior.size(),0);
                for(size_t j=0;j<current.size();++j){
                    const auto& b=current[j];
                    if(b.left.size()!=24||b.right.size()!=24)continue;
                    for(size_t i=0;i<prior.size();++i){
                        const auto& a=prior[i];
                        if(a.left.size()!=24||a.right.size()!=24||
                           a.region.period!=b.region.period||a.left!=b.left||a.right!=b.right)continue;
                        size_t shift=0;
                        if(cyclicDistance(a.region.pattern,b.region.pattern,shift)!=0)continue;
                        candidates[j].push_back(i);++oldCounts[i];
                    }
                }
                std::vector<Match> matches;size_t lastOld=0;bool hasLast=false;
                for(size_t j=0;j<current.size();++j){
                    if(candidates[j].size()!=1||oldCounts[candidates[j][0]]!=1){
                        if(!candidates[j].empty())++ambiguous;
                        continue;
                    }
                    const size_t i=candidates[j][0];
                    if(hasLast&&i<=lastOld){++ambiguous;continue;}
                    hasLast=true;lastOld=i;
                    const auto& a=prior[i].region;const auto& b=current[j].region;
                    matches.push_back({a.start,b.start,a.end-a.start,a.period,
                        static_cast<long long>(b.start)-static_cast<long long>(a.start)});
                }
                anchors+=matches.size();
                // Maximal contiguous ordered groups with displacement spread <= tolerance.
                // No grouping across unmatched region gaps: indices in both scans must
                // be consecutive. Exact groups use zero tolerance; near groups use 32 bits.
                for(size_t tolerance:{size_t(0),size_t(32)}){
                    if(matches.empty())continue;
                    size_t first=0;long long lo=matches[0].delta,hi=lo;
                    for(size_t k=1;k<=matches.size();++k){
                        bool extend=k<matches.size();
                        if(extend){
                            const long long candidateLo=std::min(lo,matches[k].delta);
                            const long long candidateHi=std::max(hi,matches[k].delta);
                            // Large coordinate gaps may be preserved unchanged; we
                            // deliberately do not require physical adjacency.
                            extend=candidateHi-candidateLo<=static_cast<long long>(tolerance);
                        }
                        if(!extend){
                            emitSegment(name,s.record,prev.image,s.image,matches,first,k,
                                tolerance,segments,exact,near,single,detail);
                            if(k<matches.size()){first=k;lo=hi=matches[k].delta;}
                        }else{
                            lo=std::min(lo,matches[k].delta);
                            hi=std::max(hi,matches[k].delta);
                        }
                    }
                }
                if(detail++<100)
                    std::printf("SPR_SEGMENT_PAIR resource=%s record=%zu imageA=%zu imageB=%zu anchoredMatches=%zu regionsA=%zu regionsB=%zu\n",
                        name.c_str(),s.record,prev.image,s.image,matches.size(),prior.size(),current.size());
            }
            prior=std::move(current);prev=s;havePrev=true;
        }
        std::printf("SPR_SEGMENT_SUMMARY resource=%s direct=%zu neighboringPairs=%zu anchoredMatches=%zu segmentsBothTolerances=%zu exactMulti=%zu nearMulti=%zu singletons=%zu ambiguousCandidates=%zu detailLimit=100\n",
            name.c_str(),samples.size(),pairs,anchors,segments,exact,near,single,ambiguous);
    }
    std::puts("SPR_SEGMENT_NOTE Anchors use M243 strict contexts and ordered uniqueness. Segment spans cover anchors and intervening unknown data, not proven equal bytes. Exact (0-bit) and near (32-bit) partitions are both reported; counts combine both partitions. No codec semantics inferred.");
    return failures?1:0;
}
