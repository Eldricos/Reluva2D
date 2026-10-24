// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 48: observational DNAR and neighboring candidate image header relationships.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>
namespace {
constexpr const char* names[]={"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR","brdgdoor.spr","brdgtitl.spr","viewscr.spr"};
uint32_t le32(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
bool image(const reluva::stafu::SprSequenceIndex::Event& e){return e.hasImageHeader;}
struct Counts {size_t n=0,div100=0,div1000=0,withinA=0,withinB=0,withinProduct=0,equalA=0,equalB=0;};
void update(Counts& c,uint32_t value,const reluva::stafu::SprSequenceIndex::Event& e){
 ++c.n;c.div100+=(value>=466&&(value-466)%100==0);c.div1000+=(value>=466&&(value-466)%1000==0);
 c.withinA+=value<=e.headerA;c.withinB+=value<=e.headerB;
 c.withinProduct+=uint64_t(value)<=uint64_t(e.headerA)*e.headerB;
 c.equalA+=value==e.headerA;c.equalB+=value==e.headerB;
}
void print(const char* label,const Counts& c){
 std::cout<<"STATS "<<label<<" observations="<<c.n<<" minus466Mod100="<<c.div100
  <<" minus466Mod1000="<<c.div1000<<" withinA="<<c.withinA<<" withinB="<<c.withinB
  <<" withinProduct="<<c.withinProduct<<" equalsA="<<c.equalA<<" equalsB="<<c.equalB<<"\n";
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-spr-dnar-relations <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager manager;std::string error;
 if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 size_t unavailable=0,records=0,dnar=0,eight=0,neighbors=0,sameNeighborHeaders=0;
 size_t bothMinus466Mod100=0,bothMinus666Mod1000=0;
 Counts firstPrev,secondPrev,firstNext,secondNext;
 std::map<std::pair<uint32_t,uint32_t>,size_t> headerPairs;
 for(const char* name:names){
  std::vector<uint8_t> bytes;
  if(!manager.read(name,bytes,error)){std::cerr<<"ERROR "<<name<<" "<<error<<"\n";++unavailable;continue;}
  reluva::stafu::SprSequenceIndex index;
  if(!index.build(bytes,error)){std::cerr<<"ERROR "<<name<<" "<<error<<"\n";++unavailable;continue;}
  for(size_t ri=0;ri<index.records().size();++ri){
   ++records;const auto& es=index.records()[ri].events;
   for(size_t i=0;i<es.size();++i){
    const auto& e=es[i];if(e.tag!="DNAR")continue;++dnar;
    if(e.payloadSize!=8)continue;
    if(e.payloadOffset>bytes.size()||e.payloadSize>bytes.size()-e.payloadOffset){std::cerr<<"Invalid DNAR bounds\n";return 1;}
    ++eight;const uint32_t x=le32(bytes.data()+e.payloadOffset),y=le32(bytes.data()+e.payloadOffset+4);
    bothMinus466Mod100+=(x>=466&&y>=466&&(x-466)%100==0&&(y-466)%100==0);
    bothMinus666Mod1000+=(x>=666&&y>=666&&(x-666)%1000==0&&(y-666)%1000==0);
    const auto* prev=i?&es[i-1]:nullptr;
    const auto* next=i+1<es.size()?&es[i+1]:nullptr;
    const bool hp=prev&&image(*prev),hn=next&&image(*next);
    if(hp){update(firstPrev,x,*prev);update(secondPrev,y,*prev);}
    if(hn){update(firstNext,x,*next);update(secondNext,y,*next);}
    if(hp&&hn){++neighbors;sameNeighborHeaders+=prev->headerA==next->headerA&&prev->headerB==next->headerB;}
    if(hp)++headerPairs[{prev->headerA,prev->headerB}];
    std::cout<<"DNAR "<<name<<" record="<<ri+1<<" event="<<i<<" values="<<x<<","<<y
     <<" diff="<<int64_t(x)-int64_t(y)<<" ratio1000="<<(y?uint64_t(x)*1000/y:0)
     <<" prev="<<(hp?prev->tag:"NONE")<<" prevAB="<<(hp?std::to_string(prev->headerA)+","+std::to_string(prev->headerB):"-")
     <<" next="<<(hn?next->tag:"NONE")<<" nextAB="<<(hn?std::to_string(next->headerA)+","+std::to_string(next->headerB):"-")
     <<" scaled1000="<<(x>=466?(x-466)/1000:0)<<","<<(y>=466?(y-466)/1000:0)<<"\n";
   }
  }
 }
 print("word0Previous",firstPrev);print("word1Previous",secondPrev);
 print("word0Next",firstNext);print("word1Next",secondNext);
 for(const auto& item:headerPairs)std::cout<<"PREV_HEADER a="<<item.first.first<<" b="<<item.first.second<<" count="<<item.second<<"\n";
 std::cout<<"SUMMARY records="<<records<<" dnar="<<dnar<<" eightByte="<<eight
  <<" bothMinus466Mod100="<<bothMinus466Mod100<<" bothMinus666Mod1000="<<bothMinus666Mod1000
  <<" bothImageNeighbors="<<neighbors<<" equalNeighborHeaders="<<sameNeighborHeaders
  <<" distinctPreviousHeaders="<<headerPairs.size()<<" unavailable="<<unavailable<<"\n";
 std::cout<<"All arithmetic is exploratory; scaled1000 truncates and does not establish units, coordinates or time.\n";
 return unavailable?1:0;
}
