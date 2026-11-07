// SPDX-License-Identifier: GPL-3.0-or-later
// M280: inspect exact shifted-run boundaries in record-2 SPR payloads (read-only).
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
bool survey(reluva::stafu::ResourceManager& manager,const char* name,Totals& total){
    std::vector<uint8_t> bytes;std::string error;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"SPR_M280_ERROR resource=%s %s\n",name,error.c_str());return false;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)||index.records().size()<=2){
        std::fprintf(stderr,"SPR_M280_ERROR resource=%s %s\n",name,error.c_str());return false;
    }
    std::vector<Image> direct;size_t ordinal=0;
    for(const auto& event:index.records()[2].events){
        if(event.tag!="PMOC"&&event.tag!="MOCS")continue;
        View v;
        if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,event,v)){
            std::fprintf(stderr,"SPR_M280_ERROR resource=%s ordinal=%zu invalid payload\n",name,ordinal);return false;
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
        const size_t pre=prefix(a.data,b.data);
        const auto found=candidates(a.data,b.data,pre);
        const auto selected=chain(found);
        local.matches+=found.size();local.chainSegments+=selected.size();
        if(!found.empty())++local.withMatches;
        std::printf("SPR_M280_PAIR resource=%s ordinalA=%zu ordinalB=%zu prefix=%zu candidates=%zu chainSegments=%zu\n",
            name,a.ordinal,b.ordinal,pre,found.size(),selected.size());
        for(size_t i=0;i<found.size();++i){
            const auto& m=found[i];
            const bool leftA=m.a>0,leftB=m.b>0;
            const bool rightA=m.a+m.len<a.data.size(),rightB=m.b+m.len<b.data.size();
            const bool leftEqual=leftA&&leftB&&a.data[m.a-1]==b.data[m.b-1];
            const bool rightEqual=rightA&&rightB&&a.data[m.a+m.len]==b.data[m.b+m.len];
            std::printf("SPR_M280_BOUNDARY resource=%s ordinalA=%zu ordinalB=%zu candidate=%zu a=%zu b=%zu length=%zu shift=%d leftA=%s leftB=%s leftEqual=%d rightA=%s rightB=%s rightEqual=%d maximal=%d\n",
                name,a.ordinal,b.ordinal,i+1,m.a,m.b,m.len,m.shift,
                leftA?"byte":"start",leftB?"byte":"start",leftEqual?1:0,
                rightA?"byte":"end",rightB?"byte":"end",rightEqual?1:0,
                (!leftEqual&&!rightEqual)?1:0);
            if(leftA&&leftB)std::printf("SPR_M280_EDGE resource=%s ordinalA=%zu ordinalB=%zu candidate=%zu side=left byteA=0x%02X byteB=0x%02X\n",
                name,a.ordinal,b.ordinal,i+1,static_cast<unsigned>(a.data[m.a-1]),static_cast<unsigned>(b.data[m.b-1]));
            if(rightA&&rightB)std::printf("SPR_M280_EDGE resource=%s ordinalA=%zu ordinalB=%zu candidate=%zu side=right byteA=0x%02X byteB=0x%02X\n",
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
            std::printf("SPR_M280_GAP resource=%s ordinalA=%zu ordinalB=%zu gap=%zu fromA=%zu toA=%zu fromB=%zu toB=%zu bytesA=%zu bytesB=%zu deltaBytes=%lld shiftBefore=%d shiftAfter=%d deltaShift=%d consistent=%d terminal=%d\n",
                name,a.ordinal,b.ordinal,i,endA,nextA,endB,nextB,gapA,gapB,gapDelta,
                previousShift,nextShift,shiftDelta,consistent?1:0,final?1:0);
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
    std::printf("SPR_M280_RESOURCE resource=%s pairs=%zu withMatches=%zu candidates=%zu chainSegments=%zu gaps=%zu consistentGaps=%zu\n",
        name,local.pairs,local.withMatches,local.matches,local.chainSegments,local.transitions,local.consistentTransitions);
    return true;
}
}
int stafuSprM280Record2ShiftedMatchBoundaryAnalysis(const char* installed,const char* cd,const char*){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"SPR_M280_ERROR %s\n",error.c_str());return 1;
    }
    const char* names[]={"BRDGPICA.SPR","BRDGDATA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    Totals total;
    for(const char* name:names)if(!survey(manager,name,total))return 1;
    std::printf("SPR_M280_SUMMARY resources=5 pairs=%zu withMatches=%zu candidates=%zu chainSegments=%zu gaps=%zu consistentGaps=%zu\n",
        total.pairs,total.withMatches,total.matches,total.chainSegments,total.transitions,total.consistentTransitions);
    std::puts("SPR_M280_NOTE Exact runs length>=16, shifts +/-64, matching M279 selection. Gap delta equals displacement delta algebraically for ordered equal-length match segments; consistency is a sanity check, not independent evidence of insertion/deletion. Boundaries are byte-based, not decoded pixels.");
    return 0;
}
