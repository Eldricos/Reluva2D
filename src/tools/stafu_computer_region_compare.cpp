// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only comparison of three COMPUTER.DB regions; no format assumptions.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
bool printable(uint8_t c) { return c >= 32 && c <= 126; }
void region(const std::vector<uint8_t>& b,
            const std::vector<uint32_t>& offsets,
            const char* name, size_t center) {
    constexpr size_t radius = 128;
    constexpr size_t block = 32;
    const size_t begin = center > radius ? center - radius : 0;
    const size_t end = std::min(b.size(), center + radius);
    std::printf("\n=== %s center=0x%08zx range=[0x%08zx,0x%08zx) ===\n",
                name, center, begin, end);
    std::array<size_t,256> counts{};
    for(size_t p=begin;p<end;++p)++counts[b[p]];
    size_t ascii=0;
    for(size_t c=32;c<=126;++c)ascii+=counts[c];
    std::printf("Distribution: size=%zu ASCII=%zu zero=%zu 80=%zu 69=%zu 6f=%zu\n",
                end-begin,ascii,counts[0],counts[0x80],counts[0x69],counts[0x6f]);
    auto it=std::lower_bound(offsets.begin(),offsets.end(),static_cast<uint32_t>(begin));
    if(it!=offsets.begin()) {
        const auto prev=it-1;
        std::printf("Previous primary[%zu]=0x%08x\n",
                    size_t(prev-offsets.begin()),*prev);
    }
    for(;it!=offsets.end() && *it<end;++it)
        std::printf("Inside primary[%zu]=0x%08x\n",
                    size_t(it-offsets.begin()),*it);
    if(it!=offsets.end())
        std::printf("Next primary[%zu]=0x%08x\n",
                    size_t(it-offsets.begin()),*it);
    std::puts("Blocks (32 bytes; printable ASCII is an observation, not decoding):");
    for(size_t p=begin;p<end;p+=block) {
        const size_t stop=std::min(end,p+block);
        size_t text=0,zero=0,high=0;
        for(size_t j=p;j<stop;++j) {
            text+=printable(b[j]);zero+=b[j]==0;high+=b[j]>=128;
        }
        std::printf("  0x%08zx text=%2zu zero=%2zu high=%2zu  hex:",
                    p,text,zero,high);
        for(size_t j=p;j<stop;++j)std::printf(" %02x",unsigned(b[j]));
        std::printf("\n             ASCII: ");
        for(size_t j=p;j<stop;++j)std::putchar(printable(b[j])?char(b[j]):'.');
        std::putchar('\n');
    }
    std::puts("Constant-byte runs >=8 within the comparison window:");
    bool any=false;
    for(size_t p=begin;p<end;) {
        size_t q=p+1;
        while(q<end && b[q]==b[p])++q;
        if(q-p>=8) {
            any=true;
            std::printf("  0x%08zx byte=%02x length=%zu%s%s\n",
                        p,unsigned(b[p]),q-p,
                        p==begin?" [left-clipped]":"",
                        q==end?" [right-clipped]":"");
        }
        p=q;
    }
    if(!any)std::puts("  none");
    // Mark every transition between runs of eight or more repeated bytes
    // and their immediately adjacent nonmatching byte.
    std::puts("Boundary classification is descriptive only.");
}
}
int stafuCompareComputerRegions(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-region-compare: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read("computer.db",bytes,error)) {
        std::fprintf(stderr,"computer-region-compare: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if(!index.parse(bytes,error)) {
        std::fprintf(stderr,"computer-region-compare: %s\n",error.c_str());return 1;
    }
    constexpr size_t centers[]={0x7065,0x7c19,0x4e980};
    if(bytes.size()<=centers[2]) {
        std::fprintf(stderr,"computer-region-compare: file too short for reference offsets\n");
        return 1;
    }
    std::printf("computer.db size=%zu primary offsets=%zu\n",
                bytes.size(),index.primaryOffsets().size());
    std::puts("Fixed 256-byte windows around three observed reference locations.");
    region(bytes,index.primaryOffsets(),"Enterprise text / metadata",centers[0]);
    region(bytes,index.primaryOffsets(),"69 / 6f / zero patterns",centers[1]);
    region(bytes,index.primaryOffsets(),"long 80 run",centers[2]);
    std::puts("\nNo data encoding, record boundary, or offset semantics inferred.");
    return 0;
}
