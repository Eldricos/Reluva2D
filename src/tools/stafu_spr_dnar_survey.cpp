// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 47: read-only record-wide DNAR payload survey.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
namespace {
constexpr const char* names[]={"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR","brdgdoor.spr","brdgtitl.spr","viewscr.spr"};
uint32_t le32(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-spr-dnar-survey <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager manager;std::string error;
 if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 size_t unavailable=0,records=0,dnar=0,eight=0,other=0,afterImage=0,beforeImage=0;
 std::map<std::pair<uint32_t,uint32_t>,size_t> pairs;
 std::map<uint32_t,size_t> first,second;
 std::map<std::string,size_t> preceding,following;
 for(const char* name:names){
  std::vector<uint8_t> bytes;
  if(!manager.read(name,bytes,error)){std::cerr<<"ERROR "<<name<<" "<<error<<"\n";++unavailable;continue;}
  reluva::stafu::SprSequenceIndex index;
  if(!index.build(bytes,error)){std::cerr<<"ERROR "<<name<<" "<<error<<"\n";++unavailable;continue;}
  for(size_t ri=0;ri<index.records().size();++ri){
   ++records;const auto& record=index.records()[ri];
   for(size_t ei=0;ei<record.events.size();++ei){
    const auto& e=record.events[ei];if(e.tag!="DNAR")continue;
    ++dnar;
    if(e.payloadOffset>bytes.size()||e.payloadSize>bytes.size()-e.payloadOffset){
     std::cerr<<"ERROR invalid DNAR bounds "<<name<<" record="<<ri+1<<" event="<<ei<<"\n";return 1;
    }
    const std::string prev=ei?record.events[ei-1].tag:"NONE";
    const std::string next=ei+1<record.events.size()?record.events[ei+1].tag:"NONE";
    ++preceding[prev];++following[next];
    const bool prevImage=prev=="PMOC"||prev=="MOCS";
    const bool nextImage=next=="PMOC"||next=="MOCS";
    afterImage+=prevImage;beforeImage+=nextImage;
    std::cout<<"DNAR "<<name<<" record="<<ri+1<<" event="<<ei<<" payloadBytes="<<e.payloadSize
     <<" preceding="<<prev<<" following="<<next;
    if(e.payloadSize==8){
     ++eight;
     const uint32_t a=le32(bytes.data()+e.payloadOffset);
     const uint32_t b=le32(bytes.data()+e.payloadOffset+4);
     ++pairs[{a,b}];++first[a];++second[b];
     std::cout<<" word0="<<a<<" word1="<<b;
    }else ++other;
    std::cout<<"\n";
   }
  }
 }
 for(const auto& entry:pairs)
  std::cout<<"PAIR word0="<<entry.first.first<<" word1="<<entry.first.second<<" count="<<entry.second<<"\n";
 for(const auto& entry:first)
  std::cout<<"WORD0 value="<<entry.first<<" count="<<entry.second<<"\n";
 for(const auto& entry:second)
  std::cout<<"WORD1 value="<<entry.first<<" count="<<entry.second<<"\n";
 for(const auto& entry:preceding)
  std::cout<<"PRECEDING tag="<<entry.first<<" count="<<entry.second<<"\n";
 for(const auto& entry:following)
  std::cout<<"FOLLOWING tag="<<entry.first<<" count="<<entry.second<<"\n";
 std::cout<<"SUMMARY records="<<records<<" dnar="<<dnar<<" eightByte="<<eight
  <<" otherSize="<<other<<" distinctPairs="<<pairs.size()
  <<" distinctWord0="<<first.size()<<" distinctWord1="<<second.size()
  <<" immediatelyAfterImage="<<afterImage<<" immediatelyBeforeImage="<<beforeImage
  <<" unavailable="<<unavailable<<"\n";
 std::cout<<"All word values are candidate little-endian interpretations; no DNAR semantics inferred.\n";
 return unavailable?1:0;
}
