// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 49: observational normalized DNAR values versus neighboring image headers.
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
struct Counts {size_t n=0,withinA=0,withinB=0,within2A=0,within2B=0,eqA=0,eqB=0,eq2A=0,eq2B=0,eqHalfA=0,eqHalfB=0;};
void observe(Counts& c,uint32_t v,const reluva::stafu::SprSequenceIndex::Event& e){
 ++c.n;c.withinA+=v<=e.headerA;c.withinB+=v<=e.headerB;
 c.within2A+=uint64_t(v)<=uint64_t(e.headerA)*2;c.within2B+=uint64_t(v)<=uint64_t(e.headerB)*2;
 c.eqA+=v==e.headerA;c.eqB+=v==e.headerB;
 c.eq2A+=uint64_t(v)==uint64_t(e.headerA)*2;c.eq2B+=uint64_t(v)==uint64_t(e.headerB)*2;
 c.eqHalfA+=uint64_t(v)*2==e.headerA;c.eqHalfB+=uint64_t(v)*2==e.headerB;
}
void print(const char* label,const Counts& c){
 std::cout<<"STATS "<<label<<" n="<<c.n<<" withinA="<<c.withinA<<" withinB="<<c.withinB
  <<" within2A="<<c.within2A<<" within2B="<<c.within2B<<" eqA="<<c.eqA<<" eqB="<<c.eqB
  <<" eq2A="<<c.eq2A<<" eq2B="<<c.eq2B<<" eqHalfA="<<c.eqHalfA<<" eqHalfB="<<c.eqHalfB<<"\n";
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-spr-dnar-normalized <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager manager;std::string error;
 if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 size_t unavailable=0,dnar=0,normalized=0,invalidPattern=0,xGreaterY=0,xEqualY=0,xLessY=0;
 size_t exactDouble=0,nearDouble=0,evenX=0,evenY=0;
 Counts xPrev,yPrev,xNext,yNext;
 std::map<int64_t,size_t> differences;
 for(const char* name:names){
  std::vector<uint8_t> bytes;
  if(!manager.read(name,bytes,error)){std::cerr<<"ERROR "<<name<<" "<<error<<"\n";++unavailable;continue;}
  reluva::stafu::SprSequenceIndex index;
  if(!index.build(bytes,error)){std::cerr<<"ERROR "<<name<<" "<<error<<"\n";++unavailable;continue;}
  for(size_t ri=0;ri<index.records().size();++ri){
   const auto& es=index.records()[ri].events;
   for(size_t ei=0;ei<es.size();++ei){
    const auto& e=es[ei];if(e.tag!="DNAR")continue;++dnar;
    if(e.payloadSize!=8||e.payloadOffset>bytes.size()||e.payloadSize>bytes.size()-e.payloadOffset){
     std::cerr<<"Invalid DNAR payload "<<name<<"\n";return 1;
    }
    const uint32_t a=le32(bytes.data()+e.payloadOffset),b=le32(bytes.data()+e.payloadOffset+4);
    if(a<66||b<66||(a-66)%100||(b-66)%100){++invalidPattern;continue;}
    ++normalized;const uint32_t x=(a-66)/100,y=(b-66)/100;
    xGreaterY+=x>y;xEqualY+=x==y;xLessY+=x<y;
    evenX+=x%2==0;evenY+=y%2==0;
    exactDouble+=uint64_t(x)==uint64_t(y)*2;
    nearDouble+=uint64_t(x)+1==uint64_t(y)*2||uint64_t(x)==uint64_t(y)*2+1;
    ++differences[int64_t(x)-int64_t(y)];
    const auto* prev=ei?&es[ei-1]:nullptr;
    const auto* next=ei+1<es.size()?&es[ei+1]:nullptr;
    if(prev&&prev->hasImageHeader){observe(xPrev,x,*prev);observe(yPrev,y,*prev);}
    if(next&&next->hasImageHeader){observe(xNext,x,*next);observe(yNext,y,*next);}
    std::cout<<"DNAR "<<name<<" record="<<ri+1<<" event="<<ei<<" normalized="<<x<<","<<y
     <<" difference="<<int64_t(x)-int64_t(y)<<" twiceYMinusX="<<int64_t(y)*2-int64_t(x)
     <<" prevAB="<<(prev&&prev->hasImageHeader?std::to_string(prev->headerA)+","+std::to_string(prev->headerB):"-")
     <<" nextAB="<<(next&&next->hasImageHeader?std::to_string(next->headerA)+","+std::to_string(next->headerB):"-")<<"\n";
   }
  }
 }
 print("xPrevious",xPrev);print("yPrevious",yPrev);print("xNext",xNext);print("yNext",yNext);
 for(const auto& entry:differences)std::cout<<"DIFFERENCE value="<<entry.first<<" count="<<entry.second<<"\n";
 std::cout<<"SUMMARY dnar="<<dnar<<" normalized="<<normalized<<" invalidPattern="<<invalidPattern
  <<" xGreaterY="<<xGreaterY<<" xEqualY="<<xEqualY<<" xLessY="<<xLessY
  <<" exactDouble="<<exactDouble<<" nearDouble="<<nearDouble
  <<" evenX="<<evenX<<" evenY="<<evenY<<" distinctDifferences="<<differences.size()
  <<" unavailable="<<unavailable<<"\n";
 std::cout<<"Normalized numbers are mathematical transforms only; no coordinate, dimension or timing semantics asserted.\n";
 return unavailable?1:0;
}
