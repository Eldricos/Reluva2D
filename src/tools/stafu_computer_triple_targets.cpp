// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only study of three common COMPUTER.DB references.
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
uint16_t u16(const std::vector<uint8_t>& b,size_t p) {
    return uint16_t(b[p]) | uint16_t(uint16_t(b[p+1])<<8);
}
uint32_t u32(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p]) | uint32_t(b[p+1])<<8 |
           uint32_t(b[p+2])<<16 | uint32_t(b[p+3])<<24;
}
bool printable(uint8_t c){return c>=32 && c<=126;}
bool textStart(const std::vector<uint8_t>& b,size_t p) {
    if(p>=b.size() || !printable(b[p]) || (p && b[p-1]!=0))return false;
    size_t n=0;
    while(p+n<b.size() && n<128 && printable(b[p+n]))++n;
    return n>=3 && p+n<b.size() && b[p+n]==0;
}
size_t runLength(const std::vector<uint8_t>& b,size_t p) {
    size_t n=0;
    while(p+n<b.size() && n<256 && b[p+n]==b[p])++n;
    return n;
}
std::string preview(const std::vector<uint8_t>& b,size_t p) {
    std::string out;
    for(size_t j=0;j<40 && p+j<b.size();++j) {
        const uint8_t c=b[p+j];
        if(c==0){out+="\\0";break;}
        if(printable(c))out+=char(c);
        else {char tmp[8];std::snprintf(tmp,sizeof(tmp),"\\x%02x",unsigned(c));out+=tmp;}
    }
    return out;
}
}
int stafuInspectComputerTripleTargets(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-triple-targets: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> b;
    if(!manager.read("computer.db",b,error)) {
        std::fprintf(stderr,"computer-triple-targets: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if(!index.parse(b,error)) {
        std::fprintf(stderr,"computer-triple-targets: %s\n",error.c_str());return 1;
    }
    const auto& bases=index.primaryOffsets();
    const std::array<size_t,20> selected={0,4,7,19,24,35,55,79,108,124,160,162,173,207,219,221,222,223,224,345};
    std::array<size_t,3> exact{},midText{},nul{},longRun{},other{};
    std::map<uint32_t,size_t> delta[3],spacing01,spacing12;
    size_t invalid=0;
    std::printf("computer.db=%zu primary=%zu\n",b.size(),bases.size());
    for(size_t i=0;i<bases.size();++i) {
        const uint64_t raw=uint64_t(bases[i])+0x570;
        if(raw+16>b.size()){++invalid;continue;}
        const size_t p=size_t(raw);
        const uint32_t end=i+1<bases.size()?bases[i+1]:uint32_t(b.size());
        std::array<uint32_t,3> t={u32(b,p+4),u32(b,p+8),u32(b,p+12)};
        if(t[0]<bases[i] || t[2]>=end || !(t[0]<t[1] && t[1]<t[2])) {
            ++invalid;continue;
        }
        ++spacing01[t[1]-t[0]];++spacing12[t[2]-t[1]];
        for(size_t j=0;j<3;++j) {
            ++delta[j][t[j]-bases[i]];
            if(textStart(b,t[j]))++exact[j];
            else if(b[t[j]]==0)++nul[j];
            else if(runLength(b,t[j])>=8)++longRun[j];
            else if(printable(b[t[j]]) && t[j]>0 && printable(b[t[j]-1]))++midText[j];
            else ++other[j];
        }
        if(std::find(selected.begin(),selected.end(),i)==selected.end())continue;
        std::printf("\nprimary[%zu]=%08x intervalEnd=%08x fields=(%u,%u) source=%08zx\n",
                    i,bases[i],end,u16(b,p),u16(b,p+2),p);
        for(size_t j=0;j<3;++j) {
            size_t nearest=t[j],distance=0;
            // Find the closest preceding NUL-delimited printable string start,
            // but only within the same primary interval and at most 128 bytes back.
            bool found=false;
            for(size_t back=0;back<=128 && back<=size_t(t[j]-bases[i]);++back) {
                const size_t candidate=size_t(t[j])-back;
                if(textStart(b,candidate)) {
                    // Confirm that the candidate string actually contains the target.
                    size_t length=0;
                    while(candidate+length<b.size() && length<128 && printable(b[candidate+length]))++length;
                    if(t[j]<candidate+length){nearest=candidate;distance=back;found=true;break;}
                }
            }
            std::printf("  target[%zu]=%08x delta=+%u byte=%02x textStart=%s nearestContainingString=%s",
                        j,t[j],t[j]-bases[i],unsigned(b[t[j]]),
                        textStart(b,t[j])?"yes":"no",found?"yes":"no");
            if(found)std::printf(" stringStart=%08zx displacement=%zu",nearest,distance);
            std::printf(" preview=\"%s\"\n",preview(b,t[j]).c_str());
        }
    }
    std::printf("\nvalid triples=%zu invalid=%zu\n",bases.size()-invalid,invalid);
    for(size_t j=0;j<3;++j)
        std::printf("target[%zu]: exactNulStringStart=%zu middlePrintableText=%zu zeroByte=%zu run>=8=%zu other=%zu\n",
                    j,exact[j],midText[j],nul[j],longRun[j],other[j]);
    auto top=[](const char* name,const std::map<uint32_t,size_t>& counts) {
        std::vector<std::pair<uint32_t,size_t>> v(counts.begin(),counts.end());
        std::sort(v.begin(),v.end(),[](const auto& a,const auto& b){
            return a.second!=b.second?a.second>b.second:a.first<b.first;
        });
        std::printf("%s (top 12):\n",name);
        for(size_t i=0;i<std::min<size_t>(12,v.size());++i)
            std::printf("  +%u: %zu\n",v[i].first,v[i].second);
    };
    for(size_t j=0;j<3;++j) {
        char name[48];std::snprintf(name,sizeof(name),"target[%zu] displacement",j);
        top(name,delta[j]);
    }
    top("target[1]-target[0]",spacing01);
    top("target[2]-target[1]",spacing12);
    std::puts("Classification is heuristic; no string, field, or pointer semantics are assumed.");
    return invalid?1:0;
}
