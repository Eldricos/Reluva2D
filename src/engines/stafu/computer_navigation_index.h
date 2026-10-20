// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace reluva::stafu {
// Read-only explicit reference graph. Header B and the word at +0x10 are opaque.
class ComputerNavigationIndex {
public:
    struct Node {
        uint32_t primaryOffset=0;
        uint16_t childCount=0;
        uint16_t headerB=0;
        uint32_t opaqueWord=0;
        std::vector<size_t> children;
    };
    bool parse(const std::vector<uint8_t>& bytes,std::string& error);
    const std::vector<Node>& nodes() const {return nodes_;}
    size_t edges() const {return edges_;}
    size_t sharedTargets() const {return sharedTargets_;}
    size_t reachableFromRoot() const {return reachable_;}
    bool hasCycle() const {return hasCycle_;}
private:
    std::vector<Node> nodes_;
    size_t edges_=0,sharedTargets_=0,reachable_=0;
    bool hasCycle_=false;
};
}
