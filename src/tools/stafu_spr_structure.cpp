// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_container.h"
#include <cstdio>
#include <algorithm>
#include <cstdint>
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
        for(const auto& chunk:record.chunks) {
            const size_t payloadOffset=chunk.offset+8;
            const size_t payloadSize=chunk.size-8;
            std::printf("    tag=%s at=0x%zx size=%u payload=%zu",
                        chunk.tag.c_str(),chunk.offset,chunk.size,payloadSize);
            // Small chunks: show every byte and aligned LE32 values.
            // Large chunks: show only a bounded prefix, never decompress.
            const size_t sample=std::min(payloadSize,size_t(payloadSize<=16?16:12));
            if(sample) {
                std::printf(" hex=");
                for(size_t j=0;j<sample;++j)
                    std::printf("%s%02x",j?" ":"",unsigned(bytes[payloadOffset+j]));
                if(sample<payloadSize)std::printf(" ...");
            }
            if(payloadSize&&payloadSize<=16&&payloadSize%4==0) {
                std::printf(" le32=");
                for(size_t j=0;j<payloadSize;j+=4) {
                    const size_t p=payloadOffset+j;
                    const uint32_t value=uint32_t(bytes[p])|
                        (uint32_t(bytes[p+1])<<8)|
                        (uint32_t(bytes[p+2])<<16)|
                        (uint32_t(bytes[p+3])<<24);
                    std::printf("%s%u",j?",":"",value);
                }
            }
            std::printf("\n");
        }
    }
    std::puts("Validated chunk boundaries; payloads remain opaque.");
    return 0;
}
