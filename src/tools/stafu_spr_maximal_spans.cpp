// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 246: read-only maximal exact matching bit spans around SPR anchors.
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
void emitMaximal(const std::string& name,size_t record,size_t imageA,size_t imageB,
                 const std::vector<Match>& matches,const uint8_t* dataA,size_t bitsA,
                 const uint8_t* dataB,size_t bitsB,size_t& spans,size_t& uniqueBits,
                 size_t& detail,size_t& rejected){
    std::vector<Span> found;
    for(const auto& m:matches){
        // M243's cyclic equivalence permits rotation; an anchor is not
        // necessarily bit-identical at its reported starting coordinates.
        // Seed only with verified exact bits, avoiding unsupported extension.
        const size_t n=std::min({m.len,bitsA-m.a,bitsB-m.b});
        size_t bestStart=0,bestLength=0,runStart=0,runLength=0;
        for(size_t k=0;k<n;++k){
            if(bit(dataA,m.a+k)==bit(dataB,m.b+k)){
                if(runLength==0)runStart=k;
                ++runLength;
                if(runLength>bestLength){bestLength=runLength;bestStart=runStart;}
            }else runLength=0;
        }
        if(!bestLength){++rejected;continue;}
        size_t x=m.a+bestStart,y=m.b+bestStart;
        size_t endX=x+bestLength,endY=y+bestLength;
        while(x>0&&y>0&&bit(dataA,x-1)==bit(dataB,y-1)){--x;--y;}
        while(endX<bitsA&&endY<bitsB&&bit(dataA,endX)==bit(dataB,endY)){++endX;++endY;}
        found.push_back({x,y,endX-x,static_cast<long long>(y)-static_cast<long long>(x)});
    }
    // Multiple periodic anchors often expand to the same maximal span.
    std::sort(found.begin(),found.end(),[](const Span& a,const Span& b){
        if(a.a!=b.a)return a.a<b.a;
        if(a.b!=b.b)return a.b<b.b;
        return a.length<b.length;
    });
    found.erase(std::unique(found.begin(),found.end(),[](const Span& a,const Span& b){
        return a.a==b.a&&a.b==b.b&&a.length==b.length;
    }),found.end());
    for(const auto& v:found){
        ++spans;uniqueBits+=v.length;
        if(detail++>=120)continue;
        const bool leftBoundary=v.a==0||v.b==0;
        const bool rightBoundary=v.a+v.length==bitsA||v.b+v.length==bitsB;
        const long long leftA=leftBoundary?-1:static_cast<long long>(bit(dataA,v.a-1));
        const long long leftB=leftBoundary?-1:static_cast<long long>(bit(dataB,v.b-1));
        const long long rightA=rightBoundary?-1:static_cast<long long>(bit(dataA,v.a+v.length));
        const long long rightB=rightBoundary?-1:static_cast<long long>(bit(dataB,v.b+v.length));
        std::printf("SPR_MAX_SPAN resource=%s record=%zu imageA=%zu imageB=%zu startBitA=%zu endBitA=%zu startBitB=%zu endBitB=%zu lengthBits=%zu displacementBits=%lld byteAlignedA=%d byteAlignedB=%d lengthByteAligned=%d leftBoundary=%d rightBoundary=%d beforeBitA=%lld beforeBitB=%lld afterBitA=%lld afterBitB=%lld\n",
            name.c_str(),record,imageA,imageB,v.a,v.a+v.length,v.b,v.b+v.length,
            v.length,v.delta,v.a%8==0,v.b%8==0,v.length%8==0,
            leftBoundary?1:0,rightBoundary?1:0,leftA,leftB,rightA,rightB);
    }
}
int stafuSprMaximalSpans(const char* installed,const char* cd,const char* optionalName){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"spr-maximal-spans: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName?std::vector<std::string>{optionalName}:
        std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names){
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)){std::fprintf(stderr,"spr-maximal-spans: %s: %s\n",name.c_str(),error.c_str());++failures;continue;}
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)){std::fprintf(stderr,"spr-maximal-spans: %s: %s\n",name.c_str(),error.c_str());++failures;continue;}
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
        size_t pairs=0,anchors=0,spans=0,spanBits=0,rejected=0,detail=0,ambiguous=0;
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
                emitMaximal(name,s.record,prev.image,s.image,matches,
                    bytes.data()+prev.offset,prev.size*8,data,s.size*8,
                    spans,spanBits,detail,rejected);
                if(detail++<100)
                    std::printf("SPR_SEGMENT_PAIR resource=%s record=%zu imageA=%zu imageB=%zu anchoredMatches=%zu regionsA=%zu regionsB=%zu\n",
                        name.c_str(),s.record,prev.image,s.image,matches.size(),prior.size(),current.size());
            }
            prior=std::move(current);prev=s;havePrev=true;
        }
        std::printf("SPR_MAX_SUMMARY resource=%s direct=%zu neighboringPairs=%zu anchoredMatches=%zu maximalSpans=%zu sumSpanBitsMayOverlap=%zu rejectedAnchors=%zu ambiguousCandidates=%zu detailLimit=120\n",
            name.c_str(),samples.size(),pairs,anchors,spans,spanBits,rejected,ambiguous);
    }
    std::puts("SPR_MAX_NOTE Each cyclic anchor is checked for exact seed bits before extension. Reported spans are maximal for a fixed bit displacement and are deduplicated by exact coordinates; distinct spans may overlap. Boundary bit values are -1 when an edge is reached. No codec or byte-boundary interpretation is assumed.");
    return failures?1:0;
}
