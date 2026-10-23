// SPDX-License-Identifier: GPL-3.0-or-later
// Whole-record common suffix, with deterministic shuffled-order control.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include "engines/stafu/spr_sequence_suffix.h"
#include "engines/stafu/spr_suffix_segment.h"
#include "engines/stafu/spr_suffix_control.h"
#include <iostream>
#include <string>
#include <vector>
int main(int argc,char** argv) {
    if(argc!=3) {
        std::cerr<<"Usage: reluva-stafu-spr-sequence <installed-folder> <cd-folder>\n";
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
    size_t unavailable=0;
    for(const auto& target:targets) {
        std::vector<uint8_t> bytes;
        if(!manager.read(target.name,bytes,error)) {
            std::cerr<<target.name<<": "<<error<<"\n";++unavailable;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)||target.record>index.records().size()) {
            std::cerr<<target.name<<": record unavailable "<<error<<"\n";
            ++unavailable;continue;
        }
        const auto& events=index.records()[target.record-1].events;
        std::vector<reluva::stafu::SprSequencePayload> inputs;
        std::vector<size_t> eventIds;
        for(size_t ei=0;ei<events.size();++ei) {
            reluva::stafu::SprImagePayloadAccessor::View view;
            if(reluva::stafu::SprImagePayloadAccessor::get(bytes,events[ei],view)&&
               view.size>=64) {
                inputs.push_back({view.data,view.size});
                eventIds.push_back(ei);
            }
        }
        if(inputs.size()<2) {
            std::cout<<target.name<<" record="<<target.record
                     <<" insufficient substantial payloads\n";continue;
        }
        const auto real=reluva::stafu::compareSprSequenceSuffix(inputs);
        std::vector<std::vector<uint8_t>> shuffled;
        shuffled.reserve(inputs.size());
        for(size_t i=0;i<inputs.size();++i)
            shuffled.push_back(reluva::stafu::shuffledSprBytes(
                inputs[i].data,inputs[i].size,
                0x9e3779b9u+static_cast<uint32_t>(i)));
        std::vector<reluva::stafu::SprSequencePayload> control;
        for(const auto& v:shuffled)control.push_back({v.data(),v.size()});
        const auto baseline=reluva::stafu::compareSprSequenceSuffix(control);
        std::cout<<target.name<<" record="<<target.record
                 <<" payloads="<<inputs.size()
                 <<" commonBits="<<real.sharedBits
                 <<" shuffledCommonBits="<<baseline.sharedBits<<"\n";
        const auto segments=reluva::stafu::segmentSprSuffix(inputs);
        std::cout<<"SEGMENTS "<<target.name<<" record="<<target.record
                 <<" count="<<segments.size()<<" thresholdBits=128\n";
        for(const auto& seg:segments) {
            const auto& first=events[eventIds[seg.first]];
            const auto& last=events[eventIds[seg.last]];
            size_t controls=0,emit=0,kram=0;
            for(size_t ei=eventIds[seg.first];ei<=eventIds[seg.last];++ei) {
                if(events[ei].tag=="EMIT")++emit;
                if(events[ei].tag=="KRAM")++kram;
                if(events[ei].tag!="PMOC"&&events[ei].tag!="MOCS")++controls;
            }
            const bool sameHeader=first.headerA==last.headerA&&
                first.headerB==last.headerB&&first.marker==last.marker;
            std::cout<<"  SEGMENT events="<<eventIds[seg.first]<<".."
                     <<eventIds[seg.last]<<" payloads="<<seg.last-seg.first+1
                     <<" sharedBits="<<seg.commonBits
                     <<" tags="<<first.tag<<".."<<last.tag
                     <<" endpointHeadersEqual="<<(sameHeader?"yes":"no")
                     <<" interveningControls="<<controls
                     <<" EMIT="<<emit<<" KRAM="<<kram<<"\n";
            if(seg.last+1<inputs.size()) {
                const auto& left=events[eventIds[seg.last]];
                const auto& right=events[eventIds[seg.last+1]];
                const auto pair=reluva::stafu::compareSprSequenceSuffix(
                    {{inputs[seg.last].data,inputs[seg.last].size},
                     {inputs[seg.last+1].data,inputs[seg.last+1].size}});
                std::cout<<"    BREAK events="<<eventIds[seg.last]<<"->"
                         <<eventIds[seg.last+1]
                         <<" pairBits="<<pair.sharedBits
                         <<" tagChange="<<(left.tag!=right.tag?"yes":"no")
                         <<" headerChange="<<((left.headerA!=right.headerA||
                            left.headerB!=right.headerB||
                            left.marker!=right.marker)?"yes":"no")
                         <<"\n";
            }
        }
        for(size_t i=0;i<inputs.size();++i)
            std::cout<<"  event="<<eventIds[i]<<" tag="<<events[eventIds[i]].tag
                     <<" bytes="<<inputs[i].size
                     <<" trim="<<real.trims[i]<<" startBit="<<real.starts[i]
                     <<"\n";
    }
    std::cout<<"All substantial image payloads (>=64 bytes) within each selected record; "
                "4096-byte suffix cap; zero terminal trims only.\n";
    std::cout<<"Shuffled controls preserve per-payload byte frequencies but not order. "
                "No format or frame semantics inferred.\n";
    return unavailable?1:0;
}
