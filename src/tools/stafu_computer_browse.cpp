// SPDX-License-Identifier: GPL-3.0-or-later
// Interactive, read-only text browser for the original ship's computer.
#include "engines/stafu/computer_browser.h"
#include "engines/stafu/computer_string_index.h"
#include "engines/stafu/resource_manager.h"
#include <charconv>
#include <cstdio>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>
namespace {
using reluva::stafu::ComputerStringIndex;
std::string display(const ComputerStringIndex::Field& field) {
    if(field.status==ComputerStringIndex::Status::Empty)return "";
    if(field.status!=ComputerStringIndex::Status::Text)
        return std::string("[")+ComputerStringIndex::statusName(field.status)+"]";
    std::string result;
    result.reserve(field.text.size());
    // Preserve line structure, normalizing DOS CR and CRLF for the terminal.
    for(size_t i=0;i<field.text.size();++i) {
        const char c=field.text[i];
        if(c=='\r') {
            result+='\n';
            if(i+1<field.text.size()&&field.text[i+1]=='\n')++i;
        } else result+=c;
    }
    return result;
}
bool parseSelection(std::string_view input,size_t& value) {
    if(input.empty())return false;
    const char* begin=input.data();
    const char* end=begin+input.size();
    const auto [ptr,ec]=std::from_chars(begin,end,value);
    return ec==std::errc{}&&ptr==end;
}
}
int stafuBrowseComputer(const char* installed,const char* cd) {
    reluva::stafu::ResourceManager rm;
    std::string error;
    if(!rm.open(installed,cd,error)) {
        std::fprintf(stderr,"computer-browse: %s\n",error.c_str());return 1;
    }
    std::vector<uint8_t> bytes;
    if(!rm.read("computer.db",bytes,error)) {
        std::fprintf(stderr,"computer-browse: %s\n",error.c_str());return 1;
    }
    reluva::stafu::ComputerNavigationIndex graph;
    ComputerStringIndex strings;
    if(!graph.parse(bytes,error)||!strings.parse(bytes,error)) {
        std::fprintf(stderr,"computer-browse: %s\n",error.c_str());return 1;
    }
    if(graph.nodes().empty()||graph.nodes().size()!=strings.entries().size()) {
        std::fputs("computer-browse: invalid record count\n",stderr);return 1;
    }
    reluva::stafu::ComputerBrowser browser(graph);
    const auto& entries=strings.entries();
    std::puts("STAFU ship's computer (read-only). Commands: number, b=back, h=home, q=quit.");
    for(;;) {
        const size_t index=browser.current();
        const auto& entry=entries[index];
        const auto& node=graph.nodes()[index];
        std::cout<<"\n=== "<<display(entry.fields[1])<<" ===\n";
        std::cout<<"Topic: "<<display(entry.fields[0])<<" [record "<<index<<"]\n";
        const auto article=display(entry.fields[2]);
        if(!article.empty())std::cout<<"\n"<<article<<"\n";
        else std::cout<<"\n[No article text]\n";
        if(!node.children.empty()) {
            std::cout<<"\nTopics:\n";
            for(size_t i=0;i<node.children.size();++i) {
                const size_t child=node.children[i];
                std::cout<<"  "<<(i+1)<<". "<<display(entries[child].fields[0])<<"\n";
            }
        }
        std::cout<<"\nSelect number, b, h, or q > "<<std::flush;
        std::string input;
        if(!std::getline(std::cin,input)) {
            std::cout<<"\n";return 0;
        }
        if(input=="q"||input=="quit")return 0;
        if(input=="b"||input=="back") {
            if(!browser.back())std::cout<<"Already at Primary Core.\n";
            continue;
        }
        if(input=="h"||input=="home") {browser.home();continue;}
        size_t selection=0;
        if(!parseSelection(input,selection)||!browser.select(selection))
            std::cout<<"Invalid selection. Choose a listed number or command.\n";
    }
}
