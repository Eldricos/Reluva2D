// SPDX-License-Identifier: GPL-3.0-or-later
// Diagnostic only: bounded hypothetical run decoders, never renders pixels.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include "engines/stafu/spr_run_experiment.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
int main(int argc,char** argv) {
    if(argc!=3) {
        std::cerr<<"Usage: reluva-stafu-spr-run-experiment <installed-folder> <cd-folder>\n";
        return 2;
    }
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(argv[1],argv[2],error)) {
        std::cerr<<"Resource manager: "<<error<<"\n";return 1;
    }
    constexpr const char* names[]={"BRDGDATA.SPR","BRDGTROI.SPR","BRDGPICA.SPR"};
    std::cout<<"Experimental gamma/Rice alternating-run interpretation; NOT decoded graphics.\n";
    std::cout<<"Offsets 0/32/64 bits, max output 65536 symbols, max 4096 runs.\n";
    for(const char* name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::cerr<<name<<": "<<error<<"\n";return 1;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::cerr<<name<<": "<<error<<"\n";return 1;
        }
        size_t sampled=0;
        for(size_t ri=0;ri<index.records().size()&&sampled<4;++ri) {
            const auto& events=index.records()[ri].events;
            for(size_t ei=0;ei<events.size()&&sampled<4;++ei) {
                reluva::stafu::SprImagePayloadAccessor::View view;
                if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,events[ei],view)||
                   view.size<64)continue;
                ++sampled;
                std::cout<<"PAYLOAD "<<name<<" record="<<ri+1<<" event="<<ei
                         <<" tag="<<events[ei].tag<<" bytes="<<view.size
                         <<" headerA="<<view.headerA<<" headerB="<<view.headerB<<"\n";
                for(size_t start:{size_t(0),size_t(32),size_t(64)})
                    for(unsigned variant=0;variant<=5;++variant) {
                        const bool gamma=variant==0;
                        const auto result=reluva::stafu::trySprRunDecode(
                            view.data,view.size,start,
                            gamma?reluva::stafu::SprRunCode::Gamma:
                                  reluva::stafu::SprRunCode::Rice,
                            gamma?0:variant-1);
                        std::cout<<"  start="<<start<<" code="<<(gamma?"gamma":"rice")
                                 <<" k="<<(gamma?0:variant-1)
                                 <<" valid="<<(result.valid?"yes":"no")
                                 <<" complete="<<(result.complete?"yes":"no")
                                 <<" runs="<<result.runs
                                 <<" symbols="<<result.outputSymbols
                                 <<" bitsConsumed="<<result.bitsConsumed
                                 <<" trailingBits="<<result.trailingBits<<"\n";
                    }
            }
        }
    }
    std::cout<<"Header fields are not proven dimensions; matching a product is not validation.\n";
    return 0;
}
