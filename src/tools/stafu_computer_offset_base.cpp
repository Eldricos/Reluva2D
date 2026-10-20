// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only comparison of absolute and section-relative COMPUTER.DB offset interpretations.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
uint32_t read32(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p])|(uint32_t(b[p+1])<<8)|(uint32_t(b[p+2])<<16)|(uint32_t(b[p+3])<<24);
}
bool printable(uint8_t c) {return c>=32&&c<=126;}
struct Stats {
    size_t inFile=0,atNulBoundary=0,atPrintableStart=0,atPrintableMiddle=0,atZero=0,at80=0,other=0;
    void observe(const std::vector<uint8_t>& b,uint64_t at) {
        if(at>=b.size())return;
        ++inFile;
        const size_t p=size_t(at);
        const uint8_t c=b[p];
        if(p==0||b[p-1]==0)++atNulBoundary;
        if(printable(c)) {
            if(p==0||b[p-1]==0)++atPrintableStart;
            else ++atPrintableMiddle;
        } else if(c==0)++atZero;
        else if(c==0x80)++at80;
        else ++other;
    }
    void print(const char* label) const {
        std::printf("  %-26s inFile=%zu boundaryAfterNul=%zu printableStart=%zu printableMiddle=%zu zero=%zu byte80=%zu other=%zu\n",
            label,inFile,atNulBoundary,atPrintableStart,atPrintableMiddle,atZero,at80,other);
    }
};
void context(const std::vector<uint8_t>& b,uint64_t at) {
    if(at>=b.size()){std::printf("    offset %08llx outside file\n",static_cast<unsigned long long>(at));return;}
    const size_t p=size_t(at),lo=p>8?p-8:0,hi=std::min(b.size(),p+20);
    std::printf("    at %08zx:",p);
    for(size_t j=lo;j<hi;++j)std::printf(" %02x",unsigned(b[j]));
    std::printf("  | ");
    for(size_t j=lo;j<hi;++j)std::putchar(printable(b[j])?char(b[j]):'.');
    std::putchar('\n');
}
}
int stafuValidateComputerOffsetBase(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"computer-offset-base: %s\n",error.c_str());return 1;}
    std::vector<uint8_t> b;
    if(!manager.read("computer.db",b,error)){std::fprintf(stderr,"computer-offset-base: %s\n",error.c_str());return 1;}
    reluva::stafu::ComputerDatabaseIndex index;
    if(!index.parse(b,error)){std::fprintf(stderr,"computer-offset-base: %s\n",error.c_str());return 1;}
    constexpr uint64_t base=0x570;
    const auto& primary=index.primaryOffsets();
    const std::array<size_t,16> samples={0,1,2,3,4,7,19,55,124,160,162,207,219,221,223,345};
    Stats rawPrimary,shiftPrimary,rawTargets,shiftTargets;
    size_t sourceValid=0,rawOwn=0,shiftOwn=0,rawInsideFile=0,shiftInsideFile=0;
    std::printf("computer.db=%zu primary=%zu tableEnd=%zu testedBase=%08llx\n",
                b.size(),primary.size(),index.tableEnd(),static_cast<unsigned long long>(base));
    for(size_t i=0;i<primary.size();++i) {
        const uint64_t p=primary[i],next=i+1<primary.size()?primary[i+1]:b.size();
        rawPrimary.observe(b,p);
        shiftPrimary.observe(b,p+base);
        if(p<b.size())++rawInsideFile;
        if(p+base<b.size())++shiftInsideFile;
        const uint64_t source=p+base;
        if(source+16>b.size())continue;
        const std::array<uint32_t,3> refs={read32(b,size_t(source)+4),read32(b,size_t(source)+8),read32(b,size_t(source)+12)};
        if(!(refs[0]<refs[1]&&refs[1]<refs[2]))continue;
        ++sourceValid;
        bool raw=true,shift=true;
        for(uint32_t t:refs) {
            rawTargets.observe(b,t);
            shiftTargets.observe(b,uint64_t(t)+base);
            raw&=uint64_t(t)>=p&&uint64_t(t)<next;
            shift&=uint64_t(t)+base>=p+base&&uint64_t(t)+base<next+base;
        }
        rawOwn+=raw;
        shiftOwn+=shift;
        if(std::find(samples.begin(),samples.end(),i)==samples.end())continue;
        std::printf("\nprimary[%zu] raw=%08llx shifted=%08llx nextRaw=%08llx source=%08llx\n",
                    i,static_cast<unsigned long long>(p),static_cast<unsigned long long>(p+base),
                    static_cast<unsigned long long>(next),static_cast<unsigned long long>(source));
        std::printf("  references raw=(%08x,%08x,%08x) shifted=(%08llx,%08llx,%08llx) rawOwn=%s shiftedOwn=%s\n",
                    refs[0],refs[1],refs[2],
                    static_cast<unsigned long long>(uint64_t(refs[0])+base),
                    static_cast<unsigned long long>(uint64_t(refs[1])+base),
                    static_cast<unsigned long long>(uint64_t(refs[2])+base),
                    raw?"yes":"no",shift?"yes":"no");
        std::puts("  primary raw:");context(b,p);
        std::puts("  primary shifted:");context(b,p+base);
        std::puts("  first target raw:");context(b,refs[0]);
        std::puts("  first target shifted:");context(b,uint64_t(refs[0])+base);
    }
    std::printf("\nsource triples valid=%zu/%zu primary inFile raw=%zu shifted=%zu\n",
                sourceValid,primary.size(),rawInsideFile,shiftInsideFile);
    std::puts("Byte classification (not proof of record boundaries):");
    rawPrimary.print("primary raw");
    shiftPrimary.print("primary +0x570");
    rawTargets.print("references raw (3 each)");
    shiftTargets.print("references +0x570 (3 each)");
    std::printf("all three targets within corresponding intervals: raw=%zu shifted=%zu (translation-invariant test)\n",
                rawOwn,shiftOwn);
    std::puts("Caution: translating both references and interval bounds preserves membership by definition.");
    std::puts("Compare independent byte classifications, not interval membership, when assessing offset origin.");
    return sourceValid==primary.size()?0:1;
}
