// SPDX-License-Identifier: GPL-3.0-or-later
// Representative PMOC and MOCS record-local structural pair comparison.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include "engines/stafu/spr_pair_relation.h"
#include "engines/stafu/spr_structural_pair.h"
#include "engines/stafu/spr_suffix_control.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
int main(int argc,char** argv) {
    if(argc!=3) {
        std::cerr<<"Usage: reluva-stafu-spr-structure-pairs <installed-folder> <cd-folder>\n";
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
        size_t previousId=0,examined=0;
        bool have=false;
        for(size_t i=0;i<events.size();++i) {
            if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,events[i],current)
               ||current.size<64)continue;
            if(have) {
                const auto pair=reluva::stafu::compareSprPair(
                    previous.data,previous.size,current.data,current.size);
                // Skip the 12-byte candidate header for body-only measurements;
                // the exact header interpretation remains unproven.
                const auto body=reluva::stafu::surveySprStructuralPair(
                    previous.data,previous.size,current.data,current.size,4096,12);
                const auto shuffled=reluva::stafu::shuffledSprBytes(
                    current.data+12,current.size-12,
                    0x12345678u+static_cast<uint32_t>(i));
                const auto control=reluva::stafu::surveySprStructuralPair(
                    previous.data+12,previous.size-12,
                    shuffled.data(),shuffled.size());
                std::cout<<target.name<<" record="<<target.record
                         <<" events="<<previousId<<"->"<<i
                         <<" tags="<<events[previousId].tag<<"->"<<events[i].tag
                         <<" bytes="<<previous.size<<","<<current.size
                         <<" prefixBody="<<body.commonPrefix
                         <<" alignedEqual="<<body.alignedEqual<<"/"<<body.alignedCompared
                         <<" shuffledEqual="<<control.alignedEqual
                         <<" adjacentRepeat="<<body.repeatAdjacentA<<","
                         <<body.repeatAdjacentB
                         <<" longestRun="<<body.longestRunA<<","<<body.longestRunB
                         <<" suffixBits="<<pair.sharedSuffixBits
                         <<" trims="<<pair.trimA<<","<<pair.trimB<<"\n";
                ++examined;
            }
            previous=current;previousId=i;have=true;
        }
        std::cout<<"SUMMARY "<<target.name<<" record="<<target.record
                 <<" pairs="<<examined<<"\n";
    }
    std::cout<<"Read-only, first 4096 body bytes after skipping 12 candidate header bytes; "
                "suffixBits from full payload. Shuffled-byte control; no codec assumptions.\n";
    return missing?1:0;
}
