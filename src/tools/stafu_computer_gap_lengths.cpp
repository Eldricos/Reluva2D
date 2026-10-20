// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only test of whether reference gaps track nearby identifier/label/title lengths.
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
    return uint32_t(b[p])|(uint32_t(b[p+1])<<8)|(uint32_t(b[p+2])<<16)|(uint32_t(b[p+3])<<24);
}
bool printable(uint8_t c){return c>=32 && c<=126;}
struct Triple {size_t start;std::array<size_t,3> lengths;std::array<std::string,3> values;};
std::vector<Triple> findTriples(const std::vector<uint8_t>& b,size_t begin,size_t end) {
    std::vector<Triple> result;
    // Candidate triples must be three contiguous, nonempty NUL-terminated ASCII strings.
    // This deliberately admits false positives; report candidate multiplicity.
    for(size_t p=begin;p<end;++p) {
        if(!printable(b[p]) || (p && b[p-1]!=0))continue;
        size_t at=p;
        Triple t{};t.start=p;
        bool ok=true;
        for(size_t j=0;j<3;++j) {
            const size_t start=at;
            while(at<end && at-start<=96 && printable(b[at]))++at;
            if(at==start || at-start>96 || at>=end || b[at]!=0){ok=false;break;}
            t.lengths[j]=at-start;
            t.values[j].assign(reinterpret_cast<const char*>(b.data()+start),at-start);
            ++at;
        }
        if(ok)result.push_back(std::move(t));
    }
    return result;
}
void summarize(const char* name,const std::map<int,size_t>& m) {
    std::vector<std::pair<int,size_t>> v(m.begin(),m.end());
    std::sort(v.begin(),v.end(),[](const auto& a,const auto& b){
        return a.second!=b.second?a.second>b.second:a.first<b.first;
    });
    std::printf("%s (top 12):\n",name);
    for(size_t i=0;i<std::min<size_t>(12,v.size());++i)
        std::printf("  difference=%+d count=%zu\n",v[i].first,v[i].second);
}
}
int stafuCompareComputerGapLengths(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"computer-gap-lengths: %s\n",error.c_str());return 1;}
    std::vector<uint8_t> b;
    if(!manager.read("computer.db",b,error)){std::fprintf(stderr,"computer-gap-lengths: %s\n",error.c_str());return 1;}
    reluva::stafu::ComputerDatabaseIndex index;
    if(!index.parse(b,error)){std::fprintf(stderr,"computer-gap-lengths: %s\n",error.c_str());return 1;}
    const auto& bases=index.primaryOffsets();
    const std::array<size_t,16> selected={7,19,24,35,55,108,124,160,162,173,207,219,221,222,223,345};
    std::map<int,size_t> gap0Id,gap0Label,gap1Label,gap1Title;
    size_t valid=0,withCandidates=0,uniqueCandidates=0,anyExact=0,bothExact=0;
    size_t candidateCount=0;
    std::printf("computer.db=%zu primary=%zu\n",b.size(),bases.size());
    for(size_t i=0;i<bases.size();++i) {
        const uint64_t source=uint64_t(bases[i])+0x570;
        if(source+16>b.size())continue;
        const std::array<uint32_t,3> t={read32(b,size_t(source)+4),
            read32(b,size_t(source)+8),read32(b,size_t(source)+12)};
        const size_t end=i+1<bases.size()?bases[i+1]:b.size();
        if(t[0]<bases[i] || t[2]>=end || !(t[0]<t[1] && t[1]<t[2]))continue;
        ++valid;
        const int g0=int(t[1]-t[0]),g1=int(t[2]-t[1]);
        // Search around primary offset, including beyond the 256-byte window.
        // Never infer record boundaries from this heuristic.
        const size_t windowEnd=std::min(end,size_t(bases[i])+512);
        const auto triples=findTriples(b,bases[i],windowEnd);
        candidateCount+=triples.size();
        if(!triples.empty())++withCandidates;
        if(triples.size()==1)++uniqueCandidates;
        bool match0=false,match1=false,matchBoth=false;
        for(const auto& x:triples) {
            const int id=int(x.lengths[0])+1,label=int(x.lengths[1])+1,title=int(x.lengths[2])+1;
            ++gap0Id[g0-id];++gap0Label[g0-label];
            ++gap1Label[g1-label];++gap1Title[g1-title];
            const bool m0=g0==id,m1=g1==label;
            match0|=m0;match1|=m1;matchBoth|=m0&&m1;
        }
        anyExact+=match0||match1;
        bothExact+=matchBoth;
        if(std::find(selected.begin(),selected.end(),i)==selected.end())continue;
        std::printf("\nprimary[%zu]=%08x gaps=(%d,%d) candidateTriples=%zu\n",
                    i,bases[i],g0,g1,triples.size());
        for(size_t k=0;k<std::min<size_t>(3,triples.size());++k) {
            const auto& x=triples[k];
            std::printf("  candidate[%zu] at %08zx lengths=(%zu,%zu,%zu) includingNul=(%zu,%zu,%zu)\n",
                        k,x.start,x.lengths[0],x.lengths[1],x.lengths[2],
                        x.lengths[0]+1,x.lengths[1]+1,x.lengths[2]+1);
            for(size_t j=0;j<3;++j)std::printf("    string[%zu]=\"%.72s\"\n",j,x.values[j].c_str());
            std::printf("    gap0-id+Nul=%+d gap1-label+Nul=%+d\n",
                        g0-int(x.lengths[0]+1),g1-int(x.lengths[1]+1));
        }
    }
    std::printf("\nvalid triples=%zu entries with string candidates=%zu unique candidate=%zu total candidates=%zu\n",
                valid,withCandidates,uniqueCandidates,candidateCount);
    std::printf("entries with either gap matching ID/label NUL length=%zu both matching same candidate=%zu\n",
                anyExact,bothExact);
    summarize("gap0 - (identifier length+1)",gap0Id);
    summarize("gap0 - (label length+1)",gap0Label);
    summarize("gap1 - (label length+1)",gap1Label);
    summarize("gap1 - (title length+1)",gap1Title);
    std::puts("All candidate string triples counted; matches are exploratory, not record assignments.");
    return valid==bases.size()?0:1;
}
