// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/computer_string_index.h"
#include "engines/stafu/computer_database_index.h"
#include <limits>
namespace reluva::stafu {
namespace {
uint32_t read32(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p])|(uint32_t(b[p+1])<<8)|(uint32_t(b[p+2])<<16)|(uint32_t(b[p+3])<<24);
}
uint16_t read16(const std::vector<uint8_t>& b,size_t p) {
    return uint16_t(b[p])|(uint16_t(b[p+1])<<8);
}
bool printable(uint8_t c) {return c>=32&&c<=126;}
}
const char* ComputerStringIndex::statusName(Status s) {
    switch(s) {
    case Status::Text:return "text";
    case Status::Empty:return "empty";
    case Status::NonText:return "non-text";
    case Status::Unterminated:return "unterminated";
    case Status::OutOfBounds:return "out-of-bounds";
    }
    return "unknown";
}
bool ComputerStringIndex::parse(const std::vector<uint8_t>& bytes,std::string& error) {
    entries_.clear();base_=0;error.clear();
    ComputerDatabaseIndex primary;
    if(!primary.parse(bytes,error))return false;
    if(primary.tableEnd()>bytes.size()||bytes.size()-primary.tableEnd()<4) {
        error="Missing four-byte region after primary table";return false;
    }
    const size_t base=primary.tableEnd()+4;
    std::vector<Entry> parsed;
    parsed.reserve(primary.primaryOffsets().size());
    for(uint32_t p:primary.primaryOffsets()) {
        if(uint64_t(base)+p+16>bytes.size()) {
            error="Candidate metadata source outside database";return false;
        }
        Entry e;
        e.primaryOffset=p;
        e.metadataOffset=base+size_t(p);
        e.headerA=read16(bytes,e.metadataOffset);
        e.headerB=read16(bytes,e.metadataOffset+2);
        for(size_t j=0;j<3;++j) {
            Field& f=e.fields[j];
            f.storedOffset=read32(bytes,e.metadataOffset+4+4*j);
            const uint64_t absolute=uint64_t(base)+f.storedOffset;
            if(absolute>=bytes.size()||absolute>std::numeric_limits<size_t>::max()) {
                f.status=Status::OutOfBounds;continue;
            }
            f.absoluteOffset=size_t(absolute);
            const size_t start=f.absoluteOffset;
            if(bytes[start]==0) {f.status=Status::Empty;continue;}
            constexpr size_t kMaxText=4096;
            size_t n=0;
            while(n<kMaxText&&start+n<bytes.size()&&printable(bytes[start+n]))++n;
            if(start+n>=bytes.size()||n==kMaxText) {
                f.status=Status::Unterminated;continue;
            }
            if(bytes[start+n]!=0) {f.status=Status::NonText;continue;}
            f.text.assign(reinterpret_cast<const char*>(bytes.data()+start),n);
            f.status=Status::Text;
        }
        parsed.push_back(std::move(e));
    }
    base_=base;entries_=std::move(parsed);
    return true;
}
}
