// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only byte comparison of translated SPR runs.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
namespace {
struct Case {const char* file;size_t ra,ea,rb,eb,n;};
constexpr Case cases[]={{"BRDGDATA.SPR",1,18,4,59,7},{"BRDGRIKR.SPR",1,16,4,39,9},{"BRDGPICA.SPR",1,23,6,35,11},{"BRDGPICA.SPR",1,48,6,60,16}};
uint32_t le32(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
struct Anchor {size_t event;uint32_t value;};
std::vector<Anchor> anchors(const std::vector<uint8_t>& bytes,const reluva::stafu::SprSequenceIndex::Record& r){
 std::vector<Anchor> out;
 for(size_t i=0;i<r.events.size();++i){
  reluva::stafu::SprImagePayloadAccessor::View v;
  if(reluva::stafu::SprImagePayloadAccessor::get(bytes,r.events[i],v)&&v.marker==0x30000u&&v.size==4)
   out.push_back({i,le32(v.data)});
 }
 return out;
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-spr-event-bytes <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager manager;std::string error;
 if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 size_t ok=0,invalid=0,events=0,exact=0,translatedOnly=0,other=0;
 for(const auto& c:cases){
  std::vector<uint8_t> bytes;
  reluva::stafu::SprSequenceIndex index;
  if(!manager.read(c.file,bytes,error)||!index.build(bytes,error)||c.ra>index.records().size()||c.rb>index.records().size()){
   std::cerr<<"ERROR "<<c.file<<" "<<error<<"\n";++invalid;continue;
  }
  const auto& ra=index.records()[c.ra-1];const auto& rb=index.records()[c.rb-1];
  const auto aa=anchors(bytes,ra),bb=anchors(bytes,rb);
  size_t ia=0,ib=0;
  while(ia<aa.size()&&aa[ia].event!=c.ea)++ia;
  while(ib<bb.size()&&bb[ib].event!=c.eb)++ib;
  if(ia==aa.size()||ib==bb.size()||aa.size()-ia<c.n||bb.size()-ib<c.n){
   std::cerr<<"ERROR missing anchors "<<c.file<<"\n";++invalid;continue;
  }
  const int64_t shift=int64_t(bb[ib].value)-int64_t(aa[ia].value);
  bool valid=true;
  for(size_t k=0;k<c.n;++k)valid&=int64_t(bb[ib+k].value)-int64_t(aa[ia+k].value)==shift;
  const size_t endA=aa[ia+c.n-1].event,endB=bb[ib+c.n-1].event;
  valid&=endA-c.ea==endB-c.eb;
  if(!valid){std::cerr<<"ERROR run mismatch "<<c.file<<"\n";++invalid;continue;}
  ++ok;
  std::cout<<"CASE "<<c.file<<" records="<<c.ra<<","<<c.rb<<" firstEvents="<<c.ea<<","<<c.eb<<" values="<<c.n<<" shift="<<shift<<"\n";
  for(size_t k=0;k<=endA-c.ea;++k){
   const auto& a=ra.events[c.ea+k];const auto& b=rb.events[c.eb+k];
   const size_t count=std::min(size_t(a.chunkSize),size_t(b.chunkSize));
   size_t changed=0,nonValue=0;
   const bool value=a.hasImageHeader&&b.hasImageHeader&&a.marker==0x30000u&&b.marker==0x30000u&&a.imageDataSize==4&&b.imageDataSize==4;
   const size_t offset=value?a.imageDataOffset-a.chunkOffset:0;
   for(size_t p=0;p<count;++p){
    if(bytes[a.chunkOffset+p]==bytes[b.chunkOffset+p])continue;
    ++changed;
    if(!(value&&p>=offset&&p<offset+4))++nonValue;
   }
   const bool sameSize=a.chunkSize==b.chunkSize;
   ++events;
   exact+=sameSize&&changed==0;
   translatedOnly+=sameSize&&changed!=0&&nonValue==0;
   other+=!sameSize||nonValue!=0;
   std::cout<<" EVENT "<<k<<" tags="<<a.tag<<","<<b.tag<<" sizes="<<a.chunkSize<<","<<b.chunkSize
    <<" differingBytes="<<changed<<" nonValueDifferences="<<nonValue<<" valueField="<<value<<"\n";
  }
 }
 std::cout<<"SUMMARY casesOk="<<ok<<" invalid="<<invalid<<" events="<<events<<" exact="<<exact
  <<" translatedOnly="<<translatedOnly<<" other="<<other<<"\n";
 return invalid?1:0;
}
