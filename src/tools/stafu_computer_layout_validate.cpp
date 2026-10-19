// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only direct validation of the hypothesized COMPUTER.DB +0x570 layout.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
uint32_t le32(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p]) | (uint32_t(b[p+1])<<8) |
           (uint32_t(b[p+2])<<16) | (uint32_t(b[p+3])<<24);
}
void preview(const std::vector<uint8_t>& b,uint32_t p) {
    if(p>=b.size()){std::printf("<out of file>");return;}
    for(size_t j=0;j<48 && size_t(p)+j<b.size();++j) {
        const auto c=b[size_t(p)+j];
        if(c==0){std::printf("\\0");break;}
        if(c>=32 && c<=126)std::putchar(c);
        else {std::printf("\\x%02x",unsigned(c));if(j>=7)break;}
    }
}
bool likelyString(const std::vector<uint8_t>& b,uint32_t p) {
    if(p>=b.size())return false;
    size_t i=0;
    while(size_t(p)+i<b.size() && i<128 && b[size_t(p)+i]>=32 && b[size_t(p)+i]<=126)++i;
    return i>=3 && size_t(p)+i<b.size() && b[size_t(p)+i]==0;
}
bool constant(const std::vector<uint8_t>& b,uint32_t p) {
    if(p>=b.size())return false;
    const auto v=b[p];
    size_t lo=p,hi=p;
    while(lo>0 && b[lo-1]==v && size_t(p)-lo<16)--lo;
    while(hi<b.size() && b[hi]==v && hi-size_t(p)<16)++hi;
    return hi-lo>=16;
}
}
int stafuValidateComputerLayout(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"%s\n",error.c_str());return 1;}
    std::vector<uint8_t> bytes;
    if(!manager.read("computer.db",bytes,error)){std::fprintf(stderr,"%s\n",error.c_str());return 1;}
    reluva::stafu::ComputerDatabaseIndex index;
    if(!index.parse(bytes,error)){std::fprintf(stderr,"%s\n",error.c_str());return 1;}
    const auto& offsets=index.primaryOffsets();
    size_t valid=0,absent=0,outside=0,strings=0,constantTargets=0,otherTargets=0;
    struct Row {size_t idx;uint32_t source;uint32_t target[3];bool match;bool inInterval;};
    std::vector<Row> rows;
    for(size_t i=0;i<offsets.size();++i) {
        const uint32_t base=offsets[i];
        const uint32_t limit=i+1<offsets.size()?offsets[i+1]:uint32_t(bytes.size());
        const uint64_t source=uint64_t(base)+0x570;
        Row r{i,source<=UINT32_MAX?uint32_t(source):0,{0,0,0},false,false};
        if(source+16<=bytes.size()) {
            const size_t p=size_t(source);
            if(le32(bytes,p)==0) {
                r.target[0]=le32(bytes,p+4);
                r.target[1]=le32(bytes,p+8);
                r.target[2]=le32(bytes,p+12);
                r.match=r.target[0]!=0 && r.target[0]<r.target[1] &&
                        r.target[1]<r.target[2] && r.target[2]<bytes.size();
                r.inInterval=r.match && r.target[0]>=base && r.target[2]<limit;
            }
        }
        if(r.match) {
            ++valid;
            if(!r.inInterval)++outside;
            for(uint32_t t:r.target) {
                if(likelyString(bytes,t))++strings;
                else if(constant(bytes,t))++constantTargets;
                else ++otherTargets;
            }
        } else ++absent;
        rows.push_back(r);
    }
    std::printf("computer.db=%zu primary=%zu\n",bytes.size(),offsets.size());
    std::printf("at primary+0x570: candidate=%zu missing/invalid=%zu crossing target interval=%zu\n",
                valid,absent,outside);
    std::printf("candidate target classifications (all three): nul-terminated-printable=%zu constant-run=%zu other=%zu\n",
                strings,constantTargets,otherTargets);
    const size_t samples[]={4,7,19,24,35,55,79,108,111,113,124,160,162,173,207,219,221,222,223,224,285,315,319,338,345};
    std::puts("Representative primary entries (including nonmatches):");
    for(size_t i:samples) {
        if(i>=rows.size())continue;
        const auto& r=rows[i];
        const uint32_t base=offsets[i],limit=i+1<offsets.size()?offsets[i+1]:uint32_t(bytes.size());
        std::printf(" primary[%zu]=%08x next=%08x interval=%u source=%08x candidate=%s targets-in-interval=%s\n",
                    i,base,limit,limit-base,r.source,r.match?"yes":"no",r.inInterval?"yes":"no");
        if(r.match)for(int j=0;j<3;++j) {
            std::printf("   target[%d]=%08x delta=%+lld bytes: ",j,r.target[j],
                        (long long)(int64_t(r.target[j])-base));
            preview(bytes,r.target[j]);std::putchar('\n');
        }
    }
    std::puts("First 12 entries without the expected +0x570 group:");
    size_t shown=0;
    for(const auto& r:rows)if(!r.match && shown++<12) {
        const uint32_t base=offsets[r.idx],limit=r.idx+1<offsets.size()?offsets[r.idx+1]:uint32_t(bytes.size());
        std::printf(" primary[%zu]=%08x interval=%u source=%08x source-in-file=%s\n",
                    r.idx,base,limit-base,r.source,uint64_t(base)+0x570+16<=bytes.size()?"yes":"no");
    }
    std::puts("A matching signature is evidence of structure, not proof of field or string semantics.");
    return 0;
}
