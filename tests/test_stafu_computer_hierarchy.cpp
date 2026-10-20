// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/computer_hierarchy_candidate.h"
#include <cstdio>
#include <vector>
namespace {
int failures=0;
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);++failures;}}while(0)
}
int main() {
    using Entry=reluva::stafu::ComputerStringIndex::Entry;
    using reluva::stafu::analyzeComputerHierarchy;
    std::vector<Entry> entries(7);
    // Root A has two direct children, the first has two children of its own.
    // Root B has one direct child.
    for(size_t i=0;i<entries.size();++i)entries[i].headerA=0;
    entries[0].headerA=2;
    entries[1].headerA=2;
    entries[5].headerA=1;
    const auto result=analyzeComputerHierarchy(entries);
    CHECK(result.complete());
    CHECK(result.roots==2);
    CHECK(result.edges==5);
    CHECK(result.maxDepth==2);
    CHECK(result.nodes[0].directChildren==2);
    CHECK(result.nodes[1].directChildren==2);
    CHECK(result.nodes[2].hasParent&&result.nodes[2].parent==1);
    CHECK(result.nodes[3].hasParent&&result.nodes[3].parent==1);
    CHECK(result.nodes[4].hasParent&&result.nodes[4].parent==0);
    CHECK(!result.nodes[5].hasParent);
    CHECK(result.nodes[6].hasParent&&result.nodes[6].parent==5);
    entries.resize(2);
    const auto truncated=analyzeComputerHierarchy(entries);
    CHECK(!truncated.complete());
    CHECK(truncated.incompleteParents.size()==2);
    CHECK(truncated.nodes[0].directChildren==1);
    CHECK(truncated.nodes[1].directChildren==0);
    CHECK(analyzeComputerHierarchy({}).complete());
    return failures?1:0;
}
