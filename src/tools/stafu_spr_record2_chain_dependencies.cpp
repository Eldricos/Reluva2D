// SPDX-License-Identifier: GPL-3.0-or-later
// M293: compare M282, M284, and global maximum-coverage bit chains.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
using View=reluva::stafu::SprImagePayloadAccessor::View;
struct Image { size_t ordinal=0; std::vector<uint8_t> data; };
struct Match { size_t a=0,b=0,len=0; int shift=0; };
struct Totals { size_t pairs=0,withMatches=0,matches=0,chainSegments=0,transitions=0,consistentTransitions=0; };
constexpr int kMaxShift=64;
constexpr size_t kMinRun=16;
bool isRef(const View& v){return v.marker==0x00030000&&v.size==4;}
size_t prefix(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b){
    size_t i=0;while(i<std::min(a.size(),b.size())&&a[i]==b[i])++i;return i;
}
std::vector<Match> candidates(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,size_t pre){
    std::vector<Match> found;
    for(int shift=-kMaxShift;shift<=kMaxShift;++shift){
        if(shift==0)continue;
        for(size_t i=pre;i<a.size();++i){
            const long long jSigned=static_cast<long long>(i)+shift;
            if(jSigned<static_cast<long long>(pre)||jSigned>=static_cast<long long>(b.size()))continue;
            const size_t j=static_cast<size_t>(jSigned);
            if(a[i]!=b[j])continue;
            if(i>pre&&j>pre&&a[i-1]==b[j-1])continue;
            size_t len=0;
            while(i+len<a.size()&&j+len<b.size()&&a[i+len]==b[j+len])++len;
            if(len>=kMinRun)found.push_back({i,j,len,shift});
        }
    }
    std::sort(found.begin(),found.end(),[](const Match& x,const Match& y){
        if(x.len!=y.len)return x.len>y.len;
        if(x.a!=y.a)return x.a<y.a;
        return x.b<y.b;
    });
    return found;
}
std::vector<Match> chain(const std::vector<Match>& input){
    std::vector<Match> v=input;
    std::sort(v.begin(),v.end(),[](const Match& x,const Match& y){
        if(x.a!=y.a)return x.a<y.a;
        if(x.b!=y.b)return x.b<y.b;
        return x.len>y.len;
    });
    std::vector<size_t> score(v.size());
    std::vector<int> parent(v.size(),-1);
    int best=-1;
    for(size_t i=0;i<v.size();++i){
        score[i]=v[i].len;
        for(size_t j=0;j<i;++j){
            if(v[j].a+v[j].len>v[i].a||v[j].b+v[j].len>v[i].b)continue;
            if(score[j]+v[i].len>score[i]){
                score[i]=score[j]+v[i].len;parent[i]=static_cast<int>(j);
            }
        }
        if(best<0||score[i]>score[static_cast<size_t>(best)])best=static_cast<int>(i);
    }
    std::vector<Match> out;
    while(best>=0){out.push_back(v[static_cast<size_t>(best)]);best=parent[static_cast<size_t>(best)];}
    std::reverse(out.begin(),out.end());return out;
}

bool bitAt(const std::vector<uint8_t>& data,size_t bit){
    return ((data[bit/8]>>(7-(bit%8)))&1u)!=0;
}

struct BitMatch { size_t a=0,b=0,len=0; int shift=0; };
std::vector<BitMatch> bitCandidates(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                                   size_t baseA,size_t baseB,size_t nA,size_t nB,size_t threshold){
    std::vector<BitMatch> out;
    for(int shift=-32;shift<=32;++shift){
        if(shift==0)continue;
        for(size_t i=0;i<nA;++i){
            const long long js=static_cast<long long>(i)+shift;
            if(js<0||js>=static_cast<long long>(nB))continue;
            const size_t j=static_cast<size_t>(js);
            if(bitAt(a,baseA+i)!=bitAt(b,baseB+j))continue;
            if(i>0&&j>0&&bitAt(a,baseA+i-1)==bitAt(b,baseB+j-1))continue;
            size_t len=0;
            while(i+len<nA&&j+len<nB&&bitAt(a,baseA+i+len)==bitAt(b,baseB+j+len))++len;
            if(len>=threshold)out.push_back({i,j,len,shift});
        }
    }
    std::sort(out.begin(),out.end(),[](const BitMatch& x,const BitMatch& y){
        if(x.a!=y.a)return x.a<y.a;
        if(x.b!=y.b)return x.b<y.b;
        return x.len>y.len;
    });
    return out;
}

std::vector<BitMatch> globalCandidates(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                                      size_t baseA,size_t baseB,size_t nA,size_t nB,
                                      size_t threshold,int radius){
    std::vector<BitMatch> out;
    for(int shift=-radius;shift<=radius;++shift){
        for(size_t i=0;i<nA;++i){
            const long long js=static_cast<long long>(i)+shift;
            if(js<0||js>=static_cast<long long>(nB))continue;
            const size_t j=static_cast<size_t>(js);
            if(bitAt(a,baseA+i)!=bitAt(b,baseB+j))continue;
            if(i>0&&j>0&&bitAt(a,baseA+i-1)==bitAt(b,baseB+j-1))continue;
            size_t len=0;
            while(i+len<nA&&j+len<nB&&bitAt(a,baseA+i+len)==bitAt(b,baseB+j+len))++len;
            if(len>=threshold)out.push_back({i,j,len,shift});
        }
    }
    std::sort(out.begin(),out.end(),[](const BitMatch& x,const BitMatch& y){
        if(x.a!=y.a)return x.a<y.a;
        if(x.b!=y.b)return x.b<y.b;
        return x.len>y.len;
    });
    return out;
}
std::vector<BitMatch> bitChain(const std::vector<BitMatch>& v){
    std::vector<size_t> score(v.size(),0);
    std::vector<int> parent(v.size(),-1);
    int best=-1;
    for(size_t i=0;i<v.size();++i){
        score[i]=v[i].len;
        for(size_t j=0;j<i;++j){
            if(v[j].a+v[j].len>v[i].a||v[j].b+v[j].len>v[i].b)continue;
            if(score[j]+v[i].len>score[i]){
                score[i]=score[j]+v[i].len;parent[i]=static_cast<int>(j);
            }
        }
        if(best<0||score[i]>score[static_cast<size_t>(best)])best=static_cast<int>(i);
    }
    std::vector<BitMatch> out;
    while(best>=0){out.push_back(v[static_cast<size_t>(best)]);best=parent[static_cast<size_t>(best)];}
    std::reverse(out.begin(),out.end());return out;
}

std::string bits(const std::vector<uint8_t>& d,size_t start,size_t count){
    std::string out;out.reserve(count);
    for(size_t i=0;i<count;++i)out.push_back(bitAt(d,start+i)?'1':'0');
    return out;
}


size_t coverage(const std::vector<BitMatch>& v){
    size_t n=0;for(const auto& m:v)n+=m.len;return n;
}
bool containsAnchor(const std::vector<BitMatch>& chain,const BitMatch& anchor){
    for(const auto& m:chain)
        if(m.a<=anchor.a&&m.b<=anchor.b&&m.a+m.len>=anchor.a+anchor.len&&m.b+m.len>=anchor.b+anchor.len
           &&static_cast<long long>(m.b)-static_cast<long long>(m.a)==static_cast<long long>(anchor.b)-static_cast<long long>(anchor.a))
            return true;
    return false;
}

size_t transitionsOf(const std::vector<BitMatch>& v){
    size_t n=0;for(size_t i=1;i<v.size();++i)if(v[i].shift!=v[i-1].shift)++n;return n;
}
bool compatible(const BitMatch& x,const BitMatch& y){
    if(x.a==y.a&&x.b==y.b&&x.len==y.len)return true;
    if(x.a<=y.a&&x.b<=y.b&&x.a+x.len>=y.a+y.len&&x.b+x.len>=y.b+y.len&&x.shift==y.shift)return true;
    if(y.a<=x.a&&y.b<=x.b&&y.a+y.len>=x.a+x.len&&y.b+y.len>=x.b+x.len&&x.shift==y.shift)return true;
    return (x.a+x.len<=y.a&&x.b+x.len<=y.b)||
           (y.a+y.len<=x.a&&y.b+y.len<=x.b);
}
struct Optimized {
    std::vector<BitMatch> runs;
    size_t retained282=0,retained284=0;
};
Optimized preserveAnchors(std::vector<BitMatch> v,
                          const std::vector<BitMatch>& primary,
                          const std::vector<BitMatch>& refined){
    // Baseline anchors remain eligible even when below the current global threshold.
    v.insert(v.end(),refined.begin(),refined.end());
    v.insert(v.end(),primary.begin(),primary.end());
    std::sort(v.begin(),v.end(),[](const BitMatch& x,const BitMatch& y){
        if(x.a!=y.a)return x.a<y.a;
        if(x.b!=y.b)return x.b<y.b;
        if(x.len!=y.len)return x.len>y.len;
        return x.shift<y.shift;
    });
    v.erase(std::unique(v.begin(),v.end(),[](const BitMatch& x,const BitMatch& y){
        return x.a==y.a&&x.b==y.b&&x.len==y.len&&x.shift==y.shift;
    }),v.end());
    struct Score {size_t m284=0,m282=0,bits=0;};
    auto better=[](const Score& a,const Score& b){
        if(a.m284!=b.m284)return a.m284>b.m284;
        if(a.m282!=b.m282)return a.m282>b.m282;
        return a.bits>b.bits;
    };
    std::vector<Score> dp(v.size());
    std::vector<int> parent(v.size(),-1);
    int best=-1;
    for(size_t i=0;i<v.size();++i){
        Score own;
        own.bits=v[i].len;
        for(const auto& a:refined)if(containsAnchor({v[i]},a))++own.m284;
        for(const auto& a:primary)if(containsAnchor({v[i]},a))++own.m282;
        dp[i]=own;
        for(size_t j=0;j<i;++j){
            if(v[j].a+v[j].len>v[i].a||v[j].b+v[j].len>v[i].b)continue;
            Score candidate={dp[j].m284+own.m284,dp[j].m282+own.m282,dp[j].bits+own.bits};
            if(better(candidate,dp[i])){dp[i]=candidate;parent[i]=static_cast<int>(j);}
        }
        if(best<0||better(dp[i],dp[static_cast<size_t>(best)]))best=static_cast<int>(i);
    }
    Optimized result;
    while(best>=0){result.runs.push_back(v[static_cast<size_t>(best)]);best=parent[static_cast<size_t>(best)];}
    std::reverse(result.runs.begin(),result.runs.end());
    for(const auto& a:primary)if(containsAnchor(result.runs,a))++result.retained282;
    for(const auto& a:refined)if(containsAnchor(result.runs,a))++result.retained284;
    return result;
}

struct Interval {size_t a0,a1,b0,b1;};
std::vector<Interval> intervals(const std::vector<BitMatch>& chain,size_t na,size_t nb){
    std::vector<Interval> out;size_t a=0,b=0;
    for(const auto& m:chain){
        if(m.a<a||m.b<b)return {};
        out.push_back({a,m.a,b,m.b});a=m.a+m.len;b=m.b+m.len;
    }
    out.push_back({a,na,b,nb});return out;
}
bool equalBits(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
               size_t ba,size_t bb,size_t n){
    for(size_t i=0;i<n;++i)if(bitAt(a,ba+i)!=bitAt(b,bb+i))return false;
    return true;
}
size_t occurrences(const std::vector<uint8_t>& hay,const std::vector<uint8_t>& needle,
                   size_t hayBase,size_t needleBase,size_t hayBits,size_t len){
    if(len>hayBits)return 0;
    size_t n=0;
    for(size_t i=0;i+len<=hayBits;++i)
        if(equalBits(hay,needle,hayBase+i,needleBase,len))++n;
    return n;
}
enum ConflictFlags : unsigned {
    overlapA=1,overlapB=2,inversion=4,sameSource=8
};
unsigned conflictFlags(const BitMatch& x,const BitMatch& y){
    unsigned f=0;
    if(x.a<y.a+y.len&&y.a<x.a+x.len)f|=overlapA;
    if(x.b<y.b+y.len&&y.b<x.b+x.len)f|=overlapB;
    if((x.a+x.len<=y.a&&x.b>=y.b+y.len)||
       (y.a+y.len<=x.a&&y.b>=x.b+x.len))f|=inversion;
    if(x.a==y.a&&x.b!=y.b)f|=sameSource;
    return f;
}
size_t contextAgreement(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                        size_t baseA,size_t baseB,size_t na,size_t nb,
                        const BitMatch& m,size_t window,bool before,size_t& compared){
    const size_t avail=before?std::min(m.a,m.b):
        std::min(na-m.a-m.len,nb-m.b-m.len);
    compared=std::min(window,avail);
    size_t agreed=0;
    const size_t pa=before?m.a-compared:m.a+m.len;
    const size_t pb=before?m.b-compared:m.b+m.len;
    for(size_t i=0;i<compared;++i)
        if(bitAt(a,baseA+pa+i)==bitAt(b,baseB+pb+i))++agreed;
    return agreed;
}
bool validateChain(const std::vector<BitMatch>& v,size_t na,size_t nb){
    size_t pa=0,pb=0;
    for(const auto& m:v){
        if(!m.len||m.a<pa||m.b<pb||m.a+m.len>na||m.b+m.len>nb)return false;
        pa=m.a+m.len;pb=m.b+m.len;
    }
    return true;
}
size_t diagnosticGaps=0,diagnosticConflicts=0,diagnosticCompeting=0,diagnosticIntervals=0;

bool m294Mode=false;

bool sameBitMatch(const BitMatch& x,const BitMatch& y);
bool m295Mode=false;
bool m296Mode=false;
bool m297Mode=false;
bool m298Mode=false;
bool m299Mode=false;
bool m300Mode=false;
bool m301Mode=false;
bool m302Mode=false;
bool m303Mode=false;
bool m304Mode=false;
bool m305Mode=false;
bool m306Mode=false;
bool m307Mode=false;
bool m308Mode=false;
bool m309Mode=false;
bool m310Mode=false;
bool m311Mode=false;
bool m312Mode=false;
bool m313Mode=false;
bool m314Mode=false;
bool m315Mode=false;
bool m316Mode=false;
bool m317Mode=false;
bool m318Mode=false;
bool m319Mode=false;
bool m320Mode=false;
bool m321Mode=false;
bool m322Mode=false;
bool m323Mode=false;
std::vector<Image> m308Records;
size_t matchingFlank(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                     size_t baseA,size_t baseB,size_t nA,size_t nB,
                     size_t atA,size_t atB,bool left,size_t cap){
    size_t count=0;
    while(count<cap){
        if(left){
            if(atA<=count||atB<=count)break;
            if(bitAt(a,baseA+atA-count-1)!=bitAt(b,baseB+atB-count-1))break;
        }else{
            if(atA+count>=nA||atB+count>=nB)break;
            if(bitAt(a,baseA+atA+count)!=bitAt(b,baseB+atB+count))break;
        }
        ++count;
    }
    return count;
}
size_t flankMismatches(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                       size_t baseA,size_t baseB,size_t nA,size_t nB,
                       size_t atA,size_t atB,bool left,size_t cap,size_t& compared){
    compared=0;size_t mismatch=0;
    for(size_t i=0;i<cap;++i){
        if(left&&(atA<=i||atB<=i))break;
        if(!left&&(atA+i>=nA||atB+i>=nB))break;
        const size_t x=left?atA-i-1:atA+i,y=left?atB-i-1:atB+i;
        mismatch+=bitAt(a,baseA+x)!=bitAt(b,baseB+y);
        ++compared;
    }
    return mismatch;
}
size_t localOccurrences(const std::vector<uint8_t>& data,size_t base,size_t length,
                        size_t start,size_t bits){
    if(bits==0||bits>length)return 0;
    size_t count=0;
    for(size_t pos=0;pos+bits<=length;++pos){
        bool equal=true;
        for(size_t i=0;i<bits;++i)
            if(bitAt(data,base+pos+i)!=bitAt(data,base+start+i)){equal=false;break;}
        if(equal)++count;
    }
    return count;
}
void boundaryEvidence(const char* name,size_t oa,size_t ob,size_t gap,
                      const char* mode,size_t parameter,
                      const std::vector<BitMatch>& baseline,
                      const std::vector<BitMatch>& alternative,
                      const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                      size_t baseA,size_t baseB,size_t nA,size_t nB){
    for(int path=0;path<2;++path){
        const auto& current=path==0?baseline:alternative;
        const auto& other=path==0?alternative:baseline;
        for(size_t i=0;i<current.size();++i){
            const auto& m=current[i];
            bool shared=false;
            for(const auto& x:other)if(sameBitMatch(m,x)){shared=true;break;}
            if(shared)continue;
            const size_t left=matchingFlank(a,b,baseA,baseB,nA,nB,m.a,m.b,true,32);
            const size_t right=matchingFlank(a,b,baseA,baseB,nA,nB,m.a+m.len,m.b+m.len,false,32);
            size_t beforeN=0,afterN=0;
            const size_t beforeBad=flankMismatches(a,b,baseA,baseB,nA,nB,m.a,m.b,true,16,beforeN);
            const size_t afterBad=flankMismatches(a,b,baseA,baseB,nA,nB,m.a+m.len,m.b+m.len,false,16,afterN);
            const size_t sample=std::min(size_t(32),m.len);
            const size_t occurrencesA=localOccurrences(a,baseA,nA,m.a,sample);
            const size_t occurrencesB=localOccurrences(b,baseB,nB,m.b,sample);
            const size_t prevEndA=i?current[i-1].a+current[i-1].len:0;
            const size_t prevEndB=i?current[i-1].b+current[i-1].len:0;
            const size_t nextA=i+1<current.size()?current[i+1].a:nA;
            const size_t nextB=i+1<current.size()?current[i+1].b:nB;
            std::printf("SPR_M295_BOUNDARY resource=%s ordinalA=%zu ordinalB=%zu gap=%zu mode=%s parameter=%zu path=%s segment=%zu bitA=%zu bitB=%zu length=%zu shift=%lld leftExact=%zu rightExact=%zu beforeCompared=%zu beforeMismatches=%zu afterCompared=%zu afterMismatches=%zu sampleBits=%zu occurrencesA=%zu occurrencesB=%zu prevGapA=%zu prevGapB=%zu nextGapA=%zu nextGapB=%zu\n",
                name,oa,ob,gap,mode,parameter,path==0?"baseline":"alternative",i+1,m.a,m.b,m.len,
                static_cast<long long>(m.b)-static_cast<long long>(m.a),left,right,
                beforeN,beforeBad,afterN,afterBad,sample,occurrencesA,occurrencesB,
                m.a-prevEndA,m.b-prevEndB,nextA-(m.a+m.len),nextB-(m.b+m.len));
        }
    }
}


struct M296Evidence {
    size_t bits=0,runs=0,uniqueRuns=0,uniqueBits=0,repeatRuns=0;
    size_t beforeBad=0,beforeN=0,afterBad=0,afterN=0;
};
M296Evidence m296Evidence(const std::vector<BitMatch>& chain,
                          const std::vector<BitMatch>& other,
                          const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                          size_t ba,size_t bb,size_t na,size_t nb,
                          const char* name,size_t oa,size_t ob,size_t gap,
                          size_t parameter,const char* path){
    M296Evidence out;
    for(size_t i=0;i<chain.size();++i){
        const auto& m=chain[i];
        bool shared=false;
        for(const auto& x:other)if(sameBitMatch(m,x)){shared=true;break;}
        if(shared)continue;
        const size_t sample=std::min(size_t(32),m.len);
        const size_t occA=localOccurrences(a,ba,na,m.a,sample);
        const size_t occB=localOccurrences(b,bb,nb,m.b,sample);
        const bool unique=occA==1&&occB==1;
        size_t beforeN=0,afterN=0;
        const size_t beforeBad=flankMismatches(a,b,ba,bb,na,nb,m.a,m.b,true,16,beforeN);
        const size_t afterBad=flankMismatches(a,b,ba,bb,na,nb,m.a+m.len,m.b+m.len,false,16,afterN);
        ++out.runs;out.bits+=m.len;
        if(unique){++out.uniqueRuns;out.uniqueBits+=m.len;}else ++out.repeatRuns;
        out.beforeN+=beforeN;out.beforeBad+=beforeBad;
        out.afterN+=afterN;out.afterBad+=afterBad;
        std::printf("SPR_M296_CANDIDATE resource=%s ordinalA=%zu ordinalB=%zu gap=%zu mode=leave_one_out parameter=%zu path=%s segment=%zu bitA=%zu bitB=%zu length=%zu sampleBits=%zu occurrencesA=%zu occurrencesB=%zu unique=%d beforeBad=%zu beforeCompared=%zu afterBad=%zu afterCompared=%zu\n",
            name,oa,ob,gap,parameter,path,i+1,m.a,m.b,m.len,sample,occA,occB,unique?1:0,
            beforeBad,beforeN,afterBad,afterN);
    }
    std::printf("SPR_M296_PATH resource=%s ordinalA=%zu ordinalB=%zu gap=%zu mode=leave_one_out parameter=%zu path=%s changedRuns=%zu changedBits=%zu uniqueRuns=%zu uniqueBits=%zu repeatedRuns=%zu beforeBad=%zu beforeCompared=%zu afterBad=%zu afterCompared=%zu\n",
        name,oa,ob,gap,parameter,path,out.runs,out.bits,out.uniqueRuns,out.uniqueBits,out.repeatRuns,
        out.beforeBad,out.beforeN,out.afterBad,out.afterN);
    return out;
}
void m296Compare(const char* name,size_t oa,size_t ob,size_t gap,size_t parameter,
                 const std::vector<BitMatch>& baseline,const std::vector<BitMatch>& alternative,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    const auto x=m296Evidence(baseline,alternative,a,b,ba,bb,na,nb,name,oa,ob,gap,parameter,"baseline");
    const auto y=m296Evidence(alternative,baseline,a,b,ba,bb,na,nb,name,oa,ob,gap,parameter,"alternative");
    // Coverage, uniqueness and mismatch counts are kept separate. In particular,
    // mismatch counts with different denominators cannot imply dominance.
    const bool comparable=x.beforeN==y.beforeN&&x.afterN==y.afterN;
    const bool xNoWorse=x.bits>=y.bits&&x.uniqueBits>=y.uniqueBits&&
        (!comparable||(x.beforeBad<=y.beforeBad&&x.afterBad<=y.afterBad));
    const bool yNoWorse=y.bits>=x.bits&&y.uniqueBits>=x.uniqueBits&&
        (!comparable||(y.beforeBad<=x.beforeBad&&y.afterBad<=x.afterBad));
    const bool xStrict=x.bits>y.bits||x.uniqueBits>y.uniqueBits||
        (comparable&&(x.beforeBad<y.beforeBad||x.afterBad<y.afterBad));
    const bool yStrict=y.bits>x.bits||y.uniqueBits>x.uniqueBits||
        (comparable&&(y.beforeBad<x.beforeBad||y.afterBad<x.afterBad));
    const char* relation=!comparable?"incomparable_sample_sizes":
        xNoWorse&&xStrict?"baseline_dominates":
        yNoWorse&&yStrict?"alternative_dominates":
        !xStrict&&!yStrict?"equal":"tradeoff";
    std::printf("SPR_M296_COMPARE resource=%s ordinalA=%zu ordinalB=%zu gap=%zu mode=leave_one_out parameter=%zu relation=%s coverageDelta=%lld uniqueBitsDelta=%lld beforeBadDelta=%lld afterBadDelta=%lld equalBoundarySamples=%d\n",
        name,oa,ob,gap,parameter,relation,
        static_cast<long long>(y.bits)-static_cast<long long>(x.bits),
        static_cast<long long>(y.uniqueBits)-static_cast<long long>(x.uniqueBits),
        static_cast<long long>(y.beforeBad)-static_cast<long long>(x.beforeBad),
        static_cast<long long>(y.afterBad)-static_cast<long long>(x.afterBad),comparable?1:0);
}


struct M297Stats {
    size_t runs=0,bits=0,beforeBad=0,beforeN=0,afterBad=0,afterN=0;
    size_t sampled[5]={},unique[5]={},repeat[5]={};
};
constexpr size_t m297Lengths[5]={16,24,32,48,64};
M297Stats m297Path(const std::vector<BitMatch>& chain,const std::vector<BitMatch>& other,
                    const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                    size_t ba,size_t bb,size_t na,size_t nb,
                    const char* name,size_t oa,size_t ob,size_t gap,size_t parameter,const char* path){
    M297Stats out;
    for(size_t i=0;i<chain.size();++i){
        const auto& m=chain[i];
        bool shared=false;
        for(const auto& x:other)if(sameBitMatch(m,x)){shared=true;break;}
        if(shared)continue;
        ++out.runs;out.bits+=m.len;
        size_t beforeN=0,afterN=0;
        const size_t beforeBad=flankMismatches(a,b,ba,bb,na,nb,m.a,m.b,true,16,beforeN);
        const size_t afterBad=flankMismatches(a,b,ba,bb,na,nb,m.a+m.len,m.b+m.len,false,16,afterN);
        out.beforeBad+=beforeBad;out.beforeN+=beforeN;
        out.afterBad+=afterBad;out.afterN+=afterN;
        for(size_t j=0;j<5;++j){
            const size_t sample=m297Lengths[j];
            if(m.len<sample)continue; // Censored lengths are not called repeated.
            const size_t occA=localOccurrences(a,ba,na,m.a,sample);
            const size_t occB=localOccurrences(b,bb,nb,m.b,sample);
            const bool unique=occA==1&&occB==1;
            ++out.sampled[j];
            if(unique)++out.unique[j];else ++out.repeat[j];
            std::printf("SPR_M297_SAMPLE resource=%s ordinalA=%zu ordinalB=%zu gap=%zu parameter=%zu path=%s segment=%zu bitA=%zu bitB=%zu matchBits=%zu sampleBits=%zu occurrencesA=%zu occurrencesB=%zu unique=%d\n",
                name,oa,ob,gap,parameter,path,i+1,m.a,m.b,m.len,sample,occA,occB,unique?1:0);
        }
    }
    std::printf("SPR_M297_PATH resource=%s ordinalA=%zu ordinalB=%zu gap=%zu parameter=%zu path=%s changedRuns=%zu changedBits=%zu beforeBad=%zu beforeCompared=%zu beforeRate=%.6f afterBad=%zu afterCompared=%zu afterRate=%.6f combinedBad=%zu combinedCompared=%zu combinedRate=%.6f\n",
        name,oa,ob,gap,parameter,path,out.runs,out.bits,
        out.beforeBad,out.beforeN,out.beforeN?double(out.beforeBad)/out.beforeN:0.0,
        out.afterBad,out.afterN,out.afterN?double(out.afterBad)/out.afterN:0.0,
        out.beforeBad+out.afterBad,out.beforeN+out.afterN,
        out.beforeN+out.afterN?double(out.beforeBad+out.afterBad)/double(out.beforeN+out.afterN):0.0);
    for(size_t j=0;j<5;++j)
        std::printf("SPR_M297_UNIQUENESS resource=%s ordinalA=%zu ordinalB=%zu gap=%zu parameter=%zu path=%s sampleBits=%zu eligible=%zu unique=%zu repeated=%zu censored=%zu\n",
            name,oa,ob,gap,parameter,path,m297Lengths[j],out.sampled[j],out.unique[j],out.repeat[j],out.runs-out.sampled[j]);
    return out;
}
void m297Compare(const char* name,size_t oa,size_t ob,size_t gap,size_t parameter,
                 const std::vector<BitMatch>& baseline,const std::vector<BitMatch>& alternative,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    const auto x=m297Path(baseline,alternative,a,b,ba,bb,na,nb,name,oa,ob,gap,parameter,"baseline");
    const auto y=m297Path(alternative,baseline,a,b,ba,bb,na,nb,name,oa,ob,gap,parameter,"alternative");
    const size_t xn=x.beforeN+x.afterN,yn=y.beforeN+y.afterN;
    const size_t xb=x.beforeBad+x.afterBad,yb=y.beforeBad+y.afterBad;
    const double xr=xn?double(xb)/xn:0.0,yr=yn?double(yb)/yn:0.0;
    const char* rate=xn==0||yn==0?"insufficient_samples":
        xr<yr?"baseline_lower":yr<xr?"alternative_lower":"equal";
    std::printf("SPR_M297_COMPARE resource=%s ordinalA=%zu ordinalB=%zu gap=%zu parameter=%zu baselineBits=%zu alternativeBits=%zu baselineBoundaryCount=%zu alternativeBoundaryCount=%zu baselineRate=%.6f alternativeRate=%.6f lowerObservedRate=%s boundaryCountsEqual=%d\n",
        name,oa,ob,gap,parameter,x.bits,y.bits,xn,yn,xr,yr,rate,x.runs==y.runs?1:0);
}


struct M298Flank {
    size_t leftBad=0,leftN=0,rightBad=0,rightN=0,runs=0,bits=0;
};
M298Flank m298Measure(const std::vector<BitMatch>& path,const std::vector<BitMatch>& other,
                     const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                     size_t ba,size_t bb,size_t na,size_t nb,size_t window){
    M298Flank out;
    for(const auto& m:path){
        bool shared=false;
        for(const auto& x:other)if(sameBitMatch(m,x)){shared=true;break;}
        if(shared)continue;
        ++out.runs;out.bits+=m.len;
        size_t n=0;
        out.leftBad+=flankMismatches(a,b,ba,bb,na,nb,m.a,m.b,true,window,n);out.leftN+=n;
        out.rightBad+=flankMismatches(a,b,ba,bb,na,nb,m.a+m.len,m.b+m.len,false,window,n);out.rightN+=n;
    }
    return out;
}
const char* m298Winner(size_t badA,size_t nA,size_t badB,size_t nB){
    if(!nA||!nB)return "insufficient_samples";
    const auto lhs=badA*nB,rhs=badB*nA; // bounded: <= 32 * changed-run count
    return lhs<rhs?"baseline_lower":lhs>rhs?"alternative_lower":"equal";
}
void m298Compare(const char* name,size_t oa,size_t ob,size_t gap,size_t parameter,
                 const std::vector<BitMatch>& baseline,const std::vector<BitMatch>& alternative,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    const size_t windows[]={8,16,24,32};
    const char* previous=nullptr;
    size_t changes=0,comparable=0;
    for(const size_t window:windows){
        const auto x=m298Measure(baseline,alternative,a,b,ba,bb,na,nb,window);
        const auto y=m298Measure(alternative,baseline,a,b,ba,bb,na,nb,window);
        const size_t xb=x.leftBad+x.rightBad,xn=x.leftN+x.rightN;
        const size_t yb=y.leftBad+y.rightBad,yn=y.leftN+y.rightN;
        const char* winner=m298Winner(xb,xn,yb,yn);
        const bool valid=xn&&yn;
        if(valid){
            if(previous&&std::strcmp(previous,winner)!=0)++changes;
            previous=winner;++comparable;
        }
        std::printf("SPR_M298_WINDOW resource=%s ordinalA=%zu ordinalB=%zu gap=%zu parameter=%zu window=%zu baselineRuns=%zu alternativeRuns=%zu baselineBits=%zu alternativeBits=%zu baselineLeft=%zu/%zu alternativeLeft=%zu/%zu leftLower=%s baselineRight=%zu/%zu alternativeRight=%zu/%zu rightLower=%s baselineCombined=%zu/%zu alternativeCombined=%zu/%zu combinedLower=%s equalLeftSamples=%d equalRightSamples=%d equalCombinedSamples=%d\n",
            name,oa,ob,gap,parameter,window,x.runs,y.runs,x.bits,y.bits,
            x.leftBad,x.leftN,y.leftBad,y.leftN,m298Winner(x.leftBad,x.leftN,y.leftBad,y.leftN),
            x.rightBad,x.rightN,y.rightBad,y.rightN,m298Winner(x.rightBad,x.rightN,y.rightBad,y.rightN),
            xb,xn,yb,yn,winner,x.leftN==y.leftN?1:0,x.rightN==y.rightN?1:0,xn==yn?1:0);
    }
    std::printf("SPR_M298_STABILITY resource=%s ordinalA=%zu ordinalB=%zu gap=%zu parameter=%zu comparableWindows=%zu preferenceTransitions=%zu firstComparable=%s lastComparable=%s\n",
        name,oa,ob,gap,parameter,comparable,changes,
        comparable?"see_window_rows":"none",previous?previous:"none");
}


struct M299Geometry {
    size_t beforeA=0,beforeB=0,afterA=0,afterB=0;
    long long enterShift=0,exitShift=0;
    size_t unmatchedTotal=0;
    size_t maxImbalance=0;
    size_t transitions=0;
    long long shiftTravel=0;
};
M299Geometry m299Geometry(const BitMatch& left,const BitMatch& middle,const BitMatch& right){
    M299Geometry g;
    g.beforeA=middle.a-(left.a+left.len);
    g.beforeB=middle.b-(left.b+left.len);
    g.afterA=right.a-(middle.a+middle.len);
    g.afterB=right.b-(middle.b+middle.len);
    g.enterShift=static_cast<long long>(middle.shift)-left.shift;
    g.exitShift=static_cast<long long>(right.shift)-middle.shift;
    g.unmatchedTotal=g.beforeA+g.beforeB+g.afterA+g.afterB;
    const auto absDiff=[](long long x){return static_cast<size_t>(x<0?-x:x);};
    g.maxImbalance=std::max(absDiff(static_cast<long long>(g.beforeB)-g.beforeA),
                            absDiff(static_cast<long long>(g.afterB)-g.afterA));
    g.transitions=(left.shift!=middle.shift?1:0)+(middle.shift!=right.shift?1:0);
    g.shiftTravel=static_cast<long long>(absDiff(g.enterShift)+absDiff(g.exitShift));
    return g;
}

struct M300Span {size_t start=0,len=0;};
size_t m300Common(const std::vector<uint8_t>& a,size_t ba,M300Span x,
                  const std::vector<uint8_t>& b,size_t bb,M300Span y,bool suffix){
    size_t n=0,cap=std::min(x.len,y.len);
    while(n<cap){
        size_t ix=suffix?x.start+x.len-1-n:x.start+n;
        size_t iy=suffix?y.start+y.len-1-n:y.start+n;
        if(bitAt(a,ba+ix)!=bitAt(b,bb+iy))break;
        ++n;
    }
    return n;
}
size_t m300PatternCount(const std::vector<uint8_t>& d,size_t base,M300Span span,size_t bits){
    if(bits==0||bits>span.len)return 0;
    size_t count=0;
    for(size_t i=0;i+bits<=span.len;++i){
        bool repeat=true;
        for(size_t j=0;j<bits;++j)
            if(bitAt(d,base+span.start+i+j)!=bitAt(d,base+span.start+j)){repeat=false;break;}
        if(repeat)++count;
    }
    return count;
}
struct M300Offset {size_t bits=0;int shift=0;size_t matches=0;size_t compared=0;};
M300Offset m300BestOffset(const std::vector<uint8_t>& a,size_t ba,M300Span x,
                          const std::vector<uint8_t>& b,size_t bb,M300Span y,int radius){
    M300Offset best;
    for(int shift=-radius;shift<=radius;++shift){
        size_t matched=0,compared=0;
        for(size_t i=0;i<x.len;++i){
            long long j=static_cast<long long>(i)+shift;
            if(j<0||j>=static_cast<long long>(y.len))continue;
            ++compared;
            matched+=bitAt(a,ba+x.start+i)==bitAt(b,bb+y.start+static_cast<size_t>(j));
        }
        if(!compared)continue;
        // Compare mismatch rate without rewarding a tiny overlapping interval.
        if(!best.compared||matched*best.compared>best.matches*compared||
           (matched*best.compared==best.matches*compared&&compared>best.compared)){
            best={std::min(x.len,y.len),shift,matched,compared};
        }
    }
    return best;
}

struct M301Score {int shift=0;size_t matches=0,compared=0;};
std::vector<M301Score> m301Offsets(const std::vector<uint8_t>& a,size_t ba,M300Span x,
                                  const std::vector<uint8_t>& b,size_t bb,M300Span y,
                                  size_t minimum){
    std::vector<M301Score> result;
    for(int shift=-32;shift<=32;++shift){
        size_t matches=0,compared=0;
        for(size_t i=0;i<x.len;++i){
            const long long j=static_cast<long long>(i)+shift;
            if(j<0||j>=static_cast<long long>(y.len))continue;
            ++compared;
            matches+=bitAt(a,ba+x.start+i)==bitAt(b,bb+y.start+static_cast<size_t>(j));
        }
        if(compared>=minimum)result.push_back({shift,matches,compared});
    }
    return result;
}
// M302: deterministic, length-preserving bit-permutation reference distribution.
// This destroys within-span order; it is a diagnostic null, not a codec model.
uint32_t m302Random(uint32_t& state){
    state^=state<<13;state^=state>>17;state^=state<<5;return state;
}
struct M302Best {M301Score score;size_t eligible=0;};
M302Best m302Best(const std::vector<uint8_t>& a,size_t ba,M300Span x,
                  const std::vector<uint8_t>& b,size_t bb,M300Span y,size_t minimum,
                  const std::vector<size_t>* permutation){
    M302Best out;
    for(int shift=-32;shift<=32;++shift){
        M301Score candidate{shift,0,0};
        for(size_t i=0;i<x.len;++i){
            const long long j=static_cast<long long>(i)+shift;
            if(j<0||j>=static_cast<long long>(y.len))continue;
            ++candidate.compared;
            const size_t bj=permutation?(*permutation)[static_cast<size_t>(j)]:static_cast<size_t>(j);
            candidate.matches+=bitAt(a,ba+x.start+i)==bitAt(b,bb+y.start+bj);
        }
        if(candidate.compared<minimum)continue;
        ++out.eligible;
        if(out.eligible==1||candidate.matches*out.score.compared>out.score.matches*candidate.compared||
           (candidate.matches*out.score.compared==out.score.matches*candidate.compared&&candidate.compared>out.score.compared))
            out.score=candidate;
    }
    return out;
}
void m302Report(const char* name,size_t oa,size_t ob,size_t gap,
                const char* path,const char* side,const char* sample,
                const std::vector<uint8_t>& a,size_t ba,M300Span x,
                const std::vector<uint8_t>& b,size_t bb,M300Span y){
    for(size_t minimum:{size_t(16),size_t(32),size_t(48)}){
        const auto observed=m302Best(a,ba,x,b,bb,y,minimum,nullptr);
        if(!observed.eligible){
            std::printf("SPR_M302_NULL resource=%s ordinalA=%zu ordinalB=%zu gap=%zu path=%s side=%s sample=%s minOverlap=%zu eligible=0 status=insufficient_overlap\n",
                        name,oa,ob,gap,path,side,sample,minimum);
            continue;
        }
        // Each replicate reoptimizes over ALL eligible offsets, not only the observed winner.
        constexpr size_t trials=256;
        size_t atLeast=0,boundary=0;
        std::vector<size_t> permutation(y.len);
        uint32_t seed=0x9e3779b9u ^ static_cast<uint32_t>(x.start*131u+y.start*17u+
                         x.len*65537u+y.len*257u+minimum*7u+(path[0]=='a'?123u:0u)+
                         (side[0]=='a'?456u:0u)+(sample[0]=='c'?789u:0u));
        if(!seed)seed=1;
        for(size_t t=0;t<trials;++t){
            for(size_t j=0;j<y.len;++j)permutation[j]=j;
            for(size_t j=y.len;j>1;--j){
                const size_t k=m302Random(seed)%j;
                std::swap(permutation[j-1],permutation[k]);
            }
            const auto nullBest=m302Best(a,ba,x,b,bb,y,minimum,&permutation);
            if(nullBest.score.matches*observed.score.compared>=
               observed.score.matches*nullBest.score.compared)++atLeast;
            if(nullBest.score.shift==-32||nullBest.score.shift==32)++boundary;
        }
        std::printf("SPR_M302_NULL resource=%s ordinalA=%zu ordinalB=%zu gap=%zu path=%s side=%s sample=%s minOverlap=%zu eligible=%zu observedShift=%d observedMatches=%zu observedCompared=%zu observedBoundary=%d nullTrials=%zu nullAtLeast=%zu empiricalTailNumerator=%zu empiricalTailDenominator=%zu nullBoundary=%zu nullMethod=deterministic_bit_permutation interpretation=exploratory_not_calibrated\n",
                    name,oa,ob,gap,path,side,sample,minimum,observed.eligible,
                    observed.score.shift,observed.score.matches,observed.score.compared,
                    (observed.score.shift==-32||observed.score.shift==32)?1:0,
                    trials,atLeast,atLeast+1,trials+1,boundary);
    }
}
void m302Compare(const char* name,size_t oa,size_t ob,size_t gap,
                 const std::vector<BitMatch>& baseline,const std::vector<BitMatch>& alternative,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    if(baseline.size()<10)return;
    size_t li=0,ri=0;bool lf=false,rf=false;
    for(size_t i=0;i<alternative.size();++i){
        if(sameBitMatch(alternative[i],baseline[7])){li=i;lf=true;}
        if(sameBitMatch(alternative[i],baseline[9])){ri=i;rf=true;}
    }
    if(!lf||!rf||ri!=li+2)return;
    const auto& left=baseline[7];const auto& right=baseline[9];
    const BitMatch mids[]={baseline[8],alternative[li+1]};
    for(int p=0;p<2;++p){
        const auto& m=mids[p];
        if(left.a+left.len>m.a||left.b+left.len>m.b||
           m.a+m.len>right.a||m.b+m.len>right.b||
           right.a+right.len>na||right.b+right.len>nb)return;
        const M300Span ax[]={{left.a+left.len,m.a-left.a-left.len},{m.a+m.len,right.a-m.a-m.len}};
        const M300Span by[]={{left.b+left.len,m.b-left.b-left.len},{m.b+m.len,right.b-m.b-m.len}};
        for(int side=0;side<2;++side){
            const char* path=p?"alternative":"baseline";
            const char* sideName=side?"after":"before";
            m302Report(name,oa,ob,gap,path,sideName,"target",a,ba,ax[side],b,bb,by[side]);
            const size_t la=ax[side].len,lb=by[side].len;
            if(la&&lb&&left.a>=la&&left.b>=lb)
                m302Report(name,oa,ob,gap,path,sideName,"control_before_left",
                           a,ba,{left.a-la,la},b,bb,{left.b-lb,lb});
            else std::printf("SPR_M302_CONTROL_SKIP path=%s side=%s reason=insufficient_pre_anchor_bits\n",path,sideName);
            if(la&&lb&&right.a+right.len+la<=na&&right.b+right.len+lb<=nb)
                m302Report(name,oa,ob,gap,path,sideName,"control_after_right",
                           a,ba,{right.a+right.len,la},b,bb,{right.b+right.len,lb});
            else std::printf("SPR_M302_CONTROL_SKIP path=%s side=%s reason=insufficient_post_anchor_bits\n",path,sideName);
        }
    }
}

// M303: structure-preserving descriptive controls. Reoptimize all eligible offsets
// under each surrogate, and compare at the observed winner's exact overlap.
void m303Report(const char* name,size_t oa,size_t ob,size_t gap,const char* path,
                const char* side,const char* sample,const std::vector<uint8_t>& a,size_t ba,
                M300Span x,const std::vector<uint8_t>& b,size_t bb,M300Span y){
    if(!x.len||!y.len)return;
    const size_t trials=256;
    for(size_t minimum:{size_t(16),size_t(32),size_t(48)}){
        const auto observed=m302Best(a,ba,x,b,bb,y,minimum,nullptr);
        if(!observed.eligible){
            std::printf("SPR_M303_CONTROL path=%s side=%s sample=%s minOverlap=%zu status=insufficient_overlap\n",
                        path,side,sample,minimum);
            continue;
        }
        const auto winner=observed.score;
        const bool edge=winner.shift==-32||winner.shift==32;
        // Match the observed overlap exactly, not merely the minimum overlap.
        const size_t overlap=winner.compared;
        const size_t startA=winner.shift<0?static_cast<size_t>(-winner.shift):0;
        const size_t startB=winner.shift>0?static_cast<size_t>(winner.shift):0;
        const size_t fixedA=startA,fixedB=startB;
        const size_t maxBlock=16;
        for(size_t block:{size_t(0),size_t(4),size_t(8),maxBlock}){
            uint32_t seed=0x85ebca6bu^static_cast<uint32_t>(x.start*131u+y.start*17u+x.len*65537u+
                         y.len*257u+minimum*7u+block*19u+(path[0]=='a'?123u:0u)+
                         (side[0]=='a'?456u:0u)+(sample[0]=='c'?789u:0u));
            if(!seed)seed=1;
            size_t nullAtLeast=0,fixedAtLeast=0,nullEdge=0,valid=0;
            std::vector<size_t> perm(y.len);
            for(size_t t=0;t<trials;++t){
                if(block==0){
                    // Circular rotation retains the entire cyclic bit ordering.
                    const size_t rotation=y.len>1?1+m302Random(seed)%(y.len-1):0;
                    for(size_t j=0;j<y.len;++j)perm[j]=(j+rotation)%y.len;
                }else{
                    // Shuffle intact blocks, including a possible short tail block.
                    const size_t count=(y.len+block-1)/block;
                    std::vector<size_t> order(count);
                    for(size_t j=0;j<count;++j)order[j]=j;
                    for(size_t j=count;j>1;--j)std::swap(order[j-1],order[m302Random(seed)%j]);
                    // Variable-length final blocks must remain valid: concatenate blocks.
                    size_t k=0;
                    for(size_t id:order)
                        for(size_t j=id*block;j<std::min(y.len,(id+1)*block);++j)perm[k++]=j;
                }
                const auto best=m302Best(a,ba,x,b,bb,y,minimum,&perm);
                if(!best.eligible)continue;
                ++valid;
                if(best.score.matches*winner.compared>=winner.matches*best.score.compared)++nullAtLeast;
                if(best.score.shift==-32||best.score.shift==32)++nullEdge;
                size_t matched=0;
                for(size_t i=0;i<overlap;++i)
                    matched+=bitAt(a,ba+x.start+fixedA+i)==
                             bitAt(b,bb+y.start+perm[fixedB+i]);
                if(matched>=winner.matches)++fixedAtLeast;
            }
            std::printf("SPR_M303_CONTROL resource=%s ordinalA=%zu ordinalB=%zu gap=%zu path=%s side=%s sample=%s minOverlap=%zu eligible=%zu observedShift=%d observedMatches=%zu observedCompared=%zu observedBoundary=%d method=%s blockBits=%zu trials=%zu valid=%zu searchedAtLeast=%zu fixedOverlapAtLeast=%zu nullBoundary=%zu interpretation=exploratory_structure_preserving\n",
                name,oa,ob,gap,path,side,sample,minimum,observed.eligible,
                winner.shift,winner.matches,winner.compared,edge?1:0,
                block==0?"circular_shift":"block_permutation",block,trials,valid,nullAtLeast,fixedAtLeast,nullEdge);
        }
    }
}


void m304Inspect(const char* name,size_t oa,size_t ob,size_t gap,const char* path,
                 const char* side,const std::vector<uint8_t>& a,size_t ba,
                 M300Span x,const std::vector<uint8_t>& b,size_t bb,M300Span y,
                 size_t na,size_t nb){
    const size_t minimum=(path[0]=='b'?32:16);
    const auto best=m302Best(a,ba,x,b,bb,y,minimum,nullptr);
    if(!best.eligible){
        std::printf("SPR_M304_SKIP path=%s side=%s reason=insufficient_overlap\\n",path,side);
        return;
    }
    const int shift=best.score.shift;
    const size_t startA=shift<0?static_cast<size_t>(-shift):0;
    const size_t startB=shift>0?static_cast<size_t>(shift):0;
    const size_t count=best.score.compared;
    const size_t posA=x.start+startA,posB=y.start+startB;
    // Extend at the same absolute displacement, not by reoptimizing.
    size_t left=0,right=0;
    while(posA>left&&posB>left&&
          bitAt(a,ba+posA-left-1)==bitAt(b,bb+posB-left-1))++left;
    while(posA+count+right<na&&posB+count+right<nb&&
          bitAt(a,ba+posA+count+right)==bitAt(b,bb+posB+count+right))++right;
    size_t longest=0,longestStart=0,run=0,runStart=0,mismatches=0,firstBad=count,lastBad=0;
    std::string mask;mask.reserve(count);
    for(size_t i=0;i<count;++i){
        const bool equal=bitAt(a,ba+posA+i)==bitAt(b,bb+posB+i);
        mask.push_back(equal?'=':'X');
        if(equal){
            if(!run)runStart=i;
            ++run;
            if(run>longest){longest=run;longestStart=runStart;}
        }else{
            ++mismatches;firstBad=std::min(firstBad,i);lastBad=i;run=0;
        }
    }
    const size_t edgeLeft=std::min(size_t(16),std::min(posA,posB));
    const size_t edgeRight=std::min(size_t(16),std::min(na-posA-count,nb-posB-count));
    std::printf("SPR_M304_ALIGN resource=%s ordinalA=%zu ordinalB=%zu gap=%zu path=%s side=%s minOverlap=%zu shift=%d boundary=%d aSpanStart=%zu aSpanBits=%zu bSpanStart=%zu bSpanBits=%zu aStart=%zu bStart=%zu compared=%zu matches=%zu mismatches=%zu longestExact=%zu longestExactAt=%zu firstMismatch=%lld lastMismatch=%lld exactLeft=%zu exactRight=%zu leftCensored=%d rightCensored=%d\\n",
        name,oa,ob,gap,path,side,minimum,shift,(shift==-32||shift==32)?1:0,
        x.start,x.len,y.start,y.len,posA,posB,count,best.score.matches,mismatches,
        longest,longestStart,mismatches?static_cast<long long>(firstBad):-1LL,
        mismatches?static_cast<long long>(lastBad):-1LL,left,right,
        (posA==left||posB==left)?1:0,
        (posA+count+right==na||posB+count+right==nb)?1:0);
    std::printf("SPR_M304_BITS path=%s side=%s a=%s b=%s mask=%s\\n",
        path,side,bits(a,ba+posA,count).c_str(),
        bits(b,bb+posB,count).c_str(),mask.c_str());
    std::printf("SPR_M304_FLANK path=%s side=%s leftA=%s leftB=%s rightA=%s rightB=%s\\n",
        path,side,bits(a,ba+posA-edgeLeft,edgeLeft).c_str(),
        bits(b,bb+posB-edgeLeft,edgeLeft).c_str(),
        bits(a,ba+posA+count,edgeRight).c_str(),
        bits(b,bb+posB+count,edgeRight).c_str());
    // Report each maximal exact island, including short islands; no interpretation
    // of a match island as a codeword or a valid decoder state.
    size_t i=0;
    while(i<count){
        if(mask[i]=='X'){++i;continue;}
        const size_t start=i;
        while(i<count&&mask[i]=='=')++i;
        std::printf("SPR_M304_ISLAND path=%s side=%s aStart=%zu bStart=%zu length=%zu touchesWindowStart=%d touchesWindowEnd=%d\\n",
            path,side,posA+start,posB+start,i-start,start==0?1:0,i==count?1:0);
    }
}

struct M305Run {size_t start=0,len=0;};
std::vector<M305Run> m305Runs(const std::vector<uint8_t>& a,size_t ba,size_t pa,
                              const std::vector<uint8_t>& b,size_t bb,size_t pb,size_t count){
    std::vector<M305Run> out;
    size_t i=0;
    while(i<count){
        if(bitAt(a,ba+pa+i)!=bitAt(b,bb+pb+i)){++i;continue;}
        const size_t start=i;
        while(i<count&&bitAt(a,ba+pa+i)==bitAt(b,bb+pb+i))++i;
        out.push_back({start,i-start});
    }
    return out;
}
void m305Inspect(const char* name,size_t oa,size_t ob,size_t gap,const char* path,
                 const char* side,const std::vector<uint8_t>& a,size_t ba,M300Span x,
                 const std::vector<uint8_t>& b,size_t bb,M300Span y,
                 size_t na,size_t nb){
    const size_t minimum=path[0]=='b'?32:16;
    const auto best=m302Best(a,ba,x,b,bb,y,minimum,nullptr);
    if(!best.eligible){
        std::printf("SPR_M305_SKIP path=%s side=%s reason=insufficient_overlap aBits=%zu bBits=%zu\\n",
                    path,side,x.len,y.len);return;
    }
    const int shift=best.score.shift;
    const size_t offA=shift<0?static_cast<size_t>(-shift):0;
    const size_t offB=shift>0?static_cast<size_t>(shift):0;
    const size_t pa=x.start+offA,pb=y.start+offB,n=best.score.compared;
    const auto runs=m305Runs(a,ba,pa,b,bb,pb,n);
    size_t longest=0,at=0,interior=0,touching=0;
    for(const auto& run:runs){
        if(run.len>longest){longest=run.len;at=run.start;}
        const bool left=run.start==0,right=run.start+run.len==n;
        if(left||right)++touching;else ++interior;
        std::printf("SPR_M305_ISLAND path=%s side=%s aStart=%zu bStart=%zu length=%zu confined=1 touchesSpanStart=%d touchesSpanEnd=%d internal=%d\\n",
                    path,side,pa+run.start,pb+run.start,run.len,left?1:0,right?1:0,
                    (!left&&!right)?1:0);
    }
    // Fixed-displacement extension outside the unmatched interval is reported
    // separately; these bits cannot count as independent unmatched evidence.
    size_t outsideLeft=0,outsideRight=0;
    while(pa>outsideLeft&&pb>outsideLeft&&
          bitAt(a,ba+pa-outsideLeft-1)==bitAt(b,bb+pb-outsideLeft-1))++outsideLeft;
    while(pa+n+outsideRight<na&&pb+n+outsideRight<nb&&
          bitAt(a,ba+pa+n+outsideRight)==bitAt(b,bb+pb+n+outsideRight))++outsideRight;
    const size_t withinLeft=std::min(outsideLeft,std::min(offA,offB));
    const size_t withinRight=std::min(outsideRight,std::min(x.len-offA-n,y.len-offB-n));
    std::printf("SPR_M305_CONFINED resource=%s ordinalA=%zu ordinalB=%zu gap=%zu path=%s side=%s shift=%d searchBoundary=%d aSpanStart=%zu aSpanEnd=%zu bSpanStart=%zu bSpanEnd=%zu overlapStartA=%zu overlapStartB=%zu overlapBits=%zu matchingBits=%zu confinedLongest=%zu confinedLongestAt=%zu islands=%zu interiorIslands=%zu edgeIslands=%zu exactLeftInside=%zu exactRightInside=%zu exactLeftOutsidePossible=%zu exactRightOutsidePossible=%zu interpretation=anchor_constrained_observation\\n",
                name,oa,ob,gap,path,side,shift,(shift==-32||shift==32)?1:0,
                x.start,x.start+x.len,y.start,y.start+y.len,pa,pb,n,best.score.matches,
                longest,at,runs.size(),interior,touching,withinLeft,withinRight,
                outsideLeft>withinLeft?outsideLeft-withinLeft:0,
                outsideRight>withinRight?outsideRight-withinRight:0);
}
void m305Compare(const char* name,size_t oa,size_t ob,size_t gap,
                 const std::vector<BitMatch>& baseline,const std::vector<BitMatch>& alternative,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    if(baseline.size()<10)return;
    size_t li=0,ri=0;bool lf=false,rf=false;
    for(size_t i=0;i<alternative.size();++i){
        if(sameBitMatch(alternative[i],baseline[7])){li=i;lf=true;}
        if(sameBitMatch(alternative[i],baseline[9])){ri=i;rf=true;}
    }
    if(!lf||!rf||ri!=li+2)return;
    const auto& left=baseline[7];const auto& right=baseline[9];
    const BitMatch mids[]={baseline[8],alternative[li+1]};
    for(int p=0;p<2;++p){
        const auto& m=mids[p];
        if(left.a+left.len>m.a||left.b+left.len>m.b||
           m.a+m.len>right.a||m.b+m.len>right.b||
           right.a+right.len>na||right.b+right.len>nb)return;
        const M300Span ax[]={{left.a+left.len,m.a-left.a-left.len},{m.a+m.len,right.a-m.a-m.len}};
        const M300Span by[]={{left.b+left.len,m.b-left.b-left.len},{m.b+m.len,right.b-m.b-m.len}};
        m305Inspect(name,oa,ob,gap,p?"alternative":"baseline","after",a,ba,ax[1],b,bb,by[1],na,nb);
        if(p)m305Inspect(name,oa,ob,gap,"alternative","before",a,ba,ax[0],b,bb,by[0],na,nb);
    }
}


size_t m306Longest(const std::vector<uint8_t>& a,size_t ba,size_t pa,
                   const std::vector<uint8_t>& b,size_t bb,size_t pb,size_t n){
    size_t best=0,run=0;
    for(size_t i=0;i<n;++i){
        if(bitAt(a,ba+pa+i)==bitAt(b,bb+pb+i)){++run;best=std::max(best,run);}
        else run=0;
    }
    return best;
}


size_t m308Count(const std::vector<uint8_t>& data,const std::vector<uint8_t>& motif,
                 size_t motifBits,size_t& first,size_t& near,size_t center){
    first=static_cast<size_t>(-1);near=0;size_t count=0;
    const size_t bits=data.size()*8;
    if(bits<motifBits)return 0;
    for(size_t at=0;at<=bits-motifBits;++at){
        bool equal=true;
        for(size_t k=0;k<motifBits;++k)
            if(bitAt(data,at+k)!=bitAt(motif,k)){equal=false;break;}
        if(!equal)continue;
        if(first==static_cast<size_t>(-1))first=at;
        ++count;
        if(at<=center+128&&center<=at+128)++near;
        if(count<=12)std::printf("SPR_M308_HIT bit=%zu length=%zu\n",at,motifBits);
    }
    return count;
}
void m308Compare(const std::vector<BitMatch>& baseline,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    if(baseline.size()<10)return;
    const auto& middle=baseline[8],&right=baseline[9];
    const size_t startA=middle.a+middle.len,startB=middle.b+middle.len;
    if(startA>right.a||startB>right.b||right.a>na||right.b>nb)return;
    const M300Span x{startA,right.a-startA},y{startB,right.b-startB};
    const auto selected=m302Best(a,ba,x,b,bb,y,32,nullptr);
    if(!selected.eligible)return;
    const int shift=selected.score.shift;
    const size_t offA=shift<0?static_cast<size_t>(-shift):0;
    const size_t offB=shift>0?static_cast<size_t>(shift):0;
    const size_t n=selected.score.compared;
    size_t run=0,best=0,bestAt=0;
    for(size_t i=0;i<n;++i){
        if(bitAt(a,ba+x.start+offA+i)==bitAt(b,bb+y.start+offB+i)){
            ++run;if(run>best){best=run;bestAt=i+1-run;}
        }else run=0;
    }
    if(best==0)return;
    const size_t bitA=ba+x.start+offA+bestAt,bitB=bb+y.start+offB+bestAt;
    std::vector<uint8_t> motif((best+7)/8,0);
    for(size_t k=0;k<best;++k)
        if(bitAt(a,bitA+k))motif[k/8]|=static_cast<uint8_t>(0x80u>>(k%8));
    std::printf("SPR_M308_MOTIF resource=BRDGRIKR.SPR ordinalA=5 ordinalB=6 gap=0 shift=%d length=%zu gapBitA=%zu gapBitB=%zu recordBitA=%zu recordBitB=%zu\n",
                shift,best,x.start+offA+bestAt,y.start+offB+bestAt,bitA,bitB);
    for(const auto& rec:m308Records){
        if(rec.ordinal<4||rec.ordinal>7)continue;
        size_t first=0,near=0;
        const size_t center=rec.ordinal==5?bitA:rec.ordinal==6?bitB:0;
        std::printf("SPR_M308_RECORD_BEGIN ordinal=%zu bits=%zu\n",rec.ordinal,rec.data.size()*8);
        const size_t hits=m308Count(rec.data,motif,best,first,near,center);
        std::printf("SPR_M308_RECORD ordinal=%zu motifBits=%zu occurrences=%zu firstBit=%lld near128=%zu center=%zu\n",
                    rec.ordinal,best,hits,first==static_cast<size_t>(-1)?-1LL:static_cast<long long>(first),
                    rec.ordinal==5||rec.ordinal==6?near:0,center);
    }
    // Count occurrences in each anchor-constrained unmatched span separately.
    for(int side=0;side<2;++side){
        const auto& data=side==0?a:b;
        const size_t base=side==0?ba:bb;
        const M300Span span=side==0?x:y;
        size_t count=0;
        for(size_t at=0;at+best<=span.len;++at){
            bool equal=true;
            for(size_t k=0;k<best;++k)
                if(bitAt(data,base+span.start+at+k)!=bitAt(motif,k)){equal=false;break;}
            if(equal)++count;
        }
        std::printf("SPR_M308_LOCAL side=%s spanStart=%zu spanBits=%zu motifBits=%zu occurrences=%zu\n",
                    side==0?"A":"B",span.start,span.len,best,count);
    }
}


struct M309Hit {size_t ordinal=0,at=0,bits=0;};
std::vector<M309Hit> m309Hits(const std::vector<uint8_t>& data,size_t ordinal,
                             const std::vector<uint8_t>& motif,size_t length){
    std::vector<M309Hit> result;
    const size_t bits=data.size()*8;
    if(length==0||bits<length)return result;
    for(size_t at=0;at<=bits-length;++at){
        size_t k=0;
        for(;k<length;++k)if(bitAt(data,at+k)!=bitAt(motif,k))break;
        if(k==length)result.push_back({ordinal,at,bits});
    }
    return result;
}
size_t m309Side(const std::vector<uint8_t>& a,size_t atA,
                const std::vector<uint8_t>& b,size_t atB,
                size_t length,bool left,size_t limit){
    size_t n=0;
    while(n<limit){
        if(left){
            if(n>=atA||n>=atB)break;
            if(bitAt(a,atA-n-1)!=bitAt(b,atB-n-1))break;
        }else{
            if(atA+length+n>=a.size()*8||atB+length+n>=b.size()*8)break;
            if(bitAt(a,atA+length+n)!=bitAt(b,atB+length+n))break;
        }
        ++n;
    }
    return n;
}
size_t m309WindowMismatches(const std::vector<uint8_t>& a,size_t atA,
                           const std::vector<uint8_t>& b,size_t atB,
                           size_t length,bool left,size_t& compared){
    compared=0;size_t errors=0;
    for(size_t k=0;k<32;++k){
        if(left){
            if(k>=atA||k>=atB)break;
            errors+=bitAt(a,atA-k-1)!=bitAt(b,atB-k-1)?1u:0u;
        }else{
            if(atA+length+k>=a.size()*8||atB+length+k>=b.size()*8)break;
            errors+=bitAt(a,atA+length+k)!=bitAt(b,atB+length+k)?1u:0u;
        }
        ++compared;
    }
    return errors;
}
void m309Compare(const std::vector<BitMatch>& baseline,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    if(baseline.size()<10)return;
    const auto& mid=baseline[8],&right=baseline[9];
    if(mid.a+mid.len>right.a||mid.b+mid.len>right.b||right.a>na||right.b>nb)return;
    const M300Span x{mid.a+mid.len,right.a-mid.a-mid.len};
    const M300Span y{mid.b+mid.len,right.b-mid.b-mid.len};
    const auto selected=m302Best(a,ba,x,b,bb,y,32,nullptr);
    if(!selected.eligible)return;
    const int shift=selected.score.shift;
    const size_t offA=shift<0?static_cast<size_t>(-shift):0;
    const size_t offB=shift>0?static_cast<size_t>(shift):0;
    size_t best=0,run=0,at=0;
    for(size_t i=0;i<selected.score.compared;++i){
        if(bitAt(a,ba+x.start+offA+i)==bitAt(b,bb+y.start+offB+i)){
            ++run;if(run>best){best=run;at=i+1-run;}
        }else run=0;
    }
    if(!best)return;
    const size_t chosenA=ba+x.start+offA+at,chosenB=bb+y.start+offB+at;
    std::vector<uint8_t> motif((best+7)/8,0);
    for(size_t k=0;k<best;++k)
        if(bitAt(a,chosenA+k))motif[k/8]|=static_cast<uint8_t>(0x80u>>(k%8));
    std::vector<M309Hit> hits;
    for(const auto& rec:m308Records){
        if(rec.ordinal<4||rec.ordinal>7)continue;
        auto h=m309Hits(rec.data,rec.ordinal,motif,best);
        hits.insert(hits.end(),h.begin(),h.end());
    }
    std::printf("SPR_M309_MOTIF length=%zu chosenA=%zu chosenB=%zu occurrences=%zu flankBits=32\n",
                best,chosenA,chosenB,hits.size());
    for(const auto& h:hits){
        size_t samePositionRecords=0;
        for(const auto& other:hits)if(other.at==h.at)++samePositionRecords;
        std::printf("SPR_M309_HIT ordinal=%zu bit=%zu fixedAcrossFour=%d positionMultiplicity=%zu selected=%d\n",
                    h.ordinal,h.at,samePositionRecords==4?1:0,samePositionRecords,
                    (h.ordinal==5&&h.at==chosenA)||(h.ordinal==6&&h.at==chosenB)?1:0);
    }
    size_t pairs=0,selectedPairs=0;
    for(size_t i=0;i<hits.size();++i)for(size_t j=i+1;j<hits.size();++j){
        const auto& h=hits[i],&k=hits[j];
        if(h.ordinal==k.ordinal)continue;
        const Image* ra=nullptr;const Image* rb=nullptr;
        for(const auto& rec:m308Records){
            if(rec.ordinal==h.ordinal)ra=&rec;
            if(rec.ordinal==k.ordinal)rb=&rec;
        }
        if(!ra||!rb)continue;
        size_t nl=0,nr=0;
        const size_t ml=m309WindowMismatches(ra->data,h.at,rb->data,k.at,best,true,nl);
        const size_t mr=m309WindowMismatches(ra->data,h.at,rb->data,k.at,best,false,nr);
        const size_t exactLeft=m309Side(ra->data,h.at,rb->data,k.at,best,true,32);
        const size_t exactRight=m309Side(ra->data,h.at,rb->data,k.at,best,false,32);
        const bool selectedPair=(h.ordinal==5&&h.at==chosenA&&k.ordinal==6&&k.at==chosenB)||
                                (k.ordinal==5&&k.at==chosenA&&h.ordinal==6&&h.at==chosenB);
        if(selectedPair)++selectedPairs;
        const bool fixed=h.at==k.at;
        std::printf("SPR_M309_PAIR ordinalA=%zu bitA=%zu ordinalB=%zu bitB=%zu fixedPosition=%d selected=%d leftMatch=%zu/%zu rightMatch=%zu/%zu exactLeft=%zu exactRight=%zu totalMatch=%zu/%zu\n",
                    h.ordinal,h.at,k.ordinal,k.at,fixed?1:0,selectedPair?1:0,
                    nl-ml,nl,nr-mr,nr,exactLeft,exactRight,nl+nr-ml-mr,nl+nr);
        ++pairs;
    }
    std::printf("SPR_M309_SUMMARY hits=%zu crossRecordPairs=%zu selectedPairs=%zu selectedA=%zu selectedB=%zu\n",
                hits.size(),pairs,selectedPairs,chosenA,chosenB);
}


void m310Compare(const std::vector<BitMatch>& baseline,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    if(baseline.size()<10)return;
    const auto& mid=baseline[8],&right=baseline[9];
    if(mid.a+mid.len>right.a||mid.b+mid.len>right.b||right.a>na||right.b>nb)return;
    const M300Span x{mid.a+mid.len,right.a-mid.a-mid.len};
    const M300Span y{mid.b+mid.len,right.b-mid.b-mid.len};
    const auto selected=m302Best(a,ba,x,b,bb,y,32,nullptr);
    if(!selected.eligible)return;
    const int shift=selected.score.shift;
    const size_t offA=shift<0?static_cast<size_t>(-shift):0;
    const size_t offB=shift>0?static_cast<size_t>(shift):0;
    size_t best=0,run=0,at=0;
    for(size_t i=0;i<selected.score.compared;++i){
        if(bitAt(a,ba+x.start+offA+i)==bitAt(b,bb+y.start+offB+i)){
            ++run;if(run>best){best=run;at=i+1-run;}
        }else run=0;
    }
    if(!best)return;
    const size_t chosenA=ba+x.start+offA+at,chosenB=bb+y.start+offB+at;
    std::vector<uint8_t> motif((best+7)/8,0);
    for(size_t k=0;k<best;++k)
        if(bitAt(a,chosenA+k))motif[k/8]|=static_cast<uint8_t>(0x80u>>(k%8));
    std::vector<M309Hit> hits;
    for(const auto& rec:m308Records){
        if(rec.ordinal<4||rec.ordinal>7)continue;
        auto h=m309Hits(rec.data,rec.ordinal,motif,best);
        hits.insert(hits.end(),h.begin(),h.end());
    }
    struct Edge {size_t ra,rb,pa,pb,left,right;bool fixed;};
    std::vector<Edge> edges;
    size_t examined=0,full=0,adjacent=0,fixed=0,selectedPerfect=0;
    for(size_t i=0;i<hits.size();++i)for(size_t j=i+1;j<hits.size();++j){
        const auto& h=hits[i],&k=hits[j];
        if(h.ordinal==k.ordinal)continue;
        const Image* ra=nullptr;const Image* rb=nullptr;
        for(const auto& rec:m308Records){
            if(rec.ordinal==h.ordinal)ra=&rec;
            if(rec.ordinal==k.ordinal)rb=&rec;
        }
        if(!ra||!rb)continue;
        ++examined;
        size_t nl=0,nr=0;
        const size_t ml=m309WindowMismatches(ra->data,h.at,rb->data,k.at,best,true,nl);
        const size_t mr=m309WindowMismatches(ra->data,h.at,rb->data,k.at,best,false,nr);
        if(nl!=32||nr!=32||ml||mr)continue;
        ++full;
        const size_t left=m309Side(ra->data,h.at,rb->data,k.at,best,true,
                                   std::min(h.at,k.at));
        const size_t right=m309Side(ra->data,h.at,rb->data,k.at,best,false,
                                    std::min(ra->data.size()*8-h.at-best,
                                             rb->data.size()*8-k.at-best));
        const bool isFixed=h.at==k.at;
        const bool isAdjacent=(h.ordinal+1==k.ordinal)||(k.ordinal+1==h.ordinal);
        const bool isSelected=(h.ordinal==5&&h.at==chosenA&&k.ordinal==6&&k.at==chosenB)||
                              (k.ordinal==5&&k.at==chosenA&&h.ordinal==6&&h.at==chosenB);
        if(isFixed)++fixed;
        if(isAdjacent)++adjacent;
        if(isSelected)++selectedPerfect;
        const size_t ordinalA=std::min(h.ordinal,k.ordinal);
        const size_t ordinalB=std::max(h.ordinal,k.ordinal);
        const size_t posA=h.ordinal<k.ordinal?h.at:k.at;
        const size_t posB=h.ordinal<k.ordinal?k.at:h.at;
        edges.push_back({ordinalA,ordinalB,posA,posB,left,right,isFixed});
        std::printf("SPR_M310_PERFECT ordinalA=%zu bitA=%zu ordinalB=%zu bitB=%zu fixedPosition=%d adjacent=%d exactLeft=%zu motif=%zu exactRight=%zu totalExact=%zu startA=%zu startB=%zu endA=%zu endB=%zu selected=%d\n",
                    ordinalA,posA,ordinalB,posB,isFixed?1:0,isAdjacent?1:0,
                    left,best,right,left+best+right,posA-left,posB-left,
                    posA+best+right,posB+best+right,isSelected?1:0);
    }
    // A chain requires consecutive record ordinals and increasing bit positions in
    // both records. It does not imply a unique source-to-destination mapping.
    size_t bestLinks=0,chains=0;
    for(const auto& first:edges){
        if(first.rb!=first.ra+1)continue;
        std::vector<Edge> path{first};
        while(true){
            const Edge* next=nullptr;
            for(const auto& e:edges){
                if(e.ra!=path.back().rb||e.rb!=e.ra+1)continue;
                if(e.pa<=path.back().pb)continue;
                if(!next||e.pa<next->pa||(e.pa==next->pa&&e.pb<next->pb))next=&e;
            }
            if(!next)break;
            path.push_back(*next);
        }
        ++chains;
        if(path.size()>bestLinks){
            bestLinks=path.size();
            std::printf("SPR_M310_CHAIN links=%zu firstRecord=%zu lastRecord=%zu",
                        path.size(),path.front().ra,path.back().rb);
            for(const auto& e:path)
                std::printf(" edge=%zu:%zu>%zu:%zu",e.ra,e.pa,e.rb,e.pb);
            std::putchar('\n');
        }
    }
    std::printf("SPR_M310_SUMMARY motifBits=%zu occurrences=%zu examinedPairs=%zu perfect64=%zu adjacentPerfect=%zu fixedPerfect=%zu selectedPerfect=%zu chainStarts=%zu longestChainLinks=%zu\n",
                best,hits.size(),examined,full,adjacent,fixed,selectedPerfect,chains,bestLinks);
}



size_t m313CommonSuffix(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b){
    const size_t limit=std::min(a.size(),b.size())*8;
    size_t matched=0;
    while(matched<limit&&bitAt(a,a.size()*8-1-matched)==bitAt(b,b.size()*8-1-matched))
        ++matched;
    return matched;
}
size_t m313CommonPrefix(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b){
    const size_t limit=std::min(a.size(),b.size())*8;
    size_t matched=0;
    while(matched<limit&&bitAt(a,matched)==bitAt(b,matched))++matched;
    return matched;
}
bool m313Selftest(){
    const std::vector<uint8_t> a{0x12,0xAB,0xCD};
    const std::vector<uint8_t> b{0x34,0xAB,0xCD};
    const std::vector<uint8_t> c{0x56,0xAB,0xCD};
    const std::vector<uint8_t> d{0x12,0xAB,0xCE};
    return m313CommonSuffix(a,b)==17&&m313CommonSuffix(b,c)==17&&
           m313CommonSuffix(a,d)==0&&m313CommonPrefix(a,d)==22&&
           m313CommonSuffix(a,a)==24&&m313CommonSuffix(a,{})==0;
}
void m313Compare(){
    std::vector<const Image*> records;
    for(const auto& rec:m308Records)
        if(rec.ordinal>=4&&rec.ordinal<=7)records.push_back(&rec);
    std::sort(records.begin(),records.end(),[](const Image* a,const Image* b){return a->ordinal<b->ordinal;});
    if(records.size()!=4||records[0]->ordinal!=4||records[1]->ordinal!=5||
       records[2]->ordinal!=6||records[3]->ordinal!=7){
        std::printf("SPR_M313_ERROR expectedFourDirectRecords=1 found=%zu\\n",records.size());
        return;
    }
    size_t suffix[4][4]={};
    size_t longest=0;
    for(size_t i=0;i<4;++i){
        const auto& a=records[i]->data;
        const size_t bits=a.size()*8;
        size_t trailingZero=0,trailingOne=0;
        while(trailingZero<bits&&!bitAt(a,bits-1-trailingZero))++trailingZero;
        while(trailingOne<bits&&bitAt(a,bits-1-trailingOne))++trailingOne;
        std::printf("SPR_M313_RECORD ordinal=%zu bits=%zu bytes=%zu terminalByte=%02X trailingZeroBits=%zu trailingOneBits=%zu\\n",
                    records[i]->ordinal,bits,a.size(),a.empty()?0u:static_cast<unsigned>(a.back()),trailingZero,trailingOne);
        for(size_t j=i+1;j<4;++j){
            const auto& b=records[j]->data;
            const size_t nb=b.size()*8;
            const size_t common=m313CommonSuffix(a,b);
            const size_t pre=m313CommonPrefix(a,b);
            suffix[i][j]=suffix[j][i]=common;
            longest=std::max(longest,common);
            const size_t startA=bits-common,startB=nb-common;
            size_t mismatch32=0,compared32=0;
            for(size_t k=0;k<32&&k<startA&&k<startB;++k){
                mismatch32+=bitAt(a,startA-1-k)!=bitAt(b,startB-1-k)?1u:0u;
                ++compared32;
            }
            size_t prefixMismatches=0;
            const size_t sharedPrefix=std::min(startA,startB);
            for(size_t k=0;k<sharedPrefix;++k)
                prefixMismatches+=bitAt(a,k)!=bitAt(b,k)?1u:0u;
            std::printf("SPR_M313_PAIR ordinalA=%zu ordinalB=%zu bitsA=%zu bitsB=%zu commonSuffix=%zu startA=%zu startB=%zu shift=%lld commonPrefix=%zu prefixCompared=%zu prefixMismatches=%zu boundaryMismatch=%d preBoundaryMismatches=%zu/%zu suffixReachesStart=%d\\n",
                        records[i]->ordinal,records[j]->ordinal,bits,nb,common,startA,startB,
                        static_cast<long long>(startB)-static_cast<long long>(startA),pre,sharedPrefix,prefixMismatches,
                        (startA>0&&startB>0)?1:0,mismatch32,compared32,
                        (startA==0||startB==0)?1:0);
        }
    }
    size_t triples=0,violations=0;
    for(size_t i=0;i<4;++i)for(size_t j=i+1;j<4;++j)for(size_t k=j+1;k<4;++k){
        const size_t lower=std::min(suffix[i][j],suffix[j][k]);
        const bool consistent=suffix[i][k]>=lower;
        if(!consistent)++violations;
        ++triples;
        std::printf("SPR_M313_TRIPLE ordinalA=%zu ordinalB=%zu ordinalC=%zu suffixAB=%zu suffixBC=%zu suffixAC=%zu nested=%d\\n",
                    records[i]->ordinal,records[j]->ordinal,records[k]->ordinal,
                    suffix[i][j],suffix[j][k],suffix[i][k],consistent?1:0);
    }
    const size_t all=std::min(std::min(suffix[0][1],suffix[0][2]),suffix[0][3]);
    std::printf("SPR_M313_SUMMARY records=4 pairs=6 triples=%zu nestingViolations=%zu commonSuffixAllFour=%zu longestPairSuffix=%zu bitOrder=MSB_first suffixAlignment=record_end\\n",
                triples,violations,all,longest);
}


bool m314Selftest(){
    const std::vector<uint8_t> a{0x12,0xAB,0xCD};
    const std::vector<uint8_t> b{0xFF,0x12,0xAB,0xCD};
    const std::vector<uint8_t> c{0x34,0xAB,0xCD};
    return m313CommonSuffix(a,b)==24&&m313CommonSuffix(a,c)==17&&
           b.size()*8-m313CommonSuffix(a,b)==8&&
           m313CommonSuffix(a,{})==0;
}
void m314Window(const char* label,const std::vector<uint8_t>& data,size_t begin,size_t end){
    const size_t bits=data.size()*8;
    begin=std::min(begin,bits);end=std::min(end,bits);
    std::printf("SPR_M314_WINDOW label=%s begin=%zu end=%zu bits=",label,begin,end);
    for(size_t i=begin;i<end;++i)std::putchar(bitAt(data,i)?'1':'0');
    std::putchar('\n');
}
void m314Compare(){
    const Image* a=nullptr;const Image* b=nullptr;const Image* c=nullptr;
    for(const auto& rec:m308Records){
        if(rec.ordinal==5)a=&rec;
        if(rec.ordinal==6)b=&rec;
        if(rec.ordinal==7)c=&rec;
    }
    if(!a||!b||!c){
        std::puts("SPR_M314_ERROR missingDirectRecord=1");
        return;
    }
    const auto& x=a->data;const auto& y=b->data;const auto& z=c->data;
    const size_t na=x.size()*8,nb=y.size()*8,nc=z.size()*8;
    const size_t suffix=m313CommonSuffix(x,y);
    const size_t sa=na-suffix,sb=nb-suffix;
    const long long shift=static_cast<long long>(sb)-static_cast<long long>(sa);
    const bool byteShift=shift%8==0;
    std::printf("SPR_M314_SUFFIX ordinalA=5 ordinalB=6 bitsA=%zu bitsB=%zu suffixBits=%zu startA=%zu startB=%zu shift=%lld shiftBytes=%lld byteAlignedShift=%d startBitA=%zu startBitB=%zu terminalBitA=%zu terminalBitB=%zu\n",
        na,nb,suffix,sa,sb,shift,shift/8,byteShift?1:0,sa%8,sb%8,na%8,nb%8);
    const size_t left=std::min(size_t(64),std::min(sa,sb));
    size_t mismatches=0;
    for(size_t i=0;i<left;++i)
        if(bitAt(x,sa-left+i)!=bitAt(y,sb-left+i))++mismatches;
    std::printf("SPR_M314_BOUNDARY beforeBits=%zu beforeMismatches=%zu immediatelyPrecedingEqual=%d suffixBytesWhole=%zu suffixRemainderBits=%zu\n",
        left,mismatches,(sa&&sb&&bitAt(x,sa-1)==bitAt(y,sb-1))?1:0,suffix/8,suffix%8);
    m314Window("record5_pre",x,sa-left,sa);
    m314Window("record6_pre",y,sb-left,sb);
    m314Window("record5_suffix_first",x,sa,std::min(na,sa+64));
    m314Window("record6_suffix_first",y,sb,std::min(nb,sb+64));
    m314Window("record5_suffix_last",x,na-std::min(size_t(64),suffix),na);
    m314Window("record6_suffix_last",y,nb-std::min(size_t(64),suffix),nb);
    // M310's longest shifted 6->7 interval: [1631,2872) vs [1582,2823).
    const size_t ia=1631,ib=1582,il=1241;
    size_t checked=0,bad=0;
    if(ia+il<=nb&&ib+il<=nc){
        checked=il;
        for(size_t k=0;k<il;++k)if(bitAt(y,ia+k)!=bitAt(z,ib+k))++bad;
    }
    const size_t suffixOverlap=sa<na&&ia+il>sb&&ia<nb?
        (std::min(nb,ia+il)>std::max(sb,ia)?std::min(nb,ia+il)-std::max(sb,ia):0):0;
    std::printf("SPR_M314_INTERNAL_PAIR ordinalA=6 ordinalB=7 startA=%zu endA=%zu startB=%zu endB=%zu length=%zu compared=%zu mismatches=%zu suffix56OverlapInRecord6=%zu suffix56Start=%zu internalEnd=%zu separationBits=%lld\n",
        ia,ia+il,ib,ib+il,il,checked,bad,suffixOverlap,sb,ia+il,
        static_cast<long long>(sb)-static_cast<long long>(ia+il));
    // M310's longest shifted 4->6 interval [1882,2816) -> [1934,2868).
    const size_t otherStart=1934,otherEnd=2868;
    const size_t otherOverlap=std::min(nb,otherEnd)>std::max(sb,otherStart)?
        std::min(nb,otherEnd)-std::max(sb,otherStart):0;
    std::printf("SPR_M314_INTERNAL_REFERENCE pair=4_to_6 record6Start=%zu record6End=%zu suffix56Overlap=%zu separationBits=%lld referenceSource=M310\n",
        otherStart,otherEnd,otherOverlap,static_cast<long long>(sb)-static_cast<long long>(otherEnd));
    std::printf("SPR_M314_SUMMARY suffixBits=%zu boundaryMismatches=%zu/%zu internal67Checked=%zu internal67Mismatches=%zu internal67Overlap=%zu internal46Overlap=%zu\n",
        suffix,mismatches,left,checked,bad,suffixOverlap,otherOverlap);
}


bool m315Selftest(){
    const std::vector<uint8_t> a{0x10,0xAA,0xCC};
    const std::vector<uint8_t> b{0x20,0xAA,0xCC};
    const std::vector<uint8_t> c{0x30,0xAA,0xCC};
    const std::vector<uint8_t> d{0x40,0xAA,0xCC};
    const std::vector<uint8_t> e{0x40,0xAA,0xCD};
    return m313CommonSuffix(a,b)==20&&m313CommonSuffix(a,c)==21&&
           m313CommonSuffix(a,d)==20&&m313CommonSuffix(a,e)==0&&
           m313CommonSuffix(a,a)==24;
}
void m315Compare(){
    const Image* rec[4]={nullptr,nullptr,nullptr,nullptr};
    for(const auto& item:m308Records)
        if(item.ordinal>=4&&item.ordinal<=7)rec[item.ordinal-4]=&item;
    for(size_t i=0;i<4;++i)if(!rec[i]){
        std::printf("SPR_M315_ERROR missingOrdinal=%zu\\n",i+4);
        return;
    }
    size_t common=rec[0]->data.size()*8;
    for(size_t i=1;i<4;++i)common=std::min(common,m313CommonSuffix(rec[0]->data,rec[i]->data));
    std::printf("SPR_M315_COMMON records=4 commonSuffixBits=%zu wholeBytes=%zu remainingBits=%zu alignment=record_end bitOrder=MSB_first\\n",
                common,common/8,common%8);
    for(size_t i=0;i<4;++i){
        const auto& data=rec[i]->data;
        const size_t n=data.size()*8,start=n-common;
        const size_t window=std::min(size_t(64),start);
        std::printf("SPR_M315_RECORD ordinal=%zu bits=%zu commonStart=%zu commonStartBitOffset=%zu unmatchedPrefixBits=%zu precedingWindowBits=%zu\\n",
                    i+4,n,start,start%8,start,window);
        const std::string label="record"+std::to_string(i+4)+"_before";
        m314Window(label.c_str(),data,start-window,start);
        const std::string terminal="record"+std::to_string(i+4)+"_terminal";
        m314Window(terminal.c_str(),data,n-std::min(size_t(64),n),n);
    }
    size_t minPair=static_cast<size_t>(-1),maxPair=0;
    for(size_t i=0;i<4;++i)for(size_t j=i+1;j<4;++j){
        const auto& a=rec[i]->data;const auto& b=rec[j]->data;
        const size_t na=a.size()*8,nb=b.size()*8;
        const size_t pair=m313CommonSuffix(a,b);
        minPair=std::min(minPair,pair);maxPair=std::max(maxPair,pair);
        const size_t sa=na-pair,sb=nb-pair;
        const size_t extra=pair-common;
        const size_t w=std::min(size_t(64),std::min(sa,sb));
        size_t mismatches=0;
        for(size_t k=0;k<w;++k)
            mismatches+=bitAt(a,sa-w+k)!=bitAt(b,sb-w+k)?1u:0u;
        std::printf("SPR_M315_PAIR ordinalA=%zu ordinalB=%zu suffixBits=%zu extraBeyondAllFour=%zu startA=%zu startB=%zu shift=%lld precedingMismatches=%zu/%zu immediateBoundaryMismatch=%d\\n",
                    i+4,j+4,pair,extra,sa,sb,
                    static_cast<long long>(sb)-static_cast<long long>(sa),
                    mismatches,w,(sa>0&&sb>0&&bitAt(a,sa-1)!=bitAt(b,sb-1))?1:0);
    }
    const size_t left=std::min(size_t(64),common);
    bool identical=true;
    for(size_t i=1;i<4;++i)
        for(size_t k=0;k<common;++k)
            if(bitAt(rec[0]->data,rec[0]->data.size()*8-common+k)!=
               bitAt(rec[i]->data,rec[i]->data.size()*8-common+k))identical=false;
    std::printf("SPR_M315_SUMMARY commonSuffixAllFour=%zu minPairSuffix=%zu maxPairSuffix=%zu verifiedAllFour=%d terminalWindowBits=%zu\\n",
                common,minPair,maxPair,identical?1:0,left);
}


bool m316Selftest(){
    const std::vector<uint8_t> a{0x12,0xAB,0xCD};
    const std::vector<uint8_t> b{0x34,0xAB,0xCD};
    const std::vector<uint8_t> c{0x12,0xAB,0xCE};
    if(m313CommonSuffix(a,b)!=17||m313CommonSuffix(a,c)!=0)return false;
    if(bits(a,7,9)!=bits(b,7,9))return false;
    if(bits(a,0,8)==bits(b,0,8))return false;
    return (24-m313CommonSuffix(a,b))%8==7;
}
void m316Compare(){
    const Image* a=nullptr;const Image* b=nullptr;
    for(const auto& item:m308Records){
        if(item.ordinal==5)a=&item;
        if(item.ordinal==6)b=&item;
    }
    if(!a||!b){std::puts("SPR_M316_ERROR missing records 5 or 6");return;}
    const auto& x=a->data;const auto& y=b->data;
    const size_t na=x.size()*8,nb=y.size()*8;
    const size_t suffix=m313CommonSuffix(x,y),sa=na-suffix,sb=nb-suffix;
    const long long shift=static_cast<long long>(sb)-static_cast<long long>(sa);
    std::printf("SPR_M316_SUFFIX ordinalA=5 ordinalB=6 bits=%zu startA=%zu startB=%zu endA=%zu endB=%zu shift=%lld startByteA=%zu startBitA=%zu startByteB=%zu startBitB=%zu wholeBytes=%zu trailingBits=%zu alignedToSameBitPhase=%d\\n",
                suffix,sa,sb,na,nb,shift,sa/8,sa%8,sb/8,sb%8,suffix/8,suffix%8,sa%8==sb%8?1:0);
    const size_t before=std::min(size_t(128),std::min(sa,sb));
    for(size_t w: {size_t(8),size_t(16),size_t(32),size_t(64),size_t(128)}){
        const size_t n=std::min(w,before);size_t bad=0;
        for(size_t k=0;k<n;++k)bad+=bitAt(x,sa-n+k)!=bitAt(y,sb-n+k);
        std::printf("SPR_M316_PRECEDING window=%zu compared=%zu mismatches=%zu exact=%d\\n",w,n,bad,bad==0?1:0);
    }
    const size_t windowStartA=sa-before,windowStartB=sb-before;
    m314Window("m316_record5_preceding",x,windowStartA,sa);
    m314Window("m316_record6_preceding",y,windowStartB,sb);
    m314Window("m316_record5_suffix_head",x,sa,std::min(na,sa+128));
    m314Window("m316_record6_suffix_head",y,sb,std::min(nb,sb+128));
    m314Window("m316_record5_suffix_tail",x,na-std::min(size_t(128),suffix),na);
    m314Window("m316_record6_suffix_tail",y,nb-std::min(size_t(128),suffix),nb);
    // Compare the terminal interval with the previously reported record 5->6
    // alternative segment-nine exact island, using absolute record bit offsets.
    const size_t islandA=1939,islandB=1840,islandLen=91;
    const size_t islandEndA=islandA+islandLen,islandEndB=islandB+islandLen;
    const bool islandInRange=islandEndA<=na&&islandEndB<=nb;
    size_t islandBad=0;
    if(islandInRange)for(size_t k=0;k<islandLen;++k)
        islandBad+=bitAt(x,islandA+k)!=bitAt(y,islandB+k);
    std::printf("SPR_M316_REFERENCE kind=m310_record5_6_alternative_segment9 startA=%zu endA=%zu startB=%zu endB=%zu shift=%lld withinBounds=%d mismatches=%zu gapA=%lld gapB=%lld shiftChange=%lld\\n",
                islandA,islandEndA,islandB,islandEndB,
                static_cast<long long>(islandB)-static_cast<long long>(islandA),
                islandInRange?1:0,islandBad,
                static_cast<long long>(sa)-static_cast<long long>(islandEndA),
                static_cast<long long>(sb)-static_cast<long long>(islandEndB),
                shift-(static_cast<long long>(islandB)-static_cast<long long>(islandA)));
    // Scan all shifts in a narrow band around the terminal alignment, restricted
    // to the preceding 128 bits, to detect other exact islands without assuming
    // that they form one uninterrupted correspondence.
    size_t best=0,bestA=0,bestB=0;
    for(int delta=-64;delta<=64;++delta){
        size_t run=0;
        for(size_t k=0;k<before;++k){
            const long long posB=static_cast<long long>(sb-before+k)+delta;
            if(posB<0||posB>=static_cast<long long>(nb)){run=0;continue;}
            if(bitAt(x,sa-before+k)==bitAt(y,static_cast<size_t>(posB))){
                ++run;if(run>best){best=run;bestA=sa-before+k+1-run;bestB=static_cast<size_t>(posB)+1-run;}
            }else run=0;
        }
    }
    std::printf("SPR_M316_NEARBY longestPrecedingRun=%zu startA=%zu startB=%zu shift=%lld suffixStartA=%zu suffixStartB=%zu\\n",
                best,bestA,bestB,static_cast<long long>(bestB)-static_cast<long long>(bestA),sa,sb);
    std::printf("SPR_M316_SUMMARY suffixBits=%zu precedingWindow=%zu nearestReferenceGapA=%lld nearestReferenceGapB=%lld codecInference=none\\n",
                suffix,before,static_cast<long long>(sa)-static_cast<long long>(islandEndA),
                static_cast<long long>(sb)-static_cast<long long>(islandEndB));
}


bool m317Equal(const std::vector<uint8_t>& a,size_t pa,
               const std::vector<uint8_t>& b,size_t pb,size_t n){
    if(pa>a.size()*8||pb>b.size()*8||n>a.size()*8-pa||n>b.size()*8-pb)return false;
    for(size_t i=0;i<n;++i)if(bitAt(a,pa+i)!=bitAt(b,pb+i))return false;
    return true;
}
bool m317Selftest(){
    const std::vector<uint8_t> a{0xA5,0x5A,0xC3};
    const std::vector<uint8_t> b{0x00,0xA5,0x5A,0xC3};
    return m317Equal(a,0,b,8,24)&&!m317Equal(a,0,b,7,24)&&
           !m317Equal(a,0,b,8,25)&&m317Equal(a,0,b,8,0);
}

bool m318Selftest(){
    const std::vector<uint8_t> a{0xA0},b{0x80};
    return m317Equal(a,0,b,0,1)&&!m317Equal(a,0,b,1,1);
}

bool m319Selftest(){
    const std::vector<uint8_t> a{0xA0},b{0x80};
    return m317Equal(a,0,b,0,1)&&!m317Equal(a,0,b,1,1);
}



std::vector<size_t> m322Candidates(const std::vector<uint8_t>&,size_t,size_t,int);

struct M323Hit {size_t start5=0,startOther=0,length=0;};
std::vector<M323Hit> m323Search(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                                size_t candidate,size_t projected,size_t radius,size_t minLength){
    const size_t na=a.size()*8,nb=b.size()*8;
    std::vector<M323Hit> hits;
    if(candidate>=na||projected>=nb)return hits;
    const size_t lo=projected>radius?projected-radius:0;
    const size_t hi=std::min(nb-1,projected+radius);
    for(size_t q=lo;q<=hi;++q){
        if(bitAt(a,candidate)!=bitAt(b,q))continue;
        size_t left=0,right=0;
        while(candidate>left&&q>left&&bitAt(a,candidate-left-1)==bitAt(b,q-left-1))++left;
        while(candidate+right+1<na&&q+right+1<nb&&
              bitAt(a,candidate+right+1)==bitAt(b,q+right+1))++right;
        const size_t len=left+1+right;
        if(len>=minLength)hits.push_back({candidate-left,q-left,len});
    }
    std::sort(hits.begin(),hits.end(),[](const M323Hit& x,const M323Hit& y){
        if(x.length!=y.length)return x.length>y.length;
        if(x.start5!=y.start5)return x.start5<y.start5;
        return x.startOther<y.startOther;
    });
    return hits;
}
bool m323Selftest(){
    const std::vector<uint8_t> a{0xA5,0x5A,0xC3},b{0x00,0xA5,0x5A,0xC3,0x00};
    const auto hits=m323Search(a,b,8,16,8,16);
    if(hits.empty()||hits[0].start5!=0||hits[0].startOther!=8||hits[0].length<24)return false;
    const std::vector<uint8_t> c{0x00},d{0xFF};
    return m323Search(c,d,3,3,3,1).empty();
}
void m323Compare(){
    const Image *r4=nullptr,*r5=nullptr,*r6=nullptr,*r7=nullptr;
    for(const auto& r:m308Records){
        if(r.ordinal==4)r4=&r;
        if(r.ordinal==5)r5=&r;
        if(r.ordinal==6)r6=&r;
        if(r.ordinal==7)r7=&r;
    }
    if(!r4||!r5||!r6||!r7){std::puts("SPR_M323_ERROR missing records 4-7");return;}
    const auto& a=r5->data;const auto& b=r6->data;
    const bool anchors=m317Equal(a,2445,b,2520,100)&&
        m317Equal(a,2557,b,2621,251)&&
        m317Equal(r4->data,1882,b,1934,934)&&
        m317Equal(r4->data,1882,r7->data,1885,934)&&
        m317Equal(b,1631,r7->data,1582,1241);
    if(!anchors||bits(a,2545,12)!="011100111000"||bitAt(b,2620)!=1){
        std::puts("SPR_M323_ERROR reference validation failed");return;
    }
    const auto candidates=m322Candidates(a,2545,2557,1);
    const struct {int ordinal;const std::vector<uint8_t>* data;size_t projected;} targets[]={
        {4,&r4->data,2568},{6,&r6->data,2620},{7,&r7->data,2571}
    };
    std::printf("SPR_M323_REFERENCE anchorsVerified=1 candidateCount=%zu searchRadius=256 minReportLength=8 strongThreshold=32\n",candidates.size());
    size_t totalStrong=0,totalUnique=0;
    for(size_t pos:candidates){
        size_t strong=0,unique=0;
        for(const auto& target:targets){
            // Projection only centers the bounded search; it is NOT a verified alignment.
            const auto hits=m323Search(a,*target.data,pos,target.projected,256,8);
            size_t best=hits.empty()?0:hits[0].length;
            size_t strongHits=0;
            for(const auto& hit:hits)if(hit.length>=32)++strongHits;
            const bool uniqueStrong=strongHits==1;
            if(strongHits)++strong;
            if(uniqueStrong)++unique;
            std::printf("SPR_M323_TARGET record5Bit=%zu otherRecord=%d projectedCenter=%zu hits8=%zu hits32=%zu bestLength=%zu uniqueStrong=%d\n",
                pos,target.ordinal,target.projected,hits.size(),strongHits,best,uniqueStrong?1:0);
            for(size_t i=0;i<std::min<size_t>(hits.size(),5);++i){
                const auto& h=hits[i];
                std::printf("SPR_M323_HIT record5Bit=%zu otherRecord=%d start5=%zu startOther=%zu length=%zu coversCandidate=1 independentlyAnchored=0\n",
                    pos,target.ordinal,h.start5,h.startOther,h.length);
            }
        }
        totalStrong+=strong;totalUnique+=unique;
        std::printf("SPR_M323_CANDIDATE record5Bit=%zu recordsWith32BitHit=%zu recordsWithUnique32BitHit=%zu eliminated=0\n",pos,strong,unique);
    }
    std::printf("SPR_M323_SUMMARY candidates=%zu strongRecordCandidatePairs=%zu uniqueStrongPairs=%zu independentlyVerifiedCandidateMappings=0 eliminatedByIndependentEvidence=0 uniqueRetained=0 inference=none\n",
        candidates.size(),totalStrong,totalUnique);
    std::puts("SPR_M323_NOTE Exhaustive bounded candidate-centered exact runs, not constrained to the old shift. Overlapping hits and repeated data are correlated; 32-bit threshold and uniqueness within search window are diagnostics, not independent proof. No candidate elimination or codec inference.");
}

std::vector<size_t> m322Candidates(const std::vector<uint8_t>& a,size_t begin,size_t end,int value){
    std::vector<size_t> out;
    for(size_t p=begin;p<end;++p)if(bitAt(a,p)==value)out.push_back(p);
    return out;
}
bool m322Selftest(){
    const std::vector<uint8_t> a{0x73,0x80}; // 011100111000
    const auto candidates=m322Candidates(a,0,12,1);
    return candidates==std::vector<size_t>({1,2,3,6,7,8}) &&
        bits(a,0,12)=="011100111000" &&
        m317Equal(a,1,a,1,5) && !m317Equal(a,0,a,1,5);
}
void m322Compare(){
    const Image *r4=nullptr,*r5=nullptr,*r6=nullptr,*r7=nullptr;
    for(const auto& r:m308Records){
        if(r.ordinal==4)r4=&r;
        if(r.ordinal==5)r5=&r;
        if(r.ordinal==6)r6=&r;
        if(r.ordinal==7)r7=&r;
    }
    if(!r4||!r5||!r6||!r7){std::puts("SPR_M322_ERROR missing records 4-7");return;}
    const auto& a=r5->data;const auto& b=r6->data;
    const bool anchors=
        m317Equal(a,2445,b,2520,100)&&m317Equal(a,2557,b,2621,251)&&
        m317Equal(r4->data,1882,b,1934,934)&&
        m317Equal(r4->data,1882,r7->data,1885,934)&&
        m317Equal(b,1631,r7->data,1582,1241);
    if(!anchors||bits(a,2545,12)!="011100111000"||bitAt(b,2620)!=1){
        std::puts("SPR_M322_ERROR anchor or gap validation failed");return;
    }
    const auto candidates=m322Candidates(a,2545,2557,bitAt(b,2620));
    const bool triple=bitAt(r4->data,2568)==bitAt(b,2620)&&
                      bitAt(r7->data,2571)==bitAt(b,2620);
    std::printf("SPR_M322_REFERENCE anchorsVerified=%d tripleGapVerified=%d gapA=%s gapB=%s candidateCount=%zu\n",
        anchors?1:0,triple?1:0,bits(a,2545,12).c_str(),bits(b,2620,1).c_str(),candidates.size());
    for(size_t pos:candidates){
        // Hypothetical mapping aligns the candidate itself with record 6 bit 2620.
        // Only the central bit is constrained by the minimum-edit scripts.
        const size_t radii[]={2,4,8,16};
        std::printf("SPR_M322_CANDIDATE record5Bit=%zu gapOffset=%zu centralValue=%d\n",
            pos,pos-2545,bitAt(a,pos));
        for(size_t radius:radii){
            const size_t startA=pos-radius,startB=2620-radius;
            const size_t count=radius*2+1;
            size_t equal=0,firstMismatch=count;
            for(size_t i=0;i<count;++i){
                if(bitAt(a,startA+i)==bitAt(b,startB+i))++equal;
                else if(firstMismatch==count)firstMismatch=i;
            }
            const size_t leftAvailable=pos-2545;
            const size_t rightAvailable=2556-pos;
            const size_t gapOnlyLeft=std::min(radius,leftAvailable);
            const size_t gapOnlyRight=std::min(radius,rightAvailable);
            std::printf("SPR_M322_CONTEXT record5Bit=%zu radius=%zu match=%zu compared=%zu firstMismatchOffset=%lld windowA=%s windowB=%s gapOnlyLeft=%zu gapOnlyRight=%zu independentlyMapped=0\n",
                pos,radius,equal,count,firstMismatch==count?-1LL:static_cast<long long>(firstMismatch)-static_cast<long long>(radius),
                bits(a,startA,count).c_str(),bits(b,startB,count).c_str(),gapOnlyLeft,gapOnlyRight);
        }
        // Count adjacent bit agreement at the hypothetical shift, without
        // converting agreement into a verified cross-record correspondence.
        size_t left=0,right=0;
        while(pos>left&&2620>left&&left<32&&
              bitAt(a,pos-left-1)==bitAt(b,2620-left-1))++left;
        while(pos+right+1<a.size()*8&&2621+right<b.size()*8&&right<32&&
              bitAt(a,pos+right+1)==bitAt(b,2621+right))++right;
        std::printf("SPR_M322_FLANK record5Bit=%zu hypotheticalLeft=%zu hypotheticalRight=%zu independentSupport=0\n",
            pos,left,right);
    }
    std::printf("SPR_M322_SUMMARY candidates=%zu eliminatedByIndependentEvidence=0 uniqueRetained=0 contextComparisonIsHypothetical=1 inference=none\n",candidates.size());
    std::puts("SPR_M322_NOTE Context similarity is descriptive: the proposed record5-to-record6 mapping is unverified before the terminal anchor. Do not rank or eliminate candidates using hypothetical flank matches.");
}

bool m321Selftest(){
    const std::vector<uint8_t> a{0xA0},b{0xA0},c{0x80};
    return m317Equal(a,0,b,0,8)&&!m317Equal(a,0,c,0,8);
}
void m321Compare(){
    const Image *r4=nullptr,*r5=nullptr,*r6=nullptr,*r7=nullptr;
    for(const auto& r:m308Records){
        if(r.ordinal==4)r4=&r;
        if(r.ordinal==5)r5=&r;
        if(r.ordinal==6)r6=&r;
        if(r.ordinal==7)r7=&r;
    }
    if(!r4||!r5||!r6||!r7){std::puts("SPR_M321_ERROR missing records 4-7");return;}
    struct Pair {const Image* x;const Image* y;size_t sx;size_t sy;size_t len;const char* label;};
    const Pair pairs[]={
        {r4,r6,1882,1934,934,"4_to_6"},
        {r4,r7,1882,1885,934,"4_to_7"},
        {r6,r7,1631,1582,1241,"6_to_7"},
        {r5,r6,2557,2621,251,"5_to_6_terminal"},
        {r5,r6,2445,2520,100,"5_to_6_nearby"}
    };
    bool verified=true;
    for(const auto& p:pairs){
        const bool ok=m317Equal(p.x->data,p.sx,p.y->data,p.sy,p.len);
        verified=verified&&ok;
        std::printf("SPR_M321_ANCHOR pair=%s startA=%zu startB=%zu length=%zu verified=%d\n",
            p.label,p.sx,p.sy,p.len,ok?1:0);
    }
    if(!verified){std::puts("SPR_M321_ERROR reference anchors failed");return;}
    // Measure maximal left extension and nearby exact runs at the SAME shift.
    // A run outside the anchor is independently observed, not a continuation.
    for(const auto& p:pairs){
        size_t left=0;
        while(left<p.sx&&left<p.sy&&
              bitAt(p.x->data,p.sx-left-1)==bitAt(p.y->data,p.sy-left-1))++left;
        const size_t window=128,limit=std::min({window,p.sx,p.sy});
        size_t best=0,bestEnd=0,cur=0,matched=0;
        for(size_t k=limit;k>0;--k){
            const size_t ax=p.sx-k,by=p.sy-k;
            const bool eq=bitAt(p.x->data,ax)==bitAt(p.y->data,by);
            matched+=eq?1u:0u;
            if(eq){++cur;if(cur>best){best=cur;bestEnd=k-1;}}
            else cur=0;
        }
        // bestEnd is a distance from the anchor to the run's final bit.
        const size_t endA=p.sx-bestEnd,endB=p.sy-bestEnd;
        std::printf("SPR_M321_PREBOUNDARY pair=%s leftExtension=%zu windowBits=%zu equalBits=%zu bestExactRun=%zu bestRunStartA=%zu bestRunEndA=%zu bestRunStartB=%zu bestRunEndB=%zu independentExtension=0\n",
            p.label,left,limit,matched,best,endA-best,endA,endB-best,endB);
        const size_t show=std::min(size_t(32),limit);
        std::printf("SPR_M321_WINDOW pair=%s bitsA=%s bitsB=%s\n",p.label,
            bits(p.x->data,p.sx-show,show).c_str(),
            bits(p.y->data,p.sy-show,show).c_str());
    }
    // The 4/6/7 anchors span record 6 positions [1934,2868).
    // They cover record 6's 2620 gap bit, but do not map record 5's gap.
    const size_t record6Gap=2620;
    const size_t pos4=record6Gap-52,pos7=record6Gap-49;
    const bool triple=record6Gap>=1934&&record6Gap<2868&&
        record6Gap>=1631&&record6Gap<2872&&
        bitAt(r4->data,pos4)==bitAt(r6->data,record6Gap)&&
        bitAt(r7->data,pos7)==bitAt(r6->data,record6Gap);
    std::printf("SPR_M321_TRIPLE_GAP record4Bit=%zu record6Bit=%zu record7Bit=%zu value4=%d value6=%d value7=%d verified=%d record5Mapped=0\n",
        pos4,record6Gap,pos7,bitAt(r4->data,pos4),bitAt(r6->data,record6Gap),
        bitAt(r7->data,pos7),triple?1:0);
    size_t candidateCount=0;
    for(size_t pos=2545;pos<2557;++pos){
        if(bitAt(r5->data,pos)!=bitAt(r6->data,record6Gap))continue;
        ++candidateCount;
        std::printf("SPR_M321_CANDIDATE record5Bit=%zu value=%d record6GapBit=%zu record5To6Verified=0 independentConstraint=0\n",
            pos,bitAt(r5->data,pos),record6Gap);
    }
    std::printf("SPR_M321_SUMMARY anchorsVerified=1 tripleGapVerified=%d candidates=%zu eliminated=0 uniquelyLocalized=0 codecInference=none\n",
        triple?1:0,candidateCount);
    std::puts("SPR_M321_NOTE A verified 4/6/7 mapping of the record-6 gap bit does not imply any verified record-5 gap mapping. Fixed-shift preboundary runs are descriptive only.");
}

bool m320Selftest(){
    const std::string gap="011100111000";
    size_t candidates=0;
    for(char c:gap)if(c=='1')++candidates;
    const std::vector<uint8_t> a{0xA5,0x5A},b{0xA5,0x5A};
    return candidates==6&&m317Equal(a,0,b,0,16)&&!m317Equal(a,0,b,1,15);
}
void m320Compare(){
    const Image *r4=nullptr,*r5=nullptr,*r6=nullptr,*r7=nullptr;
    for(const auto& r:m308Records){
        if(r.ordinal==4)r4=&r;
        if(r.ordinal==5)r5=&r;
        if(r.ordinal==6)r6=&r;
        if(r.ordinal==7)r7=&r;
    }
    if(!r4||!r5||!r6||!r7){std::puts("SPR_M320_ERROR missing records 4-7");return;}
    const auto& a=r5->data;const auto& b=r6->data;
    const size_t sa=2557,sb=2621,ea=2545,eb=2620;
    if(a.size()*8!=2808||b.size()*8!=2872||
       !m317Equal(a,2445,b,2520,100)||!m317Equal(a,sa,b,sb,251)){
        std::puts("SPR_M320_ERROR fixed M319 anchors failed");return;
    }
    const std::string gapA=bits(a,ea,sa-ea),gapB=bits(b,eb,sb-eb);
    if(gapA!="011100111000"||gapB!="1"){
        std::puts("SPR_M320_ERROR M319 gap content changed");return;
    }
    // Independently verify historical M310 cross-record intervals.
    struct Cross {const Image* x;const Image* y;size_t px;size_t py;size_t n;const char* label;};
    const Cross cross[]={
        {r4,r6,1882,1934,934,"4_to_6"},
        {r4,r7,1882,1885,934,"4_to_7"},
        {r6,r7,1631,1582,1241,"6_to_7"}
    };
    bool all=true;
    for(const auto& c:cross){
        const bool ok=m317Equal(c.x->data,c.px,c.y->data,c.py,c.n);
        all=all&&ok;
        std::printf("SPR_M320_REFERENCE pair=%s startA=%zu startB=%zu length=%zu verified=%d\n",
            c.label,c.px,c.py,c.n,ok?1:0);
    }
    // The M310 common interval ends at bit 2868 in record 6.
    // Candidate positions in record 5 are before its verified 5->6 suffix.
    const size_t commonBegin6=std::max(size_t(1934),size_t(1631));
    const size_t commonEnd6=std::min(size_t(2868),size_t(2872));
    const size_t sharedBegin6=std::max(commonBegin6,sb);
    const size_t sharedEnd6=std::min(commonEnd6,sb+251);
    const bool intersection=all&&sharedEnd6>sharedBegin6&&
        m317Equal(r4->data,sharedBegin6-52,r5->data,sharedBegin6-64,sharedEnd6-sharedBegin6)&&
        m317Equal(r5->data,sharedBegin6-64,r6->data,sharedBegin6,sharedEnd6-sharedBegin6)&&
        m317Equal(r6->data,sharedBegin6,r7->data,sharedBegin6-49,sharedEnd6-sharedBegin6);
    std::printf("SPR_M320_FOUR_WAY record6Start=%zu record6End=%zu length=%zu verified=%d\n",
        sharedBegin6,sharedEnd6,sharedEnd6-sharedBegin6,intersection?1:0);
    size_t candidates=0,covered=0;
    for(size_t pos=ea;pos<sa;++pos){
        if(bitAt(a,pos)!=bitAt(b,eb))continue;
        ++candidates;
        // Only the verified 5->6 terminal mapping licenses a 5->6 coordinate.
        const bool in56=pos>=sa&&pos<sa+251;
        const bool in4=in56&&(pos+64>=1934)&&(pos+64<2868);
        const bool in7=in56&&(pos+64>=1631)&&(pos+64<2872);
        if(in4&&in7)++covered;
        // For visibility only, inspect a hypothetical extrapolation of the
        // terminal offsets, without treating it as a verified correspondence.
        const size_t q6=pos+64,q4=q6-52,q7=q6-49;
        const bool valid=q4<r4->data.size()*8&&q6<r6->data.size()*8&&q7<r7->data.size()*8;
        const int v4=valid?bitAt(r4->data,q4):-1;
        const int v6=valid?bitAt(r6->data,q6):-1;
        const int v7=valid?bitAt(r7->data,q7):-1;
        std::printf("SPR_M320_CANDIDATE record5Bit=%zu value=1 verifiedCrossRecordCoverage=%d extrapolatedRecord4Bit=%zu extrapolatedRecord6Bit=%zu extrapolatedRecord7Bit=%zu extrapolatedValues=%d,%d,%d extrapolationIsEvidence=0\n",
            pos,(in4&&in7)?1:0,q4,q6,q7,v4,v6,v7);
    }
    std::printf("SPR_M320_SUMMARY candidates=%zu independentlyCoveredCandidates=%zu eliminatedByVerifiedCrossRecordEvidence=0 fourWayVerified=%d uniqueRetained=0 inference=none\n",
        candidates,covered,intersection?1:0);
    std::puts("SPR_M320_NOTE Records 4/7 independently verify the terminal overlap, but that overlap begins AFTER all six record-5 gap candidates. Extrapolated coordinates are diagnostic only and cannot eliminate candidates.");
}

void m319Compare(){
    const Image *a=nullptr,*b=nullptr;
    for(const auto& r:m308Records){if(r.ordinal==5)a=&r;if(r.ordinal==6)b=&r;}
    if(!a||!b){std::puts("SPR_M319_ERROR missing records 5 or 6");return;}
    const auto& x=a->data;const auto& y=b->data;
    const size_t na=x.size()*8,nb=y.size()*8,tail=m313CommonSuffix(x,y);
    const size_t sa=na-tail,sb=nb-tail,ia=2445,ib=2520,len=100;
    if(!m317Equal(x,ia,y,ib,len)||!m317Equal(x,sa,y,sb,tail)||ia+len>sa||ib+len>sb){
        std::puts("SPR_M319_ERROR reference interval verification failed");return;
    }
    const size_t ea=ia+len,eb=ib+len,ga=sa-ea,gb=sb-eb;
    std::string ax=bits(x,ea,ga),by=bits(y,eb,gb);
    std::printf("SPR_M319_BOUNDARIES priorLastA=%zu priorLastB=%zu suffixFirstA=%zu suffixFirstB=%zu gapA=%zu gapB=%zu\n",
        ea-1,eb-1,sa,sb,ga,gb);
    std::printf("SPR_M319_BITS record5=%s record6=%s\n",ax.c_str(),by.c_str());
    // Global unit-cost edit alignment of only the intervening bits.
    // Count and enumerate ALL optimal scripts (bounded to avoid exponential output).
    std::vector<std::vector<size_t>> dp(ga+1,std::vector<size_t>(gb+1,0));
    for(size_t i=0;i<=ga;++i)dp[i][0]=i;
    for(size_t j=0;j<=gb;++j)dp[0][j]=j;
    for(size_t i=1;i<=ga;++i)for(size_t j=1;j<=gb;++j)
        dp[i][j]=std::min({dp[i-1][j]+1,dp[i][j-1]+1,
            dp[i-1][j-1]+(ax[i-1]==by[j-1]?0u:1u)});
    struct Path {std::string ops;std::vector<size_t> retained;};
    std::vector<Path> paths;
    const size_t cap=100000;
    auto visit=[&](auto&& self,size_t i,size_t j,std::string rev,std::vector<size_t> kept)->void{
        if(paths.size()>=cap)return;
        if(!i&&!j){
            std::reverse(rev.begin(),rev.end());
            std::reverse(kept.begin(),kept.end());
            paths.push_back({rev,kept});return;
        }
        if(i&&j&&dp[i][j]==dp[i-1][j-1]+(ax[i-1]==by[j-1]?0u:1u)){
            auto k=kept;
            if(ax[i-1]==by[j-1])k.push_back(i-1);
            self(self,i-1,j-1,rev+(ax[i-1]==by[j-1]?'M':'S'),std::move(k));
        }
        if(i&&dp[i][j]==dp[i-1][j]+1)self(self,i-1,j,rev+'D',kept);
        if(j&&dp[i][j]==dp[i][j-1]+1)self(self,i,j-1,rev+'I',kept);
    };
    visit(visit,ga,gb,"",{});
    std::vector<size_t> keptCounts(ga,0);
    for(size_t n=0;n<paths.size();++n){
        const auto& path=paths[n];
        for(size_t k:path.retained)++keptCounts[k];
        std::printf("SPR_M319_OPTIMAL index=%zu ops=%s retainedA=",n,path.ops.c_str());
        if(path.retained.empty())std::printf("none");
        for(size_t j=0;j<path.retained.size();++j)
            std::printf("%s%zu",j?",":"",ea+path.retained[j]);
        std::printf("\n");
    }
    size_t possible=0,forced=0;
    for(size_t i=0;i<ga;++i)if(keptCounts[i]){
        ++possible;if(keptCounts[i]==paths.size()&&paths.size()<cap)++forced;
        std::printf("SPR_M319_RETAIN_CANDIDATE record5Bit=%zu value=%c optimalPaths=%zu totalEnumerated=%zu\n",
            ea+i,ax[i],keptCounts[i],paths.size());
    }
    const bool complete=paths.size()<cap;
    std::printf("SPR_M319_SUMMARY distance=%zu optimalEnumerated=%zu enumerationComplete=%d possibleRetainedPositions=%zu forcedRetainedPositions=%zu uniqueRetained=%d inference=none\n",
        dp[ga][gb],paths.size(),complete?1:0,possible,forced,
        complete&&possible==1&&forced==1?1:0);
}

void m318Compare(){
    const Image *a=nullptr,*b=nullptr;
    for(const auto& r:m308Records){if(r.ordinal==5)a=&r;if(r.ordinal==6)b=&r;}
    if(!a||!b){std::puts("SPR_M318_ERROR missing records 5 or 6");return;}
    const auto& x=a->data;const auto& y=b->data;
    const size_t na=x.size()*8,nb=y.size()*8;
    const size_t suffix=m313CommonSuffix(x,y),sa=na-suffix,sb=nb-suffix;
    const size_t ia=2445,ib=2520,il=100,ea=ia+il,eb=ib+il;
    if(ea>sa||eb>sb||!m317Equal(x,ia,y,ib,il)||!m317Equal(x,sa,y,sb,suffix)){
        std::puts("SPR_M318_ERROR reference intervals invalid");return;
    }
    const size_t ga=sa-ea,gb=sb-eb;
    std::printf("SPR_M318_GEOMETRY recordA=5 recordB=6 nearbyEndA=%zu nearbyEndB=%zu suffixStartA=%zu suffixStartB=%zu gapA=%zu gapB=%zu shiftBefore=%lld shiftAfter=%lld\n",
        ea,eb,sa,sb,ga,gb,static_cast<long long>(eb)-static_cast<long long>(ea),
        static_cast<long long>(sb)-static_cast<long long>(sa));
    m314Window("m318_gap_record5",x,ea,sa);
    m314Window("m318_gap_record6",y,eb,sb);
    m314Window("m318_context_record5",x,ea-32,std::min(na,sa+32));
    m314Window("m318_context_record6",y,eb-32,std::min(nb,sb+32));
    // Needleman-Wunsch unit-cost edit distance on the fixed intervening bits.
    // This describes one possible bit edit script; it does not imply a codec operation.
    std::vector<std::vector<size_t>> dp(ga+1,std::vector<size_t>(gb+1));
    for(size_t i=0;i<=ga;++i)dp[i][0]=i;
    for(size_t j=0;j<=gb;++j)dp[0][j]=j;
    for(size_t i=1;i<=ga;++i)for(size_t j=1;j<=gb;++j){
        const size_t cost=bitAt(x,ea+i-1)==bitAt(y,eb+j-1)?0:1;
        dp[i][j]=std::min({dp[i-1][j]+1,dp[i][j-1]+1,dp[i-1][j-1]+cost});
    }
    size_t i=ga,j=gb,kept=0,substituted=0,deleted=0,inserted=0;
    std::string ops;
    while(i||j){
        if(i&&j&&dp[i][j]==dp[i-1][j-1]+(bitAt(x,ea+i-1)==bitAt(y,eb+j-1)?0u:1u)){
            const bool same=bitAt(x,ea+i-1)==bitAt(y,eb+j-1);
            ops.push_back(same?'M':'S');same?++kept:++substituted;--i;--j;
        }else if(i&&dp[i][j]==dp[i-1][j]+1){ops.push_back('D');++deleted;--i;}
        else {ops.push_back('I');++inserted;--j;}
    }
    std::reverse(ops.begin(),ops.end());
    std::printf("SPR_M318_GAP_EDIT distance=%zu kept=%zu substitutions=%zu deletions=%zu insertions=%zu operations=%s tieBreak=diagonal_first\n",
        dp[ga][gb],kept,substituted,deleted,inserted,ops.c_str());
    // For every intermediate alignment, compare the gap plus 32-bit flanks
    // without moving either verified anchor. Expose mismatches and longest runs.
    for(long long shift=static_cast<long long>(eb)-static_cast<long long>(ea);
        shift>=static_cast<long long>(sb)-static_cast<long long>(sa);--shift){
        size_t compared=0,bad=0,longest=0,run=0;
        const size_t begin=ea>=32?ea-32:0;
        const size_t end=std::min(na,sa+32);
        for(size_t pos=begin;pos<end;++pos){
            const long long q=static_cast<long long>(pos)+shift;
            if(q<0||q>=static_cast<long long>(nb))continue;
            ++compared;
            if(bitAt(x,pos)==bitAt(y,static_cast<size_t>(q))){++run;longest=std::max(longest,run);}
            else {++bad;run=0;}
        }
        std::printf("SPR_M318_SHIFT shift=%lld compared=%zu mismatches=%zu longestExactRun=%zu fixedWindowA=%zu:%zu\n",
            shift,compared,bad,longest,begin,end);
    }
    // A gap-only edit distance is bounded below by the length difference.
    std::printf("SPR_M318_SUMMARY gapA=%zu gapB=%zu editDistance=%zu lengthDifference=%zu lowerBoundAttained=%d suffixBits=%zu note=descriptive_not_decoder_evidence\n",
        ga,gb,dp[ga][gb],ga>gb?ga-gb:gb-ga,dp[ga][gb]==(ga>gb?ga-gb:gb-ga)?1:0,suffix);
}

void m317Compare(){
    const Image* a=nullptr;const Image* b=nullptr;
    for(const auto& r:m308Records){if(r.ordinal==5)a=&r;if(r.ordinal==6)b=&r;}
    if(!a||!b){std::puts("SPR_M317_ERROR missing records 5 or 6");return;}
    const auto& x=a->data;const auto& y=b->data;
    const size_t na=x.size()*8,nb=y.size()*8;
    const size_t suffix=m313CommonSuffix(x,y),sa=na-suffix,sb=nb-suffix;
    // The nearby interval was observed in M316, not inferred from this routine.
    const size_t ia=2445,ib=2520,il=100;
    const bool nearby=m317Equal(x,ia,y,ib,il);
    const bool terminal=m317Equal(x,sa,y,sb,suffix);
    const bool ordered=ia+il<=sa&&ib+il<=sb;
    const size_t ga=ordered?sa-(ia+il):0,gb=ordered?sb-(ib+il):0;
    std::printf("SPR_M317_INTERVAL kind=nearby startA=%zu endA=%zu startB=%zu endB=%zu length=%zu shift=%lld verified=%d\n",
                ia,ia+il,ib,ib+il,il,static_cast<long long>(ib)-static_cast<long long>(ia),nearby?1:0);
    std::printf("SPR_M317_INTERVAL kind=terminal startA=%zu endA=%zu startB=%zu endB=%zu length=%zu shift=%lld verified=%d\n",
                sa,na,sb,nb,suffix,static_cast<long long>(sb)-static_cast<long long>(sa),terminal?1:0);
    std::printf("SPR_M317_TRANSITION ordered=%d gapA=%zu gapB=%zu gapDifference=%lld shiftChange=%lld\n",
                ordered?1:0,ga,gb,static_cast<long long>(gb)-static_cast<long long>(ga),
                (static_cast<long long>(sb)-static_cast<long long>(sa))-
                (static_cast<long long>(ib)-static_cast<long long>(ia)));
    if(ordered){
        m314Window("m317_gap_record5",x,ia+il,sa);
        m314Window("m317_gap_record6",y,ib+il,sb);
        m314Window("m317_transition_record5",x,ia+il>=16?ia+il-16:0,std::min(na,sa+16));
        m314Window("m317_transition_record6",y,ib+il>=16?ib+il-16:0,std::min(nb,sb+16));
    }
    // Count exact 100-bit matches at every possible position in record 6.
    // This is exhaustive for the fixed record-5 window, not for arbitrary windows.
    size_t all=0,local=0;size_t first=0,last=0;
    for(size_t pos=0;pos+il<=nb;++pos){
        if(!m317Equal(x,ia,y,pos,il))continue;
        if(all==0)first=pos;last=pos;++all;
        if(pos>=ib-128&&pos<=ib+128){
            ++local;
            std::printf("SPR_M317_COMPETITOR startA=%zu startB=%zu shift=%lld length=%zu\n",
                        ia,pos,static_cast<long long>(pos)-static_cast<long long>(ia),il);
        }
    }
    // Test exact extensions at the two reported alignments, with no overlap assumptions.
    for(int which=0;which<2;++which){
        const size_t pa=which?sa:ia,pb=which?sb:ib,len=which?suffix:il;
        size_t left=0,right=0;
        while(pa>left&&pb>left&&bitAt(x,pa-left-1)==bitAt(y,pb-left-1))++left;
        while(pa+len+right<na&&pb+len+right<nb&&
              bitAt(x,pa+len+right)==bitAt(y,pb+len+right))++right;
        std::printf("SPR_M317_EXTENSION kind=%s left=%zu right=%zu maximalStartA=%zu maximalStartB=%zu maximalLength=%zu\n",
                    which?"terminal":"nearby",left,right,pa-left,pb-left,len+left+right);
    }
    // Enumerate alternative exact runs >=16 bits for shifts near the transition,
    // confined to a fixed A window [2400,sa). These can overlap one another.
    size_t candidateCount=0,best=0;long long bestShift=0;
    const size_t begin=std::min(size_t(2400),sa);
    for(long long shift=static_cast<long long>(ib)-static_cast<long long>(ia)-32;
        shift<=static_cast<long long>(ib)-static_cast<long long>(ia)+32;++shift){
        size_t run=0;
        for(size_t pos=begin;pos<sa;++pos){
            const long long q=static_cast<long long>(pos)+shift;
            if(q>=0&&q<static_cast<long long>(nb)&&bitAt(x,pos)==bitAt(y,static_cast<size_t>(q))){
                ++run;
            }else{
                if(run>=16){++candidateCount;if(run>best){best=run;bestShift=shift;}}
                run=0;
            }
        }
        if(run>=16){++candidateCount;if(run>best){best=run;bestShift=shift;}}
    }
    std::printf("SPR_M317_SUMMARY nearbyVerified=%d terminalVerified=%d exact100Occurrences=%zu localOccurrences=%zu firstPosition=%zu lastPosition=%zu candidateRuns16=%zu bestRun=%zu bestShift=%lld codecInference=none\n",
                nearby?1:0,terminal?1:0,all,local,first,last,candidateCount,best,bestShift);
}

struct M311Interval {
    size_t ra,rb,sa,sb,length,observations;
};
void m311Compare(const std::vector<BitMatch>& baseline,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    if(baseline.size()<10)return;
    const auto& mid=baseline[8],&right=baseline[9];
    if(mid.a+mid.len>right.a||mid.b+mid.len>right.b||right.a>na||right.b>nb)return;
    const M300Span x{mid.a+mid.len,right.a-mid.a-mid.len};
    const M300Span y{mid.b+mid.len,right.b-mid.b-mid.len};
    const auto selected=m302Best(a,ba,x,b,bb,y,32,nullptr);
    if(!selected.eligible)return;
    const int shift=selected.score.shift;
    const size_t offA=shift<0?static_cast<size_t>(-shift):0;
    const size_t offB=shift>0?static_cast<size_t>(shift):0;
    size_t best=0,run=0,at=0;
    for(size_t i=0;i<selected.score.compared;++i){
        if(bitAt(a,ba+x.start+offA+i)==bitAt(b,bb+y.start+offB+i)){
            ++run;if(run>best){best=run;at=i+1-run;}
        }else run=0;
    }
    if(!best)return;
    const size_t chosenA=ba+x.start+offA+at,chosenB=bb+y.start+offB+at;
    std::vector<uint8_t> motif((best+7)/8,0);
    for(size_t k=0;k<best;++k)
        if(bitAt(a,chosenA+k))motif[k/8]|=static_cast<uint8_t>(0x80u>>(k%8));
    std::vector<M309Hit> hits;
    for(const auto& rec:m308Records){
        if(rec.ordinal<4||rec.ordinal>7)continue;
        auto h=m309Hits(rec.data,rec.ordinal,motif,best);
        hits.insert(hits.end(),h.begin(),h.end());
    }
    std::vector<M311Interval> intervals;
    size_t examined=0,qualified=0;
    for(size_t i=0;i<hits.size();++i)for(size_t j=i+1;j<hits.size();++j){
        const auto& h=hits[i],&k=hits[j];
        if(h.ordinal==k.ordinal)continue;
        const Image* ra=nullptr;const Image* rb=nullptr;
        for(const auto& rec:m308Records){
            if(rec.ordinal==h.ordinal)ra=&rec;
            if(rec.ordinal==k.ordinal)rb=&rec;
        }
        if(!ra||!rb)continue;
        ++examined;
        const size_t left=m309Side(ra->data,h.at,rb->data,k.at,best,true,std::min(h.at,k.at));
        const size_t right=m309Side(ra->data,h.at,rb->data,k.at,best,false,
            std::min(ra->data.size()*8-h.at-best,rb->data.size()*8-k.at-best));
        const size_t length=left+best+right;
        if(length<64)continue;
        ++qualified;
        const bool ordered=h.ordinal<k.ordinal;
        const M311Interval candidate={
            ordered?h.ordinal:k.ordinal,ordered?k.ordinal:h.ordinal,
            (ordered?h.at:k.at)-left,(ordered?k.at:h.at)-left,length,1};
        bool found=false;
        for(auto& v:intervals){
            if(v.ra==candidate.ra&&v.rb==candidate.rb&&v.sa==candidate.sa&&
               v.sb==candidate.sb&&v.length==candidate.length){
                ++v.observations;found=true;break;
            }
        }
        if(!found)intervals.push_back(candidate);
    }
    std::sort(intervals.begin(),intervals.end(),[](const M311Interval& l,const M311Interval& r){
        if(l.length!=r.length)return l.length>r.length;
        if(l.ra!=r.ra)return l.ra<r.ra;
        if(l.rb!=r.rb)return l.rb<r.rb;
        if(l.sa!=r.sa)return l.sa<r.sa;
        return l.sb<r.sb;
    });
    size_t prefixes=0,fixed=0,shifted=0,overlap9=0;
    const size_t segA=ba+baseline[8].a,segB=bb+baseline[8].b;
    const size_t segEndA=segA+baseline[8].len,segEndB=segB+baseline[8].len;
    for(size_t i=0;i<intervals.size();++i){
        const auto& v=intervals[i];
        const bool prefix=v.sa==0&&v.sb==0;
        const bool fixedPosition=v.sa==v.sb;
        const bool intersectsA=(v.ra==5&&v.sa<segEndA&&segA<v.sa+v.length)||
                               (v.rb==5&&v.sb<segEndA&&segA<v.sb+v.length);
        const bool intersectsB=(v.ra==6&&v.sa<segEndB&&segB<v.sa+v.length)||
                               (v.rb==6&&v.sb<segEndB&&segB<v.sb+v.length);
        const bool intersects=intersectsA||intersectsB;
        if(prefix)++prefixes;
        else if(fixedPosition)++fixed;
        else ++shifted;
        if(intersects)++overlap9;
        std::printf("SPR_M311_INTERVAL rank=%zu ordinalA=%zu ordinalB=%zu startA=%zu endA=%zu startB=%zu endB=%zu length=%zu shift=%lld class=%s observations=%zu segment9Intersect=%d segment9SideA=%d segment9SideB=%d\n",
            i+1,v.ra,v.rb,v.sa,v.sa+v.length,v.sb,v.sb+v.length,v.length,
            static_cast<long long>(v.sb)-static_cast<long long>(v.sa),
            prefix?"prefix":fixedPosition?"fixed_internal":"shifted_internal",
            v.observations,intersects?1:0,intersectsA?1:0,intersectsB?1:0);
    }
    if(m312Mode){
        size_t reported=0;
        for(size_t i=0;i<intervals.size()&&reported<3;++i){
            const auto& v=intervals[i];
            if(v.sa==v.sb)continue;
            const Image* ra=nullptr;const Image* rb=nullptr;
            for(const auto& rec:m308Records){
                if(rec.ordinal==v.ra)ra=&rec;
                if(rec.ordinal==v.rb)rb=&rec;
            }
            if(!ra||!rb)continue;
            ++reported;
            const size_t bitsA=ra->data.size()*8,bitsB=rb->data.size()*8;
            const size_t endA=v.sa+v.length,endB=v.sb+v.length;
            for(int side=0;side<2;++side){
                const bool before=side==0;
                const size_t availableA=before?v.sa:bitsA-endA;
                const size_t availableB=before?v.sb:bitsB-endB;
                const size_t compared=std::min(size_t(32),std::min(availableA,availableB));
                size_t mismatches=0;
                unsigned long long sigA=0,sigB=0;
                for(size_t k=0;k<compared;++k){
                    const size_t pA=before?v.sa-compared+k:endA+k;
                    const size_t pB=before?v.sb-compared+k:endB+k;
                    const bool baBit=bitAt(ra->data,pA),bbBit=bitAt(rb->data,pB);
                    sigA=(sigA<<1)|static_cast<unsigned>(baBit);
                    sigB=(sigB<<1)|static_cast<unsigned>(bbBit);
                    mismatches+=baBit!=bbBit;
                }
                const bool edge=availableA==0||availableB==0;
                const bool immediateEqual=!edge&&
                    bitAt(ra->data,before?v.sa-1:endA)==bitAt(rb->data,before?v.sb-1:endB);
                std::printf("SPR_M312_BOUNDARY rank=%zu ordinalA=%zu ordinalB=%zu side=%s startA=%zu endA=%zu startB=%zu endB=%zu length=%zu shift=%lld edge=%d immediateEqual=%d contextBits=%zu contextMismatches=%zu contextA=0x%08llX contextB=0x%08llX\n",
                    i+1,v.ra,v.rb,before?"left":"right",v.sa,endA,v.sb,endB,v.length,
                    static_cast<long long>(v.sb)-static_cast<long long>(v.sa),
                    edge?1:0,immediateEqual?1:0,compared,mismatches,sigA,sigB);
            }
            // Nearest other qualifying interval for the same record pair, in each
            // direction in A. This is descriptive: intervals can overlap or cross.
            for(int side=0;side<2;++side){
                const bool before=side==0;
                const M311Interval* neighbor=nullptr;
                size_t distance=static_cast<size_t>(-1);
                for(const auto& w:intervals){
                    if(w.ra!=v.ra||w.rb!=v.rb||(&w==&v))continue;
                    if(before){
                        if(w.sa+w.length>v.sa)continue;
                        const size_t d=v.sa-(w.sa+w.length);
                        if(d<distance){distance=d;neighbor=&w;}
                    }else{
                        if(w.sa<endA)continue;
                        const size_t d=w.sa-endA;
                        if(d<distance){distance=d;neighbor=&w;}
                    }
                }
                if(!neighbor){
                    std::printf("SPR_M312_NEIGHBOR rank=%zu side=%s found=0\n",i+1,before?"left":"right");
                    continue;
                }
                const size_t gapA=before?v.sa-(neighbor->sa+neighbor->length):neighbor->sa-endA;
                const long long gapB=before?
                    static_cast<long long>(v.sb)-static_cast<long long>(neighbor->sb+neighbor->length):
                    static_cast<long long>(neighbor->sb)-static_cast<long long>(endB);
                const long long delta=gapB-static_cast<long long>(gapA);
                const long long neighborShift=static_cast<long long>(neighbor->sb)-static_cast<long long>(neighbor->sa);
                const long long currentShift=static_cast<long long>(v.sb)-static_cast<long long>(v.sa);
                std::printf("SPR_M312_NEIGHBOR rank=%zu side=%s found=1 neighborStartA=%zu neighborStartB=%zu neighborLength=%zu gapA=%zu gapB=%lld gapDelta=%lld currentShift=%lld neighborShift=%lld shiftDelta=%lld consistent=%d\n",
                    i+1,before?"left":"right",neighbor->sa,neighbor->sb,neighbor->length,
                    gapA,gapB,delta,currentShift,neighborShift,
                    before?currentShift-neighborShift:neighborShift-currentShift,
                    delta==(before?currentShift-neighborShift:neighborShift-currentShift)?1:0);
            }
        }
        std::printf("SPR_M312_SUMMARY examinedShifted=%zu contextWindow=32 neighborPolicy=nearest_nonoverlapping_A\n",reported);
    }
    std::printf("SPR_M311_SEGMENT9 recordA=5 startA=%zu endA=%zu recordB=6 startB=%zu endB=%zu selectedMotifA=%zu selectedMotifB=%zu motifBits=%zu\n",
        segA,segEndA,segB,segEndB,chosenA,chosenB,best);
    std::printf("SPR_M311_SUMMARY hits=%zu examinedPairs=%zu qualifiedObservations=%zu uniqueIntervals=%zu prefixes=%zu fixedInternal=%zu shiftedInternal=%zu segment9Intersect=%zu\n",
        hits.size(),examined,qualified,intervals.size(),prefixes,fixed,shifted,overlap9);
}

struct M307Window {size_t length;size_t threshold;const char* label;};
constexpr M307Window m307Windows[]={{38,26,"baseline_after"},{54,15,"alternative_before"},{22,19,"alternative_after"}};
void m307Compare(const std::vector<BitMatch>& baseline,const std::vector<BitMatch>& alternative,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    if(baseline.size()<10)return;
    size_t li=0,ri=0;bool lf=false,rf=false;
    for(size_t i=0;i<alternative.size();++i){
        if(sameBitMatch(alternative[i],baseline[7])){li=i;lf=true;}
        if(sameBitMatch(alternative[i],baseline[9])){ri=i;rf=true;}
    }
    if(!lf||!rf||ri!=li+2)return;
    const auto& left=baseline[7];const auto& right=baseline[9];
    const BitMatch mids[]={baseline[8],alternative[li+1]};
    for(int p=0;p<2;++p){
        const auto& m=mids[p];
        if(left.a+left.len>m.a||left.b+left.len>m.b||
           m.a+m.len>right.a||m.b+m.len>right.b||
           right.a+right.len>na||right.b+right.len>nb)return;
        const M300Span ax[]={{left.a+left.len,m.a-left.a-left.len},{m.a+m.len,right.a-m.a-m.len}};
        const M300Span by[]={{left.b+left.len,m.b-left.b-left.len},{m.b+m.len,right.b-m.b-m.len}};
        for(int side=0;side<2;++side){
            if(p==0&&side==0)continue;
            const auto x=ax[side],y=by[side];
            const size_t minimum=p==0?32:16;
            const auto selected=m302Best(a,ba,x,b,bb,y,minimum,nullptr);
            if(!selected.eligible)continue;
            const int selectedShift=selected.score.shift;
            const size_t sa=selectedShift<0?static_cast<size_t>(-selectedShift):0;
            const size_t sb=selectedShift>0?static_cast<size_t>(selectedShift):0;
            const size_t selectedRun=m306Longest(a,ba,x.start+sa,b,bb,y.start+sb,selected.score.compared);
            size_t eligible=0,atLeast=0,greater=0,bestRun=0,bestOverlap=0;
            int bestShift=0;
            // Enumerate every shift with the required overlap, not merely +/-32.
            const int minShift=-static_cast<int>(x.len-minimum);
            const int maxShift=static_cast<int>(y.len-minimum);
            for(int shift=minShift;shift<=maxShift;++shift){
                const size_t offA=shift<0?static_cast<size_t>(-shift):0;
                const size_t offB=shift>0?static_cast<size_t>(shift):0;
                if(offA>=x.len||offB>=y.len)continue;
                const size_t n=std::min(x.len-offA,y.len-offB);
                if(n<minimum)continue;
                const size_t run=m306Longest(a,ba,x.start+offA,b,bb,y.start+offB,n);
                ++eligible;
                if(run>=selectedRun)++atLeast;
                if(run>selectedRun)++greater;
                if(run>bestRun||(run==bestRun&&n>bestOverlap)){
                    bestRun=run;bestShift=shift;bestOverlap=n;
                }
                std::printf("SPR_M307_OFFSET path=%s side=%s shift=%d overlap=%zu longest=%zu selected=%d\n",
                            p?"alternative":"baseline",side?"after":"before",shift,n,run,shift==selectedShift?1:0);
            }
            std::printf("SPR_M307_WIDE path=%s side=%s spanA=%zu spanB=%zu minOverlap=%zu shiftMin=%d shiftMax=%d selectedShift=%d selectedLongest=%zu selectedOverlap=%zu eligible=%zu atLeastSelected=%zu greater=%zu bestShift=%d bestLongest=%zu bestOverlap=%zu selectedAtSearchEdge=%d\n",
                        p?"alternative":"baseline",side?"after":"before",x.len,y.len,minimum,minShift,maxShift,
                        selectedShift,selectedRun,selected.score.compared,eligible,atLeast,greater,
                        bestShift,bestRun,bestOverlap,(selectedShift==minShift||selectedShift==maxShift)?1:0);
        }
    }
}
void m307Nearby(const char* name,size_t oa,size_t ob,const std::vector<uint8_t>& a,
                const std::vector<uint8_t>& b){
    if(std::string(name)!="BRDGRIKR.SPR"||!((oa==4&&ob==5)||(oa==6&&ob==7)))return;
    constexpr size_t stride=16,limit=2048;
    const size_t bitsA=a.size()*8,bitsB=b.size()*8;
    for(const auto& w:m307Windows){
        size_t starts=0,compared=0,atLeast=0,greater=0,best=0;
        int bestShift=0;size_t bestStart=0;
        // Fixed-length, fixed-coordinate-offset windows in nearby record pairs.
        // Shift zero is excluded to avoid identical-position comparisons.
        for(size_t start=0;start+w.length<=std::min(bitsA,limit);start+=stride){
            ++starts;
            for(int shift=-64;shift<=64;++shift){
                if(shift==0)continue;
                const long long other=static_cast<long long>(start)+shift;
                if(other<0||static_cast<size_t>(other)+w.length>bitsB)continue;
                const size_t run=m306Longest(a,0,start,b,0,static_cast<size_t>(other),w.length);
                ++compared;
                if(run>=w.threshold)++atLeast;
                if(run>w.threshold)++greater;
                if(run>best){best=run;bestShift=shift;bestStart=start;}
            }
        }
        std::printf("SPR_M307_CONTROL resource=%s ordinalA=%zu ordinalB=%zu candidate=%s width=%zu threshold=%zu stride=%zu shiftMin=-64 shiftMax=64 excludeShiftZero=1 scanLimitBits=%zu starts=%zu comparisons=%zu atLeast=%zu greater=%zu maximum=%zu maxShift=%d maxStartA=%zu sampling=deterministic_overlapping\n",
                    name,oa,ob,w.label,w.length,w.threshold,stride,std::min(bitsA,limit),starts,compared,atLeast,greater,best,bestShift,bestStart);
    }
}

void m306Compare(const std::vector<BitMatch>& baseline,const std::vector<BitMatch>& alternative,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    if(baseline.size()<10)return;
    size_t li=0,ri=0;bool lf=false,rf=false;
    for(size_t i=0;i<alternative.size();++i){
        if(sameBitMatch(alternative[i],baseline[7])){li=i;lf=true;}
        if(sameBitMatch(alternative[i],baseline[9])){ri=i;rf=true;}
    }
    if(!lf||!rf||ri!=li+2)return;
    const auto& left=baseline[7];const auto& right=baseline[9];
    const BitMatch mids[]={baseline[8],alternative[li+1]};
    for(int p=0;p<2;++p){
        const auto& m=mids[p];
        if(left.a+left.len>m.a||left.b+left.len>m.b||
           m.a+m.len>right.a||m.b+m.len>right.b||
           right.a+right.len>na||right.b+right.len>nb)return;
        const M300Span ax[]={{left.a+left.len,m.a-left.a-left.len},{m.a+m.len,right.a-m.a-m.len}};
        const M300Span by[]={{left.b+left.len,m.b-left.b-left.len},{m.b+m.len,right.b-m.b-m.len}};
        for(int side=0;side<2;++side){
            if(p==0&&side==0)continue;
            const auto x=ax[side],y=by[side];
            const size_t minimum=p==0?32:16;
            const auto observed=m302Best(a,ba,x,b,bb,y,minimum,nullptr);
            if(!observed.eligible)continue;
            const int fixed=observed.score.shift;
            size_t bestRun=0;int bestShift=0;size_t valid=0,atLeast=0,greater=0,boundaryAtLeast=0;
            const size_t observedA=fixed<0?size_t(-fixed):0;
            const size_t observedB=fixed>0?size_t(fixed):0;
            const size_t observedRun=m306Longest(a,ba,x.start+observedA,b,bb,y.start+observedB,observed.score.compared);
            for(int shift=-32;shift<=32;++shift){
                const size_t offA=shift<0?size_t(-shift):0;
                const size_t offB=shift>0?size_t(shift):0;
                if(offA>=x.len||offB>=y.len)continue;
                const size_t n=std::min(x.len-offA,y.len-offB);
                if(n<minimum)continue;
                const size_t longest=m306Longest(a,ba,x.start+offA,b,bb,y.start+offB,n);
                ++valid;
                if(longest>=observedRun)++atLeast;
                if(longest>observedRun)++greater;
                if((shift==-32||shift==32)&&longest>=observedRun)++boundaryAtLeast;
                if(longest>bestRun){bestRun=longest;bestShift=shift;}
                std::printf("SPR_M306_OFFSET path=%s side=%s shift=%d overlap=%zu longest=%zu selected=%d\\n",
                            p?"alternative":"baseline",side?"after":"before",shift,n,longest,shift==fixed?1:0);
            }
            std::printf("SPR_M306_SUMMARY path=%s side=%s selectedShift=%d selectedOverlap=%zu selectedLongest=%zu eligibleOffsets=%zu atLeast=%zu greater=%zu boundaryAtLeast=%zu maximumLongest=%zu maximumShift=%d\\n",
                        p?"alternative":"baseline",side?"after":"before",fixed,observed.score.compared,observedRun,valid,atLeast,greater,boundaryAtLeast,bestRun,bestShift);
        }
    }
}
void m306Nearby(const char* name,size_t oa,size_t ob,const std::vector<uint8_t>& a,
                const std::vector<uint8_t>& b){
    if(std::string(name)!="BRDGRIKR.SPR"||!((oa==4&&ob==5)||(oa==6&&ob==7)))return;
    const size_t bitsA=a.size()*8,bitsB=b.size()*8;
    constexpr size_t width=38,stride=64,limit=2048;
    size_t windows=0,offsets=0,atLeast26=0,maxRun=0;int maxShift=0;size_t maxStart=0;
    for(size_t start=0;start+width<=std::min(bitsA,limit);start+=stride){
        for(int shift=-32;shift<=32;++shift){
            const long long other=static_cast<long long>(start)+shift;
            if(other<0||static_cast<size_t>(other)+width>bitsB)continue;
            const size_t longest=m306Longest(a,0,start,b,0,static_cast<size_t>(other),width);
            ++offsets;
            if(longest>=26)++atLeast26;
            if(longest>maxRun){maxRun=longest;maxShift=shift;maxStart=start;}
        }
        ++windows;
    }
    std::printf("SPR_M306_NEARBY resource=%s ordinalA=%zu ordinalB=%zu width=%zu stride=%zu scanLimitBits=%zu windows=%zu comparisons=%zu atLeast26=%zu maximumLongest=%zu maximumShift=%d maximumStartA=%zu sampling=deterministic_nonexhaustive\\n",
                name,oa,ob,width,stride,limit,windows,offsets,atLeast26,maxRun,maxShift,maxStart);
}

void m304Compare(const char* name,size_t oa,size_t ob,size_t gap,
                 const std::vector<BitMatch>& baseline,const std::vector<BitMatch>& alternative,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    if(baseline.size()<10)return;
    size_t li=0,ri=0;bool lf=false,rf=false;
    for(size_t i=0;i<alternative.size();++i){
        if(sameBitMatch(alternative[i],baseline[7])){li=i;lf=true;}
        if(sameBitMatch(alternative[i],baseline[9])){ri=i;rf=true;}
    }
    if(!lf||!rf||ri!=li+2)return;
    const auto& left=baseline[7];const auto& right=baseline[9];
    const BitMatch mids[]={baseline[8],alternative[li+1]};
    for(int p=0;p<2;++p){
        const auto& m=mids[p];
        if(left.a+left.len>m.a||left.b+left.len>m.b||
           m.a+m.len>right.a||m.b+m.len>right.b||
           right.a+right.len>na||right.b+right.len>nb)return;
        const M300Span ax[]={{left.a+left.len,m.a-left.a-left.len},{m.a+m.len,right.a-m.a-m.len}};
        const M300Span by[]={{left.b+left.len,m.b-left.b-left.len},{m.b+m.len,right.b-m.b-m.len}};
        // The two strongest M303 target candidates, with no additional offset search.
        const int side=p?1:1;
        m304Inspect(name,oa,ob,gap,p?"alternative":"baseline",side?"after":"before",
                    a,ba,ax[side],b,bb,by[side],na,nb);
        if(p==1)m304Inspect(name,oa,ob,gap,"alternative","before",
                             a,ba,ax[0],b,bb,by[0],na,nb);
    }
}

void m303Compare(const char* name,size_t oa,size_t ob,size_t gap,
                 const std::vector<BitMatch>& baseline,const std::vector<BitMatch>& alternative,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    if(baseline.size()<10)return;
    size_t li=0,ri=0;bool lf=false,rf=false;
    for(size_t i=0;i<alternative.size();++i){
        if(sameBitMatch(alternative[i],baseline[7])){li=i;lf=true;}
        if(sameBitMatch(alternative[i],baseline[9])){ri=i;rf=true;}
    }
    if(!lf||!rf||ri!=li+2)return;
    const auto& left=baseline[7];const auto& right=baseline[9];
    const BitMatch mids[]={baseline[8],alternative[li+1]};
    for(int p=0;p<2;++p){
        const auto& m=mids[p];
        if(left.a+left.len>m.a||left.b+left.len>m.b||
           m.a+m.len>right.a||m.b+m.len>right.b||
           right.a+right.len>na||right.b+right.len>nb)return;
        const M300Span ax[]={{left.a+left.len,m.a-left.a-left.len},{m.a+m.len,right.a-m.a-m.len}};
        const M300Span by[]={{left.b+left.len,m.b-left.b-left.len},{m.b+m.len,right.b-m.b-m.len}};
        for(int side=0;side<2;++side){
            const char* path=p?"alternative":"baseline";
            const char* sideName=side?"after":"before";
            m303Report(name,oa,ob,gap,path,sideName,"target",a,ba,ax[side],b,bb,by[side]);
            const size_t la=ax[side].len,lb=by[side].len;
            if(la&&lb&&left.a>=la&&left.b>=lb)
                m303Report(name,oa,ob,gap,path,sideName,"control_before_left",
                           a,ba,{left.a-la,la},b,bb,{left.b-lb,lb});
            else std::printf("SPR_M303_CONTROL_SKIP path=%s side=%s reason=insufficient_pre_anchor_bits\n",path,sideName);
            if(la&&lb&&right.a+right.len+la<=na&&right.b+right.len+lb<=nb)
                m303Report(name,oa,ob,gap,path,sideName,"control_after_right",
                           a,ba,{right.a+right.len,la},b,bb,{right.b+right.len,lb});
            else std::printf("SPR_M303_CONTROL_SKIP path=%s side=%s reason=insufficient_post_anchor_bits\n",path,sideName);
        }
    }
}

bool m301Better(const M301Score& x,const M301Score& y){
    const auto lhs=x.matches*y.compared,rhs=y.matches*x.compared;
    if(lhs!=rhs)return lhs>rhs;
    if(x.compared!=y.compared)return x.compared>y.compared;
    return (x.shift<0?-x.shift:x.shift)<(y.shift<0?-y.shift:y.shift);
}
void m301Report(const char* name,size_t oa,size_t ob,size_t gap,
                const char* path,const char* side,const char* sample,
                const std::vector<uint8_t>& a,size_t ba,M300Span x,
                const std::vector<uint8_t>& b,size_t bb,M300Span y){
    for(size_t minimum:{size_t(16),size_t(32),size_t(48)}){
        auto scores=m301Offsets(a,ba,x,b,bb,y,minimum);
        if(scores.empty()){
            std::printf("SPR_M301_OFFSET resource=%s ordinalA=%zu ordinalB=%zu gap=%zu path=%s side=%s sample=%s minOverlap=%zu eligible=0 status=insufficient_overlap\n",
                        name,oa,ob,gap,path,side,sample,minimum);
            continue;
        }
        auto best=*std::max_element(scores.begin(),scores.end(),[](const M301Score& x,const M301Score& y){return m301Better(y,x);});
        // The nearest eligible offsets measure how sharply the selected maximum peaks.
        size_t nearMatches=0,nearCompared=0,nearCount=0;
        for(const auto& score:scores){
            if(score.shift==best.shift-1||score.shift==best.shift+1){
                nearMatches+=score.matches;nearCompared+=score.compared;++nearCount;
            }
        }
        size_t betterOrEqual=0;
        for(const auto& score:scores)
            if(score.matches*best.compared>=best.matches*score.compared)++betterOrEqual;
        std::printf("SPR_M301_OFFSET resource=%s ordinalA=%zu ordinalB=%zu gap=%zu path=%s side=%s sample=%s minOverlap=%zu eligible=%zu bestShift=%d bestMatches=%zu bestCompared=%zu tiedOrBetter=%zu adjacentCount=%zu adjacentMatches=%zu adjacentCompared=%zu aBits=%zu bBits=%zu interpretation=descriptive\n",
                    name,oa,ob,gap,path,side,sample,minimum,scores.size(),best.shift,
                    best.matches,best.compared,betterOrEqual,nearCount,nearMatches,nearCompared,x.len,y.len);
    }
}

void m301Compare(const char* name,size_t oa,size_t ob,size_t gap,
                 const std::vector<BitMatch>& baseline,const std::vector<BitMatch>& alternative,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    if(baseline.size()<10)return;
    size_t li=0,ri=0;bool lf=false,rf=false;
    for(size_t i=0;i<alternative.size();++i){
        if(sameBitMatch(alternative[i],baseline[7])){li=i;lf=true;}
        if(sameBitMatch(alternative[i],baseline[9])){ri=i;rf=true;}
    }
    if(!lf||!rf||ri!=li+2)return;
    const auto& left=baseline[7];const auto& right=baseline[9];
    const BitMatch mids[]={baseline[8],alternative[li+1]};
    for(int p=0;p<2;++p){
        const auto& m=mids[p];
        if(left.a+left.len>m.a||left.b+left.len>m.b||
           m.a+m.len>right.a||m.b+m.len>right.b||
           right.a+right.len>na||right.b+right.len>nb)return;
        const M300Span ax[]={{left.a+left.len,m.a-left.a-left.len},{m.a+m.len,right.a-m.a-m.len}};
        const M300Span by[]={{left.b+left.len,m.b-left.b-left.len},{m.b+m.len,right.b-m.b-m.len}};
        for(int side=0;side<2;++side){
            const char* path=p?"alternative":"baseline";
            const char* sideName=side?"after":"before";
            m301Report(name,oa,ob,gap,path,sideName,"target",a,ba,ax[side],b,bb,by[side]);
            // Fixed-location controls, sampled from outside the two exact-match anchors.
            // These are descriptive within-record controls, not independent null draws.
            const size_t lengthA=ax[side].len,lengthB=by[side].len;
            if(lengthA&&lengthB&&left.a>=lengthA&&left.b>=lengthB)
                m301Report(name,oa,ob,gap,path,sideName,"control_before_left",
                           a,ba,{left.a-lengthA,lengthA},b,bb,{left.b-lengthB,lengthB});
            else
                std::printf("SPR_M301_CONTROL_SKIP path=%s side=%s reason=insufficient_pre_anchor_bits\\n",path,sideName);
            if(lengthA&&lengthB&&right.a+right.len+lengthA<=na&&right.b+right.len+lengthB<=nb)
                m301Report(name,oa,ob,gap,path,sideName,"control_after_right",
                           a,ba,{right.a+right.len,lengthA},b,bb,{right.b+right.len,lengthB});
            else
                std::printf("SPR_M301_CONTROL_SKIP path=%s side=%s reason=insufficient_post_anchor_bits\\n",path,sideName);
        }
    }
}

void m300Compare(const char* name,size_t oa,size_t ob,size_t gap,
                 const std::vector<BitMatch>& baseline,const std::vector<BitMatch>& alternative,
                 const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
                 size_t ba,size_t bb,size_t na,size_t nb){
    if(baseline.size()<10)return;
    size_t li=0,ri=0;bool leftFound=false,rightFound=false;
    for(size_t i=0;i<alternative.size();++i){
        if(sameBitMatch(alternative[i],baseline[7])){li=i;leftFound=true;}
        if(sameBitMatch(alternative[i],baseline[9])){ri=i;rightFound=true;}
    }
    if(!leftFound||!rightFound||ri!=li+2)return;
    const auto& left=baseline[7];const auto& right=baseline[9];
    const BitMatch mids[]={baseline[8],alternative[li+1]};
    for(int p=0;p<2;++p){
        const auto& m=mids[p];
        if(left.a+left.len>m.a||left.b+left.len>m.b||
           m.a+m.len>right.a||m.b+m.len>right.b||
           right.a+right.len>na||right.b+right.len>nb)return;
        const M300Span aa[]={{left.a+left.len,m.a-left.a-left.len},{m.a+m.len,right.a-m.a-m.len}};
        const M300Span bSpans[]={{left.b+left.len,m.b-left.b-left.len},{m.b+m.len,right.b-m.b-m.len}};
        for(int side=0;side<2;++side){
            const auto x=aa[side],y=bSpans[side];
            size_t prefix=m300Common(a,ba,x,b,bb,y,false);
            size_t suffix=m300Common(a,ba,x,b,bb,y,true);
            auto best=m300BestOffset(a,ba,x,b,bb,y,32);
            std::printf("SPR_M300_SPAN resource=%s ordinalA=%zu ordinalB=%zu gap=%zu path=%s side=%s aStart=%zu aBits=%zu bStart=%zu bBits=%zu commonPrefix=%zu commonSuffix=%zu offset=%d offsetMatches=%zu offsetCompared=%zu aPattern8=%zu bPattern8=%zu aPattern16=%zu bPattern16=%zu\n",
                name,oa,ob,gap,p?"alternative":"baseline",side?"after":"before",
                x.start,x.len,y.start,y.len,prefix,suffix,best.shift,best.matches,best.compared,
                m300PatternCount(a,ba,x,8),m300PatternCount(b,bb,y,8),
                m300PatternCount(a,ba,x,16),m300PatternCount(b,bb,y,16));
        }
    }
    std::printf("SPR_M300_NOTE resource=%s ordinalA=%zu ordinalB=%zu gap=%zu interpretation=unmatched_bit_pattern_observations_not_codec_validation\n",name,oa,ob,gap);
}

void m299Compare(const char* name,size_t oa,size_t ob,size_t gap,
                 const std::vector<BitMatch>& baseline,const std::vector<BitMatch>& alternative,
                 size_t na,size_t nb){
    // Segment nine is only meaningful if the competing paths retain its exact neighbors.
    if(baseline.size()<10)return;
    const auto& target=baseline[8];
    size_t leftIndex=0,rightIndex=0;
    bool foundLeft=false,foundRight=false;
    for(size_t i=0;i<alternative.size();++i){
        if(sameBitMatch(alternative[i],baseline[7])){leftIndex=i;foundLeft=true;}
        if(sameBitMatch(alternative[i],baseline[9])){rightIndex=i;foundRight=true;}
    }
    if(!foundLeft||!foundRight||rightIndex!=leftIndex+2||
       !sameBitMatch(baseline[7],alternative[leftIndex])||
       !sameBitMatch(baseline[9],alternative[rightIndex])){
        std::printf("SPR_M299_SKIP resource=%s ordinalA=%zu ordinalB=%zu gap=%zu reason=neighbor_anchors_not_isolated\\n",name,oa,ob,gap);
        return;
    }
    const auto& candidate=alternative[leftIndex+1];
    if(sameBitMatch(target,candidate)||!validateChain(baseline,na,nb)||
       !validateChain(alternative,na,nb))return;
    const auto& left=baseline[7];
    const auto& right=baseline[9];
    const auto x=m299Geometry(left,target,right);
    const auto y=m299Geometry(left,candidate,right);
    const auto emit=[&](const char* label,const BitMatch& m,const M299Geometry& g){
        std::printf("SPR_M299_PATH resource=%s ordinalA=%zu ordinalB=%zu gap=%zu path=%s leftA=%zu leftB=%zu leftLength=%zu leftShift=%d matchA=%zu matchB=%zu matchLength=%zu matchShift=%d rightA=%zu rightB=%zu rightLength=%zu rightShift=%d beforeA=%zu beforeB=%zu beforeDelta=%lld afterA=%zu afterB=%zu afterDelta=%lld enterShiftDelta=%lld exitShiftDelta=%lld unmatchedTotal=%zu maxImbalance=%zu shiftTransitions=%zu shiftTravel=%lld\\n",
            name,oa,ob,gap,label,left.a,left.b,left.len,left.shift,
            m.a,m.b,m.len,m.shift,right.a,right.b,right.len,right.shift,
            g.beforeA,g.beforeB,static_cast<long long>(g.beforeB)-g.beforeA,
            g.afterA,g.afterB,static_cast<long long>(g.afterB)-g.afterA,
            g.enterShift,g.exitShift,g.unmatchedTotal,g.maxImbalance,g.transitions,g.shiftTravel);
    };
    emit("baseline",target,x);
    emit("alternative",candidate,y);
    std::printf("SPR_M299_COMPARE resource=%s ordinalA=%zu ordinalB=%zu gap=%zu baselineBits=%zu alternativeBits=%zu coverageDelta=%lld unmatchedDelta=%lld maxImbalanceDelta=%lld shiftTravelDelta=%lld baselineRejoins=%d alternativeRejoins=%d sharedAnchors=2 interpretation=geometry_only\\n",
        name,oa,ob,gap,target.len,candidate.len,
        static_cast<long long>(candidate.len)-target.len,
        static_cast<long long>(y.unmatchedTotal)-x.unmatchedTotal,
        static_cast<long long>(y.maxImbalance)-x.maxImbalance,
        y.shiftTravel-x.shiftTravel,1,1);
}

bool sameBitMatch(const BitMatch& x,const BitMatch& y){
    return x.a==y.a&&x.b==y.b&&x.len==y.len&&x.shift==y.shift;
}
// Split alternative paths at exact shared anchors, rather than assuming that
// all changed matches belong to one continuous dependency.
void transitionAnalysis(const char* name,size_t oa,size_t ob,size_t gap,
                        const char* mode,size_t parameter,
                        const std::vector<BitMatch>& baseline,
                        const std::vector<BitMatch>& alternative,
                        size_t na,size_t nb){
    struct Shared {size_t oldIndex,newIndex;};
    std::vector<Shared> shared;
    for(size_t i=0;i<baseline.size();++i)
        for(size_t j=0;j<alternative.size();++j)
            if(sameBitMatch(baseline[i],alternative[j])){shared.push_back({i,j});break;}
    size_t oldBegin=0,newBegin=0;
    size_t prevA=0,prevB=0;
    long long prevShift=0;
    for(size_t boundary=0;boundary<=shared.size();++boundary){
        const bool terminal=boundary==shared.size();
        const size_t oldEnd=terminal?baseline.size():shared[boundary].oldIndex;
        const size_t newEnd=terminal?alternative.size():shared[boundary].newIndex;
        const size_t nextA=terminal?na:baseline[oldEnd].a;
        const size_t nextB=terminal?nb:baseline[oldEnd].b;
        if(oldEnd>oldBegin||newEnd>newBegin){
            const long long nextShift=static_cast<long long>(nextB)-static_cast<long long>(nextA);
            size_t oldBits=0,newBits=0;
            for(size_t i=oldBegin;i<oldEnd;++i)oldBits+=baseline[i].len;
            for(size_t i=newBegin;i<newEnd;++i)newBits+=alternative[i].len;
            std::printf("SPR_M294_REGION resource=%s ordinalA=%zu ordinalB=%zu gap=%zu mode=%s parameter=%zu region=%zu leftAnchor=%d rightAnchor=%d fromA=%zu toA=%zu fromB=%zu toB=%zu oldRuns=%zu newRuns=%zu oldBits=%zu newBits=%zu coverageDelta=%lld leftShift=%lld rightShift=%lld rejoined=%d\n",
                name,oa,ob,gap,mode,parameter,boundary+1,oldBegin>0?1:0,terminal?0:1,
                prevA,nextA,prevB,nextB,oldEnd-oldBegin,newEnd-newBegin,oldBits,newBits,
                static_cast<long long>(newBits)-static_cast<long long>(oldBits),
                prevShift,nextShift,terminal?0:1);
            for(int path=0;path<2;++path){
                const auto& runs=path==0?baseline:alternative;
                const size_t begin=path==0?oldBegin:newBegin;
                const size_t end=path==0?oldEnd:newEnd;
                size_t cursorA=prevA,cursorB=prevB;
                long long shiftBefore=prevShift;
                for(size_t i=begin;i<=end;++i){
                    const bool last=i==end;
                    const size_t a=last?nextA:runs[i].a;
                    const size_t b=last?nextB:runs[i].b;
                    const long long shiftAfter=static_cast<long long>(b)-static_cast<long long>(a);
                    const size_t gapA=a-cursorA,gapB=b-cursorB;
                    std::printf("SPR_M294_TRANSITION resource=%s ordinalA=%zu ordinalB=%zu gap=%zu mode=%s parameter=%zu region=%zu path=%s step=%zu fromA=%zu toA=%zu fromB=%zu toB=%zu unmatchedA=%zu unmatchedB=%zu unmatchedDelta=%lld shiftBefore=%lld shiftAfter=%lld shiftDelta=%lld terminal=%d\n",
                        name,oa,ob,gap,mode,parameter,boundary+1,path==0?"baseline":"alternative",
                        i-begin+1,cursorA,a,cursorB,b,gapA,gapB,
                        static_cast<long long>(gapB)-static_cast<long long>(gapA),
                        shiftBefore,shiftAfter,shiftAfter-shiftBefore,last?1:0);
                    if(!last){
                        cursorA=runs[i].a+runs[i].len;
                        cursorB=runs[i].b+runs[i].len;
                        shiftBefore=shiftAfter;
                    }
                }
            }
        }
        if(!terminal){
            const auto& anchor=baseline[oldEnd];
            prevA=anchor.a+anchor.len;
            prevB=anchor.b+anchor.len;
            prevShift=static_cast<long long>(anchor.b)-static_cast<long long>(anchor.a);
            oldBegin=oldEnd+1;newBegin=newEnd+1;
        }
    }
}

void bitGap(const char* name,size_t oa,size_t ob,size_t gap,
            const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,
            size_t fromA,size_t toA,size_t fromB,size_t toB){
    if(!((std::string(name)=="BRDGRIKR.SPR"&&
          ((oa==5&&ob==6&&gap==0)||(oa==2&&ob==3&&gap==0)))||
         (std::string(name)=="BRDGPICA.SPR"&&oa==6&&ob==7&&gap==0)))return;
    const size_t na=(toA-fromA)*8,nb=(toB-fromB)*8;
    const size_t ba=fromA*8,bb=fromB*8;
    if(!na||!nb)return;
    ++diagnosticGaps;
    const auto primary=bitChain(bitCandidates(a,b,ba,bb,na,nb,32));
    std::vector<BitMatch> refined=primary;
    size_t prevA=0,prevB=0;
    for(size_t i=0;i<=primary.size();++i){
        const bool last=i==primary.size();
        const size_t nextA=last?na:primary[i].a,nextB=last?nb:primary[i].b;
        const auto local=bitChain(bitCandidates(a,b,ba+prevA,bb+prevB,nextA-prevA,nextB-prevB,32));
        for(const auto& m:local)refined.push_back({prevA+m.a,prevB+m.b,m.len,
            static_cast<int>(static_cast<long long>(prevB+m.b)-static_cast<long long>(prevA+m.a))});
        if(!last){prevA=primary[i].a+primary[i].len;prevB=primary[i].b+primary[i].len;}
    }
    std::sort(refined.begin(),refined.end(),[](const BitMatch& x,const BitMatch& y){return x.a<y.a;});
    const auto found=globalCandidates(a,b,ba,bb,na,nb,32,128);
    const auto unconstrained=bitChain(found);
    const auto preserved=preserveAnchors(found,primary,refined);
    if(!validateChain(unconstrained,na,nb)||!validateChain(preserved.runs,na,nb)){
        std::fprintf(stderr,"SPR_M293_ERROR invalid chain resource=%s\n",name);return;
    }
    std::printf("SPR_M293_BASELINE resource=%s ordinalA=%zu ordinalB=%zu gap=%zu bitsA=%zu bitsB=%zu m282Bits=%zu m284Bits=%zu m285Bits=%zu m286Bits=%zu m284Anchors=%zu retainedM284=%zu\n",
        name,oa,ob,gap,na,nb,coverage(primary),coverage(refined),coverage(unconstrained),
        coverage(preserved.runs),refined.size(),preserved.retained284);
    const auto spans=intervals(preserved.runs,na,nb);
    for(size_t i=0;i<spans.size();++i){
        const auto& z=spans[i];
        size_t contained=0;
        for(const auto& m:unconstrained)
            if(m.a>=z.a0&&m.a+m.len<=z.a1&&m.b>=z.b0&&m.b+m.len<=z.b1)++contained;
        std::printf("SPR_M293_INTERVAL resource=%s ordinalA=%zu ordinalB=%zu gap=%zu interval=%zu kind=%s fromA=%zu toA=%zu fromB=%zu toB=%zu bitsA=%zu bitsB=%zu deltaBits=%lld unconstrainedContained=%zu\n",
            name,oa,ob,gap,i+1,i==0?"leading":i+1==spans.size()?"trailing":"internal",
            z.a0,z.a1,z.b0,z.b1,z.a1-z.a0,z.b1-z.b0,
            static_cast<long long>(z.b1-z.b0)-static_cast<long long>(z.a1-z.a0),contained);
        ++diagnosticIntervals;
    }
    for(size_t i=0;i<unconstrained.size();++i){
        const auto& m=unconstrained[i];
        if(containsAnchor(preserved.runs,m))continue;
        ++diagnosticCompeting;
        unsigned flags=0;size_t conflicts=0;
        for(size_t j=0;j<preserved.runs.size();++j){
            const auto& p=preserved.runs[j];
            const unsigned f=conflictFlags(m,p);
            if(f){flags|=f;++conflicts;
                std::printf("SPR_M293_CONFLICT resource=%s ordinalA=%zu ordinalB=%zu gap=%zu candidate=%zu preserved=%zu overlapA=%d overlapB=%d inversion=%d sameSource=%d\n",
                    name,oa,ob,gap,i+1,j+1,(f&overlapA)!=0,(f&overlapB)!=0,(f&inversion)!=0,(f&sameSource)!=0);
                ++diagnosticConflicts;
            }
        }
        if(!flags)std::printf("SPR_M293_CONFLICT resource=%s ordinalA=%zu ordinalB=%zu gap=%zu candidate=%zu preserved=0 noDirectConflict=1\n",name,oa,ob,gap,i+1);
        const size_t occurA=occurrences(a,a,ba,ba+m.a,na,m.len);
        const size_t occurB=occurrences(b,a,bb,ba+m.a,nb,m.len);
        std::printf("SPR_M293_REPEAT resource=%s ordinalA=%zu ordinalB=%zu gap=%zu candidate=%zu bitA=%zu bitB=%zu length=%zu shift=%d occurrencesA=%zu occurrencesB=%zu ambiguous=%d conflicts=%zu\n",
            name,oa,ob,gap,i+1,m.a,m.b,m.len,m.shift,occurA,occurB,(occurA>1||occurB>1)?1:0,conflicts);
        for(size_t window:{size_t(8),size_t(16),size_t(32)}){
            size_t leftN=0,rightN=0;
            const size_t left=contextAgreement(a,b,ba,bb,na,nb,m,window,true,leftN);
            const size_t right=contextAgreement(a,b,ba,bb,na,nb,m,window,false,rightN);
            std::printf("SPR_M293_CONTEXT resource=%s ordinalA=%zu ordinalB=%zu gap=%zu candidate=%zu window=%zu leftAgree=%zu leftCompared=%zu rightAgree=%zu rightCompared=%zu\n",
                name,oa,ob,gap,i+1,window,left,leftN,right,rightN);
        }
    }


    // Sensitivity study: same candidate pool, five independently optimized
    // objectives. Equal-score alternatives are counted (saturated at 2).
    std::vector<BitMatch> pool=found;
    for(const auto& p:preserved.runs){
        bool exists=false;
        for(const auto& m:pool)
            if(m.a==p.a&&m.b==p.b&&m.len==p.len){exists=true;break;}
        if(!exists)pool.push_back(p);
    }
    std::sort(pool.begin(),pool.end(),[](const BitMatch& x,const BitMatch& y){
        if(x.a!=y.a)return x.a<y.a;
        if(x.b!=y.b)return x.b<y.b;
        return x.len>y.len;
    });
    std::vector<int> evidenceScores;
    for(const auto& m:pool){
        const size_t ua=occurrences(a,a,ba,ba+m.a,na,m.len);
        const size_t ub=occurrences(b,a,bb,ba+m.a,nb,m.len);
        size_t ln=0,rn=0;
        const size_t la=contextAgreement(a,b,ba,bb,na,nb,m,32,true,ln);
        const size_t ra=contextAgreement(a,b,ba,bb,na,nb,m,32,false,rn);
        evidenceScores.push_back(static_cast<int>(std::min(m.len,size_t(128)))+
            ((ua==1&&ub==1)?32:0)+
            ((ln+rn)?static_cast<int>(25*(la+ra)/(ln+rn)):0));
    }
    struct Solution {
        int weight=0;
        long long objective=0;
        size_t ties=0;
        std::vector<BitMatch> runs;
    };
    std::vector<Solution> solutions;
    for(int weight:{0,1,2,4,8,16}){
        std::vector<long long> dp(pool.size(),0);
        std::vector<int> parent(pool.size(),-1);
        std::vector<unsigned> ways(pool.size(),1);
        int best=-1;
        long long optimum=0;
        unsigned optimalWays=1; // empty solution, superseded by positive runs
        for(size_t i=0;i<pool.size();++i){
            const long long score=static_cast<long long>(pool[i].len)*4+
                static_cast<long long>(weight)*evidenceScores[i];
            dp[i]=score;
            for(size_t j=0;j<i;++j){
                if(pool[j].a+pool[j].len>pool[i].a||
                   pool[j].b+pool[j].len>pool[i].b)continue;
                const long long proposed=dp[j]+score;
                if(proposed>dp[i]){
                    dp[i]=proposed;parent[i]=static_cast<int>(j);ways[i]=ways[j];
                }else if(proposed==dp[i]){
                    ways[i]=std::min(2u,ways[i]+ways[j]);
                }
            }
            if(dp[i]>optimum){
                optimum=dp[i];best=static_cast<int>(i);optimalWays=ways[i];
            }else if(dp[i]==optimum){
                optimalWays=std::min(2u,optimalWays+ways[i]);
            }
        }
        Solution result;
        result.weight=weight;result.objective=optimum;result.ties=optimalWays;
        while(best>=0){
            result.runs.push_back(pool[static_cast<size_t>(best)]);
            best=parent[static_cast<size_t>(best)];
        }
        std::reverse(result.runs.begin(),result.runs.end());
        if(!validateChain(result.runs,na,nb)){
            std::puts("SPR_M293_ERROR invalid sensitivity chain");return;
        }
        size_t kept=0;
        for(const auto& p:preserved.runs)if(containsAnchor(result.runs,p))++kept;
        std::printf("SPR_M293_SENSITIVITY resource=%s ordinalA=%zu ordinalB=%zu gap=%zu evidenceWeight=%d coverageScale=4 objective=%lld bits=%zu runs=%zu retainedPreserved=%zu droppedPreserved=%zu optimalTie=%d\n",
            name,oa,ob,gap,weight,optimum,coverage(result.runs),result.runs.size(),
            kept,preserved.runs.size()-kept,optimalWays>1?1:0);
        solutions.push_back(std::move(result));
    }
    const auto& weighted=solutions[3].runs; // weight 4: sensitivity point
    const auto& baseline=solutions[1].runs; // weight 1: exact M290 objective
    size_t stable=0;
    for(const auto& m:baseline){
        size_t present=0;
        for(const auto& sol:solutions)
            for(const auto& v:sol.runs)
                if(m.a==v.a&&m.b==v.b&&m.len==v.len){++present;break;}
        if(present==solutions.size())++stable;
        std::printf("SPR_M293_STABILITY resource=%s ordinalA=%zu ordinalB=%zu gap=%zu bitA=%zu bitB=%zu length=%zu selectedAcross=%zu configurations=%zu stable=%d\n",
            name,oa,ob,gap,m.a,m.b,m.len,present,solutions.size(),
            present==solutions.size()?1:0);
    }
    std::printf("SPR_M293_GLOBAL resource=%s ordinalA=%zu ordinalB=%zu gap=%zu pool=%zu m290Bits=%zu coverageOnlyBits=%zu highEvidenceBits=%zu stableBaselineSegments=%zu baselineSegments=%zu\n",
        name,oa,ob,gap,pool.size(),coverage(baseline),
        coverage(solutions[0].runs),coverage(solutions.back().runs),
        stable,baseline.size());
    for(size_t i=0;i<baseline.size();++i){
        const auto& m=baseline[i];
        std::printf("SPR_M293_SELECTED resource=%s ordinalA=%zu ordinalB=%zu gap=%zu segment=%zu bitA=%zu bitB=%zu length=%zu shift=%d preserved=%d\n",
            name,oa,ob,gap,i+1,m.a,m.b,m.len,m.shift,
            containsAnchor(preserved.runs,m)?1:0);
    }

    // M293: perturb the actual M290 candidate pool, not the evidence weights.
    // Score remains 4*matchedBits + evidence, as in M290.
    auto optimizePool=[&](const std::vector<size_t>& eligible){
        std::vector<long long> dp(eligible.size(),0);
        std::vector<int> parent(eligible.size(),-1);
        long long optimum=0;
        int best=-1;
        for(size_t i=0;i<eligible.size();++i){
            const auto& m=pool[eligible[i]];
            const long long score=static_cast<long long>(m.len)*4+evidenceScores[eligible[i]];
            dp[i]=score;
            for(size_t j=0;j<i;++j){
                const auto& prev=pool[eligible[j]];
                if(prev.a+prev.len>m.a||prev.b+prev.len>m.b)continue;
                if(dp[j]+score>dp[i]){
                    dp[i]=dp[j]+score;parent[i]=static_cast<int>(j);
                }
            }
            if(dp[i]>optimum){optimum=dp[i];best=static_cast<int>(i);}
        }
        Solution result;
        result.weight=1;result.objective=optimum;
        while(best>=0){
            result.runs.push_back(pool[eligible[static_cast<size_t>(best)]]);
            best=parent[static_cast<size_t>(best)];
        }
        std::reverse(result.runs.begin(),result.runs.end());
        return result;
    };

    auto sameMatch=[](const BitMatch& x,const BitMatch& y){
        return x.a==y.a&&x.b==y.b&&x.len==y.len;
    };
    auto inChain=[&](const std::vector<BitMatch>& runs,const BitMatch& m){
        return std::any_of(runs.begin(),runs.end(),[&](const BitMatch& v){return sameMatch(v,m);});
    };
    auto ambiguity=[&](const BitMatch& m){
        return occurrences(a,a,ba,ba+m.a,na,m.len)!=1||
               occurrences(b,a,bb,ba+m.a,nb,m.len)!=1;
    };
    auto describeChange=[&](const char* mode,size_t parameter,const Solution& alternative){
        size_t lost=0,added=0,ambiguousAdded=0,uniqueAddedBits=0,ambiguousAddedBits=0;
        size_t minA=na,maxA=0,minB=nb,maxB=0;
        for(size_t i=0;i<baseline.size();++i){
            const auto& m=baseline[i];
            if(inChain(alternative.runs,m))continue;
            ++lost;minA=std::min(minA,m.a);maxA=std::max(maxA,m.a+m.len);
            minB=std::min(minB,m.b);maxB=std::max(maxB,m.b+m.len);
            std::printf("SPR_M293_EXIT resource=%s ordinalA=%zu ordinalB=%zu gap=%zu mode=%s parameter=%zu baselineSegment=%zu bitA=%zu bitB=%zu length=%zu shift=%d ambiguous=%d\n",
                name,oa,ob,gap,mode,parameter,i+1,m.a,m.b,m.len,m.shift,ambiguity(m)?1:0);
        }
        for(size_t i=0;i<alternative.runs.size();++i){
            const auto& m=alternative.runs[i];
            if(inChain(baseline,m))continue;
            ++added;minA=std::min(minA,m.a);maxA=std::max(maxA,m.a+m.len);
            minB=std::min(minB,m.b);maxB=std::max(maxB,m.b+m.len);
            const bool ambiguous=ambiguity(m);
            if(ambiguous){++ambiguousAdded;ambiguousAddedBits+=m.len;}
            else uniqueAddedBits+=m.len;
            std::printf("SPR_M293_ENTER resource=%s ordinalA=%zu ordinalB=%zu gap=%zu mode=%s parameter=%zu alternativeSegment=%zu bitA=%zu bitB=%zu length=%zu shift=%d ambiguous=%d\n",
                name,oa,ob,gap,mode,parameter,i+1,m.a,m.b,m.len,m.shift,ambiguous?1:0);
        }
        const size_t spanA=lost||added?maxA-minA:0;
        const size_t spanB=lost||added?maxB-minB:0;
        std::printf("SPR_M293_DEPENDENCY resource=%s ordinalA=%zu ordinalB=%zu gap=%zu mode=%s parameter=%zu exited=%zu entered=%zu changedSpanA=%zu changedSpanB=%zu minBitA=%zu maxBitA=%zu minBitB=%zu maxBitB=%zu enteredUniqueBits=%zu enteredAmbiguousBits=%zu enteredAmbiguousRuns=%zu localizedA=%d localizedB=%d\n",
            name,oa,ob,gap,mode,parameter,lost,added,spanA,spanB,
            lost||added?minA:0,lost||added?maxA:0,lost||added?minB:0,lost||added?maxB:0,
            uniqueAddedBits,ambiguousAddedBits,ambiguousAdded,
            spanA<=256?1:0,spanB<=256?1:0);
    };
    auto runPerturb=[&](const char* mode,size_t parameter,const std::vector<size_t>& eligible){
        const auto alternative=optimizePool(eligible);
        if(!validateChain(alternative.runs,na,nb)){
            std::puts("SPR_M293_ERROR invalid perturbed chain");return;
        }
        size_t retained=0;
        for(const auto& m:baseline)
            for(const auto& v:alternative.runs)
                if(m.a==v.a&&m.b==v.b&&m.len==v.len){++retained;break;}
        describeChange(mode,parameter,alternative);
        if((m294Mode||m295Mode)&&std::string(mode)=="leave_one_out"&&
           (parameter==3||parameter==4||parameter==5||parameter==6||parameter==9))
            transitionAnalysis(name,oa,ob,gap,mode,parameter,baseline,alternative.runs,na,nb);
        if((m295Mode||m296Mode)&&std::string(mode)=="leave_one_out"&&
           (parameter==3||parameter==4||parameter==5||parameter==6||parameter==9))
            boundaryEvidence(name,oa,ob,gap,mode,parameter,baseline,alternative.runs,a,b,ba,bb,na,nb);
        if(m317Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m317Compare();
        if(m318Mode && std::string(mode)=="leave_one_out" && parameter==9 && std::string(name)=="BRDGRIKR.SPR" && oa==5 && ob==6 && gap==0){
            m318Compare();
        }
        if(m323Mode && std::string(mode)=="leave_one_out" && parameter==9 && std::string(name)=="BRDGRIKR.SPR" && oa==5 && ob==6 && gap==0)
            m323Compare();
        if(m322Mode && std::string(mode)=="leave_one_out" && parameter==9 && std::string(name)=="BRDGRIKR.SPR" && oa==5 && ob==6 && gap==0)
            m322Compare();
        if(m321Mode && std::string(mode)=="leave_one_out" && parameter==9 && std::string(name)=="BRDGRIKR.SPR" && oa==5 && ob==6 && gap==0)
            m321Compare();
        if(m320Mode && std::string(mode)=="leave_one_out" && parameter==9 && std::string(name)=="BRDGRIKR.SPR" && oa==5 && ob==6 && gap==0)
            m320Compare();
        if(m319Mode && std::string(mode)=="leave_one_out" && parameter==9 && std::string(name)=="BRDGRIKR.SPR" && oa==5 && ob==6 && gap==0)
            m319Compare();
        if(m316Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m316Compare();
        if(m315Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m315Compare();
        if(m314Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m314Compare();
        if(m313Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m313Compare();
        if((m311Mode||m312Mode)&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m311Compare(baseline,a,b,ba,bb,na,nb);
        if(m310Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m310Compare(baseline,a,b,ba,bb,na,nb);
        if(m309Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m309Compare(baseline,a,b,ba,bb,na,nb);
        if(m308Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m308Compare(baseline,a,b,ba,bb,na,nb);
        if(m307Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m307Compare(baseline,alternative.runs,a,b,ba,bb,na,nb);
        if(m306Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m306Compare(baseline,alternative.runs,a,b,ba,bb,na,nb);
        if(m305Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m305Compare(name,oa,ob,gap,baseline,alternative.runs,a,b,ba,bb,na,nb);
        if(m304Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m304Compare(name,oa,ob,gap,baseline,alternative.runs,a,b,ba,bb,na,nb);
        if(m303Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m303Compare(name,oa,ob,gap,baseline,alternative.runs,a,b,ba,bb,na,nb);
        if(m302Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m302Compare(name,oa,ob,gap,baseline,alternative.runs,a,b,ba,bb,na,nb);
        if(m301Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m301Compare(name,oa,ob,gap,baseline,alternative.runs,a,b,ba,bb,na,nb);
        if(m300Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m300Compare(name,oa,ob,gap,baseline,alternative.runs,a,b,ba,bb,na,nb);
        if(m299Mode&&std::string(mode)=="leave_one_out"&&parameter==9&&
           std::string(name)=="BRDGRIKR.SPR"&&oa==5&&ob==6&&gap==0)
            m299Compare(name,oa,ob,gap,baseline,alternative.runs,na,nb);
        if(m298Mode&&std::string(mode)=="leave_one_out"&&
           (parameter==3||parameter==4||parameter==5||parameter==6||parameter==9))
            m298Compare(name,oa,ob,gap,parameter,baseline,alternative.runs,a,b,ba,bb,na,nb);
        if(m297Mode&&std::string(mode)=="leave_one_out"&&
           (parameter==3||parameter==4||parameter==5||parameter==6||parameter==9))
            m297Compare(name,oa,ob,gap,parameter,baseline,alternative.runs,a,b,ba,bb,na,nb);
        if(m296Mode&&std::string(mode)=="leave_one_out"&&
           (parameter==3||parameter==4||parameter==5||parameter==6||parameter==9))
            m296Compare(name,oa,ob,gap,parameter,baseline,alternative.runs,a,b,ba,bb,na,nb);
        const size_t bits=coverage(alternative.runs);
        const size_t originalBits=coverage(baseline);
        std::printf("SPR_M293_PERTURB resource=%s ordinalA=%zu ordinalB=%zu gap=%zu mode=%s parameter=%zu pool=%zu bits=%zu deltaBits=%lld objective=%lld deltaObjective=%lld runs=%zu retainedBaseline=%zu replacedBaseline=%zu\n",
            name,oa,ob,gap,mode,parameter,eligible.size(),bits,
            static_cast<long long>(bits)-static_cast<long long>(originalBits),
            alternative.objective,alternative.objective-solutions[1].objective,
            alternative.runs.size(),retained,baseline.size()-retained);
    };
    for(size_t removed=0;removed<baseline.size();++removed){
        std::vector<size_t> eligible;
        const auto& target=baseline[removed];
        for(size_t i=0;i<pool.size();++i)
            if(!(pool[i].a==target.a&&pool[i].b==target.b&&pool[i].len==target.len))
                eligible.push_back(i);
        runPerturb("leave_one_out",removed+1,eligible);
    }
    for(size_t minimum:{size_t(32),size_t(40),size_t(48),size_t(64),size_t(96),size_t(128)}){
        std::vector<size_t> eligible;
        for(size_t i=0;i<pool.size();++i)
            if(pool[i].len>=minimum)eligible.push_back(i);
        runPerturb("min_length",minimum,eligible);
    }
    for(size_t radius:{size_t(0),size_t(1)}){
        std::vector<size_t> eligible;
        for(size_t i=0;i<pool.size();++i){
            const auto& m=pool[i];
            const size_t ua=occurrences(a,a,ba,ba+m.a,na,m.len);
            const size_t ub=occurrences(b,a,bb,ba+m.a,nb,m.len);
            if(radius==1||(ua==1&&ub==1))eligible.push_back(i);
        }
        runPerturb(radius==0?"unique_only":"all_candidates",radius,eligible);
    }
    std::printf("SPR_M293_POOL_SUMMARY resource=%s ordinalA=%zu ordinalB=%zu gap=%zu baselineBits=%zu baselineRuns=%zu leaveOneOut=%zu minLengthTests=6 ambiguityTests=2\n",
        name,oa,ob,gap,coverage(baseline),baseline.size(),baseline.size());
    // Test each M285-only run against the actual M286 segments, without assuming
    // that uniqueness or a high exact-match score proves structural equivalence.
    for(size_t i=0;i<unconstrained.size();++i){
        const auto& m=unconstrained[i];
        if(containsAnchor(preserved.runs,m))continue;
        const size_t leftA=m.a,rightA=m.a+m.len,leftB=m.b,rightB=m.b+m.len;
        size_t overlapCount=0,overlapBitsA=0,overlapBitsB=0,nearest=static_cast<size_t>(-1);
        long long closestShiftDelta=0;
    
    for(size_t j=0;j<preserved.runs.size();++j){
            const auto& p=preserved.runs[j];
            const size_t commonA=std::min(rightA,p.a+p.len)>std::max(leftA,p.a)?
                std::min(rightA,p.a+p.len)-std::max(leftA,p.a):0;
            const size_t commonB=std::min(rightB,p.b+p.len)>std::max(leftB,p.b)?
                std::min(rightB,p.b+p.len)-std::max(leftB,p.b):0;
            if(commonA||commonB){
                ++overlapCount;overlapBitsA+=commonA;overlapBitsB+=commonB;
                const long long d=static_cast<long long>(m.shift)-static_cast<long long>(p.shift);
                std::printf("SPR_M293_ALIGNMENT resource=%s ordinalA=%zu ordinalB=%zu gap=%zu candidate=%zu preserved=%zu length=%zu shift=%d preservedShift=%d shiftDelta=%lld overlapA=%zu overlapB=%zu\n",
                    name,oa,ob,gap,i+1,j+1,m.len,m.shift,p.shift,d,commonA,commonB);
            }
            const size_t distance=m.a>p.a+p.len?m.a-(p.a+p.len):
                p.a>rightA?p.a-rightA:0;
            if(distance<nearest){nearest=distance;closestShiftDelta=
                static_cast<long long>(m.shift)-static_cast<long long>(p.shift);}
        }
        // A candidate can only replace whole segments when their ordered
        // predecessor/successor coordinates remain non-overlapping in BOTH streams.
        size_t removed=0,removedBits=0;
        std::vector<BitMatch> alternatives;
        for(const auto& p:preserved.runs){
            if(conflictFlags(m,p)){
                ++removed;removedBits+=p.len;
            }else alternatives.push_back(p);
        }
        alternatives.push_back(m);
        std::sort(alternatives.begin(),alternatives.end(),[](const BitMatch& x,const BitMatch& y){
            return x.a<y.a||(x.a==y.a&&x.b<y.b);
        });
        const bool valid=validateChain(alternatives,na,nb);
        const long long gain=static_cast<long long>(m.len)-static_cast<long long>(removedBits);
        std::printf("SPR_M293_REPLACEMENT resource=%s ordinalA=%zu ordinalB=%zu gap=%zu candidate=%zu bitA=%zu bitB=%zu length=%zu removed=%zu removedBits=%zu nominalGain=%lld ordered=%d overlaps=%zu overlapBitsA=%zu overlapBitsB=%zu nearestShiftDelta=%lld\n",
            name,oa,ob,gap,i+1,m.a,m.b,m.len,removed,removedBits,gain,valid?1:0,
            overlapCount,overlapBitsA,overlapBitsB,closestShiftDelta);
        // Exact context on both sides is measured independently, including
        // truncated windows at stream boundaries.
        for(const size_t w:{size_t(8),size_t(16),size_t(32),size_t(64)}){
            size_t ln=0,rn=0;
            const size_t la=contextAgreement(a,b,ba,bb,na,nb,m,w,true,ln);
            const size_t ra=contextAgreement(a,b,ba,bb,na,nb,m,w,false,rn);
            std::printf("SPR_M293_FLANK resource=%s ordinalA=%zu ordinalB=%zu gap=%zu candidate=%zu window=%zu before=%zu/%zu after=%zu/%zu\n",
                name,oa,ob,gap,i+1,w,la,ln,ra,rn);
        }
        // Boundary adjustment: trim equal bits from either end while retaining
        // the same displacement. Such trimming cannot eliminate a conflict
        // unless it removes the conflicting overlap or changes chain order.
        for(const size_t trim:{size_t(0),size_t(8),size_t(16),size_t(32)}){
            if(trim*2>=m.len)continue;
            BitMatch shortened={m.a+trim,m.b+trim,m.len-2*trim,m.shift};
            size_t remaining=0;
            for(const auto& p:preserved.runs)if(conflictFlags(shortened,p))++remaining;
            std::printf("SPR_M293_TRIM resource=%s ordinalA=%zu ordinalB=%zu gap=%zu candidate=%zu trimEach=%zu retainedLength=%zu remainingConflicts=%zu\n",
                name,oa,ob,gap,i+1,trim,shortened.len,remaining);
        }
    }

    // Deterministic, descriptive evidence score. It is not a probability
    // or a decoder confidence. Compare matches under identical criteria.
    auto evidence=[&](const BitMatch& m){
        const size_t ua=occurrences(a,a,ba,ba+m.a,na,m.len);
        const size_t ub=occurrences(b,a,bb,ba+m.a,nb,m.len);
        size_t ln=0,rn=0;
        const size_t la=contextAgreement(a,b,ba,bb,na,nb,m,32,true,ln);
        const size_t ra=contextAgreement(a,b,ba,bb,na,nb,m,32,false,rn);
        const int unique=(ua==1&&ub==1)?1:0;
        const int flank=(ln+rn)?static_cast<int>(100*(la+ra)/(ln+rn)):0;
        // Length capped at 128; uniqueness + flank support are independent.
        const int score=static_cast<int>(std::min(m.len,size_t(128)))+
            unique*32+flank/4;
        return score;
    };
    for(size_t j=0;j<preserved.runs.size();++j){
        const auto& p=preserved.runs[j];
        size_t competitors=0;
        for(const auto& m:unconstrained)
            if(!containsAnchor(preserved.runs,m)&&conflictFlags(m,p))++competitors;
        std::printf("SPR_M293_ANCHOR_SCORE resource=%s ordinalA=%zu ordinalB=%zu gap=%zu anchor=%zu bitA=%zu bitB=%zu length=%zu shift=%d evidence=%d competitors=%zu\n",
            name,oa,ob,gap,j+1,p.a,p.b,p.len,p.shift,evidence(p),competitors);
    }
    for(size_t i=0;i<unconstrained.size();++i){
        const auto& m=unconstrained[i];
        if(containsAnchor(preserved.runs,m))continue;
        std::vector<BitMatch> replacement;
        size_t removed=0,removedBits=0;
        int removedScore=0;
        for(const auto& p:preserved.runs){
            if(conflictFlags(m,p)){++removed;removedBits+=p.len;removedScore+=evidence(p);}
            else replacement.push_back(p);
        }
        replacement.push_back(m);
        std::sort(replacement.begin(),replacement.end(),[](const BitMatch& x,const BitMatch& y){
            return x.a<y.a||(x.a==y.a&&x.b<y.b);
        });
        const bool ordered=validateChain(replacement,na,nb);
        const int candidateScore=evidence(m);
        const long long gain=static_cast<long long>(m.len)-static_cast<long long>(removedBits);
        const long long evidenceDelta=static_cast<long long>(candidateScore)-removedScore;
        std::printf("SPR_M293_RELIABILITY resource=%s ordinalA=%zu ordinalB=%zu gap=%zu candidate=%zu length=%zu shift=%d candidateEvidence=%d removed=%zu removedBits=%zu removedEvidence=%d coverageGain=%lld evidenceDelta=%lld ordered=%d\n",
            name,oa,ob,gap,i+1,m.len,m.shift,candidateScore,removed,removedBits,
            removedScore,gain,evidenceDelta,ordered?1:0);
        if(ordered&&gain>0){
            std::printf("SPR_M293_ALTERNATIVE resource=%s ordinalA=%zu ordinalB=%zu gap=%zu candidate=%zu baselineBits=%zu alternativeBits=%zu anchorLoss=%zu evidenceDelta=%lld status=coverage_gain_not_proof\n",
                name,oa,ob,gap,i+1,coverage(preserved.runs),coverage(replacement),
                removed,evidenceDelta);
        }
    }
    std::printf("SPR_M293_COMPARISON resource=%s ordinalA=%zu ordinalB=%zu gap=%zu radius=128 threshold=32 unconstrainedBits=%zu preservedBits=%zu deltaBits=%lld unconstrainedRuns=%zu preservedRuns=%zu competing=%zu\n",
        name,oa,ob,gap,coverage(unconstrained),coverage(preserved.runs),
        static_cast<long long>(coverage(preserved.runs))-static_cast<long long>(coverage(unconstrained)),
        unconstrained.size(),preserved.runs.size(),diagnosticCompeting);
}
bool survey(reluva::stafu::ResourceManager& manager,const char* name,Totals& total){
    std::vector<uint8_t> bytes;std::string error;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"SPR_M293_ERROR resource=%s %s\n",name,error.c_str());return false;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)||index.records().size()<=2){
        std::fprintf(stderr,"SPR_M293_ERROR resource=%s %s\n",name,error.c_str());return false;
    }
    std::vector<Image> direct;size_t ordinal=0;
    for(const auto& event:index.records()[2].events){
        if(event.tag!="PMOC"&&event.tag!="MOCS")continue;
        View v;
        if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,event,v)){
            std::fprintf(stderr,"SPR_M293_ERROR resource=%s ordinal=%zu invalid payload\n",name,ordinal);return false;
        }
        if(!isRef(v)){
            Image im;im.ordinal=ordinal;im.data.assign(v.data,v.data+v.size);
            direct.push_back(std::move(im));
        }
        ++ordinal;
    }
    if((m308Mode||m309Mode||m310Mode||m311Mode||m312Mode||m313Mode||m314Mode||m315Mode||m316Mode||m317Mode||m318Mode||m319Mode||m320Mode||m321Mode||m322Mode||m323Mode)&&std::string(name)=="BRDGRIKR.SPR")m308Records=direct;
    Totals local;
    for(size_t k=1;k<direct.size();++k){
        const auto& a=direct[k-1];const auto& b=direct[k];
        if(b.ordinal!=a.ordinal+1)continue;
        ++local.pairs;
        if(m307Mode)m307Nearby(name,a.ordinal,b.ordinal,a.data,b.data);
        if(m306Mode)m306Nearby(name,a.ordinal,b.ordinal,a.data,b.data);
        const size_t pre=prefix(a.data,b.data);
        const auto found=candidates(a.data,b.data,pre);
        const auto selected=chain(found);
        local.matches+=found.size();local.chainSegments+=selected.size();
        if(!found.empty())++local.withMatches;
        std::printf("SPR_M293_PAIR resource=%s ordinalA=%zu ordinalB=%zu prefix=%zu candidates=%zu chainSegments=%zu\n",
            name,a.ordinal,b.ordinal,pre,found.size(),selected.size());
        for(size_t i=0;i<found.size();++i){
            const auto& m=found[i];
            const bool leftA=m.a>0,leftB=m.b>0;
            const bool rightA=m.a+m.len<a.data.size(),rightB=m.b+m.len<b.data.size();
            const bool leftEqual=leftA&&leftB&&a.data[m.a-1]==b.data[m.b-1];
            const bool rightEqual=rightA&&rightB&&a.data[m.a+m.len]==b.data[m.b+m.len];
            std::printf("SPR_M293_BOUNDARY resource=%s ordinalA=%zu ordinalB=%zu candidate=%zu a=%zu b=%zu length=%zu shift=%d leftA=%s leftB=%s leftEqual=%d rightA=%s rightB=%s rightEqual=%d maximal=%d\n",
                name,a.ordinal,b.ordinal,i+1,m.a,m.b,m.len,m.shift,
                leftA?"byte":"start",leftB?"byte":"start",leftEqual?1:0,
                rightA?"byte":"end",rightB?"byte":"end",rightEqual?1:0,
                (!leftEqual&&!rightEqual)?1:0);
            if(leftA&&leftB)std::printf("SPR_M293_EDGE resource=%s ordinalA=%zu ordinalB=%zu candidate=%zu side=left byteA=0x%02X byteB=0x%02X\n",
                name,a.ordinal,b.ordinal,i+1,static_cast<unsigned>(a.data[m.a-1]),static_cast<unsigned>(b.data[m.b-1]));
            if(rightA&&rightB)std::printf("SPR_M293_EDGE resource=%s ordinalA=%zu ordinalB=%zu candidate=%zu side=right byteA=0x%02X byteB=0x%02X\n",
                name,a.ordinal,b.ordinal,i+1,static_cast<unsigned>(a.data[m.a+m.len]),static_cast<unsigned>(b.data[m.b+m.len]));
        }
        // Compare the intervening raw spans, including prefix-to-first and last-to-end.
        size_t endA=pre,endB=pre;int previousShift=0;
        for(size_t i=0;i<=selected.size();++i){
            const bool final=i==selected.size();
            const size_t nextA=final?a.data.size():selected[i].a;
            const size_t nextB=final?b.data.size():selected[i].b;
            const size_t gapA=nextA-endA,gapB=nextB-endB;
            const long long gapDelta=static_cast<long long>(gapB)-static_cast<long long>(gapA);
            const int nextShift=final?static_cast<int>(b.data.size())-static_cast<int>(a.data.size()):selected[i].shift;
            const int shiftDelta=nextShift-previousShift;
            const bool consistent=gapDelta==shiftDelta;
            ++local.transitions;if(consistent)++local.consistentTransitions;
            std::printf("SPR_M293_GAP resource=%s ordinalA=%zu ordinalB=%zu gap=%zu fromA=%zu toA=%zu fromB=%zu toB=%zu bytesA=%zu bytesB=%zu deltaBytes=%lld shiftBefore=%d shiftAfter=%d deltaShift=%d consistent=%d terminal=%d\n",
                name,a.ordinal,b.ordinal,i,endA,nextA,endB,nextB,gapA,gapB,gapDelta,
                previousShift,nextShift,shiftDelta,consistent?1:0,final?1:0);
            if((std::string(name)=="BRDGPICA.SPR"&&a.ordinal==6&&b.ordinal==7)||
               (std::string(name)=="BRDGRIKR.SPR"&&
                ((a.ordinal==2&&b.ordinal==3)||(a.ordinal==5&&b.ordinal==6)))){
                bitGap(name,a.ordinal,b.ordinal,i,a.data,b.data,endA,nextA,endB,nextB);
            }
            if(!final){
                endA=selected[i].a+selected[i].len;
                endB=selected[i].b+selected[i].len;
                previousShift=selected[i].shift;
            }
        }
    }
    total.pairs+=local.pairs;total.withMatches+=local.withMatches;total.matches+=local.matches;
    total.chainSegments+=local.chainSegments;total.transitions+=local.transitions;
    total.consistentTransitions+=local.consistentTransitions;
    std::printf("SPR_M293_RESOURCE resource=%s pairs=%zu withMatches=%zu candidates=%zu chainSegments=%zu gaps=%zu consistentGaps=%zu\n",
        name,local.pairs,local.withMatches,local.matches,local.chainSegments,local.transitions,local.consistentTransitions);
    return true;
}

bool selfTest(){
    const std::vector<BitMatch> baseline={{0,0,40,0},{50,50,40,0}};
    const std::vector<BitMatch> candidates={{0,0,40,0},{45,46,55,1},{50,50,40,0}};
    const auto u=bitChain(candidates);
    const auto p=preserveAnchors(candidates,baseline,baseline);
    if(coverage(u)!=95||coverage(p.runs)!=80||p.retained284!=2||p.retained282!=2)return false;
    if(p.runs.size()!=2||p.runs[0].a!=0||p.runs[1].a!=50)return false;
    if(preserveAnchors(candidates,baseline,baseline).runs.size()!=p.runs.size())return false;
    const BitMatch conflict={20,60,40,40};
    if(compatible(baseline[0],conflict))return false;
    for(size_t i=1;i<p.runs.size();++i)
        if(p.runs[i-1].a+p.runs[i-1].len>p.runs[i].a||
           p.runs[i-1].b+p.runs[i-1].len>p.runs[i].b)return false;
    const auto iv=intervals(baseline,100,100);
    if(iv.size()!=3||iv[1].a0!=40||iv[1].a1!=50||iv[2].b0!=90)return false;
    if(!(conflictFlags({0,0,40,0},{20,60,40,40})&overlapA))return false;
    if(!(conflictFlags({0,40,20,40},{30,10,20,-20})&inversion))return false;
    const std::vector<uint8_t> pattern={0xAA,0xAA};
    if(occurrences(pattern,pattern,0,0,16,8)<2)return false;
    if(!equalBits(pattern,pattern,0,8,8))return false;
    // M297: normalize by actual available flank bits and censor short samples.
    size_t m297N=0;
    if(flankMismatches(pattern,pattern,0,0,16,16,0,0,true,16,m297N)!=0||m297N!=0)return false;
    if(flankMismatches(pattern,pattern,0,0,16,16,8,8,true,16,m297N)!=0||m297N!=8)return false;
    if(localOccurrences(pattern,0,16,0,8)!=5)return false; // Overlapping AA-pattern occurrences at offsets 0,2,4,6,8.
    if(localOccurrences(pattern,0,16,0,16)!=1)return false;
    if(m297Lengths[0]!=16||m297Lengths[4]!=64)return false;
    if(!validateChain(baseline,100,100))return false;
    const BitMatch test={10,20,64,10};
    const BitMatch trimmed={18,28,48,10};
    if(test.a+8!=trimmed.a||test.b+8!=trimmed.b||test.shift!=trimmed.shift)return false;
    if(conflictFlags({0,0,40,0},{20,60,40,40})==0)return false;
    const int scoreA=static_cast<int>(std::min(size_t(103),size_t(128)))+32+25;
    if(scoreA!=160)return false;
    const std::vector<BitMatch> ordered={{0,0,32,0},{40,40,40,0}};
    if(!validateChain(ordered,100,100)||coverage(ordered)!=72)return false;
    if(4*103+149!=561)return false;
    const std::vector<BitMatch> alternate={{0,0,40,0},{55,55,35,0}};
    size_t exits=0,entries=0;
    for(const auto& m:baseline){bool found=false;for(const auto& n:alternate)if(m.a==n.a&&m.b==n.b&&m.len==n.len)found=true;if(!found)++exits;}
    for(const auto& m:alternate){bool found=false;for(const auto& n:baseline)if(m.a==n.a&&m.b==n.b&&m.len==n.len)found=true;if(!found)++entries;}
    if(exits!=1||entries!=1||!validateChain(alternate,100,100))return false;
    const std::vector<BitMatch> changed={{0,0,40,0},{48,49,35,1},{90,90,10,0}};
    const std::vector<BitMatch> reference={{0,0,40,0},{50,50,30,0},{90,90,10,0}};
    if(!validateChain(changed,110,110)||!validateChain(reference,110,110))return false;
    // M294 must accept both a shared downstream rejoin and a changed internal path.
    if(m294Mode)transitionAnalysis("SELFTEST",1,2,0,"leave_one_out",3,reference,changed,110,110);
    // M299: equal endpoints, ordered nonoverlapping paths, and shift conservation.
    const BitMatch l={0,5,20,5},mid={30,40,20,10},right={70,75,20,5};
    const auto geo=m299Geometry(l,mid,right);
    if(geo.beforeA!=10||geo.beforeB!=15||geo.afterA!=20||geo.afterB!=15||
       geo.enterShift!=5||geo.exitShift!=-5||geo.unmatchedTotal!=60||
       geo.maxImbalance!=5||geo.transitions!=2||geo.shiftTravel!=10)return false;
    std::puts("SPR_M293_SELFTEST PASS");
    return true;
}
}
int stafuSprM293Record2ChainDependencies(const char* installed,const char* cd,const char*){
    if(std::string(installed)=="__selftest__")return selfTest()?0:1;
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"SPR_M293_ERROR %s\n",error.c_str());return 1;
    }
    const char* names[]={"BRDGPICA.SPR","BRDGDATA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    Totals total;
    for(const char* name:names)if(!survey(manager,name,total))return 1;
    std::printf("SPR_M293_SUMMARY resources=5 pairs=%zu withMatches=%zu candidates=%zu chainSegments=%zu gaps=%zu consistentGaps=%zu\n",
        total.pairs,total.withMatches,total.matches,total.chainSegments,total.transitions,total.consistentTransitions);
    std::printf("SPR_M293_SUMMARY_DIAGNOSTIC analyzedGaps=%zu competing=%zu conflicts=%zu intervals=%zu\n",diagnosticGaps,diagnosticCompeting,diagnosticConflicts,diagnosticIntervals);
    std::puts("SPR_M293_NOTE Read-only chain-entry and exit dependency diagnostics; localized means changed coordinate span <=256 bits per stream, not structural validation.");
    return 0;
}

int stafuSprM294Record2AlternativeTransitions(const char* installed,const char* cd,const char* optional){
    m294Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m294Mode=false;
    if(result==0)std::puts("SPR_M294_NOTE Shared anchors mark observed chain rejoins, not decoder state equivalence; all coordinates are gap-relative bits.");
    return result;
}

int stafuSprM295Record2BoundaryEvidence(const char* installed,const char* cd,const char* optional){
    m295Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m295Mode=false;
    if(result==0)std::puts("SPR_M295_NOTE Bit boundaries and local repetitions are observational evidence, not compression-code or decoder-state validation.");
    return result;
}

int stafuSprM296Record2EvidenceRanking(const char* installed,const char* cd,const char* optional){
    m296Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m296Mode=false;
    if(result==0)std::puts("SPR_M296_NOTE Changed-path evidence only; no scalar confidence, codec inference, or decoder-state claim. Dominance requires equal boundary sample counts.");
    return result;
}

int stafuSprM297Record2BoundaryNormalized(const char* installed,const char* cd,const char* optional){
    m297Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m297Mode=false;
    if(result==0)std::puts("SPR_M297_NOTE Boundary rates use up to 16 flanking bits per side per changed match. Different run counts imply different sampled boundaries; rates are descriptive, not calibrated likelihoods. Exact-match interiors are guaranteed by candidate construction. Uniqueness sample lengths are nested and correlated, not independent evidence. Censored short runs are excluded from each sample-length denominator. No decoder-state or codec validation.");
    return result;
}

int stafuSprM298Record2BoundarySensitivity(const char* installed,const char* cd,const char* optional){
    m298Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m298Mode=false;
    if(result==0)std::puts("SPR_M298_NOTE Boundary windows 8/16/24/32 are nested and correlated; rates are descriptive. Equal sample counts are explicitly marked. Preference changes are measured only between comparable nonempty windows. Exact matches and uniqueness are separate observations, not decoder validation.");
    return result;
}

int stafuSprM299Record2Segment9Discrimination(const char* installed,const char* cd,const char* optional){
    m299Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m299Mode=false;
    if(result==0)std::puts("SPR_M299_NOTE Segment-nine comparisons hold neighboring exact-match anchors fixed. Unmatched spans and shift travel are geometric descriptions of competing bit correspondences, not decoder-state or codec validation.");
    return result;
}

int stafuSprM300Record2UnmatchedPatterns(const char* installed,const char* cd,const char* optional){
    m300Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m300Mode=false;
    if(result==0)std::puts("SPR_M300_NOTE Prefix/suffix and local offsets are descriptive bit-pattern comparisons. Repeated pattern counts include the initial occurrence. No codec inference is asserted.");
    return result;
}

int stafuSprM301Record2OffsetRobustness(const char* installed,const char* cd,const char* optional){
    m301Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m301Mode=false;
    if(result==0)std::puts("SPR_M301_NOTE Fixed +/-32 offsets, minimum overlaps 16/32/48; controls use flanking record regions. Observational only, not codec validation.");
    return result;
}

int stafuSprM302Record2OffsetSignificance(const char* installed,const char* cd,const char* optional){
    m302Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m302Mode=false;
    if(result==0)std::puts("SPR_M302_NOTE 256 deterministic bit permutations per sample and overlap threshold; each null trial searches all eligible +/-32 offsets. Empirical tail is descriptive, not a calibrated p-value: permutation destroys local dependence, tests share data, and paths were selected post hoc.");
    return result;
}


int stafuSprM303Record2StructureControls(const char* installed,const char* cd,const char* optional){
    m303Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m303Mode=false;
    if(result==0)std::puts("SPR_M303_NOTE 256 circular shifts and 4/8/16-bit block shuffles per sample/threshold; searched and fixed-overlap counts are descriptive, not independent calibrated p-values. Boundary means offset +/-32.");
    return result;
}

int stafuSprM304Record2ExactExtension(const char* installed,const char* cd,const char* optional){
    m304Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m304Mode=false;
    if(result==0)std::puts("SPR_M304_NOTE Fixed-shift exact islands and flanks are observational; extensions may cross segment anchors and do not validate a codec or decoder state.");
    return result;
}

int stafuSprM305Record2AnchorConstrained(const char* installed,const char* cd,const char* optional){
    m305Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m305Mode=false;
    if(result==0)std::puts("SPR_M305_NOTE Exact runs are clipped to each path's unmatched spans. Outside extensions can cross anchors and are excluded from confined evidence; fixed offsets remain post-hoc observations, not codec validation.");
    return result;
}

int stafuSprM306Record2FixedOffsetSpecificity(const char* installed,const char* cd,const char* optional){
    m306Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m306Mode=false;
    if(result==0)std::puts("SPR_M306_NOTE All eligible offsets +/-32 within the same unmatched intervals; nearby record-pair windows are deterministic sparse samples, not matched-distribution controls. Selected offsets and 26-bit threshold were chosen post hoc. No calibrated significance or codec validation.");
    return result;
}

int stafuSprM307Record2WideMatchedControls(const char* installed,const char* cd,const char* optional){
    m307Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m307Mode=false;
    if(result==0)std::puts("SPR_M307_NOTE All overlap-eligible shifts in anchored unmatched spans; nearby controls match comparison length, exclude shift zero, and use deterministic overlapping windows. Selected runs and thresholds are post hoc; control pairs are not an exchangeable null distribution. No calibrated significance or codec validation.");
    return result;
}

int stafuSprM308Record2MotifCensus(const char* installed,const char* cd,const char* optional){
    m308Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m308Mode=false;
    m308Records.clear();
    if(result==0)std::puts("SPR_M308_NOTE Motif selected post hoc from baseline exact run. Overlapping occurrences and adjacent records are descriptive, not independent null samples or codec validation.");
    return result;
}

int stafuSprM309Record2MotifContext(const char* installed,const char* cd,const char* optional){
    m309Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m309Mode=false;
    m308Records.clear();
    if(result==0)std::puts("SPR_M309_NOTE All cross-record pairs of post-hoc motif occurrences; 32-bit left/right context and contiguous extensions are descriptive, correlated and not calibrated significance or codec validation.");
    return result;
}

int stafuSprM310Record2PerfectContextChains(const char* installed,const char* cd,const char* optional){
    m310Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m310Mode=false;
    m308Records.clear();
    if(result==0)std::puts("SPR_M310_NOTE Perfect 32+32 flank pairs are selected post hoc from a repeated motif. Full exact extensions are bounded by record edges. Chains require adjacent record ordinals and increasing positions but are descriptive, not codec validation.");
    return result;
}

int stafuSprM311Record2UniqueExactIntervals(const char* installed,const char* cd,const char* optional){
    m311Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m311Mode=false;
    m308Records.clear();
    if(result==0)std::puts("SPR_M311_NOTE Motif-seeded exact intervals >=64 bits, fully extended to record boundaries and deduplicated by record pair and both endpoints. Overlapping observations are not independent. Segment-nine intersections use baseline coordinates, not decoder-state evidence.");
    return result;
}

int stafuSprM312Record2ShiftBoundaryAnalysis(const char* installed,const char* cd,const char* optional){
    m312Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m312Mode=false;
    m308Records.clear();
    if(result==0)std::puts("SPR_M312_NOTE Three longest shifted motif-seeded intervals; 32-bit boundary contexts and nearest nonoverlapping A-coordinate intervals. Gap/shift arithmetic is an identity, not independent evidence for insertion, deletion, replacement, or codec behavior.");
    return result;
}

int stafuSprM313Record2SuffixStructure(const char* installed,const char* cd,const char* optional){
    if(!m313Selftest()){
        std::fprintf(stderr,"SPR_M313_SELFTEST FAIL\\n");
        return 1;
    }
    if(std::string(installed)=="__selftest__"){
        std::puts("SPR_M313_SELFTEST PASS");
        return 0;
    }
    m313Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m313Mode=false;
    m308Records.clear();
    if(result==0)std::puts("SPR_M313_NOTE Direct record-end aligned bit suffixes for records 4-7; prefix mismatch counts and suffix nesting are descriptive. A shared suffix may be padding or repeated content; no decoder or format inference is established.");
    return result;
}

int stafuSprM314Record2SuffixBoundary(const char* installed,const char* cd,const char* optional){
    if(!m314Selftest()){
        std::fprintf(stderr,"SPR_M314_SELFTEST FAIL\n");
        return 1;
    }
    if(std::string(installed)=="__selftest__"){
        std::puts("SPR_M314_SELFTEST PASS");
        return 0;
    }
    m314Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m314Mode=false;
    m308Records.clear();
    if(result==0)std::puts("SPR_M314_NOTE Bit-level suffix boundary and fixed M310 internal interval coordinates. Reference intervals are independently observed, not proof of common codec structure. Read-only diagnostic.");
    return result;
}

int stafuSprM315Record2CommonTerminal(const char* installed,const char* cd,const char* optional){
    if(!m315Selftest()){
        std::fprintf(stderr,"SPR_M315_SELFTEST FAIL\\n");
        return 1;
    }
    if(std::string(installed)=="__selftest__"){
        std::puts("SPR_M315_SELFTEST PASS");
        return 0;
    }
    m315Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m315Mode=false;
    m308Records.clear();
    if(result==0)std::puts("SPR_M315_NOTE Direct record-end aligned bit comparisons across records 4-7. Pairwise extra suffixes and boundary contexts are descriptive, not evidence of padding, encoding, or decoder behavior.");
    return result;
}

int stafuSprM316Record2TerminalContext(const char* installed,const char* cd,const char* optional){
    if(!m316Selftest()){
        std::fprintf(stderr,"SPR_M316_SELFTEST FAIL\\n");
        return 1;
    }
    if(std::string(installed)=="__selftest__"){
        std::puts("SPR_M316_SELFTEST PASS");
        return 0;
    }
    m316Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m316Mode=false;
    m308Records.clear();
    if(result==0)std::puts("SPR_M316_NOTE Direct terminal suffix and nearby shifted comparisons. Historical segment-nine coordinates are reference observations, not proven related structure; no decoder inference.");
    return result;
}

int stafuSprM317Record2TerminalTransition(const char* installed,const char* cd,const char* optional){
    if(!m317Selftest()){
        std::fprintf(stderr,"SPR_M317_SELFTEST FAIL\n");
        return 1;
    }
    if(std::string(installed)=="__selftest__"){
        std::puts("SPR_M317_SELFTEST PASS");
        return 0;
    }
    m317Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m317Mode=false;
    m308Records.clear();
    if(result==0)std::puts("SPR_M317_NOTE Fixed M316 intervals, direct gap bits, exhaustive fixed-window occurrences, and nearby-shift exact runs. Geometry and uniqueness are descriptive; no insertion/deletion or decoder inference.");
    return result;
}

int stafuSprM318Record2GapAlignment(const char* installed,const char* cd,const char* optional){
    if(!m318Selftest()){std::fprintf(stderr,"SPR_M318_SELFTEST FAIL\\n");return 1;}
    if(std::string(installed)=="__selftest__"){std::puts("SPR_M318_SELFTEST PASS");return 0;}
    m318Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m318Mode=false;
    m308Records.clear();
    if(result==0)std::puts("SPR_M318_NOTE Gap edit scripts and intermediate shifts are observational, not evidence of insertion/deletion operations or codec semantics.");
    return result;
}

int stafuSprM319Record2BoundaryAlternatives(const char* installed,const char* cd,const char* optional){
    if(!m319Selftest()){std::fprintf(stderr,"SPR_M319_SELFTEST FAIL\n");return 1;}
    if(std::string(installed)=="__selftest__"){std::puts("SPR_M319_SELFTEST PASS");return 0;}
    m319Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m319Mode=false;
    m308Records.clear();
    if(result==0)std::puts("SPR_M319_NOTE Enumerates minimum-cost bit edit scripts for fixed gaps only; alternatives do not establish codec semantics.");
    return result;
}

int stafuSprM320Record2CrossRecordCandidates(const char* installed,const char* cd,const char* optional){
    if(!m320Selftest()){std::fprintf(stderr,"SPR_M320_SELFTEST FAIL\n");return 1;}
    if(std::string(installed)=="__selftest__"){std::puts("SPR_M320_SELFTEST PASS");return 0;}
    m320Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m320Mode=false;
    m308Records.clear();
    return result;
}

int stafuSprM321Record2PreOverlapBoundary(const char* installed,const char* cd,const char* optional){
    if(!m321Selftest()){std::fprintf(stderr,"SPR_M321_SELFTEST FAIL\n");return 1;}
    if(std::string(installed)=="__selftest__"){std::puts("SPR_M321_SELFTEST PASS");return 0;}
    m321Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m321Mode=false;
    m308Records.clear();
    return result;
}

int stafuSprM322Record2CandidateContext(const char* installed,const char* cd,const char* optional){
    if(!m322Selftest()){std::fprintf(stderr,"SPR_M322_SELFTEST FAIL\n");return 1;}
    if(std::string(installed)=="__selftest__"){std::puts("SPR_M322_SELFTEST PASS");return 0;}
    m322Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m322Mode=false;
    m308Records.clear();
    return result;
}

int stafuSprM323Record2IndependentAlignment(const char* installed,const char* cd,const char* optional){
    if(!m323Selftest()){std::fprintf(stderr,"SPR_M323_SELFTEST FAIL\n");return 1;}
    if(std::string(installed)=="__selftest__"){std::puts("SPR_M323_SELFTEST PASS");return 0;}
    m323Mode=true;
    const int result=stafuSprM293Record2ChainDependencies(installed,cd,optional);
    m323Mode=false;
    m308Records.clear();
    return result;
}
