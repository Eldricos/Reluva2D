// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only inspection of candidate LE32 references following repeated-byte runs.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
uint32_t le32(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p]) | (uint32_t(b[p+1])<<8) |
           (uint32_t(b[p+2])<<16) | (uint32_t(b[p+3])<<24);
}
void inspectTarget(const std::vector<uint8_t>& b,const std::vector<uint32_t>& offsets,
                   uint32_t target) {
    const size_t p=target;
    const size_t begin=p>=16?p-16:0;
    const size_t end=std::min(b.size(),p+32);
    auto it=std::upper_bound(offsets.begin(),offsets.end(),target);
    if(it!=offsets.begin()) {
        --it;
        std::printf("  preceding primary[%zu]=0x%08x delta=%zu\n",
                    size_t(it-offsets.begin()),*it,p-*it);
    }
    size_t runStart=p,runEnd=p+1;
    while(runStart>0 && b[runStart-1]==b[p])--runStart;
    while(runEnd<b.size() && b[runEnd]==b[p])++runEnd;
    std::printf("  target byte=%02x constant-run=[0x%08zx,0x%08zx) length=%zu\n",
                unsigned(b[p]),runStart,runEnd,runEnd-runStart);
    for(size_t q=begin;q<end;q+=16) {
        const size_t stop=std::min(q+16,end);
        std::printf("  %08zx:",q);
        for(size_t j=q;j<stop;++j)std::printf(" %02x",unsigned(b[j]));
        std::printf(" |");
        for(size_t j=q;j<stop;++j)std::putchar(b[j]>=32 && b[j]<=126?char(b[j]):'.');
        std::puts("|");
    }
}
void inspectGroup(const std::vector<uint8_t>& b,const std::vector<uint32_t>& offsets,
                  const char* name,size_t start) {
    std::printf("\n=== %s: candidate LE32 words at 0x%08zx ===\n",name,start);
    if(start+12>b.size()) {std::puts("  out of bounds");return;}
    std::array<uint32_t,3> values{};
    for(size_t i=0;i<3;++i) {
        values[i]=le32(b,start+i*4);
        std::printf(" word[%zu] source=0x%08zx value=0x%08x %s\n",
                    i,start+i*4,values[i],
                    values[i]<b.size()?"in-file":"OUT OF FILE");
        if(values[i]<b.size())inspectTarget(b,offsets,values[i]);
    }
    std::printf(" strictly increasing=%s\n",
                values[0]<values[1] && values[1]<values[2]?"yes":"no");
}
}
int stafuInspectComputerRunReferences(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-run-references: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read("computer.db",bytes,error)) {
        std::fprintf(stderr,"computer-run-references: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if(!index.parse(bytes,error)) {
        std::fprintf(stderr,"computer-run-references: %s\n",error.c_str());return 1;
    }
    if(bytes.size()<=0x554a0) {
        std::fprintf(stderr,"computer-run-references: reference offsets exceed file size\n");
        return 1;
    }
    std::printf("computer.db=%zu bytes; primary=%zu\n",
                bytes.size(),index.primaryOffsets().size());
    // Candidate groups start four bytes after each observed run end.
    inspectGroup(bytes,index.primaryOffsets(),"after 5085-byte 80 run",0x55491);
    inspectGroup(bytes,index.primaryOffsets(),"after 439-byte 80 run",0x47745);
    std::puts("\nSurvey: 00 00 00 00 followed by three increasing in-file LE32 values.");
    size_t candidates=0,near80=0,allText=0;
    for(size_t p=index.tableEnd();p+16<=bytes.size();++p) {
        if(le32(bytes,p)!=0)continue;
        const uint32_t a=le32(bytes,p+4),b=le32(bytes,p+8),c=le32(bytes,p+12);
        if(a==0 || !(a<b && b<c) || c>=bytes.size())continue;
        ++candidates;
        bool preceding80=p>0 && bytes[p-1]==0x80;
        if(preceding80)++near80;
        if(bytes[a]>=32 && bytes[a]<=126 &&
           bytes[b]>=32 && bytes[b]<=126 &&
           bytes[c]>=32 && bytes[c]<=126)++allText;
        if(candidates<=20)
            std::printf(" candidate 0x%08zx -> %08x %08x %08x previous=%02x%s\n",
                        p,a,b,c,p?unsigned(bytes[p-1]):0,
                        preceding80?" [after 80]":"");
    }
    std::printf("Survey summary: candidates=%zu immediately-after-80=%zu all-printable-target-bytes=%zu\n",
                candidates,near80,allText);
    std::puts("Candidate patterns are not asserted to be pointers or records.");
    return 0;
}
