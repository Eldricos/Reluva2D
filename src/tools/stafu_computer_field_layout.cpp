// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only field layout comparison at primary offsets + 0x570.
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
uint16_t le16(const std::vector<uint8_t>& b,size_t p) {
    return uint16_t(b[p]) | uint16_t(uint16_t(b[p+1])<<8);
}
uint32_t le32(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p]) | uint32_t(b[p+1])<<8 |
           uint32_t(b[p+2])<<16 | uint32_t(b[p+3])<<24;
}
struct Sample {
    size_t index;
    uint32_t base,next;
    size_t source;
    uint16_t field0,field1;
    std::array<uint32_t,7> words{};
    size_t prefix=0;
    bool firstThreeInInterval=false;
};
bool validOffset(uint32_t value,size_t size) {return value!=0 && value<size;}
void ranked(const char* label,const std::map<uint32_t,size_t>& counts) {
    std::vector<std::pair<uint32_t,size_t>> sorted(counts.begin(),counts.end());
    std::sort(sorted.begin(),sorted.end(),[](const auto& a,const auto& b){
        return a.second!=b.second?a.second>b.second:a.first<b.first;
    });
    std::printf("%s (%zu distinct, top 16):\n",label,counts.size());
    for(size_t i=0;i<std::min<size_t>(16,sorted.size());++i)
        std::printf("  %u (0x%04x): %zu\n",sorted[i].first,sorted[i].first,sorted[i].second);
}
}
int stafuCompareComputerFieldLayout(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-field-layout: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read("computer.db",bytes,error)) {
        std::fprintf(stderr,"computer-field-layout: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if(!index.parse(bytes,error)) {
        std::fprintf(stderr,"computer-field-layout: %s\n",error.c_str());return 1;
    }
    const auto& offsets=index.primaryOffsets();
    std::vector<Sample> samples;
    std::map<uint32_t,size_t> field0,field1,pairs,offsetCounts;
    size_t outOfFile=0,threeWithin=0,sourceWithin=0,zeroWord=0;
    for(size_t i=0;i<offsets.size();++i) {
        const uint64_t raw=uint64_t(offsets[i])+0x570;
        if(raw+32>bytes.size()){++outOfFile;continue;}
        Sample s{};
        s.index=i;s.base=offsets[i];
        s.next=i+1<offsets.size()?offsets[i+1]:uint32_t(bytes.size());
        s.source=size_t(raw);
        s.field0=le16(bytes,s.source);
        s.field1=le16(bytes,s.source+2);
        for(size_t j=0;j<7;++j)s.words[j]=le32(bytes,s.source+4+4*j);
        // Count the initial run of strictly increasing, nonzero in-file LE32 values.
        // This is a heuristic, not a validated pointer-array length.
        uint32_t prev=0;
        for(uint32_t word:s.words) {
            if(!validOffset(word,bytes.size()) || (s.prefix && word<=prev))break;
            prev=word;++s.prefix;
        }
        s.firstThreeInInterval=s.prefix>=3 && s.words[0]>=s.base && s.words[2]<s.next;
        ++field0[s.field0];++field1[s.field1];
        ++pairs[(uint32_t(s.field0)<<16)|s.field1];
        ++offsetCounts[uint32_t(s.prefix)];
        sourceWithin+=s.source<s.next;
        threeWithin+=s.firstThreeInInterval;
        zeroWord+=s.field0==0 && s.field1==0;
        samples.push_back(s);
    }
    std::printf("computer.db=%zu primary=%zu samples=%zu out-of-file=%zu\n",
                bytes.size(),offsets.size(),samples.size(),outOfFile);
    std::printf("source within primary interval=%zu first 3 targets within primary interval=%zu initial four-byte zero=%zu\n",
                sourceWithin,threeWithin,zeroWord);
    ranked("First LE16 field",field0);
    ranked("Second LE16 field",field1);
    std::vector<std::pair<uint32_t,size_t>> ordered(pairs.begin(),pairs.end());
    std::sort(ordered.begin(),ordered.end(),[](const auto& a,const auto& b){
        return a.second!=b.second?a.second>b.second:a.first<b.first;
    });
    std::puts("Field-pair frequencies (top 20):");
    for(size_t i=0;i<std::min<size_t>(20,ordered.size());++i)
        std::printf("  (%u,%u): %zu\n",ordered[i].first>>16,ordered[i].first&0xffff,ordered[i].second);
    ranked("Increasing in-file LE32 prefix length (maximum 7)",offsetCounts);
    std::puts("Prefix length by field pair (top 20 pairs; columns 0..7):");
    for(size_t i=0;i<std::min<size_t>(20,ordered.size());++i) {
        const uint32_t key=ordered[i].first;
        std::array<size_t,8> hist{};
        for(const auto& s:samples)
            if(((uint32_t(s.field0)<<16)|s.field1)==key)++hist[s.prefix];
        std::printf("  (%u,%u) n=%zu:",key>>16,key&0xffff,ordered[i].second);
        for(size_t n:hist)std::printf(" %zu",n);
        std::putchar('\n');
    }
    const std::array<size_t,23> selected={0,4,7,19,24,35,55,79,108,124,160,162,173,207,219,221,222,223,224,285,315,319,345};
    std::puts("Representative entries: seven LE32 words after the two LE16 fields:");
    for(size_t idx:selected) {
        const auto it=std::find_if(samples.begin(),samples.end(),[idx](const Sample& s){return s.index==idx;});
        if(it==samples.end())continue;
        const auto& s=*it;
        std::printf("  primary[%zu]=%08x next=%08x source=%08zx inInterval=%s fields=(%u,%u) prefix=%zu targetsInInterval=%s\n",
                    s.index,s.base,s.next,s.source,s.source<s.next?"yes":"no",
                    s.field0,s.field1,s.prefix,s.firstThreeInInterval?"yes":"no");
        std::printf("    words:");
        for(uint32_t word:s.words)std::printf(" %08x",word);
        std::putchar('\n');
    }
    std::puts("Observational only: field values and increasing words have no assigned semantics.");
    return 0;
}
