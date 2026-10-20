// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/computer_hierarchy_candidate.h"
#include "engines/stafu/computer_string_index.h"
#include "engines/stafu/resource_manager.h"
#include <cstdio>
#include <string>
#include <vector>
int stafuValidateComputerHierarchy(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager rm;
    std::string error;
    if(!rm.open(installed,cd,error)) {std::fprintf(stderr,"computer-hierarchy: %s\n",error.c_str());return 1;}
    std::vector<uint8_t> bytes;
    if(!rm.read("computer.db",bytes,error)) {std::fprintf(stderr,"computer-hierarchy: %s\n",error.c_str());return 1;}
    reluva::stafu::ComputerStringIndex index;
    if(!index.parse(bytes,error)) {std::fprintf(stderr,"computer-hierarchy: %s\n",error.c_str());return 1;}
    const auto& entries=index.entries();
    const auto candidate=reluva::stafu::analyzeComputerHierarchy(entries);
    std::printf("entries=%zu roots=%zu edges=%zu maxDepth=%zu incompleteParents=%zu\n",
        entries.size(),candidate.roots,candidate.edges,candidate.maxDepth,candidate.incompleteParents.size());
    for(size_t i=0;i<entries.size();++i) {
        const auto& e=entries[i];
        const auto& n=candidate.nodes[i];
        const auto& f=e.fields[0];
        const char* label=f.status==reluva::stafu::ComputerStringIndex::Status::Text?f.text.c_str():"<empty-or-unknown>";
        std::printf("[%zu] depth=%zu parent=",i,n.depth);
        if(n.hasParent)std::printf("%zu",n.parent);
        else std::printf("ROOT");
        std::printf(" header=(%u,%u) children=%zu/%u label=\"%s\"\n",
            unsigned(e.headerA),unsigned(e.headerB),n.directChildren,unsigned(e.headerA),label);
    }
    if(!candidate.incompleteParents.empty()) {
        std::puts("INCOMPLETE nodes:");
        for(size_t i:candidate.incompleteParents)
            std::printf("[%zu] expected=%u observed=%zu\n",i,
                unsigned(entries[i].headerA),candidate.nodes[i].directChildren);
    }
    std::puts("Complete counts do not prove this is the original navigation hierarchy.");
    return 0;
}
