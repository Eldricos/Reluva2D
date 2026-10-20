// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/computer_string_index.h"
#include "engines/stafu/resource_manager.h"
#include <array>
#include <cstdio>
#include <string>
#include <vector>
int stafuListComputerStrings(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-strings: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!manager.read("computer.db",bytes,error)) {
        std::fprintf(stderr,"computer-strings: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerStringIndex index;
    if(!index.parse(bytes,error)) {
        std::fprintf(stderr,"computer-strings: %s\n",error.c_str());return 1;
    }
    std::array<size_t,5> counts{};
    std::printf("computer.db=%zu entries=%zu dataBase=0x%zx\n",bytes.size(),index.entries().size(),index.dataBase());
    for(size_t i=0;i<index.entries().size();++i) {
        const auto& e=index.entries()[i];
        std::printf("[%zu] primary=%08x metadata=%08zx header=(%u,%u)\n",i,e.primaryOffset,e.metadataOffset,unsigned(e.headerA),unsigned(e.headerB));
        for(size_t j=0;j<3;++j) {
            const auto& f=e.fields[j];
            ++counts[static_cast<size_t>(f.status)];
            std::printf("  field%zu stored=%08x absolute=%08zx status=%s",j,f.storedOffset,f.absoluteOffset,
                        reluva::stafu::ComputerStringIndex::statusName(f.status));
            if(f.status==reluva::stafu::ComputerStringIndex::Status::Text)
                std::printf(" value=\"%s\"",f.text.c_str());
            std::putchar('\n');
        }
    }
    std::printf("summary: text=%zu empty=%zu non-text=%zu unterminated=%zu out-of-bounds=%zu\n",
                counts[0],counts[1],counts[2],counts[3],counts[4]);
    return 0;
}
