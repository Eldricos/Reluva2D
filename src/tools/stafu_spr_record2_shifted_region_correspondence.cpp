// SPDX-License-Identifier: GPL-3.0-or-later
// M279: bounded exact shifted-byte matches after record-2 direct payload prefixes.
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
struct Totals { size_t pairs=0,withCandidates=0,withChain=0,candidates=0,chainBytes=0,endSuffixPairs=0; };
constexpr int kMaxShift=64;
constexpr size_t kMinRun=16,kMaxShown=8;
bool isRef(const View& v){return v.marker==0x00030000&&v.size==4;}
size_t commonPrefix(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b){
    size_t i=0;while(i<std::min(a.size(),b.size())&&a[i]==b[i])++i;return i;
}
size_t commonSuffix(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b){
    size_t i=0;while(i<std::min(a.size(),b.size())&&a[a.size()-i-1]==b[b.size()-i-1])++i;return i;
}
std::vector<Match> candidates(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b,size_t prefix){
    std::vector<Match> found;
    // Only search the region after the shared prefix, at nonzero relative displacements.
    for(int shift=-kMaxShift;shift<=kMaxShift;++shift){
        if(shift==0)continue;
        for(size_t i=prefix;i<a.size();++i){
            const long long jSigned=static_cast<long long>(i)+shift;
            if(jSigned<static_cast<long long>(prefix)||jSigned<0||
               jSigned>=static_cast<long long>(b.size()))continue;
            const size_t j=static_cast<size_t>(jSigned);
            if(a[i]!=b[j])continue;
            if(i>prefix&&j>prefix&&a[i-1]==b[j-1])continue; // maximal at this shift
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
// Weighted monotone chain over candidate matches, ordered by A then B; no overlapping ranges.
std::vector<Match> bestChain(const std::vector<Match>& sortedByLength){
    std::vector<Match> v=sortedByLength;
    std::sort(v.begin(),v.end(),[](const Match& x,const Match& y){
        if(x.a!=y.a)return x.a<y.a;
        if(x.b!=y.b)return x.b<y.b;
        return x.len>y.len;
    });
    std::vector<size_t> score(v.size()),count(v.size());
    std::vector<int> parent(v.size(),-1);
    int best=-1;
    for(size_t i=0;i<v.size();++i){
        score[i]=v[i].len;count[i]=1;
        for(size_t j=0;j<i;++j){
            if(v[j].a+v[j].len>v[i].a||v[j].b+v[j].len>v[i].b)continue;
            if(score[j]+v[i].len>score[i]){
                score[i]=score[j]+v[i].len;count[i]=count[j]+1;parent[i]=static_cast<int>(j);
            }
        }
        if(best<0||score[i]>score[static_cast<size_t>(best)])best=static_cast<int>(i);
    }
    std::vector<Match> result;
    while(best>=0){result.push_back(v[static_cast<size_t>(best)]);best=parent[static_cast<size_t>(best)];}
    std::reverse(result.begin(),result.end());return result;
}
bool survey(reluva::stafu::ResourceManager& manager,const char* name,Totals& total){
    std::vector<uint8_t> bytes;std::string error;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"SPR_M279_ERROR resource=%s %s\n",name,error.c_str());return false;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)||index.records().size()<=2){
        std::fprintf(stderr,"SPR_M279_ERROR resource=%s %s\n",name,error.c_str());return false;
    }
    std::vector<Image> direct;
    size_t ordinal=0;
    for(const auto& event:index.records()[2].events){
        if(event.tag!="PMOC"&&event.tag!="MOCS")continue;
        View v;
        if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,event,v)){
            std::fprintf(stderr,"SPR_M279_ERROR resource=%s ordinal=%zu invalid payload\n",name,ordinal);
            return false;
        }
        if(!isRef(v)){
            Image im;im.ordinal=ordinal;im.data.assign(v.data,v.data+v.size);
            direct.push_back(std::move(im));
        }
        ++ordinal;
    }
    Totals local;
    for(size_t k=1;k<direct.size();++k){
        const auto& a=direct[k-1];const auto& b=direct[k];
        if(b.ordinal!=a.ordinal+1)continue;
        ++local.pairs;
        const size_t pre=commonPrefix(a.data,b.data),suf=commonSuffix(a.data,b.data);
        const int endShift=static_cast<int>(b.data.size())-static_cast<int>(a.data.size());
        const auto matches=candidates(a.data,b.data,pre);
        const auto chain=bestChain(matches);
        size_t chainBytes=0;
        for(const auto& m:chain)chainBytes+=m.len;
        local.candidates+=matches.size();local.chainBytes+=chainBytes;
        if(!matches.empty())++local.withCandidates;
        if(!chain.empty())++local.withChain;
        if(suf>=kMinRun&&endShift!=0)++local.endSuffixPairs;
        std::printf("SPR_M279_PAIR resource=%s record=2 ordinalA=%zu ordinalB=%zu bytesA=%zu bytesB=%zu prefix=%zu endSuffix=%zu endShift=%d candidates=%zu chainRuns=%zu chainBytes=%zu\n",
            name,a.ordinal,b.ordinal,a.data.size(),b.data.size(),pre,suf,endShift,
            matches.size(),chain.size(),chainBytes);
        for(size_t i=0;i<std::min(kMaxShown,matches.size());++i){
            const auto& m=matches[i];
            std::printf("SPR_M279_MATCH resource=%s ordinalA=%zu ordinalB=%zu rank=%zu a=%zu b=%zu length=%zu shift=%d matchesEndA=%d matchesEndB=%d\n",
                name,a.ordinal,b.ordinal,i+1,m.a,m.b,m.len,m.shift,
                m.a+m.len==a.data.size()?1:0,m.b+m.len==b.data.size()?1:0);
        }
        for(size_t i=0;i<chain.size();++i){
            const auto& m=chain[i];
            std::printf("SPR_M279_CHAIN resource=%s ordinalA=%zu ordinalB=%zu segment=%zu a=%zu b=%zu length=%zu shift=%d\n",
                name,a.ordinal,b.ordinal,i+1,m.a,m.b,m.len,m.shift);
        }
    }
    total.pairs+=local.pairs;total.withCandidates+=local.withCandidates;
    total.withChain+=local.withChain;total.candidates+=local.candidates;
    total.chainBytes+=local.chainBytes;total.endSuffixPairs+=local.endSuffixPairs;
    std::printf("SPR_M279_RESOURCE resource=%s pairs=%zu withCandidates=%zu candidates=%zu chainBytes=%zu endSuffixPairs=%zu\n",
        name,local.pairs,local.withCandidates,local.candidates,local.chainBytes,local.endSuffixPairs);
    return true;
}
}
int stafuSprM279Record2ShiftedRegionCorrespondence(const char* installed,const char* cd,const char*){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"SPR_M279_ERROR %s\n",error.c_str());return 1;
    }
    const char* names[]={"BRDGPICA.SPR","BRDGDATA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    Totals totals;
    for(const char* name:names)if(!survey(manager,name,totals))return 1;
    std::printf("SPR_M279_SUMMARY resources=5 pairs=%zu withCandidates=%zu candidates=%zu withChain=%zu chainBytes=%zu endSuffixPairs=%zu\n",
        totals.pairs,totals.withCandidates,totals.candidates,totals.withChain,totals.chainBytes,totals.endSuffixPairs);
    std::puts("SPR_M279_NOTE Exact raw-byte runs length>=16 after shared prefix, nonzero shift within +/-64 bytes. Top 8 matches per pair shown; monotone nonoverlapping maximum-byte chain is diagnostic, not a codec interpretation.");
    return 0;
}
