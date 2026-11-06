// SPDX-License-Identifier: GPL-3.0-or-later
// M278: record-2 raw-byte shared regions, read-only; no codec interpretation.
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
struct Image { size_t ordinal=0; uint32_t marker=0; std::vector<uint8_t> data; };
struct Totals { size_t pairs=0,equalBytes=0,comparedBytes=0,longestRun=0,runCount=0;
                size_t prefixBytes=0,suffixBytes=0,localized=0; };
bool isRef(const View& v){return v.marker==0x00030000&&v.size==4;}
size_t prefix(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b){
    size_t i=0,n=std::min(a.size(),b.size());
    while(i<n&&a[i]==b[i])++i;
    return i;
}
size_t suffix(const std::vector<uint8_t>& a,const std::vector<uint8_t>& b){
    size_t i=0,n=std::min(a.size(),b.size());
    while(i<n&&a[a.size()-1-i]==b[b.size()-1-i])++i;
    return i;
}
bool survey(reluva::stafu::ResourceManager& manager,const char* name,Totals& global) {
    std::vector<uint8_t> bytes;std::string error;
    if(!manager.read(name,bytes,error)){
        std::fprintf(stderr,"SPR_M278_ERROR resource=%s %s\n",name,error.c_str());return false;
    }
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){
        std::fprintf(stderr,"SPR_M278_ERROR resource=%s %s\n",name,error.c_str());return false;
    }
    if(index.records().size()<=2){
        std::fprintf(stderr,"SPR_M278_ERROR resource=%s missing record=2\n",name);return false;
    }
    std::vector<Image> direct;
    size_t ordinal=0,refs=0;
    for(const auto& event:index.records()[2].events){
        if(event.tag!="PMOC"&&event.tag!="MOCS")continue;
        View v;
        if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,event,v)){
            std::fprintf(stderr,"SPR_M278_ERROR resource=%s record=2 ordinal=%zu invalid payload\n",name,ordinal);
            return false;
        }
        if(isRef(v))++refs;
        else {
            Image im;im.ordinal=ordinal;im.marker=v.marker;
            im.data.assign(v.data,v.data+v.size);direct.push_back(std::move(im));
        }
        ++ordinal;
    }
    Totals local;
    for(size_t k=1;k<direct.size();++k){
        const auto& a=direct[k-1];const auto& b=direct[k];
        if(b.ordinal!=a.ordinal+1)continue;
        const size_t n=std::min(a.data.size(),b.data.size());
        const size_t pre=prefix(a.data,b.data),suf=suffix(a.data,b.data);
        size_t equal=0,runs=0,longest=0,internalRuns=0,internalBytes=0;
        size_t firstDiff=n,lastDiff=0,diffRuns=0;
        bool inEqual=false,inDiff=false;
        for(size_t i=0;i<n;){
            const bool same=a.data[i]==b.data[i];
            size_t end=i+1;
            while(end<n&&(a.data[end]==b.data[end])==same)++end;
            if(same){
                ++runs;equal+=end-i;longest=std::max(longest,end-i);
                if(i>0&&end<n){++internalRuns;internalBytes+=end-i;}
                inEqual=true;
            }else{
                ++diffRuns;firstDiff=std::min(firstDiff,i);lastDiff=end;
                inDiff=true;
            }
            i=end;
        }
        // An identical-offset comparison can undercount a shifted suffix when lengths differ.
        const size_t uncovered=n-equal;
        const size_t tailA=a.data.size()-n,tailB=b.data.size()-n;
        const bool localized=diffRuns==1&&tailA==0&&tailB==0;
        ++local.pairs;local.equalBytes+=equal;local.comparedBytes+=n;
        local.longestRun=std::max(local.longestRun,longest);
        local.runCount+=runs;local.prefixBytes+=pre;local.suffixBytes+=suf;
        if(localized)++local.localized;
        std::printf("SPR_M278_PAIR resource=%s record=2 ordinalA=%zu ordinalB=%zu markerA=0x%08X markerB=0x%08X bytesA=%zu bytesB=%zu deltaBytes=%lld compared=%zu equal=%zu different=%zu prefix=%zu suffixEndAligned=%zu equalRuns=%zu internalEqualRuns=%zu internalEqualBytes=%zu longestEqualRun=%zu diffRuns=%zu firstDiff=%zu lastDiffExclusive=%zu tailA=%zu tailB=%zu singleDiffInterval=%d\n",
            name,a.ordinal,b.ordinal,static_cast<unsigned>(a.marker),static_cast<unsigned>(b.marker),
            a.data.size(),b.data.size(),
            static_cast<long long>(b.data.size())-static_cast<long long>(a.data.size()),
            n,equal,uncovered,pre,suf,runs,internalRuns,internalBytes,longest,diffRuns,
            firstDiff,lastDiff,tailA,tailB,localized?1:0);
    }
    global.pairs+=local.pairs;global.equalBytes+=local.equalBytes;
    global.comparedBytes+=local.comparedBytes;
    global.longestRun=std::max(global.longestRun,local.longestRun);
    global.runCount+=local.runCount;global.prefixBytes+=local.prefixBytes;
    global.suffixBytes+=local.suffixBytes;global.localized+=local.localized;
    std::printf("SPR_M278_RESOURCE resource=%s record=2 images=%zu references=%zu direct=%zu pairs=%zu comparedBytes=%zu equalBytes=%zu equalRuns=%zu maxEqualRun=%zu singleDiffInterval=%zu\n",
        name,ordinal,refs,direct.size(),local.pairs,local.comparedBytes,local.equalBytes,
        local.runCount,local.longestRun,local.localized);
    return true;
}
}
int stafuSprM278Record2SharedRegionMapping(const char* installed,const char* cd,const char*) {
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){
        std::fprintf(stderr,"SPR_M278_ERROR %s\n",error.c_str());return 1;
    }
    const char* names[]={"BRDGPICA.SPR","BRDGDATA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    Totals total;
    for(const char* name:names)if(!survey(manager,name,total))return 1;
    std::printf("SPR_M278_SUMMARY resources=5 pairs=%zu comparedBytes=%zu equalBytes=%zu equalRuns=%zu maxEqualRun=%zu singleDiffInterval=%zu\n",
        total.pairs,total.comparedBytes,total.equalBytes,total.runCount,total.longestRun,total.localized);
    std::puts("SPR_M278_NOTE Equal runs and difference intervals use identical offsets up to the shorter payload; suffixEndAligned is measured from each payload end and can overlap prefix. No shift alignment or decoding is inferred.");
    return 0;
}
