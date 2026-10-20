// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only SPR container survey. Chunk interpretations are deliberately provisional.
#include "engines/stafu/resource_manager.h"
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
uint32_t u32(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p])|(uint32_t(b[p+1])<<8)|(uint32_t(b[p+2])<<16)|(uint32_t(b[p+3])<<24);
}
std::string tag(const std::vector<uint8_t>& b,size_t p) {
    std::string s;
    for(size_t i=0;i<4;++i) {
        const auto c=b[p+i];
        s+=c>=32&&c<=126?char(c):'.';
    }
    return s;
}
void chunks(const std::vector<uint8_t>& b,size_t start,size_t end,unsigned level) {
    size_t p=start;
    unsigned count=0;
    while(p<end&&count<128) {
        if(end-p<8) {
            std::printf("%*sTRAILING offset=0x%zx bytes=%zu\n",int(level*2),"",p,end-p);
            return;
        }
        const uint32_t size=u32(b,p+4);
        if(size<8||size>end-p) {
            std::printf("%*sSTOP offset=0x%zx tag=%s declaredSize=%u available=%zu (invalid chunk boundary)\n",
                        int(level*2),"",p,tag(b,p).c_str(),size,end-p);
            return;
        }
        std::printf("%*schunk offset=0x%zx tag=%s size=%u payload=%u\n",
                    int(level*2),"",p,tag(b,p).c_str(),size,size-8);
        // LIST/TSIL is observed in the header. Its internal layout is not yet established.
        if(tag(b,p)=="TSIL"&&size>=16) {
            const uint32_t n=u32(b,p+8);
            std::printf("%*sTSIL firstWord=%u remainingBytes=%zu\n",int(level*2+2),"",n,size_t(size)-12);
            // Candidate table: firstWord 32-bit offsets, relative to the TSIL header.
            // Report validity before interpreting any nested records.
            const size_t tableStart=p+12;
            const size_t listEnd=p+size;
            if(n>4096||size_t(n)>(listEnd-tableStart)/4) {
                std::puts("    TSIL offset table exceeds available bytes or safety limit");
            } else {
                const size_t tableEnd=tableStart+size_t(n)*4;
                std::vector<size_t> starts;
                bool valid=true;
                for(uint32_t i=0;i<n;++i) {
                    const uint32_t relative=u32(b,tableStart+size_t(i)*4);
                    const size_t absolute=p+size_t(relative);
                    const bool inRange=absolute>=tableEnd&&absolute<listEnd;
                    const bool ordered=starts.empty()||absolute>starts.back();
                    std::printf("    candidate[%u] relative=0x%x absolute=0x%zx inRange=%s increasing=%s\\n",
                                i,relative,absolute,inRange?"yes":"no",ordered?"yes":"no");
                    if(!inRange||!ordered)valid=false;
                    starts.push_back(absolute);
                }
                if(valid) {
                    for(size_t i=0;i<starts.size();++i) {
                        const size_t recordEnd=i+1<starts.size()?starts[i+1]:listEnd;
                        std::printf("    record[%zu] start=0x%zx end=0x%zx bytes=%zu\\n",
                                    i,starts[i],recordEnd,recordEnd-starts[i]);
                        // Inspect chunk headers only; compressed payloads are opaque.
                        size_t cursor=starts[i];
                        unsigned scanned=0;
                        while(cursor<recordEnd&&scanned<32) {
                            if(recordEnd-cursor<8) {
                                std::printf("      trailing bytes=%zu at 0x%zx\\n",recordEnd-cursor,cursor);
                                break;
                            }
                            const uint32_t length=u32(b,cursor+4);
                            if(length<8||size_t(length)>recordEnd-cursor) {
                                std::printf("      boundary stop at 0x%zx tag=%s size=%u remaining=%zu\\n",
                                            cursor,tag(b,cursor).c_str(),length,recordEnd-cursor);
                                break;
                            }
                            std::printf("      tag=%s at=0x%zx size=%u\\n",
                                        tag(b,cursor).c_str(),cursor,length);
                            cursor+=length;
                            ++scanned;
                        }
                        if(scanned==32&&cursor<recordEnd)
                            std::puts("      chunk scan truncated at 32 records");
                    }
                } else {
                    std::puts("    Candidate offsets invalid; nested scan skipped");
                }
            }
        }
        p+=size;
        ++count;
    }
    if(count==128)std::puts("  chunk output truncated at 128 records");
}
}
int stafuInspectSprStructure(const char* installed,const char* cd,const char* name) {
    reluva::stafu::ResourceManager resources;
    std::string error;
    if(!resources.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-structure: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!resources.read(name,bytes,error)) {
        std::fprintf(stderr,"spr-structure: %s\n",error.c_str());return 1;
    }
    std::printf("SPR resource: %s\nBytes: %zu\n",name,bytes.size());
    if(bytes.size()<12||tag(bytes,0)!="TRPS") {
        std::fprintf(stderr,"spr-structure: missing TRPS signature or short header\n");return 1;
    }
    const uint32_t declared=u32(bytes,4);
    const uint32_t version=u32(bytes,8);
    std::printf("TRPS declaredSize=%u versionOrFlags=0x%08x fileBytes=%zu\n",declared,version,bytes.size());
    if(declared!=bytes.size())std::puts("WARNING: TRPS declared size differs from file length");
    chunks(bytes,12,bytes.size(),1);
    std::puts("All tags and fields are raw observations; no sprite pixels were decoded.");
    return 0;
}
