// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/computer_navigation_index.h"
#include "engines/stafu/computer_database_index.h"
#include <cstdint>
#include <unordered_map>
#include <vector>
namespace reluva::stafu {
namespace {
uint32_t read32(const std::vector<uint8_t>& b,size_t p) {
    return uint32_t(b[p])|(uint32_t(b[p+1])<<8)|
        (uint32_t(b[p+2])<<16)|(uint32_t(b[p+3])<<24);
}
uint16_t read16(const std::vector<uint8_t>& b,size_t p) {
    return uint16_t(b[p])|(uint16_t(b[p+1])<<8);
}
}
bool ComputerNavigationIndex::parse(const std::vector<uint8_t>& bytes,std::string& error) {
    nodes_.clear();edges_=sharedTargets_=reachable_=0;hasCycle_=false;error.clear();
    ComputerDatabaseIndex primary;
    if(!primary.parse(bytes,error))return false;
    if(primary.tableEnd()>bytes.size()||bytes.size()-primary.tableEnd()<4) {
        error="Missing data base after primary offset table";return false;
    }
    const size_t base=primary.tableEnd()+4;
    const auto& offsets=primary.primaryOffsets();
    std::unordered_map<uint32_t,size_t> byOffset;
    for(size_t i=0;i<offsets.size();++i)byOffset.emplace(offsets[i],i);
    std::vector<Node> parsed;
    parsed.reserve(offsets.size());
    for(size_t i=0;i<offsets.size();++i) {
        const uint64_t start=uint64_t(base)+offsets[i];
        if(start>bytes.size()||bytes.size()-size_t(start)<20) {
            error="Navigation metadata truncated at record "+std::to_string(i);return false;
        }
        const size_t p=size_t(start);
        Node node;
        node.primaryOffset=offsets[i];
        node.childCount=read16(bytes,p);
        node.headerB=read16(bytes,p+2);
        node.opaqueWord=read32(bytes,p+16);
        const uint64_t end=start+20ull+uint64_t(node.childCount)*4ull;
        if(end>bytes.size()||
           (i+1<offsets.size()&&end>uint64_t(base)+offsets[i+1])) {
            error="Navigation children exceed record bounds at record "+std::to_string(i);
            return false;
        }
        node.children.reserve(node.childCount);
        for(size_t j=0;j<node.childCount;++j) {
            const uint32_t target=read32(bytes,p+20+j*4);
            const auto it=byOffset.find(target);
            if(it==byOffset.end()) {
                error="Unresolved child reference at record "+std::to_string(i)+
                    " slot "+std::to_string(j);return false;
            }
            node.children.push_back(it->second);
        }
        parsed.push_back(std::move(node));
    }
    std::vector<size_t> incoming(parsed.size(),0);
    size_t edges=0,shared=0,reachable=0;
    for(const auto& n:parsed)for(size_t child:n.children) {
        ++edges;
        ++incoming[child];
    }
    for(size_t count:incoming)if(count>1)++shared;
    // Iterative DFS: safe for shared nodes, cycles, and long chains.
    std::vector<uint8_t> color(parsed.size(),0);
    bool cycle=false;
    auto visit=[&](size_t root) {
        std::vector<std::pair<size_t,size_t>> stack;
        color[root]=1;
        stack.emplace_back(root,0);
        while(!stack.empty()) {
            auto& frame=stack.back();
            if(frame.second==parsed[frame.first].children.size()) {
                color[frame.first]=2;stack.pop_back();continue;
            }
            const size_t child=parsed[frame.first].children[frame.second++];
            if(color[child]==1)cycle=true;
            if(color[child]==0) {
                color[child]=1;
                stack.emplace_back(child,0);
            }
        }
    };
    if(!parsed.empty()) {
        visit(0);
        for(uint8_t c:color)if(c!=0)++reachable;
    }
    for(size_t i=0;i<parsed.size();++i)if(color[i]==0)visit(i);
    nodes_=std::move(parsed);
    edges_=edges;sharedTargets_=shared;reachable_=reachable;hasCycle_=cycle;
    return true;
}
}
