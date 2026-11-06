// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 240: read-only repeated-window spacing survey of SPR payloads.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace {
constexpr size_t kWindow=8;
constexpr size_t kMaxOccurrences=256;
constexpr size_t kMaxRanked=3;
struct Sample {
    size_t record,ordinal,offset,size;
    uint32_t a,b,marker;
    std::string tag;
};
uint64_t fingerprint(const uint8_t* data) {
    uint64_t h=14695981039346656037ull;
    for(size_t i=0;i<kWindow;++i){h^=data[i];h*=1099511628211ull;}
    return h;
}
std::string hex(const uint8_t* p) {
    static constexpr char digits[]="0123456789abcdef";
    std::string result;
    for(size_t i=0;i<kWindow;++i) {
        result.push_back(digits[p[i]>>4]);
        result.push_back(digits[p[i]&15]);
    }
    return result;
}
struct WindowStats {
    size_t occurrences=0,first=0,last=0;
    std::map<size_t,size_t> spacings;
};
using WindowMap=std::map<std::string,WindowStats>;
WindowMap survey(const std::vector<uint8_t>& bytes,const Sample& s) {
    WindowMap result;
    if(s.size<kWindow)return result;
    // Key by actual bytes, rather than hash alone, to exclude collisions.
    for(size_t i=0;i+kWindow<=s.size;++i) {
        const auto key=hex(bytes.data()+s.offset+i);
        auto& stats=result[key];
        if(stats.occurrences && stats.occurrences<kMaxOccurrences)
            ++stats.spacings[i-stats.last];
        if(stats.occurrences==0)stats.first=i;
        stats.last=i;
        ++stats.occurrences;
    }
    return result;
}
}

int stafuSprRepeatedSpacing(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-repeated-spacing: %s\n",error.c_str());
        return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-repeated-spacing: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-repeated-spacing: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        std::vector<Sample> samples;
        for(size_t ri=0;ri<index.records().size();++ri) {
            size_t ordinal=0;
            for(const auto& e:index.records()[ri].events) {
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View v;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,v)&&
                   !(v.marker==0x00030000&&v.size==4))
                    samples.push_back({ri,ordinal,v.offset,v.size,v.headerA,v.headerB,v.marker,e.tag});
                ++ordinal;
            }
        }
        size_t repeatedImages=0,totalRepeatedTypes=0,neighborPairs=0,sharedTypes=0;
        size_t printedImages=0,printedPairs=0;
        WindowMap previous;
        Sample prior{};
        bool havePrevious=false;
        for(const auto& s:samples) {
            auto windows=survey(bytes,s);
            std::vector<std::pair<std::string,WindowStats>> repeated;
            for(const auto& item:windows)
                if(item.second.occurrences>=2)repeated.push_back(item);
            if(!repeated.empty())++repeatedImages;
            totalRepeatedTypes+=repeated.size();
            std::sort(repeated.begin(),repeated.end(),[](const auto& x,const auto& y){
                return x.second.occurrences!=y.second.occurrences
                    ?x.second.occurrences>y.second.occurrences:x.first<y.first;
            });
            // Bound per-resource detail to avoid enormous output.
            if(!repeated.empty()&&printedImages++<12) {
                std::printf("SPR_SPACING_IMAGE resource=%s record=%zu image=%zu payloadBytes=%zu distinctWindows=%zu repeatedTypes=%zu\n",
                    name.c_str(),s.record,s.ordinal,s.size,windows.size(),repeated.size());
                for(size_t i=0;i<std::min(kMaxRanked,repeated.size());++i) {
                    const auto& item=repeated[i];
                    std::vector<std::pair<size_t,size_t>> spacings(item.second.spacings.begin(),item.second.spacings.end());
                    std::sort(spacings.begin(),spacings.end(),[](const auto& x,const auto& y){
                        return x.second!=y.second?x.second>y.second:x.first<y.first;
                    });
                    std::printf("SPR_SPACING_REPEAT resource=%s record=%zu image=%zu window=%s count=%zu first=%zu last=%zu topSpacing=%zu topSpacingCount=%zu\n",
                        name.c_str(),s.record,s.ordinal,item.first.c_str(),
                        item.second.occurrences,item.second.first,item.second.last,
                        spacings.empty()?0:spacings[0].first,spacings.empty()?0:spacings[0].second);
                }
            }
            if(havePrevious&&prior.record==s.record&&prior.tag==s.tag&&
               prior.marker==s.marker&&prior.a==s.a&&prior.b==s.b) {
                ++neighborPairs;
                size_t shared=0,repeatedBoth=0;
                for(const auto& item:windows) {
                    auto it=previous.find(item.first);
                    if(it==previous.end())continue;
                    ++shared;
                    if(item.second.occurrences>=2&&it->second.occurrences>=2)++repeatedBoth;
                }
                sharedTypes+=shared;
                if(printedPairs++<30||repeatedBoth)
                    std::printf("SPR_SPACING_PAIR resource=%s record=%zu imageA=%zu imageB=%zu sharedWindowTypes=%zu repeatedInBoth=%zu windowsA=%zu windowsB=%zu\n",
                        name.c_str(),s.record,prior.ordinal,s.ordinal,shared,repeatedBoth,previous.size(),windows.size());
            }
            previous=std::move(windows);
            prior=s;
            havePrevious=true;
        }
        std::printf("SPR_SPACING_SUMMARY resource=%s direct=%zu windowBytes=%zu repeatedImages=%zu repeatedWindowTypes=%zu neighborPairs=%zu sharedWindowTypes=%zu detailImageLimit=12 detailPairLimit=30\n",
            name.c_str(),samples.size(),kWindow,repeatedImages,totalRepeatedTypes,neighborPairs,sharedTypes);
    }
    std::puts("SPR_SPACING_NOTE overlapping 8-byte windows are included; spacing measures consecutive occurrences of the same exact window; repeated windows do not establish encoded units or codec commands.");
    return failures?1:0;
}
