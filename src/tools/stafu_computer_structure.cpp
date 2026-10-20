// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only correlation of primary record offsets, nearby words and candidate hierarchy.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/computer_hierarchy_candidate.h"
#include "engines/stafu/computer_string_index.h"
#include "engines/stafu/resource_manager.h"
#include <array>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>
namespace {
using Entry=reluva::stafu::ComputerStringIndex::Entry;
uint32_t le32(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p])|(uint32_t(b[p+1])<<8)|
        (uint32_t(b[p+2])<<16)|(uint32_t(b[p+3])<<24);
}
const char* label(const Entry& e) {
    return e.fields[0].status==reluva::stafu::ComputerStringIndex::Status::Text?
        e.fields[0].text.c_str():"<unknown>";
}
void words(const std::vector<uint8_t>& bytes,size_t start,size_t end,
    const std::map<uint32_t,size_t>& lookup,const std::vector<Entry>& entries) {
    if(end>bytes.size())end=bytes.size();
    for(size_t p=start;p<=end&&end-p>=4;p+=4) {
        const uint32_t v=le32(bytes,p);
        const auto it=lookup.find(v);
        std::printf("    abs=0x%zx word=0x%08x",p,v);
        if(it!=lookup.end())std::printf(" PRIMARY[%zu] \"%s\"",
            it->second,label(entries[it->second]));
        std::putchar('\n');
    }
}
}
int stafuInspectComputerStructure(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager rm;
    std::string error;
    if(!rm.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-structure: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!rm.read("computer.db",bytes,error)) {
        std::fprintf(stderr,"computer-structure: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerDatabaseIndex primary;
    reluva::stafu::ComputerStringIndex strings;
    if(!primary.parse(bytes,error)||!strings.parse(bytes,error)) {
        std::fprintf(stderr,"computer-structure: %s\n",error.c_str());return 1;
    }
    const auto& entries=strings.entries();
    const auto tree=reluva::stafu::analyzeComputerHierarchy(entries);
    std::map<uint32_t,size_t> lookup;
    for(size_t i=0;i<entries.size();++i)lookup.emplace(entries[i].primaryOffset,i);
    std::printf("entries=%zu tableEnd=0x%zx base=0x%zx secondaryRefs=%zu\n",
        entries.size(),primary.tableEnd(),strings.dataBase(),primary.secondaryReferences().size());
    std::puts("\nNine-word primary-reference candidate:");
    for(const auto& r:primary.secondaryReferences()) {
        const size_t i=r.primaryIndex;
        std::printf("  abs=0x%zx target=0x%08x index=%zu header=(%u,%u) label=\"%s\"\n",
            r.sourceOffset,r.targetOffset,i,
            unsigned(entries[i].headerA),unsigned(entries[i].headerB),label(entries[i]));
    }
    std::puts("\nWords immediately after primary table (potential navigation header):");
    words(bytes,primary.tableEnd(),primary.tableEnd()+96,lookup,entries);
    constexpr std::array<size_t,12> targets={0,1,55,162,166,167,169,174,185,192,319,338};
    std::puts("\nSelected metadata records and following 64 bytes:");
    for(size_t i:targets) {
        if(i>=entries.size())continue;
        const auto& e=entries[i];
        const size_t begin=e.metadataOffset;
        std::printf("  [%zu] label=\"%s\" primary=0x%08x metadata=0x%zx header=(%u,%u) parent=",
            i,label(e),e.primaryOffset,begin,unsigned(e.headerA),unsigned(e.headerB));
        if(tree.nodes[i].hasParent)std::printf("%zu",tree.nodes[i].parent);
        else std::printf("ROOT");
        std::putchar('\n');
        // Skip the 16-byte metadata header and its three text pointers.
        // The following words may be text or unrelated data: matches are only candidates.
        if(begin<=bytes.size()&&bytes.size()-begin>=16)
            words(bytes,begin+16,begin+80,lookup,entries);
    }
    std::puts("\nAligned primary-offset matches in the first 256 bytes following metadata:");
    for(size_t i:targets) {
        if(i>=entries.size())continue;
        const size_t start=entries[i].metadataOffset+16;
        if(start>bytes.size())continue;
        const size_t available=bytes.size()-start;
        const size_t length=available<256?available:256;
        size_t hits=0;
        for(size_t delta=0;delta+4<=length;delta+=4) {
            const auto it=lookup.find(le32(bytes,start+delta));
            if(it==lookup.end())continue;
            ++hits;
            std::printf("  source=[%zu] delta=+0x%zx target=[%zu] \"%s\"\n",
                i,delta+16,it->second,label(entries[it->second]));
        }
        if(!hits)std::printf("  source=[%zu] no aligned primary-offset matches\n",i);
    }
    std::puts("Matches are structural observations, not verified navigation links.");
    return 0;
}
