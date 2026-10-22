// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only cross-resource SPR suffix survey.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include "engines/stafu/spr_suffix_compare.h"
#include "engines/stafu/spr_terminal_bits.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
namespace {
constexpr const char* resources[]={
    "BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR",
    "BRDGWORF.SPR","brdgdoor.spr","brdgtitl.spr","viewscr.spr"
};
}
int main(int argc,char** argv) {
    if(argc!=3) {
        std::cerr<<"Usage: reluva-stafu-spr-suffix <installed-folder> <cd-folder>\n";
        return 2;
    }
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(argv[1],argv[2],error)) {
        std::cerr<<"Resource manager: "<<error<<"\n";return 1;
    }
    std::cout<<"SPR suffix survey (read-only, last 4096 bytes per payload)\n";
    std::cout<<"raw=exact end alignment; best=exploratory 0..7 terminal bit trims on each stream\n";
    size_t total=0,substantial=0,unavailable=0;
    size_t longMatches=0,trimmedLong=0,zeroTrimLong=0,nonzeroTrimLong=0;
    size_t trimCounts[8][8]{};
    for(const char* resource:resources) {
        std::vector<uint8_t> bytes;
        if(!manager.read(resource,bytes,error)) {
            std::cout<<resource<<" unavailable: "<<error<<"\n";
            ++unavailable;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::cout<<resource<<" parse failure: "<<error<<"\n";
            ++unavailable;continue;
        }
        size_t pairs=0;
        std::cout<<"RESOURCE "<<resource<<" records="<<index.records().size()<<"\n";
        for(size_t ri=0;ri<index.records().size();++ri) {
            const auto& events=index.records()[ri].events;
            reluva::stafu::SprImagePayloadAccessor::View previous,current;
            size_t prior=0;
            bool havePrevious=false;
            for(size_t ei=0;ei<events.size();++ei) {
                if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,events[ei],current))
                    continue;
                if(havePrevious) {
                    ++pairs;++total;
                    const bool large=previous.size>=64&&current.size>=64;
                    if(large)++substantial;
                    const auto msb=reluva::stafu::compareSprSuffix(
                        previous.data,previous.size,current.data,current.size,true);
                    const auto lsb=reluva::stafu::compareSprSuffix(
                        previous.data,previous.size,current.data,current.size,false);
                    if(large) {
                        constexpr size_t minLongBits=128;
                        const bool isLong=msb.suffixBits>=minLongBits;
                        const bool trimmed=msb.trimmedA!=0||msb.trimmedB!=0;
                        const bool zeroA=reluva::stafu::sprTerminalBitsZero(previous.data,previous.size,msb.trimmedA);
                        const bool zeroB=reluva::stafu::sprTerminalBitsZero(current.data,current.size,msb.trimmedB);
                        if(isLong) {
                            ++longMatches;
                            ++trimCounts[msb.trimmedA][msb.trimmedB];
                            if(trimmed) {
                                ++trimmedLong;
                                if(zeroA&&zeroB)++zeroTrimLong;
                                else ++nonzeroTrimLong;
                            }
                        }
                        const auto lengthDelta=static_cast<int64_t>(current.size)-static_cast<int64_t>(previous.size);
                        std::cout<<"  record="<<ri+1<<" events="<<prior<<"->"<<ei
                                 <<" tags="<<events[prior].tag<<"->"<<events[ei].tag
                                 <<" bytes="<<previous.size<<","<<current.size
                                 <<" deltaBytes="<<lengthDelta
                                 <<" MSBraw="<<msb.rawSuffixBits
                                 <<" MSBbest="<<msb.suffixBits
                                 <<" trim="<<msb.trimmedA<<","<<msb.trimmedB
                                 <<" MSBtrimZero="<<(zeroA&&zeroB?"yes":"no")
                                 <<" long128="<<(isLong?"yes":"no")
                                 <<" LSBraw="<<lsb.rawSuffixBits
                                 <<" LSBbest="<<lsb.suffixBits
                                 <<" trim="<<lsb.trimmedA<<","<<lsb.trimmedB<<"\n";
                    }
                }
                previous=current;prior=ei;havePrevious=true;
            }
        }
        std::cout<<"SUMMARY "<<resource<<" imagePairs="<<pairs<<"\n";
    }
    std::cout<<"TOTAL imagePairs="<<total<<" substantialPairs="<<substantial
             <<" unavailableResources="<<unavailable<<"\n";
    std::cout<<"TERMINAL SUMMARY longMatches128="<<longMatches
             <<" trimmedLong="<<trimmedLong<<" zeroTrimLong="<<zeroTrimLong
             <<" nonzeroTrimLong="<<nonzeroTrimLong<<"\n";
    std::cout<<"TERMINAL TRIM COUNTS (long matches only, A,B,count)\n";
    for(size_t a=0;a<8;++a)for(size_t b=0;b<8;++b)
        if(trimCounts[a][b])std::cout<<"  "<<a<<","<<b<<","<<trimCounts[a][b]<<"\n";
    std::cout<<"Zero trimmed bits are compatible with padding, but do not establish padding semantics.\n";
    return unavailable?1:0;
}
