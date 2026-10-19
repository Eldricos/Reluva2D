// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only diagnostic: byte distributions near COMPUTER.DB primary offsets.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
void inspect(const std::vector<uint8_t>& bytes, size_t index, uint32_t offset,
             uint32_t next) {
    const size_t start = offset;
    const size_t end = std::min(bytes.size(), start + 128);
    std::array<size_t, 256> counts{};
    for (size_t p = start; p < end; ++p) ++counts[bytes[p]];
    std::printf("primary[%zu] 0x%08x next=0x%08x span=%u window=%zu\n",
                index, offset, next, next >= offset ? next-offset : 0,
                end-start);
    std::printf("  bytes: 80=%zu 69=%zu 6f=%zu 00=%zu printable=%zu\n",
                counts[0x80], counts[0x69], counts[0x6f], counts[0],
                [&]() { size_t n=0; for(size_t c=32;c<=126;++c)n+=counts[c]; return n; }());
    std::printf("  first 48:");
    for (size_t p = start; p < std::min(end, start+48); ++p)
        std::printf(" %02x", unsigned(bytes[p]));
    std::putchar('\n');
    // Maximal contiguous runs, limited to first 128 bytes of each sample.
    std::printf("  runs (>=4 bytes):");
    size_t printed=0;
    for (size_t p=start;p<end;) {
        size_t q=p+1;
        while(q<end && bytes[q]==bytes[p])++q;
        if(q-p>=4 && printed++<12)
            std::printf(" [0x%zx: %02x x%zu]",p,unsigned(bytes[p]),q-p);
        p=q;
    }
    if(!printed) std::printf(" none");
    std::putchar('\n');
}
}
int stafuInspectComputerBytePatterns(const char* installed, const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if (!manager.open(installed, cd, error)) {
        std::fprintf(stderr, "computer-byte-patterns: %s\n", error.c_str()); return 1;
    }
    std::vector<uint8_t> bytes;
    if (!manager.read("computer.db", bytes, error)) {
        std::fprintf(stderr, "computer-byte-patterns: %s\n", error.c_str()); return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if (!index.parse(bytes, error)) {
        std::fprintf(stderr, "computer-byte-patterns: %s\n", error.c_str()); return 1;
    }
    const auto& offsets=index.primaryOffsets();
    std::array<size_t, 256> global{};
    for(uint8_t value:bytes) ++global[value];
    std::printf("computer.db: %zu bytes; primary entries=%zu\n",bytes.size(),offsets.size());
    std::printf("Global: 80=%zu 69=%zu 6f=%zu 00=%zu\n",
                global[0x80],global[0x69],global[0x6f],global[0]);
    std::puts("Each sample counts at most 128 bytes starting at the indexed offset.");
    std::puts("Primary offsets are not assumed to be article or encoding boundaries.\n");
    const size_t selected[] = {1,2,3,4,5,55,56,124,160,162,163,173,207,285,315,319,338,345};
    for(size_t i:selected) {
        if(i>=offsets.size()) continue;
        inspect(bytes,i,offsets[i],
                i+1<offsets.size()?offsets[i+1]:static_cast<uint32_t>(bytes.size()));
    }
    // Find long repeated-byte runs across the file, without inferring semantics.
    struct Run {size_t start,length; uint8_t value;};
    std::vector<Run> runs;
    for(size_t p=index.tableEnd();p<bytes.size();) {
        size_t q=p+1;
        while(q<bytes.size() && bytes[q]==bytes[p])++q;
        if(q-p>=16 && (bytes[p]==0x80 || bytes[p]==0x69 ||
                        bytes[p]==0x6f || bytes[p]==0))
            runs.push_back({p,q-p,bytes[p]});
        p=q;
    }
    std::printf("\nRuns >=16 bytes (80/69/6f/00): %zu\n",runs.size());
    std::array<size_t,4> runCounts{};
    for(const auto& run:runs) {
        size_t bucket=run.value==0x80?0:run.value==0x69?1:run.value==0x6f?2:3;
        ++runCounts[bucket];
    }
    std::printf("Run counts: 80=%zu 69=%zu 6f=%zu 00=%zu\n",
                runCounts[0],runCounts[1],runCounts[2],runCounts[3]);
    std::sort(runs.begin(),runs.end(),[](const Run& a,const Run& b) {
        return a.length!=b.length?a.length>b.length:a.start<b.start;
    });
    for(size_t i=0;i<std::min<size_t>(16,runs.size());++i) {
        const auto& r=runs[i];
        auto it=std::upper_bound(offsets.begin(),offsets.end(),static_cast<uint32_t>(r.start));
        const size_t preceding=it==offsets.begin()?0:size_t(it-offsets.begin()-1);
        std::printf("  0x%08zx %02x x%zu nearest-primary[%zu]=0x%08x delta=%zu\n",
                    r.start,unsigned(r.value),r.length,preceding,offsets[preceding],
                    r.start-offsets[preceding]);
    }
    std::puts("Diagnostic only; no encoding or record boundaries inferred.");
    return 0;
}
