// SPDX-License-Identifier: GPL-3.0-or-later
// Localized byte-prefix and MSB suffix change-region survey.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include "engines/stafu/spr_change_region.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
int main(int argc,char** argv) {
    if(argc!=3) {
        std::cerr<<"Usage: reluva-stafu-spr-changes <installed-folder> <cd-folder>\n";
        return 2;
    }
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
    struct Target {const char* name;size_t record;};
    const Target targets[]={
        {"BRDGPICA.SPR",4},{"BRDGRIKR.SPR",4},
        {"BRDGDATA.SPR",1},{"BRDGTROI.SPR",4},
        {"BRDGWORF.SPR",1},{"brdgtitl.spr",1}
    };
    size_t missing=0;
    for(const auto& target:targets) {
        std::vector<uint8_t> bytes;
        if(!manager.read(target.name,bytes,error)){
            std::cerr<<target.name<<": "<<error<<"\n";++missing;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)||target.record>index.records().size()){
            std::cerr<<target.name<<": record unavailable "<<error<<"\n";
            ++missing;continue;
        }
        const auto& events=index.records()[target.record-1].events;
        reluva::stafu::SprImagePayloadAccessor::View previous,current;
        size_t previousId=0,count=0,overlaps=0;
        bool have=false;
        for(size_t i=0;i<events.size();++i) {
            if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,events[i],current)
               ||current.size<64)continue;
            if(have) {
                const auto r=reluva::stafu::describeSprChangeRegion(
                    previous.data,previous.size,current.data,current.size);
                if(r.overlapsPrefix)++overlaps;
                std::cout<<target.name<<" record="<<target.record
                         <<" events="<<previousId<<"->"<<i
                         <<" tags="<<events[previousId].tag<<"->"<<events[i].tag
                         <<" prefixBits="<<r.prefixBits
                         <<" suffixStartBits="<<r.suffixStartA<<","<<r.suffixStartB
                         <<" unmatchedBits="<<r.unmatchedA<<","<<r.unmatchedB
                         <<" unmatchedDelta="<<
                             (static_cast<long long>(r.unmatchedB)-
                              static_cast<long long>(r.unmatchedA))
                         <<" sharedSuffixBits="<<r.suffixBits
                         <<" terminalTrims="<<r.trimA<<","<<r.trimB
                         <<" overlap="<<(r.overlapsPrefix?"yes":"no")<<"\n";
                ++count;
            }
            previous=current;previousId=i;have=true;
        }
        std::cout<<"SUMMARY "<<target.name<<" record="<<target.record
                 <<" pairs="<<count<<" overlaps="<<overlaps<<"\n";
    }
    std::cout<<"Byte-aligned prefix, 4096-byte capped MSB suffix; unmatched interval "
                "is descriptive, not a decoded changed-pixel region.\n";
    return missing?1:0;
}
