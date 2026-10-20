// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only investigation of rejected COMPUTER.DB text fields.
#include "engines/stafu/computer_string_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>
namespace {
bool printable(uint8_t c) { return c>=32&&c<=126; }
void context(const std::vector<uint8_t>& bytes,size_t at) {
    const size_t lo=at>12?at-12:0,hi=std::min(bytes.size(),at+20);
    std::printf("    context [%08zx..%08zx):",lo,hi);
    for(size_t i=lo;i<hi;++i)std::printf(" %02x",unsigned(bytes[i]));
    std::printf(" | ");
    for(size_t i=lo;i<hi;++i)std::putchar(printable(bytes[i])?char(bytes[i]):'.');
    std::putchar('\n');
}
}
int stafuInspectComputerRejectedFields(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager rm;
    std::string error;
    if(!rm.open(installed,cd,error)) {std::fprintf(stderr,"computer-rejected-fields: %s\n",error.c_str());return 1;}
    std::vector<uint8_t> bytes;
    if(!rm.read("computer.db",bytes,error)) {std::fprintf(stderr,"computer-rejected-fields: %s\n",error.c_str());return 1;}
    reluva::stafu::ComputerStringIndex index;
    if(!index.parse(bytes,error)) {std::fprintf(stderr,"computer-rejected-fields: %s\n",error.c_str());return 1;}
    std::map<unsigned,size_t> rejectedBytes;
    std::map<unsigned,size_t> rejectedByField;
    std::map<unsigned,size_t> emptyByField;
    std::map<size_t,size_t> printablePrefixLengths;
    size_t rejected=0,nonText=0,unterminated=0,outOfBounds=0,empty=0;
    size_t laterNul=0,hasHighBytes=0;
    std::printf("computer.db=%zu entries=%zu dataBase=0x%zx\n",
                bytes.size(),index.entries().size(),index.dataBase());
    for(size_t i=0;i<index.entries().size();++i) {
        const auto& entry=index.entries()[i];
        for(size_t j=0;j<3;++j) {
            const auto& field=entry.fields[j];
            using Status=reluva::stafu::ComputerStringIndex::Status;
            if(field.status==Status::Empty) {++empty;++emptyByField[unsigned(j)];continue;}
            if(field.status==Status::Text)continue;
            ++rejected;++rejectedByField[unsigned(j)];
            if(field.status==Status::NonText)++nonText;
            if(field.status==Status::Unterminated)++unterminated;
            if(field.status==Status::OutOfBounds)++outOfBounds;
            const size_t start=field.absoluteOffset;
            std::printf("\n[%zu] field%zu primary=%08x stored=%08x absolute=%08zx status=%s",
                        i,j,entry.primaryOffset,field.storedOffset,start,
                        reluva::stafu::ComputerStringIndex::statusName(field.status));
            if(start>=bytes.size()) {std::putchar('\n');continue;}
            constexpr size_t limit=4096;
            size_t prefix=0;
            while(prefix<limit&&start+prefix<bytes.size()&&printable(bytes[start+prefix]))++prefix;
            printablePrefixLengths[prefix]++;
            const size_t stop=start+prefix;
            if(stop<bytes.size()) {
                const unsigned offending=bytes[stop];
                ++rejectedBytes[offending];
                std::printf(" printablePrefix=%zu firstRejected=%02x at=%08zx",
                            prefix,offending,stop);
            } else std::printf(" printablePrefix=%zu reachedEOF",prefix);
            // Look only within a bounded 256-byte window for diagnostic hints.
            const size_t end=std::min(bytes.size(),start+std::min<size_t>(256,bytes.size()-start));
            bool nul=false,high=false;
            for(size_t p=start;p<end;++p) {
                nul|=bytes[p]==0;
                high|=bytes[p]>=128;
            }
            laterNul+=nul;
            hasHighBytes+=high;
            std::printf(" nulWithin256=%s highByteWithin256=%s\n",nul?"yes":"no",high?"yes":"no");
            context(bytes,stop<bytes.size()?stop:start);
        }
    }
    std::printf("\nsummary: rejected=%zu non-text=%zu unterminated=%zu out-of-bounds=%zu empty=%zu\n",
                rejected,nonText,unterminated,outOfBounds,empty);
    std::printf("rejected fields with NUL within 256 bytes=%zu high byte within 256 bytes=%zu\n",
                laterNul,hasHighBytes);
    for(size_t j=0;j<3;++j)
        std::printf("field%zu rejected=%zu empty=%zu\n",j,rejectedByField[unsigned(j)],emptyByField[unsigned(j)]);
    std::puts("First rejected byte frequency:");
    for(const auto& [byte,count]:rejectedBytes)
        std::printf("  %02x: %zu\n",byte,count);
    std::puts("Printable prefix length frequency:");
    for(const auto& [length,count]:printablePrefixLengths)
        std::printf("  %zu: %zu\n",length,count);
    std::puts("Interpretation note: non-ASCII or control bytes may be formatting, encoding, or binary data; no decoding assumptions made.");
    return 0;
}
