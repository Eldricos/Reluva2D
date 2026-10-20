// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Structural SPR parser. No image decoding or inferred chunk semantics.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace reluva::stafu {
class SprContainer {
public:
    struct Chunk { std::string tag; size_t offset=0; uint32_t size=0; };
    struct Record { size_t start=0,end=0; std::vector<Chunk> chunks; };
    bool parse(const std::vector<uint8_t>& bytes,std::string& error) {
        records_.clear(); version_=0;
        auto fail=[&](const char* why) { error=why; records_.clear(); return false; };
        if(bytes.size()<24) return fail("SPR header too short");
        if(readTag(bytes,0)!="TRPS") return fail("SPR signature missing");
        if(read32(bytes,4)!=bytes.size()) return fail("SPR declared file size mismatch");
        version_=read32(bytes,8);
        if(readTag(bytes,12)!="TSIL") return fail("SPR TSIL chunk missing");
        if(read32(bytes,16)!=bytes.size()-12) return fail("SPR TSIL size mismatch");
        const uint32_t count=read32(bytes,20);
        if(count==0||count>4096||size_t(count)>(bytes.size()-24)/4)
            return fail("SPR invalid TSIL record count");
        const size_t tableEnd=24+size_t(count)*4;
        std::vector<size_t> offsets;
        offsets.reserve(count);
        for(uint32_t i=0;i<count;++i) {
            const size_t relative=read32(bytes,24+size_t(i)*4);
            const size_t absolute=12+relative;
            if(absolute<tableEnd||absolute>=bytes.size()||
               (!offsets.empty()&&absolute<=offsets.back()))
                return fail("SPR invalid TSIL record offset");
            offsets.push_back(absolute);
        }
        if(offsets.front()!=tableEnd) return fail("SPR unaccounted bytes before first record");
        std::vector<Record> parsed;
        parsed.reserve(count);
        for(size_t i=0;i<offsets.size();++i) {
            Record record;
            record.start=offsets[i];
            record.end=i+1<offsets.size()?offsets[i+1]:bytes.size();
            size_t pos=record.start;
            while(pos<record.end) {
                if(record.end-pos<8) return fail("SPR truncated nested chunk header");
                const uint32_t length=read32(bytes,pos+4);
                if(length<8||size_t(length)>record.end-pos)
                    return fail("SPR invalid nested chunk size");
                record.chunks.push_back({readTag(bytes,pos),pos,length});
                pos+=length;
            }
            parsed.push_back(std::move(record));
        }
        records_=std::move(parsed);
        error.clear();
        return true;
    }
    uint32_t versionOrFlags() const {return version_;}
    const std::vector<Record>& records() const {return records_;}
private:
    static uint32_t read32(const std::vector<uint8_t>& b,size_t p) {
        return uint32_t(b[p])|(uint32_t(b[p+1])<<8)|
               (uint32_t(b[p+2])<<16)|(uint32_t(b[p+3])<<24);
    }
    static std::string readTag(const std::vector<uint8_t>& b,size_t p) {
        return std::string(b.begin()+static_cast<std::ptrdiff_t>(p),
                           b.begin()+static_cast<std::ptrdiff_t>(p+4));
    }
    uint32_t version_=0;
    std::vector<Record> records_;
};
}
