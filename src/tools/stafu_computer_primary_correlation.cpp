// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only correlation survey: candidate LE32 triples versus primary offsets.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace {
uint32_t word(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p]) | uint32_t(b[p+1])<<8 |
           uint32_t(b[p+2])<<16 | uint32_t(b[p+3])<<24;
}
struct Candidate {size_t source; uint32_t a,b,c;};
bool textStart(const std::vector<uint8_t>& b,uint32_t p) {
    if(p>=b.size() || b[p]<32 || b[p]>126)return false;
    size_t count=0;
    while(p+count<b.size() && count<12 && b[p+count]>=32 && b[p+count]<=126)++count;
    return count>=4;
}
bool constantRun(const std::vector<uint8_t>& b,uint32_t p) {
    if(p>=b.size())return false;
    size_t left=p,right=p+1;
    while(left>0 && b[left-1]==b[p] && p-left<16)--left;
    while(right<b.size() && b[right]==b[p] && right-p<16)++right;
    return right-left>=16;
}
}
int stafuCorrelateComputerPrimary(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-primary-correlation: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read("computer.db",bytes,error)) {
        std::fprintf(stderr,"computer-primary-correlation: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if(!index.parse(bytes,error)) {
        std::fprintf(stderr,"computer-primary-correlation: %s\n",error.c_str());return 1;
    }
    const auto& offsets=index.primaryOffsets();
    std::vector<Candidate> candidates;
    for(size_t p=index.tableEnd();p+16<=bytes.size();++p) {
        if(word(bytes,p)!=0)continue;
        const auto a=word(bytes,p+4),b=word(bytes,p+8),c=word(bytes,p+12);
        if(a==0 || !(a<b && b<c) || c>=bytes.size())continue;
        candidates.push_back({p,a,b,c});
    }
    std::map<int64_t,size_t> distanceCounts;
    size_t correlated=0,text=0,constant=0,other=0,plus151=0;
    std::vector<size_t> examples,contradictions;
    std::printf("computer.db=%zu primary=%zu candidate groups=%zu\n",
                bytes.size(),offsets.size(),candidates.size());
    std::puts("Association rule: first target belongs to nearest preceding primary offset.");
    std::puts("Source location is reported independently (it may cross primary intervals).");
    for(size_t i=0;i<candidates.size();++i) {
        const auto& c=candidates[i];
        auto it=std::upper_bound(offsets.begin(),offsets.end(),c.a);
        if(it==offsets.begin())continue;
        --it;
        const size_t primaryIndex=size_t(it-offsets.begin());
        const uint32_t base=*it;
        ++correlated;
        const int64_t delta=int64_t(c.a)-base;
        ++distanceCounts[delta];
        if(delta==151){++plus151;if(examples.size()<12)examples.push_back(i);}
        else if(contradictions.size()<12)contradictions.push_back(i);
        if(textStart(bytes,c.a))++text;
        else if(constantRun(bytes,c.a))++constant;
        else ++other;
    }
    std::printf("associated=%zu first-target text-prefix=%zu constant-run=%zu other=%zu\n",
                correlated,text,constant,other);
    std::printf("first-target delta +151 count=%zu\n",plus151);
    std::vector<std::pair<int64_t,size_t>> ranking(distanceCounts.begin(),distanceCounts.end());
    std::sort(ranking.begin(),ranking.end(),[](const auto& x,const auto& y) {
        return x.second!=y.second?x.second>y.second:x.first<y.first;
    });
    std::puts("Most frequent first-target deltas (top 20):");
    for(size_t i=0;i<std::min<size_t>(20,ranking.size());++i)
        std::printf("  %+lld: %zu\n",(long long)ranking[i].first,ranking[i].second);
    auto print=[&](const char* label,const std::vector<size_t>& ids) {
        std::printf("%s:\n",label);
        for(size_t i:ids) {
            const auto& c=candidates[i];
            auto it=std::upper_bound(offsets.begin(),offsets.end(),c.a)-1;
            const auto base=*it;
            const char* kind=textStart(bytes,c.a)?"text-prefix":
                             constantRun(bytes,c.a)?"constant-run":"other";
            std::printf("  primary[%zu]=0x%08x source=0x%08zx targets=%08x,%08x,%08x delta=%+lld kind=%s\n",
                        size_t(it-offsets.begin()),base,c.source,c.a,c.b,c.c,
                        (long long)(int64_t(c.a)-base),kind);
        }
    };
    print("Examples at +151",examples);
    print("Counterexamples with different deltas",contradictions);
    std::puts("Correlation is observational; neither candidate groups nor primary offsets are assigned record semantics.");
    return 0;
}
