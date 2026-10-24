// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 44: compare event structure surrounding translated four-byte sequences.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
namespace {
uint32_t le32(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
struct Entry {uint32_t value;size_t event;};
struct Seq {std::string name;size_t record;std::vector<Entry> values;std::vector<reluva::stafu::SprSequenceIndex::Event> events;};
bool metadataEqual(const reluva::stafu::SprSequenceIndex::Event& a,const reluva::stafu::SprSequenceIndex::Event& b){
 return a.tag==b.tag&&a.hasImageHeader==b.hasImageHeader&&
 (!a.hasImageHeader||(a.headerA==b.headerA&&a.headerB==b.headerB&&a.marker==b.marker))&&
 a.hasRawControlValue==b.hasRawControlValue&&
 (!a.hasRawControlValue||a.rawControlValue==b.rawControlValue);
}
bool gapEqual(const Seq& a,size_t ia,const Seq& b,size_t ib){
 const size_t sa=a.values[ia].event,ea=a.values[ia+1].event;
 const size_t sb=b.values[ib].event,eb=b.values[ib+1].event;
 if(ea-sa!=eb-sb)return false;
 for(size_t k=1;k<ea-sa;++k)
  if(!metadataEqual(a.events[sa+k],b.events[sb+k]))return false;
 return true;
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-spr-event-structure <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager manager;std::string error;
 if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 constexpr const char* names[]={"BRDGDATA.SPR","BRDGRIKR.SPR","BRDGPICA.SPR"};
 std::vector<Seq> sequences;size_t unavailable=0;
 for(const char* name:names){
  std::vector<uint8_t> bytes;
  if(!manager.read(name,bytes,error)){std::cerr<<name<<": "<<error<<"\n";++unavailable;continue;}
  reluva::stafu::SprSequenceIndex index;
  if(!index.build(bytes,error)){std::cerr<<name<<": "<<error<<"\n";++unavailable;continue;}
  for(size_t ri=0;ri<index.records().size();++ri){
   const auto& record=index.records()[ri];
   Seq seq;seq.name=name;seq.record=ri+1;seq.events=record.events;
   for(size_t ei=0;ei<record.events.size();++ei){
    reluva::stafu::SprImagePayloadAccessor::View v;
    if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,record.events[ei],v)||v.marker!=0x30000u||v.size!=4)continue;
    seq.values.push_back({le32(v.data),ei});
   }
   if(seq.values.size()>=3)sequences.push_back(std::move(seq));
  }
 }
 size_t runs=0,structuralRuns=0,matchedEndpoints=0,matchedGaps=0,totalEndpoints=0,totalGaps=0;
 for(size_t a=0;a<sequences.size();++a)for(size_t b=a+1;b<sequences.size();++b){
  const auto& x=sequences[a];const auto& y=sequences[b];
  for(size_t i=0;i+2<x.values.size();++i)for(size_t j=0;j+2<y.values.size();++j){
   const int64_t shift=int64_t(y.values[j].value)-int64_t(x.values[i].value);
   if(i&&j&&int64_t(y.values[j-1].value)-int64_t(x.values[i-1].value)==shift)continue;
   size_t len=0;
   while(i+len<x.values.size()&&j+len<y.values.size()&&
         int64_t(y.values[j+len].value)-int64_t(x.values[i+len].value)==shift)++len;
   if(len<3)continue;
   ++runs;size_t endpointMatches=0,gapMatches=0;
   for(size_t k=0;k<len;++k)
    endpointMatches+=metadataEqual(x.events[x.values[i+k].event],y.events[y.values[j+k].event]);
   for(size_t k=0;k+1<len;++k)gapMatches+=gapEqual(x,i+k,y,j+k);
   totalEndpoints+=len;matchedEndpoints+=endpointMatches;
   totalGaps+=len-1;matchedGaps+=gapMatches;
   const bool full=endpointMatches==len&&gapMatches==len-1;
   structuralRuns+=full;
   std::cout<<"RUN "<<x.name<<" record="<<x.record<<" firstEvent="<<x.values[i].event
    <<" vs "<<y.name<<" record="<<y.record<<" firstEvent="<<y.values[j].event
    <<" values="<<len<<" shift="<<shift<<" endpointMeta="<<endpointMatches<<"/"<<len
    <<" interveningMeta="<<gapMatches<<"/"<<len-1<<" fullStructuralMatch="<<full<<"\n";
  }
 }
 std::cout<<"SUMMARY sequences="<<sequences.size()<<" runs="<<runs<<" fullStructuralRuns="<<structuralRuns
  <<" endpointMetaMatches="<<matchedEndpoints<<"/"<<totalEndpoints
  <<" interveningMetaMatches="<<matchedGaps<<"/"<<totalGaps
  <<" unavailable="<<unavailable<<"\n";
 std::cout<<"Metadata equality compares tags, candidate image headers and EMIT raw values; it does not compare opaque payload contents. Overlapping runs are not independent evidence.\n";
 return unavailable?1:0;
}
