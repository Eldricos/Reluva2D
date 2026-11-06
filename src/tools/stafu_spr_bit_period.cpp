// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 241: read-only exact bit-period analysis of SPR payloads.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
struct Sample { size_t record,image,offset,size; };
struct Region { size_t start,end,period; };
bool bitAt(const uint8_t* data,size_t bit) {
    return (data[bit/8] >> (7-(bit%8))) & 1u;
}
std::vector<Region> regionsForPeriod(const uint8_t* data,size_t bits,size_t period) {
    std::vector<Region> result;
    if(bits<period*3)return result;
    // A maximal run of equality bit[i] == bit[i+period].
    // Each run yields one exact periodic interval of length run+period.
    size_t i=0;
    while(i+period<bits) {
        if(bitAt(data,i)!=bitAt(data,i+period)){++i;continue;}
        const size_t start=i;
        do {++i;} while(i+period<bits&&bitAt(data,i)==bitAt(data,i+period));
        const size_t end=i+period;
        if(end-start>=std::max(size_t(80),period*3))
            result.push_back({start,end,period});
    }
    return result;
}
size_t primitivePeriod(const uint8_t* data,size_t start,size_t end) {
    for(size_t p=1;p<=64&&p*2<=end-start;++p) {
        bool exact=true;
        for(size_t i=start;i+p<end;++i)
            if(bitAt(data,i)!=bitAt(data,i+p)){exact=false;break;}
        if(exact)return p;
    }
    return 0;
}
}

int stafuSprBitPeriod(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-bit-period: %s\n",error.c_str());
        return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-bit-period: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-bit-period: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        std::vector<Sample> samples;
        for(size_t r=0;r<index.records().size();++r) {
            size_t ordinal=0;
            for(const auto& e:index.records()[r].events) {
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View v;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,v)&&
                   !(v.marker==0x00030000&&v.size==4))
                    samples.push_back({r,ordinal,v.offset,v.size});
                ++ordinal;
            }
        }
        size_t imagesWithRegions=0,totalRaw=0,totalDistinct=0;
        size_t primitive40=0,nonByte=0,printed=0;
        for(const auto& s:samples) {
            if(s.size<10)continue;
            const auto* data=bytes.data()+s.offset;
            const size_t bits=s.size*8;
            std::vector<Region> candidates;
            for(size_t p=1;p<=64;++p) {
                auto found=regionsForPeriod(data,bits,p);
                totalRaw+=found.size();
                candidates.insert(candidates.end(),found.begin(),found.end());
            }
            std::sort(candidates.begin(),candidates.end(),[](const Region& a,const Region& b){
                if(a.end-a.start!=b.end-b.start)return a.end-a.start>b.end-b.start;
                if(a.start!=b.start)return a.start<b.start;
                return a.period<b.period;
            });
            // Coalesce substantially overlapping candidates, which often
            // describe the same underlying periodic run at several periods.
            std::vector<Region> distinct;
            for(const auto& c:candidates) {
                bool duplicate=false;
                for(const auto& d:distinct) {
                    const size_t overlap=std::min(c.end,d.end)>std::max(c.start,d.start)
                        ?std::min(c.end,d.end)-std::max(c.start,d.start):0;
                    if(overlap*2>=std::min(c.end-c.start,d.end-d.start)) {
                        duplicate=true;break;
                    }
                }
                if(!duplicate)distinct.push_back(c);
            }
            if(!distinct.empty())++imagesWithRegions;
            totalDistinct+=distinct.size();
            for(const auto& d:distinct) {
                const size_t primitive=primitivePeriod(data,d.start,d.end);
                if(primitive==40)++primitive40;
                if(primitive&&primitive%8)++nonByte;
            }
            if(!distinct.empty()&&printed++<16) {
                std::printf("SPR_PERIOD_IMAGE resource=%s record=%zu image=%zu payloadBytes=%zu rawCandidates=%zu distinctRegions=%zu\n",
                    name.c_str(),s.record,s.image,s.size,candidates.size(),distinct.size());
                for(size_t j=0;j<std::min(size_t(5),distinct.size());++j) {
                    const auto& d=distinct[j];
                    const size_t primitive=primitivePeriod(data,d.start,d.end);
                    std::printf("SPR_PERIOD_REGION resource=%s record=%zu image=%zu startBit=%zu endBit=%zu lengthBits=%zu candidatePeriod=%zu primitivePeriod=%zu byteAligned=%d startByte=%zu startBitInByte=%zu\n",
                        name.c_str(),s.record,s.image,d.start,d.end,d.end-d.start,
                        d.period,primitive,primitive&&primitive%8==0?1:0,d.start/8,d.start%8);
                }
            }
        }
        std::printf("SPR_PERIOD_SUMMARY resource=%s direct=%zu analyzedPeriods=64 minRegionBits=80 minCycles=3 imagesWithRegions=%zu rawCandidates=%zu distinctRegions=%zu primitive40=%zu nonBytePrimitive=%zu detailImageLimit=16 detailRegionLimit=5\n",
            name.c_str(),samples.size(),imagesWithRegions,totalRaw,totalDistinct,primitive40,nonByte);
    }
    std::puts("SPR_PERIOD_NOTE Bits are MSB-first; exact periods 1..64 bits, runs at least 80 bits and 3 cycles; overlapping candidate intervals are heuristically merged; no codec semantics inferred.");
    return failures?1:0;
}
