// SPDX-License-Identifier: GPL-3.0-or-later
// M263: sensitivity of optimal bit alignments to substitution and gap costs.
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
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"SPR_M263_ERROR %s\n",error.c_str());return false;}
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){std::fprintf(stderr,"SPR_M263_ERROR %s\n",error.c_str());return false;}
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){std::fprintf(stderr,"SPR_M263_ERROR %s\n",error.c_str());return false;}
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
    for(size_t gi=0;gi<gaps.size();++gi){
        const auto& g=gaps[gi];
        const Sample *sa=nullptr,*sb=nullptr;
        for(const auto& x:samples){
            if(x.record==g[0]&&x.image==g[1])sa=&x;
            if(x.record==g[0]&&x.image==g[2])sb=&x;
        }
        if(!sa||!sb||g[4]>sa->size*8||g[6]>sb->size*8||
           g[3]>g[4]||g[5]>g[6]){
            std::printf("SPR_M263_ERROR resource=%s gap=%zu reason=invalid-payload-or-range\n",name.c_str(),gi+1);
            return false;
        }
        const uint8_t* a=bytes.data()+sa->offset;
        const uint8_t* b=bytes.data()+sb->offset;
        const size_t na=g[4]-g[3],nb=g[6]-g[5];
        struct Span {size_t a,b,len;int shift;};
        std::vector<Span> candidates;
        // Every maximal matching run on each tested diagonal is a candidate.
        for(int shift=-32;shift<=32;++shift){
            const size_t aa=shift<0?size_t(-shift):0;
            const size_t bb=shift>0?size_t(shift):0;
            if(aa>=na||bb>=nb)continue;
            const size_t n=std::min(na-aa,nb-bb);
            size_t runStart=0,runLength=0;
            auto record=[&](){
                if(runLength>=24)
                    candidates.push_back({aa+runStart,bb+runStart,runLength,shift});
                runLength=0;
            };
            for(size_t i=0;i<n;++i){
                if(bitAt(a,g[3]+aa+i)==bitAt(b,g[5]+bb+i)){
                    if(runLength==0)runStart=i;
                    ++runLength;
                }else record();
            }
            record();
        }
        // Deterministic, conservative selection: longest first, no overlap in
        // either payload. Then retain a monotone ordered chain.
        std::sort(candidates.begin(),candidates.end(),[](const Span& x,const Span& y){
            if(x.len!=y.len)return x.len>y.len;
            if(x.a!=y.a)return x.a<y.a;
            return x.b<y.b;
        });
        std::vector<Span> selected;
        for(const auto& c:candidates){
            bool overlap=false;
            for(const auto& p:selected){
                if(c.a<p.a+p.len&&p.a<c.a+c.len)overlap=true;
                if(c.b<p.b+p.len&&p.b<c.b+c.len)overlap=true;
                if(overlap)break;
            }
            if(!overlap)selected.push_back(c);
        }
        std::sort(selected.begin(),selected.end(),[](const Span& x,const Span& y){
            return x.a<y.a;
        });
        std::vector<Span> chain;
        for(const auto& c:selected){
            if(chain.empty()||c.b>=chain.back().b+chain.back().len)
                chain.push_back(c);
        }
        std::printf("SPR_M263_GAP resource=%s gap=%zu record=%zu imageA=%zu imageB=%zu bitsA=%zu bitsB=%zu candidates=%zu selected=%zu chain=%zu minRun=24 shiftRange=32\n",
            name.c_str(),gi+1,g[0],g[1],g[2],na,nb,candidates.size(),selected.size(),chain.size());
        auto bits=[&](const uint8_t* p,size_t offset,size_t length){
            std::string value;
            value.reserve(length);
            for(size_t k=0;k<length;++k)value.push_back(bitAt(p,offset+k)?'1':'0');
            return value;
        };
        auto commonPrefix=[](const std::string& x,const std::string& y){
            size_t n=0;while(n<std::min(x.size(),y.size())&&x[n]==y[n])++n;return n;
        };
        auto commonSuffix=[](const std::string& x,const std::string& y){
            size_t n=0;while(n<std::min(x.size(),y.size())&&x[x.size()-1-n]==y[y.size()-1-n])++n;return n;
        };
        auto subsequence=[](const std::string& shortBits,const std::string& longBits){
            return longBits.find(shortBits)!=std::string::npos;
        };
        auto deletion=[](const std::string& shorter,const std::string& longer){
            if(shorter.size()>longer.size())return false;
            const size_t extra=longer.size()-shorter.size();
            for(size_t at=0;at+extra<=longer.size();++at){
                if(longer.compare(0,at,shorter,0,at)==0&&
                   longer.compare(at+extra,std::string::npos,shorter,at,std::string::npos)==0)
                    return true;
            }
            return false;
        };
        for(size_t i=0;i<chain.size();++i){
            const auto& c=chain[i];
            const size_t absoluteA=g[3]+c.a,absoluteB=g[5]+c.b;
            const int64_t displacement=int64_t(absoluteB)-int64_t(absoluteA);
            std::printf("SPR_M263_SPAN resource=%s gap=%zu rank=%zu a=%zu b=%zu length=%zu relativeShift=%d displacement=%lld byteAlignedA=%d byteAlignedB=%d\n",
                name.c_str(),gi+1,i+1,absoluteA,absoluteB,c.len,c.shift,
                static_cast<long long>(displacement),absoluteA%8==0?1:0,absoluteB%8==0?1:0);
            if(i){
                const auto& p=chain[i-1];
                const int64_t prevDelta=int64_t(g[5]+p.b)-int64_t(g[3]+p.a);
                const size_t gapA=c.a-(p.a+p.len),gapB=c.b-(p.b+p.len);
                const int64_t change=displacement-prevDelta;
                if(change==0)continue;
                const size_t offsetA=g[3]+p.a+p.len,offsetB=g[5]+p.b+p.len;
                const std::string intervalA=bits(a,offsetA,gapA);
                const std::string intervalB=bits(b,offsetB,gapB);
                const size_t n=intervalA.size(),m=intervalB.size(),stride=m+1;
                struct Profile {const char* name;size_t substitution,gap;};
                const Profile profiles[]={{"unit",1,1},{"sub2",2,1},{"gap2",1,2},{"gap3",1,3},{"sub3",3,1}};
                struct Result {
                    std::string name,script;
                    size_t score=0,matches=0,subs=0,ins=0,dels=0,clusters=0;
                    size_t longestMatch=0;
                    std::vector<std::pair<size_t,size_t>> matched;
                    std::vector<std::pair<size_t,size_t>> insertionSites;
                    std::vector<std::pair<size_t,size_t>> deletionSites;
                };
                std::vector<Result> results;
                for(const auto& profile:profiles){
                    std::vector<size_t> dp((n+1)*stride,0);
                    for(size_t x=0;x<=n;++x)dp[x*stride]=x*profile.gap;
                    for(size_t y=0;y<=m;++y)dp[y]=y*profile.gap;
                    for(size_t x=1;x<=n;++x)
                        for(size_t y=1;y<=m;++y){
                            const size_t diag=dp[(x-1)*stride+y-1]+(intervalA[x-1]!=intervalB[y-1]?profile.substitution:0);
                            const size_t del=dp[(x-1)*stride+y]+profile.gap;
                            const size_t ins=dp[x*stride+y-1]+profile.gap;
                            dp[x*stride+y]=std::min({diag,del,ins});
                        }
                    struct Edit {char op;size_t a,b;};
                    std::vector<Edit> edits;
                    size_t x=n,y=m;
                    while(x||y){
                        if(x&&y&&dp[x*stride+y]==dp[(x-1)*stride+y-1]+(intervalA[x-1]!=intervalB[y-1]?profile.substitution:0)){
                            edits.push_back({intervalA[x-1]==intervalB[y-1]?'M':'S',x-1,y-1});--x;--y;
                        }else if(x&&dp[x*stride+y]==dp[(x-1)*stride+y]+profile.gap){
                            edits.push_back({'D',x-1,y});--x;
                        }else{
                            edits.push_back({'I',x,y-1});--y;
                        }
                    }
                    std::reverse(edits.begin(),edits.end());
                    Result result;result.name=profile.name;result.score=dp[n*stride+m];
                    size_t currentMatch=0;bool inCluster=false;
                    for(const auto& e:edits){
                        result.script.push_back(e.op);
                        if(e.op=='M'){
                            ++result.matches;++currentMatch;inCluster=false;
                            result.matched.push_back({e.a,e.b});
                        }else{
                            result.longestMatch=std::max(result.longestMatch,currentMatch);
                            currentMatch=0;
                            if(!inCluster){++result.clusters;inCluster=true;}
                            if(e.op=='S')++result.subs;
                            if(e.op=='I'){++result.ins;result.insertionSites.push_back({e.a,e.b});}
                            if(e.op=='D'){++result.dels;result.deletionSites.push_back({e.a,e.b});}
                        }
                    }
                    result.longestMatch=std::max(result.longestMatch,currentMatch);
                    std::printf("SPR_M263_PROFILE resource=%s gap=%zu from=%zu to=%zu model=%s substitutionCost=%zu gapCost=%zu score=%zu matches=%zu substitutions=%zu insertions=%zu deletions=%zu clusters=%zu longestMatch=%zu\n",
                        name.c_str(),gi+1,i,i+1,profile.name,profile.substitution,profile.gap,
                        result.score,result.matches,result.subs,result.ins,result.dels,result.clusters,result.longestMatch);
                    results.push_back(std::move(result));
                }
                const auto& baseline=results.front();
                for(size_t model=1;model<results.size();++model){
                    const auto& r=results[model];
                    size_t shared=0;
                    for(const auto& pair:baseline.matched)
                        if(std::find(r.matched.begin(),r.matched.end(),pair)!=r.matched.end())++shared;
                    size_t sharedInsert=0,sharedDelete=0;
                    for(const auto& pair:baseline.insertionSites)
                        if(std::find(r.insertionSites.begin(),r.insertionSites.end(),pair)!=r.insertionSites.end())++sharedInsert;
                    for(const auto& pair:baseline.deletionSites)
                        if(std::find(r.deletionSites.begin(),r.deletionSites.end(),pair)!=r.deletionSites.end())++sharedDelete;
                    std::printf("SPR_M263_STABILITY resource=%s gap=%zu from=%zu to=%zu model=%s sharedMatchPairs=%zu baselineMatches=%zu modelMatches=%zu sharedInsertSites=%zu sharedDeleteSites=%zu sameScript=%d\n",
                        name.c_str(),gi+1,i,i+1,r.name.c_str(),shared,baseline.matches,r.matches,
                        sharedInsert,sharedDelete,r.script==baseline.script?1:0);
                }
                std::printf("SPR_M263_INTERVAL resource=%s gap=%zu from=%zu to=%zu startA=%zu startB=%zu bitsA=%zu bitsB=%zu deltaBits=%lld\n",
                    name.c_str(),gi+1,i,i+1,offsetA,offsetB,n,m,static_cast<long long>(change));
            }
        }
    }
    return true;
}
}
int stafuSprM263AlignmentStability(const char* installed,const char* cd,const char*){
    const std::vector<std::vector<size_t>> picard={
        {5,1,2,3204,4489,3234,4526},{5,1,2,4967,5093,5004,5120},
        {5,1,2,9730,10499,9792,10603},{5,3,4,6342,6591,6464,6713}};
    const std::vector<std::vector<size_t>> data={
        {0,0,1,9973,10390,9966,10356},{0,0,1,10781,11976,10747,11952}};
    if(!load(installed,cd,"BRDGPICA.SPR",picard)||
       !load(installed,cd,"BRDGDATA.SPR",data))return 1;
    std::puts("SPR_M263_SUMMARY gaps=6 resources=2");
    std::puts("SPR_M263_NOTE Five global edit-cost models with match=0, diagonal-first ties; compare exact matched coordinate pairs against unit baseline. These are alignment sensitivities, not codec operations.");
    return 0;
}
