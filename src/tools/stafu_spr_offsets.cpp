// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include "engines/stafu/spr_offset_probe.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
namespace {
constexpr const char* names[]={"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR","brdgdoor.spr","brdgtitl.spr","viewscr.spr"};
uint32_t read32(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-spr-offsets <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager manager;std::string error;
 if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 size_t count=0,absBound=0,recBound=0,tsilBound=0,absInside=0,recInside=0,tsilInside=0,missing=0;
 for(const char* name:names){
  std::vector<uint8_t> bytes;
  if(!manager.read(name,bytes,error)){std::cerr<<name<<": "<<error<<"\n";++missing;continue;}
  reluva::stafu::SprSequenceIndex index;
  if(!index.build(bytes,error)){std::cerr<<name<<": "<<error<<"\n";++missing;continue;}
  for(size_t ri=0;ri<index.records().size();++ri){
   const auto& record=index.records()[ri];
   bool prior=false;uint32_t priorValue=0;size_t priorEvent=0;
   for(size_t ei=0;ei<record.events.size();++ei){
    const auto& event=record.events[ei];
    reluva::stafu::SprImagePayloadAccessor::View v;
    if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,event,v)||v.marker!=0x30000u||v.size!=4)continue;
    const uint32_t value=read32(v.data);
    const auto p=reluva::stafu::probeSprOffset(value,bytes.size(),record);
    ++count;absBound+=p.absoluteBoundary;recBound+=p.recordBoundary;tsilBound+=p.tsilBoundary;
    absInside+=p.absoluteInside;recInside+=p.recordInside;tsilInside+=p.tsilInside;
    std::cout<<"VALUE "<<name<<" record="<<ri+1<<" event="<<ei<<" tag="<<event.tag
     <<" value="<<value<<" recordStart="<<record.start<<" fileBytes="<<bytes.size()
     <<" absBoundary="<<p.absoluteBoundary<<" recordBoundary="<<p.recordBoundary
     <<" tsilBoundary="<<p.tsilBoundary<<" absInside="<<p.absoluteInside
     <<" recordInside="<<p.recordInside<<" tsilInside="<<p.tsilInside;
    if(prior)std::cout<<" priorEvent="<<priorEvent<<" delta="<<int64_t(value)-int64_t(priorValue);
    else std::cout<<" priorEvent=none delta=none";
    std::cout<<"\n";
    prior=true;priorValue=value;priorEvent=ei;
   }
  }
 }
 std::cout<<"SUMMARY values="<<count<<" absoluteBoundary="<<absBound
 <<" recordBoundary="<<recBound<<" tsilBoundary="<<tsilBound
 <<" absoluteInside="<<absInside<<" recordInside="<<recInside
 <<" tsilInside="<<tsilInside<<" unavailable="<<missing<<"\n";
 std::cout<<"Boundary hits test known chunk/payload/image-data starts within the same record only. "
 "Inside hits are weaker and not proof of pointer semantics.\n";
 return missing?1:0;
}
