// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/computer_navigation_index.h"
#include "engines/stafu/computer_string_index.h"
#include "engines/stafu/resource_manager.h"
#include <cstdio>
#include <string>
#include <vector>
int stafuValidateComputerNavigation(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager rm;
    std::string error;
    if(!rm.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-navigation: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!rm.read("computer.db",bytes,error)) {
        std::fprintf(stderr,"computer-navigation: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerNavigationIndex navigation;
    reluva::stafu::ComputerStringIndex strings;
    if(!navigation.parse(bytes,error)||!strings.parse(bytes,error)) {
        std::fprintf(stderr,"computer-navigation: %s\n",error.c_str());return 1;
    }
    const auto& nodes=navigation.nodes();
    const auto& entries=strings.entries();
    std::printf("entries=%zu edges=%zu sharedTargets=%zu reachableFromRoot=%zu cycles=%s\n",
        nodes.size(),navigation.edges(),navigation.sharedTargets(),
        navigation.reachableFromRoot(),navigation.hasCycle()?"yes":"no");
    std::puts("Header B and metadata +0x10 remain opaque.");
    for(size_t i=0;i<nodes.size();++i) {
        const auto& n=nodes[i];
        const auto& field=entries[i].fields[0];
        const char* label=field.status==reluva::stafu::ComputerStringIndex::Status::Text?
            field.text.c_str():"<unknown>";
        std::printf("[%zu] header=(%u,%u) opaque=0x%08x children=%zu label=\"%s\" refs=",
            i,unsigned(n.childCount),unsigned(n.headerB),n.opaqueWord,n.children.size(),label);
        for(size_t j=0;j<n.children.size();++j)
            std::printf("%s%zu",j?",":"",n.children[j]);
        std::putchar('\n');
    }
    return 0;
}
