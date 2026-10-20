// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only audit of candidate metadata-source ordering and boundary contexts.
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
uint32_t u32(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p])|(uint32_t(b[p+1])<<8)|(uint32_t(b[p+2])<<16)|(uint32_t(b[p+3])<<24);
}
uint16_t u16(const std::vector<uint8_t>& b,size_t p) {
    return uint16_t(b[p])|(uint16_t(b[p+1])<<8);
}
const char* kind(uint8_t c) {
    if(c==0)return "NUL";
    if(c==0x80)return "80";
    if(c>=32&&c<=126)return "ASCII";
    return "binary";
}
void dump(const std::vector<uint8_t>& b,size_t at) {
    const size_t lo=at>=20?at-20:0,hi=std::min(b.size(),at+36);
    std::printf("    context [%08zx..%08zx):",lo,hi);
    for(size_t p=lo;p<hi;++p)std::printf(" %02x",unsigned(b[p]));
    std::putchar('\n');
}
void top(const char* label,const std::map<size_t,size_t>& counts) {
    std::vector<std::pair<size_t,size_t>> v(counts.begin(),counts.end());
    std::sort(v.begin(),v.end(),[](const auto& a,const auto& b){
        return a.second!=b.second?a.second>b.second:a.first<b.first;
    });
    std::printf("%s (top 12):\n",label);
    for(size_t i=0;i<std::min<size_t>(12,v.size());++i)
        std::printf("  %zu: %zu\n",v[i].first,v[i].second);
}
}
int stafuAuditComputerSourceOrdering(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager rm;
    std::string error;
    if(!rm.open(installed,cd,error)){std::fprintf(stderr,"computer-source-ordering: %s\n",error.c_str());return 1;}
    std::vector<uint8_t> b;
    if(!rm.read("computer.db",b,error)){std::fprintf(stderr,"computer-source-ordering: %s\n",error.c_str());return 1;}
    reluva::stafu::ComputerDatabaseIndex idx;
    if(!idx.parse(b,error)){std::fprintf(stderr,"computer-source-ordering: %s\n",error.c_str());return 1;}
    const auto& primary=idx.primaryOffsets();
    constexpr uint64_t shift=0x570;
    const std::array<size_t,19> samples={0,1,2,3,4,7,19,55,124,160,162,207,218,219,220,221,222,223,345};
    size_t inBounds=0,valid=0,targetsOwn=0,sourceOwn=0,monotonic=0,spacingEqual=0;
    size_t precedingNul=0,preceding80=0,precedingPrintable=0,precedingOther=0;
    size_t afterTripleZero=0,afterTriplePointer=0;
    std::map<size_t,size_t> ownerDelta,primaryWidths,precedingSameRun;
    std::printf("computer.db=%zu primary=%zu tableEnd=%zu firstSource=%08llx\n",
                b.size(),primary.size(),idx.tableEnd(),static_cast<unsigned long long>(shift));
    for(size_t i=0;i<primary.size();++i) {
        const uint64_t s64=uint64_t(primary[i])+shift;
        if(s64+20>b.size())continue;
        ++inBounds;
        const size_t s=size_t(s64),end=i+1<primary.size()?primary[i+1]:b.size();
        const std::array<uint32_t,3> t={u32(b,s+4),u32(b,s+8),u32(b,s+12)};
        const bool ordered=t[0]<t[1]&&t[1]<t[2];
        const bool own=ordered&&t[0]>=primary[i]&&t[2]<end;
        valid+=ordered&&t[2]<b.size();
        targetsOwn+=own;
        sourceOwn+=s+16<=end;
        if(i+1<primary.size()) {
            const uint64_t next=uint64_t(primary[i+1])+shift;
            monotonic+=next>s64;
            spacingEqual+=next-s64==uint64_t(primary[i+1])-primary[i];
            ++primaryWidths[primary[i+1]-primary[i]];
        }
        const auto ownerIt=std::upper_bound(primary.begin(),primary.end(),uint32_t(s));
        const size_t owner=ownerIt==primary.begin()?0:size_t(ownerIt-primary.begin()-1);
        ++ownerDelta[owner-i];
        const uint8_t before=s?b[s-1]:0;
        precedingNul+=before==0;
        preceding80+=before==0x80;
        precedingPrintable+=before>=32&&before<=126;
        precedingOther+=before!=0&&before!=0x80&&!(before>=32&&before<=126);
        size_t run=0;
        while(s>run&&b[s-1-run]==before)++run;
        ++precedingSameRun[std::min<size_t>(run,32)];
        afterTripleZero+=u32(b,s+16)==0;
        afterTriplePointer+=u32(b,s+16)<b.size()&&u32(b,s+16)!=0;
        if(std::find(samples.begin(),samples.end(),i)==samples.end())continue;
        std::printf("\nprimary[%zu]=%08x next=%08zx source=%08zx owner=%zu fields=(%u,%u)\n",
                    i,primary[i],end,s,owner,unsigned(u16(b,s)),unsigned(u16(b,s+2)));
        std::printf("  targets=(%08x,%08x,%08x) ownInterval=%s sourceOwn=%s before=%s run=%zu afterTriple=%08x\n",
                    t[0],t[1],t[2],own?"yes":"no",s+16<=end?"yes":"no",kind(before),run,u32(b,s+16));
        dump(b,s);
    }
    std::printf("\ninBounds=%zu/%zu validOrderedInFile=%zu targetsInOwnInterval=%zu sourceInOwnInterval=%zu\n",
                inBounds,primary.size(),valid,targetsOwn,sourceOwn);
    std::printf("consecutive source locations increasing=%zu/%zu equal primary spacing=%zu/%zu (arithmetic identity)\n",
                monotonic,primary.size()?primary.size()-1:0,spacingEqual,primary.size()?primary.size()-1:0);
    std::printf("byte immediately before source: NUL=%zu 80=%zu printable=%zu other=%zu\n",
                precedingNul,preceding80,precedingPrintable,precedingOther);
    std::printf("word after three targets: zero=%zu nonzero in-file=%zu\n",afterTripleZero,afterTriplePointer);
    top("source owner minus corresponding primary index",ownerDelta);
    top("primary interval widths",primaryWidths);
    top("same-byte run ending immediately before source (capped at 32)",precedingSameRun);
    std::puts("All positions are observational; these source offsets are hypothesized, not proven metadata boundaries.");
    return inBounds==primary.size()?0:1;
}
