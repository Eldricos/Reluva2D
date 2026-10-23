// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include "engines/stafu/spr_triple_suffix.h"
#include <iostream>
#include <string>
#include <vector>
int main(int argc,char** argv) {
    if(argc!=3) {
        std::cerr<<"Usage: reluva-stafu-spr-triples <installed-folder> <cd-folder>\n";
        return 2;
    }
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(argv[1],argv[2],error)) {
        std::cerr<<error<<"\n";return 1;
    }
    struct Target {const char* name;size_t record;};
    const Target targets[]={{"BRDGDATA.SPR",1},{"BRDGTROI.SPR",1},
                            {"BRDGTROI.SPR",4}};
    size_t triples=0,longTriples=0,missing=0;
    std::cout<<"Three consecutive substantial image payloads within selected records.\n";
    for(const auto& target:targets) {
        std::vector<uint8_t> bytes;
        if(!manager.read(target.name,bytes,error)) {
            std::cerr<<target.name<<": "<<error<<"\n";++missing;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)||target.record>index.records().size()) {
            std::cerr<<target.name<<": record unavailable "<<error<<"\n";
            ++missing;continue;
        }
        const auto& events=index.records()[target.record-1].events;
        struct Item {
            size_t event=0;
            std::string tag;
            reluva::stafu::SprImagePayloadAccessor::View view;
        };
        std::vector<Item> items;
        for(size_t i=0;i<events.size();++i) {
            reluva::stafu::SprImagePayloadAccessor::View view;
            if(reluva::stafu::SprImagePayloadAccessor::get(bytes,events[i],view))
                items.push_back({i,events[i].tag,view});
        }
        size_t local=0;
        for(size_t i=2;i<items.size();++i) {
            const auto& a=items[i-2];const auto& b=items[i-1];
            const auto& c=items[i];
            if(a.view.size<64||b.view.size<64||c.view.size<64)continue;
            const auto r=reluva::stafu::compareSprTripleSuffix(
                a.view.data,a.view.size,b.view.data,b.view.size,
                c.view.data,c.view.size);
            if(!r.valid)continue;
            ++triples;++local;
            if(r.sharedBits>=128)++longTriples;
            const auto deltaAB=static_cast<int64_t>(r.startB)-static_cast<int64_t>(r.startA);
            const auto deltaBC=static_cast<int64_t>(r.startC)-static_cast<int64_t>(r.startB);
            std::cout<<target.name<<" record="<<target.record
                     <<" events="<<a.event<<","<<b.event<<","<<c.event
                     <<" tags="<<a.tag<<","<<b.tag<<","<<c.tag
                     <<" bytes="<<a.view.size<<","<<b.view.size<<","<<c.view.size
                     <<" sharedBits="<<r.sharedBits
                     <<" trims="<<r.trimA<<","<<r.trimB<<","<<r.trimC
                     <<" starts="<<r.startA<<","<<r.startB<<","<<r.startC
                     <<" startDeltas="<<deltaAB<<","<<deltaBC
                     <<" headerEqual="<<((a.view.headerA==b.view.headerA&&
                           b.view.headerA==c.view.headerA&&
                           a.view.headerB==b.view.headerB&&
                           b.view.headerB==c.view.headerB&&
                           a.view.marker==b.view.marker&&
                           b.view.marker==c.view.marker)?"yes":"no")
                     <<"\n";
        }
        std::cout<<"RECORD "<<target.name<<" "<<target.record
                 <<" triples="<<local<<"\n";
    }
    std::cout<<"TOTAL triples="<<triples<<" suffix128="<<longTriples
             <<" unavailable="<<missing<<"\n";
    std::cout<<"Offsets are from each full opaque payload, MSB-first. "
                "Trims require zero terminal bits; no decoder assumptions.\n";
    return missing?1:0;
}
