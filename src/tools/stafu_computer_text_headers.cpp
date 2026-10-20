// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only inspection of remaining COMPUTER.DB anomalies and header patterns.
#include "engines/stafu/computer_string_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <utility>
#include <vector>
namespace {
bool accepted(uint8_t c) { return (c>=32&&c<=126)||c==0x0d||c==0x09; }
void dump(const std::vector<uint8_t>& bytes,size_t center) {
    const size_t lo=center>32?center-32:0;
    const size_t hi=center<bytes.size()?center+std::min<size_t>(48,bytes.size()-center):bytes.size();
    for(size_t p=lo;p<hi;p+=16) {
        std::printf("  %08zx:",p);
        for(size_t q=p;q<std::min(p+16,hi);++q)std::printf(" %02x",unsigned(bytes[q]));
        std::printf(" |");
        for(size_t q=p;q<std::min(p+16,hi);++q)std::putchar(bytes[q]>=32&&bytes[q]<=126?char(bytes[q]):'.');
        std::puts("|");
    }
}
}
int stafuAuditComputerTextAndHeaders(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager rm;
    std::string error;
    if(!rm.open(installed,cd,error)) {std::fprintf(stderr,"computer-text-headers: %s\n",error.c_str());return 1;}
    std::vector<uint8_t> bytes;
    if(!rm.read("computer.db",bytes,error)) {std::fprintf(stderr,"computer-text-headers: %s\n",error.c_str());return 1;}
    reluva::stafu::ComputerStringIndex index;
    if(!index.parse(bytes,error)) {std::fprintf(stderr,"computer-text-headers: %s\n",error.c_str());return 1;}
    using Status=reluva::stafu::ComputerStringIndex::Status;
    std::map<uint16_t,size_t> aCount,bCount;
    std::map<std::pair<uint16_t,uint16_t>,size_t> pairs;
    std::printf("computer.db=%zu entries=%zu base=0x%zx\n",bytes.size(),index.entries().size(),index.dataBase());
    for(size_t i=0;i<index.entries().size();++i) {
        const auto& e=index.entries()[i];
        ++aCount[e.headerA];++bCount[e.headerB];++pairs[{e.headerA,e.headerB}];
        for(size_t j=0;j<3;++j) {
            const auto& f=e.fields[j];
            if(f.status==Status::Text||f.status==Status::Empty)continue;
            std::printf("\nUNRESOLVED entry=%zu field=%zu header=(%u,%u) primary=0x%08x stored=0x%08x absolute=0x%zx status=%s\n",
                i,j,unsigned(e.headerA),unsigned(e.headerB),e.primaryOffset,f.storedOffset,f.absoluteOffset,
                reluva::stafu::ComputerStringIndex::statusName(f.status));
            if(f.status==Status::OutOfBounds||f.absoluteOffset>=bytes.size())continue;
            const size_t start=f.absoluteOffset;
            size_t n=0;
            while(n<4096&&n<bytes.size()-start&&accepted(bytes[start+n]))++n;
            const size_t stop=start+n;
            std::printf("  acceptedPrefix=%zu",n);
            if(stop<bytes.size())std::printf(" firstStop=0x%02x at=0x%zx",unsigned(bytes[stop]),stop);
            std::putchar('\n');
            if(stop<bytes.size())dump(bytes,stop);
            const size_t end=start+std::min<size_t>(4096,bytes.size()-start);
            size_t nul=end;
            for(size_t p=start;p<end;++p)if(bytes[p]==0){nul=p;break;}
            if(nul<end) {
                std::printf("  nulOffset=0x%zx bytesToNul=%zu\n",nul,nul-start);
                std::map<unsigned,size_t> unsupported;
                for(size_t p=start;p<nul;++p)if(!accepted(bytes[p]))++unsupported[bytes[p]];
                std::puts("  unsupported bytes before NUL:");
                for(const auto& [b,count]:unsupported)std::printf("    0x%02x count=%zu\n",b,count);
            } else std::puts("  no NUL within 4096-byte decode window");
        }
    }
    std::puts("\nHeader A frequency:");
    for(const auto& [v,n]:aCount)std::printf("  %u: %zu\n",unsigned(v),n);
    std::puts("Header B frequency:");
    for(const auto& [v,n]:bCount)std::printf("  %u: %zu\n",unsigned(v),n);
    std::puts("Header pair frequency:");
    for(const auto& [pair,n]:pairs)std::printf("  (%u,%u): %zu\n",unsigned(pair.first),unsigned(pair.second),n);
    std::puts("\nEntries with nonzero headers (neighbor context; labels only):");
    for(size_t i=0;i<index.entries().size();++i) {
        const auto& e=index.entries()[i];
        if(e.headerA==0&&e.headerB==0)continue;
        std::printf("  [%zu] header=(%u,%u) label=\"%s\" next=\"%s\"\n",i,
            unsigned(e.headerA),unsigned(e.headerB),
            e.fields[0].status==Status::Text?e.fields[0].text.c_str():"<not-text>",
            i+1<index.entries().size()&&index.entries()[i+1].fields[0].status==Status::Text?
                index.entries()[i+1].fields[0].text.c_str():"<none>");
    }
    std::puts("Header meanings are unknown; no hierarchy inferred.");
    return 0;
}
