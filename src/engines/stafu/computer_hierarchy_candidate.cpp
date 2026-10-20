// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/computer_hierarchy_candidate.h"
#include <algorithm>
#include <cstddef>
#include <vector>
namespace reluva::stafu {
ComputerHierarchyCandidate analyzeComputerHierarchy(
    const std::vector<ComputerStringIndex::Entry>& entries) {
    ComputerHierarchyCandidate result;
    result.nodes.resize(entries.size());
    struct Frame {size_t index;size_t remaining;};
    std::vector<Frame> stack;
    for(size_t i=0;i<entries.size();++i) {
        while(!stack.empty()&&stack.back().remaining==0)stack.pop_back();
        auto& node=result.nodes[i];
        node.depth=stack.size();
        result.maxDepth=std::max(result.maxDepth,node.depth);
        if(stack.empty())++result.roots;
        else {
            node.hasParent=true;
            node.parent=stack.back().index;
            ++result.nodes[node.parent].directChildren;
            ++result.edges;
            --stack.back().remaining;
        }
        if(entries[i].headerA!=0)stack.push_back({i,entries[i].headerA});
    }
    for(const auto& frame:stack)
        if(frame.remaining!=0)result.incompleteParents.push_back(frame.index);
    return result;
}
}
