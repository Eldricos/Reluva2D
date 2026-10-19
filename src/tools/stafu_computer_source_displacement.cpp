// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only source/target displacement correlation for COMPUTER.DB.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace {
uint32_t read32(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p]) | (uint32_t(b[p+1])<<8) |
           (uint32_t(b[p+2])<<16) | (uint32_t(b[p+3])<<24);
}
struct Group {size_t source;uint32_t targets[3];};
struct Association {Group group;size_t index;int64_t sourceDelta,targetDelta;bool sameInterval;};
void rank(const char* label,const std::map<int64_t,size_t>& counts) {
    std::vector<std::pair<int64_t,size_t>> ordered(counts.begin(),counts.end());
    std::sort(ordered.begin(),ordered.end(),[](const auto& a,const auto& b) {
        return a.second!=b.second?a.second>b.second:a.first<b.first;
    });
    std::printf("%s (top 20):\n",label);
    for(size_t i=0;i<std::min<size_t>(20,ordered.size());++i)
        std::printf("  %+lld: %zu\n",(long long)ordered[i].first,ordered[i].second);
}
}
int stafuInspectComputerSourceDisplacement(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-source-displacement: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read("computer.db",bytes,error)) {
        std::fprintf(stderr,"computer-source-displacement: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if(!index.parse(bytes,error)) {
        std::fprintf(stderr,"computer-source-displacement: %s\n",error.c_str());return 1;
    }
    const auto& offsets=index.primaryOffsets();
    std::vector<Association> found;
    std::map<int64_t,size_t> sourceDeltas,clusterSources,zeroSources;
    size_t cluster=0,zero=0,other=0;
    size_t sameAll=0,clusterSame=0,zeroSame=0;
    for(size_t p=index.tableEnd();p+16<=bytes.size();++p) {
        if(read32(bytes,p)!=0)continue;
        Group g{p,{read32(bytes,p+4),read32(bytes,p+8),read32(bytes,p+12)}};
        if(g.targets[0]==0 || !(g.targets[0]<g.targets[1] &&
           g.targets[1]<g.targets[2]) || g.targets[2]>=bytes.size())continue;
        auto it=std::upper_bound(offsets.begin(),offsets.end(),g.targets[0]);
        if(it==offsets.begin())continue;
        --it;
        const size_t idx=size_t(it-offsets.begin());
        const uint32_t base=*it;
        const uint32_t limit=idx+1<offsets.size()?offsets[idx+1]:uint32_t(bytes.size());
        const int64_t sd=int64_t(p)-base,td=int64_t(g.targets[0])-base;
        const bool same=g.targets[2]<limit;
        found.push_back({g,idx,sd,td,same});
        ++sourceDeltas[sd];
        sameAll+=same;
        if(td==0){++zero;++zeroSources[sd];zeroSame+=same;}
        else if(td>=144 && td<=156){
            ++cluster;++clusterSources[sd];clusterSame+=same;
        }else ++other;
    }
    std::printf("computer.db=%zu primary=%zu candidate groups=%zu\n",
                bytes.size(),offsets.size(),found.size());
    std::printf("target delta categories: zero=%zu 144..156=%zu other=%zu\n",
                zero,cluster,other);
    std::printf("all three targets in first target's primary interval: total=%zu cluster=%zu zero=%zu\n",
                sameAll,clusterSame,zeroSame);
    rank("All source-primary displacements",sourceDeltas);
    rank("Source displacements for first target +144..+156",clusterSources);
    rank("Source displacements for first target +0",zeroSources);
    size_t exact=0,shownExact=0,shownDifferent=0;
    std::puts("Examples with source-primary delta +0x570:");
    for(const auto& a:found)if(a.sourceDelta==0x570){
        ++exact;
        if(shownExact++<12)
            std::printf("  primary[%zu]=%08x source=%08zx targets=%08x,%08x,%08x targetDelta=%+lld sameInterval=%s\n",
                        a.index,offsets[a.index],a.group.source,
                        a.group.targets[0],a.group.targets[1],a.group.targets[2],
                        (long long)a.targetDelta,a.sameInterval?"yes":"no");
    }
    std::printf("source delta +0x570 total=%zu\n",exact);
    std::puts("Counterexamples from target +144..+156 with different source displacement:");
    for(const auto& a:found) {
        if(a.targetDelta<144 || a.targetDelta>156 || a.sourceDelta==0x570)continue;
        if(shownDifferent++>=12)break;
        std::printf("  primary[%zu]=%08x source=%08zx sourceDelta=%+lld targetDelta=%+lld sameInterval=%s\n",
                    a.index,offsets[a.index],a.group.source,
                    (long long)a.sourceDelta,(long long)a.targetDelta,
                    a.sameInterval?"yes":"no");
    }
    std::puts("Observational correlations only; candidate triples are not validated record fields.");
    return 0;
}
