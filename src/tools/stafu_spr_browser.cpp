// SPDX-License-Identifier: GPL-3.0-or-later
// Interactive read-only SPR resource and sequence browser.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_playback_cursor.h"
#include <charconv>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {
bool number(std::string_view s,size_t& n) {
    if(s.empty())return false;
    const auto [end,error]=std::from_chars(s.data(),s.data()+s.size(),n);
    return error==std::errc{}&&end==s.data()+s.size();
}
constexpr const char* resources[]={
    "BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR",
    "BRDGWORF.SPR","brdgdoor.spr","brdgtitl.spr","viewscr.spr"
};
void showEvent(const reluva::stafu::SprSequenceIndex::Event& e,size_t i) {
    std::cout<<"  ["<<i<<"] "<<e.tag<<" offset="<<e.chunkOffset
             <<" size="<<e.chunkSize<<" payload="<<e.payloadSize;
    if(e.hasRawControlValue)std::cout<<" rawEMIT="<<e.rawControlValue;
    if(e.hasImageHeader)
        std::cout<<" header=("<<e.headerA<<","<<e.headerB<<",0x"
                 <<std::hex<<e.marker<<std::dec<<") imageBytes="<<e.imageDataSize
                 <<" imageOffset="<<e.imageDataOffset;
    std::cout<<"\\n";
}
void show(const reluva::stafu::SprSequenceIndex::Record& record) {
    std::cout<<"Record offsets ["<<record.start<<", "<<record.end<<") - "
             <<record.events.size()<<" chunks\\n";
    for(size_t i=0;i<record.events.size();++i)showEvent(record.events[i],i);
}
void stepRecord(const reluva::stafu::SprSequenceIndex& index,size_t selected) {
    reluva::stafu::SprPlaybackCursor cursor;
    if(!cursor.bind(index,selected))return;
    std::string input;
    std::cout<<"Stepping record "<<selected+1<<" (raw chunk order, no timing).\\n";
    for(;;) {
        if(const auto* event=cursor.current())showEvent(*event,cursor.position());
        else std::cout<<"[End of record]\\n";
        std::cout<<"n=next, r=restart, l=list all, b=records, q=quit > "<<std::flush;
        if(!std::getline(std::cin,input)||input=="q"||input=="quit")return;
        if(input=="b"||input=="back")return;
        if(input=="r"||input=="restart"){cursor.restart();continue;}
        if(input=="l"||input=="list"){show(index.records()[selected]);continue;}
        if(input=="n"||input=="next"||input.empty()) {
            if(!cursor.advance())std::cout<<"Already at end. Use r to restart.\\n";
            continue;
        }
        std::cout<<"Unknown command.\\n";
    }
}
}
int main(int argc,char** argv) {
    if(argc!=3) {
        std::cerr<<"Usage: reluva-stafu-spr-browser <installed-folder> <cd-folder>\n";
        return 2;
    }
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(argv[1],argv[2],error)) {
        std::cerr<<"SPR browser: "<<error<<"\n";return 1;
    }
    std::cout<<"STAFU SPR sequence browser (read-only). Control semantics unknown.\n";
    for(;;) {
        std::cout<<"\nResources:\n";
        for(size_t i=0;i<sizeof(resources)/sizeof(resources[0]);++i)
            std::cout<<"  "<<i+1<<". "<<resources[i]<<"\n";
        std::cout<<"Select resource number, or q to quit > "<<std::flush;
        std::string input;
        if(!std::getline(std::cin,input)||input=="q"||input=="quit")return 0;
        size_t choice=0;
        if(!number(input,choice)||choice<1||choice>sizeof(resources)/sizeof(resources[0])) {
            std::cout<<"Invalid selection.\n";continue;
        }
        std::vector<uint8_t> bytes;
        if(!manager.read(resources[choice-1],bytes,error)) {
            std::cout<<"Resource unavailable: "<<error<<"\n";continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::cout<<"Invalid SPR resource: "<<error<<"\n";continue;
        }
        for(;;) {
            std::cout<<"\n"<<resources[choice-1]<<": "<<index.records().size()
                     <<" records. Select 1-"<<index.records().size()
                     <<", b=resources, q=quit > "<<std::flush;
            if(!std::getline(std::cin,input)||input=="q"||input=="quit")return 0;
            if(input=="b"||input=="back")break;
            size_t selected=0;
            if(!number(input,selected)||selected<1||selected>index.records().size()) {
                std::cout<<"Invalid record.\n";continue;
            }
            stepRecord(index,selected-1);
        }
    }
}
