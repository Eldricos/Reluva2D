// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only comparison of record-local four-byte value sequences.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
namespace {
constexpr const char* names[]={"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR","brdgdoor.spr","brdgtitl.spr","viewscr.spr"};
uint32_t le32(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
struct Entry {uint32_t value=0;size_t event=0;std::string tag;};
struct Sequence {std::string resource;size_t record=0;std::vector<Entry> values;};
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-spr-translations <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager manager;std::string error;
 if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 std::vector<Sequence> sequences;
 size_t unavailable=0,deltaPairs=0,chunkSizeHits=0,chunkSizePlusHeaderHits=0,comparisons=0,matches=0;
 for(const char* name:names){
  std::vector<uint8_t> bytes;
  if(!manager.read(name,bytes,error)){std::cerr<<name<<": "<<error<<"\n";++unavailable;continue;}
  reluva::stafu::SprSequenceIndex index;
  if(!index.build(bytes,error)){std::cerr<<name<<": "<<error<<"\n";++unavailable;continue;}
  for(size_t ri=0;ri<index.records().size();++ri){
   const auto& record=index.records()[ri];
   Sequence seq;seq.resource=name;seq.record=ri+1;
   for(size_t ei=0;ei<record.events.size();++ei){
    const auto& event=record.events[ei];
    reluva::stafu::SprImagePayloadAccessor::View view;
    if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,event,view)||view.marker!=0x30000u||view.size!=4)continue;
    seq.values.push_back({le32(view.data),ei,event.tag});
   }
   if(seq.values.size()>=2){
    for(size_t i=1;i<seq.values.size();++i){
     const int64_t d=int64_t(seq.values[i].value)-int64_t(seq.values[i-1].value);
     const uint64_t magnitude=d<0?uint64_t(-d):uint64_t(d);
     bool exact=false,plusHeader=false;
     for(size_t j=seq.values[i-1].event+1;j<seq.values[i].event;++j){
      const auto& e=record.events[j];
      if(magnitude==e.chunkSize||magnitude==e.payloadSize)exact=true;
      if(magnitude==uint64_t(e.chunkSize)+8||magnitude==uint64_t(e.payloadSize)+12)plusHeader=true;
     }
     ++deltaPairs;chunkSizeHits+=exact;chunkSizePlusHeaderHits+=plusHeader;
     std::cout<<"DELTA "<<name<<" record="<<ri+1<<" events="<<seq.values[i-1].event<<"->"<<seq.values[i].event
      <<" delta="<<d<<" magnitudeMatchesInterveningChunk="<<exact
      <<" magnitudeMatchesInterveningChunkPlusHeader="<<plusHeader<<"\n";
    }
   }
   if(!seq.values.empty())sequences.push_back(std::move(seq));
  }
 }
 // Report maximal contiguous runs of equal first differences (at least two deltas).
 // Distinct records only; translation is valueB-valueA at the start of the run.
 for(size_t a=0;a<sequences.size();++a)for(size_t b=a+1;b<sequences.size();++b){
  const auto& x=sequences[a];const auto& y=sequences[b];
  if(x.values.size()<3||y.values.size()<3)continue;
  ++comparisons;
  for(size_t i=0;i+2<x.values.size();++i)for(size_t j=0;j+2<y.values.size();++j){
   const int64_t shift=int64_t(y.values[j].value)-int64_t(x.values[i].value);
   if(i&&j&&int64_t(y.values[j-1].value)-int64_t(x.values[i-1].value)==shift)continue;
   size_t len=0;
   while(i+len<x.values.size()&&j+len<y.values.size()&&
         int64_t(y.values[j+len].value)-int64_t(x.values[i+len].value)==shift)++len;
   if(len<3)continue;
   ++matches;
   std::cout<<"TRANSLATION "<<x.resource<<" record="<<x.record<<" firstEvent="<<x.values[i].event
    <<" vs "<<y.resource<<" record="<<y.record<<" firstEvent="<<y.values[j].event
    <<" values="<<len<<" deltaCount="<<len-1<<" shift="<<shift
    <<" firstA="<<x.values[i].value<<" firstB="<<y.values[j].value<<"\n";
  }
 }
 std::cout<<"SUMMARY sequences="<<sequences.size()<<" deltaPairs="<<deltaPairs
  <<" chunkSizeHits="<<chunkSizeHits<<" chunkSizePlusHeaderHits="<<chunkSizePlusHeaderHits
  <<" recordComparisons="<<comparisons<<" translatedRuns="<<matches
  <<" unavailable="<<unavailable<<"\n";
 std::cout<<"Runs match differences only; neither translation nor chunk-size agreement proves offset semantics.\n";
 return unavailable?1:0;
}
