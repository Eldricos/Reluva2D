// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only comparison of COMPUTER.DB preorder boundaries. No inferred tree is persisted.
#include "engines/stafu/computer_hierarchy_candidate.h"
#include "engines/stafu/computer_string_index.h"
#include "engines/stafu/resource_manager.h"
#include <array>
#include <cstdio>
#include <string>
#include <vector>

namespace {
using Entry=reluva::stafu::ComputerStringIndex::Entry;
using Candidate=reluva::stafu::ComputerHierarchyCandidate;
const char* label(const Entry& e) {
    const auto& f=e.fields[0];
    return f.status==reluva::stafu::ComputerStringIndex::Status::Text?
        f.text.c_str():"<empty-or-unknown>";
}
void show(const std::vector<Entry>& entries,const Candidate& tree,size_t i) {
    if(i>=entries.size())return;
    const auto& e=entries[i];
    const auto& n=tree.nodes[i];
    std::printf("  [%zu] depth=%zu parent=",i,n.depth);
    if(n.hasParent)std::printf("%zu",n.parent);
    else std::printf("ROOT");
    std::printf(" header=(%u,%u) children=%zu/%u label=\"%s\"\n",
        unsigned(e.headerA),unsigned(e.headerB),
        n.directChildren,unsigned(e.headerA),label(e));
}
void segment(const std::vector<Entry>& entries,size_t start,size_t end) {
    if(start>=end)return;
    const std::vector<Entry> part(entries.begin()+start,entries.begin()+end);
    const auto c=reluva::stafu::analyzeComputerHierarchy(part);
    std::printf("  [%zu,%zu) roots=%zu maxDepth=%zu incomplete=%zu",start,end,
        c.roots,c.maxDepth,c.incompleteParents.size());
    for(size_t local:c.incompleteParents) {
        const size_t global=start+local;
        std::printf(" [%zu:%zu/%u]",global,c.nodes[local].directChildren,
            unsigned(entries[global].headerA));
    }
    std::putchar('\n');
}
}
int stafuAnalyzeComputerBoundaries(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager rm;
    std::string error;
    if(!rm.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-boundaries: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!rm.read("computer.db",bytes,error)) {
        std::fprintf(stderr,"computer-boundaries: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerStringIndex index;
    if(!index.parse(bytes,error)) {
        std::fprintf(stderr,"computer-boundaries: %s\n",error.c_str());return 1;
    }
    const auto& entries=index.entries();
    const auto baseline=reluva::stafu::analyzeComputerHierarchy(entries);
    std::printf("baseline entries=%zu roots=%zu maxDepth=%zu incomplete=%zu\n",
        entries.size(),baseline.roots,baseline.maxDepth,baseline.incompleteParents.size());
    for(size_t i:baseline.incompleteParents) {
        std::printf("  unfinished [%zu] observed=%zu expected=%u label=\"%s\"\n",
            i,baseline.nodes[i].directChildren,unsigned(entries[i].headerA),label(entries[i]));
    }
    // These are inspection points from observed category transitions, not proven roots.
    constexpr std::array<size_t,7> boundaries={0,55,162,166,192,319,338};
    std::puts("\nBoundary contexts in the unmodified baseline:");
    for(size_t b:boundaries) {
        if(b>=entries.size())continue;
        std::printf("\nBoundary [%zu]:\n",b);
        if(b>0)show(entries,baseline,b-1);
        show(entries,baseline,b);
        if(b+1<entries.size())show(entries,baseline,b+1);
        std::puts("  active ancestors and outstanding direct children before boundary:");
        size_t ancestor=b;
        bool any=false;
        while(baseline.nodes[ancestor].hasParent) {
            ancestor=baseline.nodes[ancestor].parent;
            const auto& node=baseline.nodes[ancestor];
            if(node.directChildren<entries[ancestor].headerA) {
                std::printf("    [%zu] %zu/%u \"%s\"\n",ancestor,
                    node.directChildren,unsigned(entries[ancestor].headerA),label(entries[ancestor]));
                any=true;
            }
        }
        if(!any)std::puts("    none (using final totals; not boundary-time balances)");
    }
    std::puts("\nIndependent segment resets (counterfactual, NOT established section boundaries):");
    size_t previous=0;
    for(size_t b:boundaries) {
        if(b>previous)segment(entries,previous,b);
        previous=b;
    }
    segment(entries,previous,entries.size());
    std::puts("\nSingle-cut comparison (both sides independently interpreted):");
    for(size_t b:boundaries) {
        if(b==0||b>=entries.size())continue;
        std::printf("cut=%zu \"%s\"\n",b,label(entries[b]));
        segment(entries,0,b);
        segment(entries,b,entries.size());
    }
    std::puts("Interpretation: forced cuts discard pending parent slots. Fewer unfinished");
    std::puts("nodes alone cannot prove a boundary; compare labels and parent plausibility.");
    return 0;
}
