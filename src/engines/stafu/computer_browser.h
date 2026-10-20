// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "engines/stafu/computer_navigation_index.h"
#include <cstddef>
#include <vector>
namespace reluva::stafu {
// Navigation history is a path, not a parent pointer: nodes may be shared.
class ComputerBrowser {
public:
    explicit ComputerBrowser(const ComputerNavigationIndex& graph):graph_(graph) {
        if(!graph_.nodes().empty())path_.push_back(0);
    }
    bool select(size_t oneBasedChild) {
        if(path_.empty())return false;
        const auto& children=graph_.nodes()[path_.back()].children;
        if(oneBasedChild==0||oneBasedChild>children.size())return false;
        path_.push_back(children[oneBasedChild-1]);
        return true;
    }
    bool back() {
        if(path_.size()<2)return false;
        path_.pop_back();return true;
    }
    bool home() {
        if(path_.empty())return false;
        path_.resize(1);return true;
    }
    size_t current() const {return path_.empty()?0:path_.back();}
    size_t depth() const {return path_.size();}
    const std::vector<size_t>& path() const {return path_;}
private:
    const ComputerNavigationIndex& graph_;
    std::vector<size_t> path_;
};
}
