// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only audit of COMPUTER.DB primary-offset and +0x570 source relationships.
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
uint32_t word(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p])|(uint32_t(b[p+1])<<8)|(uint32_t(b[p+2])<<16)|(uint32_t(b[p+3])<<24);
}
uint16_t half(const std::vector<uint8_t>& b,size_t p) {
    return uint16_t(b[p])|(uint16_t(b[p+1])<<8);
}
void context(const std::vector<uint8_t>& b,size_t pos) {
    const size_t begin=pos>12?pos-12:0,end=std::min(b.size(),pos+28);
    std::printf("    bytes [%08zx..%08zx):",begin,end);
    for(size_t k=begin;k<end;++k)std::printf(" %02x",unsigned(b[k]));
    std::putchar('\n');
}
}
int stafuAuditComputerGlobalLayout(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"computer-global-layout: %s\n",error.c_str());return 1;}
    std::vector<uint8_t> b;
    if(!manager.read("computer.db",b,error)){std::fprintf(stderr,"computer-global-layout: %s\n",error.c_str());return 1;}
    reluva::stafu::ComputerDatabaseIndex index;
    if(!index.parse(b,error)){std::fprintf(stderr,"computer-global-layout: %s\n",error.c_str());return 1;}
    const auto& p=index.primaryOffsets();
    const std::array<size_t,19> chosen={0,1,2,3,4,7,19,55,124,160,162,207,218,219,220,221,222,223,345};
    size_t valid=0,inOwn=0,inNext=0,inLater=0,crossing=0,overlapNext=0,sourceInsideOther=0;
    size_t gapsSmall=0,gapsLarge=0,sourceBeforeFirst=0,sourceAfterLast=0;
    std::map<size_t,size_t> ownerDistance,sourceDeltaToNext;
    std::printf("computer.db=%zu primary=%zu tableEnd=%zu\n",b.size(),p.size(),index.tableEnd());
    std::printf("offset table region [00000004..%08zx); +0x570 for primary[0]=00000570\n",index.tableEnd());
    context(b,0x570);
    for(size_t i=0;i<p.size();++i) {
        const uint64_t s=uint64_t(p[i])+0x570;
        if(s+16>b.size())continue;
        const size_t end=i+1<p.size()?p[i+1]:b.size();
        const size_t width=end-size_t(p[i]);
        const auto upper=std::upper_bound(p.begin(),p.end(),uint32_t(s));
        const size_t owner=upper==p.begin()?0:size_t(upper-p.begin()-1);
        const std::array<uint32_t,3> t={word(b,size_t(s)+4),word(b,size_t(s)+8),word(b,size_t(s)+12)};
        const bool good=t[0]>=p[i] && t[0]<t[1] && t[1]<t[2] && t[2]<end;
        valid+=good;
        if(s>=p[i] && s+16<=end)++inOwn;
        else if(i+1<p.size() && s>=p[i+1] && (i+2==p.size() || s<p[i+2]))++inNext;
        else if(s>=end)++inLater;
        if(s<end && s+16>end)++crossing;
        if(i+1<p.size() && uint64_t(p[i+1])-p[i]<16)++overlapNext;
        if(owner!=i)++sourceInsideOther;
        ++ownerDistance[owner-i];
        if(i+1<p.size())++sourceDeltaToNext[width];
        if(width<=0x570)++gapsSmall;else ++gapsLarge;
        if(s<index.tableEnd())++sourceBeforeFirst;
        if(s>=b.size())++sourceAfterLast;
        if(std::find(chosen.begin(),chosen.end(),i)==chosen.end())continue;
        std::printf("\nprimary[%zu]=%08x next=%08zx width=%zu source=%08llx sourceOwner=%zu ownerDelta=%zu sourceInsideOwn=%s validTriple=%s\n",
                    i,p[i],end,width,static_cast<unsigned long long>(s),owner,owner-i,
                    s+16<=end?"yes":"no",good?"yes":"no");
        std::printf("  fields=(%u,%u) targets=(%08x,%08x,%08x) gaps=(%u,%u)\n",
                    unsigned(half(b,size_t(s))),unsigned(half(b,size_t(s)+2)),
                    t[0],t[1],t[2],t[1]-t[0],t[2]-t[1]);
        context(b,size_t(s));
    }
    std::printf("\nvalid triples=%zu/%zu; source fully inside own interval=%zu; source in immediate next interval=%zu; later interval=%zu\n",
                valid,p.size(),inOwn,inNext,inLater);
    std::printf("source crossing own interval end=%zu source owned by different primary interval=%zu\n",
                crossing,sourceInsideOther);
    std::printf("primary interval widths <=0x570=%zu >0x570=%zu\n",gapsSmall,gapsLarge);
    std::printf("sources before table end=%zu after file end=%zu\n",sourceBeforeFirst,sourceAfterLast);
    std::puts("Source owner index minus originating primary index (top 16):");
    std::vector<std::pair<size_t,size_t>> owners(ownerDistance.begin(),ownerDistance.end());
    std::sort(owners.begin(),owners.end(),[](const auto& a,const auto& b){return a.second!=b.second?a.second>b.second:a.first<b.first;});
    for(size_t i=0;i<std::min<size_t>(16,owners.size());++i)
        std::printf("  ownerDelta=%zu count=%zu\n",owners[i].first,owners[i].second);
    std::puts("Most common primary interval widths (top 12):");
    std::vector<std::pair<size_t,size_t>> widths(sourceDeltaToNext.begin(),sourceDeltaToNext.end());
    std::sort(widths.begin(),widths.end(),[](const auto& a,const auto& b){return a.second!=b.second?a.second>b.second:a.first<b.first;});
    for(size_t i=0;i<std::min<size_t>(12,widths.size());++i)
        std::printf("  width=%zu count=%zu\n",widths[i].first,widths[i].second);
    std::printf("adjacent 16-byte +0x570 source windows overlapping=%zu (count is not evidence of a record boundary)\n",overlapNext);
    std::puts("All positions are observational; +0x570 is a tested correlation, not an asserted record structure.");
    return valid==p.size()?0:1;
}
