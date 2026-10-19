// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only inspection of repeated 0x80 run boundaries in computer.db.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
struct Run { size_t start, length; };
void showBytes(const std::vector<uint8_t>& b, size_t from, size_t to) {
    for(size_t p=from;p<to;p+=16) {
        const size_t stop=std::min(to,p+16);
        std::printf("  %08zx:",p);
        for(size_t j=p;j<stop;++j)std::printf(" %02x",unsigned(b[j]));
        std::printf("  |");
        for(size_t j=p;j<stop;++j)
            std::putchar(b[j]>=32 && b[j]<=126?char(b[j]):'.');
        std::puts("|");
    }
}
void showRun(const std::vector<uint8_t>& b,const std::vector<uint32_t>& offsets,
             const char* name,Run r) {
    const size_t end=r.start+r.length;
    std::printf("\n%s: [0x%08zx,0x%08zx) length=%zu\n",name,r.start,end,r.length);
    auto prev=std::upper_bound(offsets.begin(),offsets.end(),uint32_t(r.start));
    if(prev!=offsets.begin()) {
        --prev;
        std::printf(" preceding primary[%zu]=0x%08x distance=%zu\n",
                    size_t(prev-offsets.begin()),*prev,r.start-*prev);
    }
    auto next=std::lower_bound(offsets.begin(),offsets.end(),uint32_t(end));
    if(next!=offsets.end())
        std::printf(" following primary[%zu]=0x%08x distance=%zu\n",
                    size_t(next-offsets.begin()),*next,*next-end);
    auto inside=std::lower_bound(offsets.begin(),offsets.end(),uint32_t(r.start));
    for(;inside!=offsets.end() && *inside<end;++inside)
        std::printf(" inside primary[%zu]=0x%08x\n",size_t(inside-offsets.begin()),*inside);
    std::puts(" 64 bytes before start:");
    showBytes(b,r.start>64?r.start-64:0,r.start);
    std::puts(" first 32 bytes:");
    showBytes(b,r.start,std::min(end,r.start+32));
    std::puts(" last 32 bytes:");
    showBytes(b,end-std::min<size_t>(32,r.length),end);
    std::puts(" 64 bytes after end:");
    showBytes(b,end,std::min(b.size(),end+64));
}
}
int stafuInspectComputerRunBoundaries(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-run-boundaries: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read("computer.db",bytes,error)) {
        std::fprintf(stderr,"computer-run-boundaries: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if(!index.parse(bytes,error)) {
        std::fprintf(stderr,"computer-run-boundaries: %s\n",error.c_str());return 1;
    }
    std::vector<Run> runs;
    for(size_t p=index.tableEnd();p<bytes.size();) {
        size_t q=p+1;
        while(q<bytes.size() && bytes[q]==bytes[p])++q;
        if(bytes[p]==0x80 && q-p>=16)runs.push_back({p,q-p});
        p=q;
    }
    std::sort(runs.begin(),runs.end(),[](Run a,Run b) {
        return a.length!=b.length?a.length>b.length:a.start<b.start;
    });
    std::printf("computer.db=%zu bytes, primary=%zu, 80-runs>=16=%zu\n",
                bytes.size(),index.primaryOffsets().size(),runs.size());
    if(runs.size()<3)return 1;
    showRun(bytes,index.primaryOffsets(),"Largest run",runs[0]);
    showRun(bytes,index.primaryOffsets(),"Second largest run",runs[1]);
    auto small=std::find_if(runs.begin()+2,runs.end(),
                            [](Run r){return r.length<=512;});
    showRun(bytes,index.primaryOffsets(),"Smaller comparison",
            small==runs.end()?runs[2]:*small);
    std::puts("\nAll offsets are file-relative; run ends are exclusive.");
    std::puts("No binary encoding or record semantics inferred.");
    return 0;
}
