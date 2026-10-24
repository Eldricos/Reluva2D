// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 50: read-only arithmetic and candidate packed-field survey.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_dnar_packed.h"
#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>
namespace {
constexpr const char* names[]={"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR","brdgdoor.spr","brdgtitl.spr","viewscr.spr"};
uint32_t le32(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
struct Distribution {
 size_t count=0;
 std::array<size_t,9> divisible{};
 std::array<size_t,16> bitSet{};
 std::map<uint32_t,size_t> lowNibble,lowByte;
 void add(uint32_t value){
  ++count;
  for(unsigned i=0;i<divisible.size();++i)divisible[i]+=(value%(uint32_t(1)<<i)==0);
  for(unsigned i=0;i<bitSet.size();++i)bitSet[i]+=((value>>i)&1u)!=0;
  ++lowNibble[value&15u];++lowByte[value&255u];
 }
 void print(const char* label)const{
  std::cout<<"DIST "<<label<<" count="<<count;
  for(unsigned i=0;i<divisible.size();++i)std::cout<<" div"<<(uint32_t(1)<<i)<<"="<<divisible[i];
  for(unsigned i=0;i<bitSet.size();++i)std::cout<<" bit"<<i<<"="<<bitSet[i];
  std::cout<<" distinctLow4="<<lowNibble.size()<<" distinctLow8="<<lowByte.size()<<"\n";
  for(const auto& p:lowNibble)std::cout<<"LOW4 "<<label<<" value="<<p.first<<" count="<<p.second<<"\n";
 }
};
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-spr-dnar-packed <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager manager;std::string error;
 if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 Distribution dx,dy,dh;
 size_t unavailable=0,dnar=0,valid=0,invalid=0,halfGreaterY=0,halfEqualY=0,halfLessY=0;
 size_t sameLow4=0,sameLow8=0,sameHigh4=0;
 for(const char* name:names){
  std::vector<uint8_t> bytes;
  if(!manager.read(name,bytes,error)){std::cerr<<"ERROR "<<name<<" "<<error<<"\n";++unavailable;continue;}
  reluva::stafu::SprSequenceIndex index;
  if(!index.build(bytes,error)){std::cerr<<"ERROR "<<name<<" "<<error<<"\n";++unavailable;continue;}
  for(size_t ri=0;ri<index.records().size();++ri){
   const auto& events=index.records()[ri].events;
   for(size_t ei=0;ei<events.size();++ei){
    const auto& e=events[ei];if(e.tag!="DNAR")continue;++dnar;
    if(e.payloadSize!=8||e.payloadOffset>bytes.size()||e.payloadSize>bytes.size()-e.payloadOffset){
     std::cerr<<"Invalid DNAR bounds "<<name<<"\n";return 1;
    }
    const auto p=reluva::stafu::inspectSprDnarPacked(le32(bytes.data()+e.payloadOffset),le32(bytes.data()+e.payloadOffset+4));
    if(!p.valid){++invalid;continue;}
    ++valid;dx.add(p.x);dy.add(p.y);dh.add(p.halfX);
    halfGreaterY+=p.halfX>p.y;halfEqualY+=p.halfX==p.y;halfLessY+=p.halfX<p.y;
    sameLow4+=(p.halfX&15u)==(p.y&15u);
    sameLow8+=(p.halfX&255u)==(p.y&255u);
    sameHigh4+=(p.halfX>>4)==(p.y>>4);
    std::cout<<"DNAR "<<name<<" record="<<ri+1<<" event="<<ei<<" x="<<p.x<<" y="<<p.y
     <<" halfX="<<p.halfX<<" halfXMinusY="<<int64_t(p.halfX)-int64_t(p.y)
     <<" low4="<<p.lowX<<","<<p.lowY<<","<<p.lowHalfX
     <<" high4="<<p.highX<<","<<p.highY<<","<<p.highHalfX
     <<" low8="<<(p.x&255u)<<","<<(p.y&255u)<<","<<(p.halfX&255u)<<"\n";
   }
  }
 }
 dx.print("x");dy.print("y");dh.print("halfX");
 std::cout<<"SUMMARY dnar="<<dnar<<" valid="<<valid<<" invalid="<<invalid
  <<" halfGreaterY="<<halfGreaterY<<" halfEqualY="<<halfEqualY<<" halfLessY="<<halfLessY
  <<" halfSameLow4="<<sameLow4<<" halfSameLow8="<<sameLow8
  <<" halfSameHigh4="<<sameHigh4<<" unavailable="<<unavailable<<"\n";
 std::cout<<"All bit splits are exploratory arithmetic views; no packed format inferred.\n";
 return unavailable?1:0;
}
