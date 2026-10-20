// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only comparison of bytes surrounding the proposed +0x570 group.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace {
uint32_t le32(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p]) | uint32_t(b[p+1])<<8 |
           uint32_t(b[p+2])<<16 | uint32_t(b[p+3])<<24;
}
bool signature(const std::vector<uint8_t>& b,size_t p) {
    if(p+16>b.size() || le32(b,p)!=0)return false;
    const auto a=le32(b,p+4),c=le32(b,p+8),d=le32(b,p+12);
    return a!=0 && a<c && c<d && d<b.size();
}
void dump(const std::vector<uint8_t>& b,size_t begin,size_t end) {
    for(size_t p=begin;p<end;p+=16) {
        std::printf("  %08zx: ",p);
        for(size_t j=0;j<16;++j) {
            if(p+j<end)std::printf("%02x ",unsigned(b[p+j]));
            else std::printf("   ");
        }
        std::printf(" |");
        for(size_t j=0;j<16 && p+j<end;++j) {
            const auto c=b[p+j];
            std::putchar(c>=32 && c<=126?c:'.');
        }
        std::puts("|");
    }
}
}
int stafuInspectComputerGroupSurroundings(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-group-surroundings: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read("computer.db",bytes,error)) {
        std::fprintf(stderr,"computer-group-surroundings: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if(!index.parse(bytes,error)) {
        std::fprintf(stderr,"computer-group-surroundings: %s\n",error.c_str());return 1;
    }
    const auto& offsets=index.primaryOffsets();
    const std::array<size_t,17> selected={4,7,19,24,35,55,79,108,124,162,207,219,221,222,223,224,345};
    std::printf("computer.db=%zu primary=%zu\n",bytes.size(),offsets.size());
    std::puts("Hex windows: 32 bytes before and 48 bytes after primary+0x570; source bytes remain read-only.");
    for(size_t i:selected) {
        if(i>=offsets.size())continue;
        const uint64_t raw=uint64_t(offsets[i])+0x570;
        const uint32_t next=i+1<offsets.size()?offsets[i+1]:uint32_t(bytes.size());
        if(raw>=bytes.size()) {
            std::printf("\nprimary[%zu]=%08x candidate outside file\n",i,offsets[i]);
            continue;
        }
        const size_t p=size_t(raw);
        const bool match=signature(bytes,p);
        std::printf("\nprimary[%zu]=%08x next=%08x interval=%u source=%08zx match=%s source-in-interval=%s\n",
                    i,offsets[i],next,next-offsets[i],p,match?"yes":"no",
                    p<next?"yes":"no");
        if(match) {
            std::printf("  candidate LE32: %08x %08x %08x %08x\n",
                        le32(bytes,p),le32(bytes,p+4),le32(bytes,p+8),le32(bytes,p+12));
        }
        const size_t begin=p>=32?p-32:0;
        const size_t end=std::min(bytes.size(),p+48);
        dump(bytes,begin,end);
    }
    std::map<uint32_t,size_t> matchPrefix,missPrefix;
    size_t matches=0,misses=0;
    for(uint32_t base:offsets) {
        const uint64_t raw=uint64_t(base)+0x570;
        if(raw+16>bytes.size())continue;
        const size_t p=size_t(raw);
        const bool match=signature(bytes,p);
        (match?matchPrefix:missPrefix)[p>=4?le32(bytes,p-4):0]++;
        match?++matches:++misses;
    }
    auto report=[](const char* label,const std::map<uint32_t,size_t>& values) {
        std::vector<std::pair<uint32_t,size_t>> sorted(values.begin(),values.end());
        std::sort(sorted.begin(),sorted.end(),[](const auto& a,const auto& b){
            return a.second!=b.second?a.second>b.second:a.first<b.first;
        });
        std::printf("%s (preceding LE32, top 12):\n",label);
        for(size_t i=0;i<std::min<size_t>(12,sorted.size());++i)
            std::printf("  %08x: %zu\n",sorted[i].first,sorted[i].second);
    };
    std::printf("\nAll primary offsets: matching=%zu nonmatching=%zu\n",matches,misses);
    report("Matching-group preceding words",matchPrefix);
    report("Nonmatching-location preceding words",missPrefix);
    std::puts("No record boundaries, pointer semantics, or fixed-size headers are inferred.");
    return 0;
}
