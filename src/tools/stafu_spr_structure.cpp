// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_container.h"
#include <cstdio>
#include <string>
#include <vector>
int stafuInspectSprStructure(const char* installed,const char* cd,const char* name) {
    reluva::stafu::ResourceManager resources;
    std::string error;
    if(!resources.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-structure: %s\n",error.c_str()); return 1;
    }
    std::vector<uint8_t> bytes;
    if(!resources.read(name,bytes,error)) {
        std::fprintf(stderr,"spr-structure: %s\n",error.c_str()); return 1;
    }
    reluva::stafu::SprContainer container;
    if(!container.parse(bytes,error)) {
        std::fprintf(stderr,"spr-structure: %s: %s\n",name,error.c_str()); return 1;
    }
    std::printf("SPR resource: %s\nBytes: %zu\nTRPS versionOrFlags=0x%08x\nRecords: %zu\n",
                name,bytes.size(),container.versionOrFlags(),container.records().size());
    for(size_t i=0;i<container.records().size();++i) {
        const auto& record=container.records()[i];
        std::printf("  record[%zu] start=0x%zx end=0x%zx bytes=%zu chunks=%zu\n",
                    i,record.start,record.end,record.end-record.start,record.chunks.size());
        for(const auto& chunk:record.chunks)
            std::printf("    tag=%s at=0x%zx size=%u payload=%u\n",
                        chunk.tag.c_str(),chunk.offset,chunk.size,chunk.size-8);
    }
    std::puts("Validated chunk boundaries; payloads remain opaque.");
    return 0;
}
