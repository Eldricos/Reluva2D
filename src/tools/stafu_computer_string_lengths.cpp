// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only comparison of COMPUTER.DB triple targets with nearby string spans.
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
uint32_t read32(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p]) | (uint32_t(b[p+1])<<8) |
           (uint32_t(b[p+2])<<16) | (uint32_t(b[p+3])<<24);
}
bool ascii(uint8_t c){return c>=32 && c<=126;}
struct Span {size_t begin,end;};
std::vector<Span> stringsIn(const std::vector<uint8_t>& b,size_t begin,size_t end) {
    std::vector<Span> out;
    for(size_t p=begin;p<end;) {
        if(!ascii(b[p])){++p;continue;}
        const size_t start=p;
        while(p<end && ascii(b[p]))++p;
        if(p<end && b[p]==0 && p-start>=3)out.push_back({start,p});
        if(p==start)++p;
    }
    return out;
}
std::string excerpt(const std::vector<uint8_t>& b,size_t p,size_t n) {
    std::string s;
    for(size_t j=0;j<n && p+j<b.size();++j) {
        const auto c=b[p+j];
        if(c==0)s+="\\0";
        else if(ascii(c))s+=char(c);
        else {char tmp[8];std::snprintf(tmp,sizeof(tmp),"\\x%02x",unsigned(c));s+=tmp;}
    }
    return s;
}
}
int stafuCompareComputerStringLengths(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-string-lengths: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> b;
    if(!manager.read("computer.db",b,error)) {
        std::fprintf(stderr,"computer-string-lengths: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if(!index.parse(b,error)) {
        std::fprintf(stderr,"computer-string-lengths: %s\n",error.c_str());return 1;
    }
    const auto& bases=index.primaryOffsets();
    const std::array<size_t,18> chosen={4,7,19,24,35,55,79,108,124,160,162,173,207,219,221,222,223,345};
    size_t valid=0,exactAdjacent=0,starts=0,ends=0,within=0,neither=0;
    size_t consecutiveStarts=0,consecutiveEnds=0;
    std::map<size_t,size_t> nearbySpanCounts;
    std::printf("computer.db=%zu primary=%zu\n",b.size(),bases.size());
    for(size_t i=0;i<bases.size();++i) {
        const uint64_t source=uint64_t(bases[i])+0x570;
        if(source+16>b.size())continue;
        const std::array<uint32_t,3> t={read32(b,size_t(source)+4),
                                       read32(b,size_t(source)+8),
                                       read32(b,size_t(source)+12)};
        const size_t end=i+1<bases.size()?bases[i+1]:b.size();
        if(t[0]<bases[i] || t[2]>=end || !(t[0]<t[1] && t[1]<t[2]))continue;
        ++valid;
        // Restrict candidate strings to the primary interval, and to the first
        // 256 bytes following its beginning. Do not infer record boundaries.
        const size_t windowEnd=std::min(end,size_t(bases[i])+256);
        const auto spans=stringsIn(b,bases[i],windowEnd);
        ++nearbySpanCounts[spans.size()];
        std::array<int,3> beginIndex={-1,-1,-1},endIndex={-1,-1,-1};
        for(size_t j=0;j<3;++j) {
            bool inside=false;
            for(size_t k=0;k<spans.size();++k) {
                if(t[j]==spans[k].begin)beginIndex[j]=int(k);
                if(t[j]==spans[k].end)endIndex[j]=int(k);
                if(t[j]>spans[k].begin && t[j]<spans[k].end)inside=true;
            }
            if(beginIndex[j]>=0)++starts;
            if(endIndex[j]>=0)++ends;
            if(beginIndex[j]<0 && endIndex[j]<0) {
                if(inside)++within;else ++neither;
            }
        }
        if(beginIndex[0]>=0 && beginIndex[1]==beginIndex[0]+1 &&
           beginIndex[2]==beginIndex[1]+1)++consecutiveStarts;
        if(endIndex[0]>=0 && endIndex[1]==endIndex[0]+1 &&
           endIndex[2]==endIndex[1]+1)++consecutiveEnds;
        if(t[1]-t[0]<=64 && t[2]-t[1]<=64)++exactAdjacent;
        if(std::find(chosen.begin(),chosen.end(),i)==chosen.end())continue;
        std::printf("\nprimary[%zu]=%08x fields=%02x %02x %02x %02x targets=%08x,%08x,%08x gaps=%u,%u spans=%zu\n",
                    i,bases[i],b[size_t(source)],b[size_t(source)+1],
                    b[size_t(source)+2],b[size_t(source)+3],
                    t[0],t[1],t[2],t[1]-t[0],t[2]-t[1],spans.size());
        for(size_t j=0;j<3;++j) {
            const size_t p=t[j],lo=p>=12?p-12:0;
            std::printf("  target[%zu] context [%08zx..]: %s\n",
                        j,lo,excerpt(b,lo,32).c_str());
        }
        for(size_t k=0;k<std::min<size_t>(6,spans.size());++k) {
            const auto& s=spans[k];
            std::printf("  string[%zu] start=%08zx endNul=%08zx length=%zu text=\"%s\"\n",
                        k,s.begin,s.end,s.end-s.begin,excerpt(b,s.begin,std::min<size_t>(s.end-s.begin,48)).c_str());
        }
        std::printf("  target start-string indexes=(%d,%d,%d) end-string indexes=(%d,%d,%d)\n",
                    beginIndex[0],beginIndex[1],beginIndex[2],endIndex[0],endIndex[1],endIndex[2]);
    }
    std::printf("\nvalid triples=%zu; gaps both <=64=%zu\n",valid,exactAdjacent);
    std::printf("target classification (all %zu targets): atStringStart=%zu atNulEnd=%zu insideString=%zu other=%zu\n",
                valid*3,starts,ends,within,neither);
    std::printf("three consecutive string starts=%zu three consecutive string ends=%zu\n",
                consecutiveStarts,consecutiveEnds);
    std::puts("Number of printable NUL-terminated spans in first 256 bytes of each primary interval:");
    for(const auto& [count,n]:nearbySpanCounts)std::printf("  spans=%zu entries=%zu\n",count,n);
    std::puts("String detection is ASCII/NUL heuristic; intervals are comparison windows, not asserted records.");
    return valid==bases.size()?0:1;
}
