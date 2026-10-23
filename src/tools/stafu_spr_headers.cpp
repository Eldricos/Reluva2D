// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only candidate header audit over every image-like SPR event.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>
namespace {
constexpr const char* names[]={
    "BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR",
    "BRDGWORF.SPR","brdgdoor.spr","brdgtitl.spr","viewscr.spr"
};
struct Group {
    size_t count=0,minBytes=0,maxBytes=0,small=0;
    size_t equalA=0,equalB=0,equalProduct=0;
};
using Key=std::tuple<std::string,uint32_t,uint32_t,uint32_t>;
void add(Group& g,size_t n,uint32_t a,uint32_t b) {
    if(!g.count){g.minBytes=n;g.maxBytes=n;}
    if(n<g.minBytes)g.minBytes=n;
    if(n>g.maxBytes)g.maxBytes=n;
    ++g.count;
    if(n<64)++g.small;
    if(n==a)++g.equalA;
    if(n==b)++g.equalB;
    if(uint64_t(a)*uint64_t(b)==uint64_t(n))++g.equalProduct;
}
}
int main(int argc,char** argv) {
    if(argc!=3){
        std::cerr<<"Usage: reluva-stafu-spr-headers <installed-folder> <cd-folder>\n";
        return 2;
    }
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
    std::map<Key,Group> groups;
    size_t missing=0,events=0,records=0;
    for(const char* name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)){
            std::cerr<<name<<": "<<error<<"\n";++missing;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)){
            std::cerr<<name<<": "<<error<<"\n";++missing;continue;
        }
        records+=index.records().size();
        for(size_t ri=0;ri<index.records().size();++ri) {
            const auto& ev=index.records()[ri].events;
            for(size_t ei=0;ei<ev.size();++ei) {
                reluva::stafu::SprImagePayloadAccessor::View v;
                if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,ev[ei],v))
                    continue;
                ++events;
                add(groups[{ev[ei].tag,v.headerA,v.headerB,v.marker}],
                    v.size,v.headerA,v.headerB);
                std::cout<<"EVENT "<<name<<" record="<<ri+1<<" event="<<ei
                         <<" tag="<<ev[ei].tag
                         <<" a="<<v.headerA<<" b="<<v.headerB
                         <<" marker="<<v.marker<<" bodyBytes="<<v.size
                         <<" aTimesB="<<uint64_t(v.headerA)*v.headerB<<"\n";
            }
        }
    }
    size_t groupedEvents=0,variable=0;
    for(const auto& [key,g]:groups) {
        const auto& [tag,a,b,marker]=key;
        groupedEvents+=g.count;
        if(g.minBytes!=g.maxBytes)++variable;
        std::cout<<"GROUP tag="<<tag<<" a="<<a<<" b="<<b<<" marker="<<marker
                 <<" count="<<g.count<<" minBytes="<<g.minBytes
                 <<" maxBytes="<<g.maxBytes<<" variableSize="<<(g.minBytes!=g.maxBytes?"yes":"no")
                 <<" smallUnder64="<<g.small<<" bytesEqualA="<<g.equalA
                 <<" bytesEqualB="<<g.equalB<<" bytesEqualProduct="<<g.equalProduct<<"\n";
    }
    std::cout<<"SUMMARY records="<<records<<" imageEvents="<<events
             <<" groups="<<groups.size()<<" variableSizeGroups="<<variable
             <<" groupedEvents="<<groupedEvents<<" unavailable="<<missing<<"\n";
    std::cout<<"Header fields are candidate LE32 values, not validated dimensions; "
                "aTimesB is a hypothesis check, not a decoded pixel count.\n";
    return missing?1:0;
}
