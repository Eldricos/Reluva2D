// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only segmentation survey across all known bridge SPR resources.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include "engines/stafu/spr_suffix_segment.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
namespace {
constexpr const char* resources[]={
    "BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR",
    "BRDGWORF.SPR","brdgdoor.spr","brdgtitl.spr","viewscr.spr"
};
struct Stats {
    size_t records=0,eligible=0,segments=0,multi=0,breaks=0;
    size_t breakTag=0,breakHeader=0,breakBoth=0,breakNeither=0;
    size_t withinTag=0,withinHeader=0,withinBoth=0,withinNeither=0;
};
void addBoundary(Stats& s,bool isBreak,bool tag,bool header) {
    if(isBreak) {
        ++s.breaks;
        if(tag)++s.breakTag;
        if(header)++s.breakHeader;
        if(tag&&header)++s.breakBoth;
        if(!tag&&!header)++s.breakNeither;
    } else {
        if(tag)++s.withinTag;
        if(header)++s.withinHeader;
        if(tag&&header)++s.withinBoth;
        if(!tag&&!header)++s.withinNeither;
    }
}
void printStats(const char* name,const Stats& s) {
    std::cout<<"SUMMARY "<<name<<" records="<<s.records
             <<" eligibleRecords="<<s.eligible<<" segments="<<s.segments
             <<" multiPayloadSegments="<<s.multi<<" breaks="<<s.breaks
             <<" breakTagChange="<<s.breakTag
             <<" breakHeaderChange="<<s.breakHeader
             <<" breakBoth="<<s.breakBoth<<" breakNeither="<<s.breakNeither
             <<" withinTagChange="<<s.withinTag
             <<" withinHeaderChange="<<s.withinHeader
             <<" withinBoth="<<s.withinBoth<<" withinNeither="<<s.withinNeither<<"\n";
}
}
int main(int argc,char** argv) {
    if(argc!=3) {
        std::cerr<<"Usage: reluva-stafu-spr-segments <installed-folder> <cd-folder>\n";
        return 2;
    }
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(argv[1],argv[2],error)) {
        std::cerr<<error<<"\n";return 1;
    }
    Stats total;
    size_t unavailable=0;
    for(const char* name:resources) {
        Stats stats;
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::cerr<<name<<": "<<error<<"\n";++unavailable;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::cerr<<name<<": "<<error<<"\n";++unavailable;continue;
        }
        for(size_t ri=0;ri<index.records().size();++ri) {
            ++stats.records;++total.records;
            const auto& events=index.records()[ri].events;
            std::vector<reluva::stafu::SprSequencePayload> inputs;
            std::vector<size_t> ids;
            for(size_t ei=0;ei<events.size();++ei) {
                reluva::stafu::SprImagePayloadAccessor::View view;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,events[ei],view)&&
                   view.size>=64) {
                    inputs.push_back({view.data,view.size});
                    ids.push_back(ei);
                }
            }
            if(inputs.empty())continue;
            ++stats.eligible;++total.eligible;
            const auto segments=reluva::stafu::segmentSprSuffix(inputs);
            stats.segments+=segments.size();total.segments+=segments.size();
            std::cout<<name<<" record="<<ri+1<<" payloads="<<inputs.size()
                     <<" segments="<<segments.size()<<"\n";
            for(const auto& seg:segments) {
                const bool multi=seg.last>seg.first;
                if(multi){++stats.multi;++total.multi;}
                std::cout<<"  SEGMENT events="<<ids[seg.first]<<".."<<ids[seg.last]
                         <<" count="<<seg.last-seg.first+1
                         <<" commonBits="<<seg.commonBits
                         <<" tags="<<events[ids[seg.first]].tag<<".."
                         <<events[ids[seg.last]].tag<<"\n";
            }
            for(size_t i=1;i<inputs.size();++i) {
                bool isBreak=false;
                for(const auto& seg:segments)
                    if(seg.last==i-1){isBreak=true;break;}
                const auto& a=events[ids[i-1]];
                const auto& b=events[ids[i]];
                const bool tag=a.tag!=b.tag;
                const bool header=a.headerA!=b.headerA||
                                  a.headerB!=b.headerB||a.marker!=b.marker;
                addBoundary(stats,isBreak,tag,header);
                addBoundary(total,isBreak,tag,header);
                if(isBreak)
                    std::cout<<"  BREAK events="<<ids[i-1]<<"->"<<ids[i]
                             <<" tagChange="<<(tag?"yes":"no")
                             <<" headerChange="<<(header?"yes":"no")<<"\n";
            }
        }
        printStats(name,stats);
    }
    printStats("ALL",total);
    std::cout<<"Substantial image-like payloads >=64 bytes; 128-bit greedy segment threshold; "
                "4096-byte suffix cap. No decoding or animation semantics inferred.\n";
    return unavailable?1:0;
}
