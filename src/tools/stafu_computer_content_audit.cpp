// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only audit of COMPUTER.DB field correlations; no content-type assumptions.
#include "engines/stafu/computer_navigation_index.h"
#include "engines/stafu/computer_string_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <cstdio>
#include <map>
#include <string>
#include <tuple>
#include <vector>
namespace {
using Index=reluva::stafu::ComputerStringIndex;
struct Counts {
    size_t records=0,withChildren=0,withoutChildren=0,opaqueZero=0,opaqueNonzero=0;
    size_t text=0,empty=0,nonText=0,unterminated=0,outOfBounds=0;
};
void countField(Counts& c,Index::Status s) {
    switch(s) {
    case Index::Status::Text:++c.text;break;
    case Index::Status::Empty:++c.empty;break;
    case Index::Status::NonText:++c.nonText;break;
    case Index::Status::Unterminated:++c.unterminated;break;
    case Index::Status::OutOfBounds:++c.outOfBounds;break;
    }
}
std::string preview(const Index::Field& field,size_t limit=72) {
    if(field.status!=Index::Status::Text)
        return std::string("<")+Index::statusName(field.status)+">";
    std::string out;
    for(char c:field.text) {
        if(out.size()>=limit) {out+="...";break;}
        if(c=='\r')out+="\\r";
        else if(c=='\n')out+="\\n";
        else if(c=='\t')out+="\\t";
        else if(c=='"')out+="'";
        else out+=c;
    }
    return out;
}
}
int stafuAuditComputerContent(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager rm;
    std::string error;
    if(!rm.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-content-audit: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!rm.read("computer.db",bytes,error)) {
        std::fprintf(stderr,"computer-content-audit: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerNavigationIndex navigation;
    Index strings;
    if(!navigation.parse(bytes,error)||!strings.parse(bytes,error)) {
        std::fprintf(stderr,"computer-content-audit: %s\n",error.c_str());return 1;
    }
    const auto& nodes=navigation.nodes();
    const auto& entries=strings.entries();
    if(nodes.size()!=entries.size()) {
        std::fprintf(stderr,"computer-content-audit: mismatched record counts\n");return 1;
    }
    using Key=std::tuple<unsigned,bool,bool>;
    std::map<Key,Counts> groups;
    std::map<unsigned,Counts> byHeader;
    std::map<unsigned,size_t> opaqueEqualPrimary,opaqueEqualField;
    for(size_t i=0;i<nodes.size();++i) {
        const auto& n=nodes[i];
        const auto& e=entries[i];
        auto tally=[&](Counts& c) {
            ++c.records;
            if(n.children.empty())++c.withoutChildren;else ++c.withChildren;
            if(n.opaqueWord==0)++c.opaqueZero;else ++c.opaqueNonzero;
            for(const auto& f:e.fields)countField(c,f.status);
        };
        tally(groups[{n.headerB,!n.children.empty(),n.opaqueWord!=0}]);
        tally(byHeader[n.headerB]);
        if(n.opaqueWord!=0) {
            for(const auto& other:entries)
                if(n.opaqueWord==other.primaryOffset)++opaqueEqualPrimary[n.headerB];
            for(const auto& f:e.fields)
                if(n.opaqueWord==f.storedOffset)++opaqueEqualField[n.headerB];
        }
    }
    std::printf("entries=%zu edges=%zu reachable=%zu cycles=%s\n",nodes.size(),
        navigation.edges(),navigation.reachableFromRoot(),
        navigation.hasCycle()?"yes":"no");
    std::puts("Fields: headerB=unknown classification; +0x10=opaque word; text[0..2]=observed strings.");
    std::puts("\nSummary by Header B:");
    for(const auto& [b,c]:byHeader)
        std::printf("  B=%u records=%zu children=%zu leaves=%zu opaqueZero=%zu opaqueNonzero=%zu text=%zu empty=%zu nonText=%zu unterminated=%zu outOfBounds=%zu opaqueEqualsPrimary=%zu opaqueEqualsOwnTextOffset=%zu\n",
            b,c.records,c.withChildren,c.withoutChildren,c.opaqueZero,c.opaqueNonzero,
            c.text,c.empty,c.nonText,c.unterminated,c.outOfBounds,
            opaqueEqualPrimary[b],opaqueEqualField[b]);
    std::puts("\nCross-tab: B, hasChildren, opaqueNonzero:");
    for(const auto& [key,c]:groups)
        std::printf("  B=%u children=%s opaque=%s count=%zu\n",
            std::get<0>(key),std::get<1>(key)?"yes":"no",
            std::get<2>(key)?"nonzero":"zero",c.records);
    std::puts("\nAll records with nonzero +0x10 (previews capped; no article interpretation):");
    for(size_t i=0;i<nodes.size();++i) {
        const auto& n=nodes[i];
        if(n.opaqueWord==0)continue;
        const auto& e=entries[i];
        std::printf("  [%zu] B=%u children=%zu opaque=0x%08x fields={%s} {%s} {%s}\n",
            i,unsigned(n.headerB),n.children.size(),n.opaqueWord,
            preview(e.fields[0]).c_str(),preview(e.fields[1]).c_str(),
            preview(e.fields[2]).c_str());
    }
    std::puts("\nRepresentative zero-opaque entries (first 3 of each Header B):");
    std::map<unsigned,size_t> printed;
    for(size_t i=0;i<nodes.size();++i) {
        const auto& n=nodes[i];
        if(n.opaqueWord!=0||printed[n.headerB]>=3)continue;
        ++printed[n.headerB];
        const auto& e=entries[i];
        std::printf("  [%zu] B=%u children=%zu fields={%s} {%s} {%s}\n",
            i,unsigned(n.headerB),n.children.size(),
            preview(e.fields[0]).c_str(),preview(e.fields[1]).c_str(),
            preview(e.fields[2]).c_str());
    }
    std::puts("No inferred display behavior or resource type is asserted.");
    return 0;
}
