// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Read-only, record-local SPR sequence index. Chunk meanings remain unknown.
#include "engines/stafu/spr_container.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace reluva::stafu {
class SprSequenceIndex {
public:
    struct Event {
        std::string tag;
        size_t chunkOffset=0;
        uint32_t chunkSize=0;
        size_t payloadOffset=0;
        size_t payloadSize=0;
        // Only present for four-byte EMIT payloads; no time unit implied.
        bool hasRawControlValue=false;
        uint32_t rawControlValue=0;
        // Only present for image-like chunks with at least 12 payload bytes.
        bool hasImageHeader=false;
        uint32_t headerA=0,headerB=0,marker=0;
        size_t imageDataOffset=0,imageDataSize=0;
    };
    struct Record {
        size_t start=0,end=0;
        std::vector<Event> events;
    };
    bool build(const std::vector<uint8_t>& bytes,std::string& error) {
        records_.clear();
        SprContainer container;
        if(!container.parse(bytes,error))return false;
        std::vector<Record> parsed;
        parsed.reserve(container.records().size());
        for(const auto& source:container.records()) {
            Record record;
            record.start=source.start;record.end=source.end;
            record.events.reserve(source.chunks.size());
            for(const auto& chunk:source.chunks) {
                Event e;
                e.tag=chunk.tag;e.chunkOffset=chunk.offset;e.chunkSize=chunk.size;
                e.payloadOffset=chunk.offset+8;
                e.payloadSize=static_cast<size_t>(chunk.size)-8;
                if(e.tag=="EMIT"&&e.payloadSize==4) {
                    e.hasRawControlValue=true;
                    e.rawControlValue=read32(bytes,e.payloadOffset);
                }
                if((e.tag=="PMOC"||e.tag=="MOCS")&&e.payloadSize>=12) {
                    e.hasImageHeader=true;
                    e.headerA=read32(bytes,e.payloadOffset);
                    e.headerB=read32(bytes,e.payloadOffset+4);
                    e.marker=read32(bytes,e.payloadOffset+8);
                    e.imageDataOffset=e.payloadOffset+12;
                    e.imageDataSize=e.payloadSize-12;
                }
                record.events.push_back(std::move(e));
            }
            parsed.push_back(std::move(record));
        }
        records_=std::move(parsed);
        error.clear();
        return true;
    }
    const std::vector<Record>& records() const {return records_;}
private:
    static uint32_t read32(const std::vector<uint8_t>& b,size_t p) {
        return uint32_t(b[p])|(uint32_t(b[p+1])<<8)|
               (uint32_t(b[p+2])<<16)|(uint32_t(b[p+3])<<24);
    }
    std::vector<Record> records_;
};
}
