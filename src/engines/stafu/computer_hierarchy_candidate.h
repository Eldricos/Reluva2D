// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "engines/stafu/computer_string_index.h"
#include <cstddef>
#include <vector>
namespace reluva::stafu {
// Hypothesis only: headerA is the number of immediate children in preorder.
// A completed forest is not proof that the original UI used this hierarchy.
struct ComputerHierarchyCandidate {
    struct Node {
        size_t parent=0;
        size_t depth=0;
        size_t directChildren=0;
        bool hasParent=false;
    };
    std::vector<Node> nodes;
    std::vector<size_t> incompleteParents;
    size_t roots=0;
    size_t maxDepth=0;
    size_t edges=0;
    bool complete() const {return incompleteParents.empty();}
};
ComputerHierarchyCandidate analyzeComputerHierarchy(
    const std::vector<ComputerStringIndex::Entry>& entries);
}
