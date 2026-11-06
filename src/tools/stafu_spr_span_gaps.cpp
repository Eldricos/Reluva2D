// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 247: read-only gaps between exact matching SPR bit spans.
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


struct Span {size_t a,b,length;long long delta;};
void emitGaps(const std::string& name,size_t record,size_t imageA,size_t imageB,
              const std::vector<Match>& matches,const uint8_t* dataA,size_t bitsA,
              const uint8_t* dataB,size_t bitsB,size_t& spans,size_t& gaps,
              size_t& unequal,size_t& rejected,size_t& detail){
    std::vector<Span> found;
    for(const auto& m:matches){
        const size_t n=std::min({m.len,bitsA-m.a,bitsB-m.b});
        size_t bestStart=0,bestLength=0,runStart=0,runLength=0;
        for(size_t k=0;k<n;++k){
            if(bit(dataA,m.a+k)==bit(dataB,m.b+k)){
                if(!runLength)runStart=k;
                ++runLength;
                if(runLength>bestLength){bestLength=runLength;bestStart=runStart;}
            }else runLength=0;
        }
        if(!bestLength){++rejected;continue;}
        size_t x=m.a+bestStart,y=m.b+bestStart;
        size_t ex=x+bestLength,ey=y+bestLength;
        while(x&&y&&bit(dataA,x-1)==bit(dataB,y-1)){--x;--y;}
        while(ex<bitsA&&ey<bitsB&&bit(dataA,ex)==bit(dataB,ey)){++ex;++ey;}
        found.push_back({x,y,ex-x,static_cast<long long>(y)-static_cast<long long>(x)});
    }
    std::sort(found.begin(),found.end(),[](const Span& a,const Span& b){
        if(a.a!=b.a)return a.a<b.a;
        if(a.b!=b.b)return a.b<b.b;
        return a.length>b.length;
    });
    found.erase(std::unique(found.begin(),found.end(),[](const Span& a,const Span& b){
        return a.a==b.a&&a.b==b.b&&a.length==b.length;
    }),found.end());
    spans+=found.size();
    // Keep a monotone, non-overlapping chain in both payloads. A span
    // that crosses the chain is excluded from gap measurement, not deleted.
    std::vector<Span> chain;
    size_t skipped=0;
    for(const auto& v:found){
        if(!chain.empty()&&(v.a<chain.back().a+chain.back().length||
                            v.b<chain.back().b+chain.back().length)){
            ++skipped;continue;
        }
        chain.push_back(v);
    }
    size_t pairGaps=0,pairUnequal=0;
    for(size_t i=1;i<chain.size();++i){
        const auto& a=chain[i-1];const auto& b=chain[i];
        const size_t endA=a.a+a.length,endB=a.b+a.length;
        const size_t gapA=b.a-endA,gapB=b.b-endB;
        const long long change=b.delta-a.delta;
        ++gaps;++pairGaps;
        if(gapA!=gapB){++unequal;++pairUnequal;}
        if(detail++>=120)continue;
        std::printf("SPR_GAP resource=%s record=%zu imageA=%zu imageB=%zu leftEndA=%zu rightStartA=%zu leftEndB=%zu rightStartB=%zu gapBitsA=%zu gapBitsB=%zu gapDeltaBits=%lld displacementBefore=%lld displacementAfter=%lld boundaryAlignedA=%d boundaryAlignedB=%d bothGapsByteMultiple=%d zeroGapA=%d zeroGapB=%d\n",
            name.c_str(),record,imageA,imageB,endA,b.a,endB,b.b,gapA,gapB,
            change,a.delta,b.delta,(endA%8==0&&b.a%8==0)?1:0,
            (endB%8==0&&b.b%8==0)?1:0,(gapA%8==0&&gapB%8==0)?1:0,
            gapA==0?1:0,gapB==0?1:0);
    }
    if(detail++<120)
        std::printf("SPR_GAP_PAIR resource=%s record=%zu imageA=%zu imageB=%zu anchors=%zu maximalSpans=%zu chainSpans=%zu skippedOverlaps=%zu interiorGaps=%zu unequalGaps=%zu\n",
            name.c_str(),record,imageA,imageB,matches.size(),found.size(),chain.size(),skipped,pairGaps,pairUnequal);
}
int stafuSprSpanGaps(const char* installed,const char* cd,const char* optionalName){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"spr-span-gaps: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName?std::vector<std::string>{optionalName}:
        std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names){
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)){std::fprintf(stderr,"spr-span-gaps: %s: %s\n",name.c_str(),error.c_str());++failures;continue;}
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)){std::fprintf(stderr,"spr-span-gaps: %s: %s\n",name.c_str(),error.c_str());++failures;continue;}
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
        size_t pairs=0,anchors=0,spans=0,gaps=0,unequal=0,rejected=0,detail=0,ambiguous=0;
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
                emitGaps(name,s.record,prev.image,s.image,matches,
                    bytes.data()+prev.offset,prev.size*8,data,s.size*8,
                    spans,gaps,unequal,rejected,detail);
                if(detail++<100)
                    std::printf("SPR_SEGMENT_PAIR resource=%s record=%zu imageA=%zu imageB=%zu anchoredMatches=%zu regionsA=%zu regionsB=%zu\n",
                        name.c_str(),s.record,prev.image,s.image,matches.size(),prior.size(),current.size());
            }
            prior=std::move(current);prev=s;havePrev=true;
        }
        std::printf("SPR_GAP_SUMMARY resource=%s direct=%zu neighboringPairs=%zu anchoredMatches=%zu maximalSpans=%zu interiorGaps=%zu unequalGaps=%zu rejectedAnchors=%zu ambiguousCandidates=%zu detailLimit=120\n",
            name.c_str(),samples.size(),pairs,anchors,spans,gaps,unequal,rejected,ambiguous);
    }
    std::puts("SPR_GAP_NOTE Interior gaps are measured only between consecutive non-overlapping, monotone exact spans. Crossing or overlapping spans are excluded from gap measurement. Length differences do not establish insertion, deletion, or codec semantics.");
    return failures?1:0;
}
