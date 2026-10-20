// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_container.h"
#include <cstdio>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
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

    // Correlate compact image-like chunks against large chunks in the same file.
    // Candidate address bases are hypotheses, not decoded pointer semantics.
    struct Large { std::string tag; size_t offset,payload; uint32_t a,b; };
    std::vector<Large> large;
    auto word=[&](size_t p)->uint32_t {
        return uint32_t(bytes[p])|(uint32_t(bytes[p+1])<<8)|
               (uint32_t(bytes[p+2])<<16)|(uint32_t(bytes[p+3])<<24);
    };
    for(const auto& record:container.records())
        for(const auto& chunk:record.chunks)
            if((chunk.tag=="PMOC"||chunk.tag=="MOCS")&&chunk.size>24)
                large.push_back({chunk.tag,chunk.offset,chunk.offset+8,
                                 word(chunk.offset+8),word(chunk.offset+12)});
    std::map<std::string,size_t> hits;
    size_t compact=0,flagged=0,unmatched=0;
    std::puts("CORRELATION (compact PMOC/MOCS, same-file large chunks)");
    for(size_t ri=0;ri<container.records().size();++ri) {
        const auto& record=container.records()[ri];
        for(const auto& chunk:record.chunks) {
            if((chunk.tag!="PMOC"&&chunk.tag!="MOCS")||chunk.size!=24)continue;
            ++compact;
            const uint32_t a=word(chunk.offset+8),b=word(chunk.offset+12);
            const uint32_t marker=word(chunk.offset+16),relative=word(chunk.offset+20);
            if(marker==0x00030000)++flagged;
            bool matched=false;
            // Test absolute file, TSIL-relative and record-relative addresses.
            const size_t candidates[]={size_t(relative),size_t(relative)+12,
                                       size_t(relative)+record.start};
            const char* bases[]={"file","TSIL","record"};
            for(size_t bi=0;bi<3;++bi) {
                const size_t address=candidates[bi];
                for(const auto& target:large) {
                    if(target.tag!=chunk.tag||target.a!=a||target.b!=b)continue;
                    const char* location=nullptr;
                    if(address==target.offset)location="chunk";
                    else if(address==target.payload)location="payload";
                    if(!location)continue;
                    matched=true;
                    const std::string key=std::string(bases[bi])+"-"+location;
                    ++hits[key];
                    std::printf("  match record=%zu compact=0x%zx tag=%s dims=%u,%u marker=0x%08x ref=%u base=%s location=%s target=0x%zx direction=%s\n",
                                ri,chunk.offset,chunk.tag.c_str(),a,b,marker,relative,
                                bases[bi],location,target.offset,
                                target.offset<chunk.offset?"earlier":"later");
                }
            }
            if(!matched)++unmatched;
        }
    }
    std::printf("  summary compact=%zu marker00030000=%zu unmatched=%zu large=%zu\n",
                compact,flagged,unmatched,large.size());
    for(const auto& hit:hits)
        std::printf("  match-type %s count=%zu\n",hit.first.c_str(),hit.second);
    std::puts("LARGE PAYLOAD HEADER SURVEY");
    std::map<std::string,size_t> markerCounts;
    for(size_t ri=0;ri<container.records().size();++ri)
        for(const auto& chunk:container.records()[ri].chunks) {
            if((chunk.tag!="PMOC"&&chunk.tag!="MOCS")||chunk.size<=24)continue;
            const size_t p=chunk.offset+8;
            const uint32_t a=word(p),b=word(p+4),marker=word(p+8);
            const size_t dataBytes=chunk.size-20;
            const uint64_t area=uint64_t(a)*b;
            char label[80];
            std::snprintf(label,sizeof(label),"%s/0x%08x",chunk.tag.c_str(),marker);
            ++markerCounts[label];
            std::printf("  record=%zu offset=0x%zx tag=%s a=%u b=%u marker=0x%08x dataBytes=%zu area=%llu ratioX1000=%llu prefix=",
                ri,chunk.offset,chunk.tag.c_str(),a,b,marker,dataBytes,
                static_cast<unsigned long long>(area),
                static_cast<unsigned long long>(area?uint64_t(dataBytes)*1000/area:0));
            for(size_t j=0;j<std::min(dataBytes,size_t(16));++j)
                std::printf("%s%02x",j?" ":"",unsigned(bytes[p+12+j]));
            std::puts("");
        }
    std::puts("LARGE PAYLOAD MARKER FREQUENCIES");
    for(const auto& pair:markerCounts)
        std::printf("  %s count=%zu\n",pair.first.c_str(),pair.second);
    // Read-only byte-pattern survey. Never interpret output as decoded pixels.
    struct PatternStats {
        size_t chunks=0, bytes=0, adjacentPairs=0, samePairs=0;
        size_t runs3=0, longest=0, zeroBytes=0, ffBytes=0;
        size_t highBit=0, distinctSum=0, pairTokens=0, pairZero=0;
    };
    std::map<std::string,PatternStats> patterns;
    std::puts("PAYLOAD BYTE PATTERN SURVEY");
    for(size_t ri=0;ri<container.records().size();++ri)
        for(const auto& chunk:container.records()[ri].chunks) {
            if((chunk.tag!="PMOC"&&chunk.tag!="MOCS")||chunk.size<=24)continue;
            const size_t p=chunk.offset+8;
            const uint32_t marker=word(p+8);
            const size_t start=p+12, n=chunk.size-20;
            char label[80];
            std::snprintf(label,sizeof(label),"%s/0x%08x",chunk.tag.c_str(),marker);
            auto& st=patterns[label];
            ++st.chunks;st.bytes+=n;
            bool seen[256]={};
            size_t distinct=0,longest=0,runs3=0,same=0,zeros=0,ffs=0,high=0;
            for(size_t j=0;j<n;) {
                const uint8_t v=bytes[start+j];
                if(!seen[v]) {seen[v]=true;++distinct;}
                if(v==0)++zeros;
                if(v==255)++ffs;
                if(v&128)++high;
                if(j&&v==bytes[start+j-1])++same;
                ++j;
            }
            for(size_t j=0;j<n;) {
                size_t end=j+1;
                while(end<n&&bytes[start+end]==bytes[start+j])++end;
                const size_t length=end-j;
                longest=std::max(longest,length);
                if(length>=3)++runs3;
                j=end;
            }
            st.adjacentPairs+=n?n-1:0;st.samePairs+=same;
            st.runs3+=runs3;st.longest=std::max(st.longest,longest);
            st.zeroBytes+=zeros;st.ffBytes+=ffs;st.highBit+=high;
            st.distinctSum+=distinct;
            // Hypothetical byte-pair RLE: count-byte, value-byte.
            // Zero-count incidence is a diagnostic only, not a validation.
            for(size_t j=0;j+1<n;j+=2) {
                ++st.pairTokens;
                if(bytes[start+j]==0)++st.pairZero;
            }
            std::printf("  record=%zu at=0x%zx family=%s n=%zu distinct=%zu adjacentEqual=%zu runs3plus=%zu longest=%zu zeros=%zu ff=%zu highBit=%zu pairCountZero=%zu/%zu\n",
                ri,chunk.offset,label,n,distinct,same,runs3,longest,zeros,ffs,high,
                [&](){size_t z=0;for(size_t j=0;j+1<n;j+=2)if(bytes[start+j]==0)++z;return z;}(),
                n/2);
        }
    std::puts("PAYLOAD PATTERN FAMILY SUMMARY");
    for(const auto& item:patterns) {
        const auto& st=item.second;
        std::printf("  %s chunks=%zu bytes=%zu adjacentEqual=%zu/%zu runs3plus=%zu maxRun=%zu zero=%zu ff=%zu highBit=%zu distinctSum=%zu pairCountZero=%zu/%zu\n",
            item.first.c_str(),st.chunks,st.bytes,st.samePairs,st.adjacentPairs,
            st.runs3,st.longest,st.zeroBytes,st.ffBytes,st.highBit,
            st.distinctSum,st.pairZero,st.pairTokens);
    }
    std::puts("Pair-token counts are hypothetical; no RLE scheme has been established.");
    std::puts("Raw observations only: field semantics and compression remain unknown.");
    std::puts("Correlations are hypotheses; no payloads decoded or addresses dereferenced.");
    std::puts("Validated chunk boundaries; payloads remain opaque.");
    return 0;
}
