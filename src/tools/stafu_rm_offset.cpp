// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_offset_probe.h"
#include <iostream>
#include <string>
#include <vector>
namespace {
void report(const char* name,const std::vector<uint8_t>& data){
 std::cout<<"RESOURCE "<<name<<" bytes="<<data.size()<<"\n";
 for(size_t offset:{size_t(0),size_t(128),size_t(256),size_t(384)}){
  const auto s=reluva::stafu::rmOffsetScore(data.data(),data.size(),offset);
  std::cout<<"OFFSET "<<offset<<" completeRows="<<s.rows<<" verticalEqual="<<s.equalVertical
   <<"/"<<s.validPairs<<" horizontalEqual="<<s.equalHorizontal<<"\n";
  for(size_t seam:{size_t(256),size_t(384)}){
   const auto v=reluva::stafu::rmSeamScore(data.data(),data.size(),offset,640,seam);
   std::cout<<" SEAM "<<seam<<" across="<<v.equalAcross<<"/"<<v.rows
    <<" nextRowLeft="<<v.equalOffsetOne<<"/"<<(v.rows?v.rows-1:0)<<"\n";
  }
 }
 std::cout<<"NOTE: Equal-byte continuity is a weak heuristic for large flat image regions.\n";
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-rm-offset <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 for(const char* name:{"BRIDGE.RM","VIEWSCR.RM"}){
  std::vector<uint8_t> data;
  if(!resources.read(name,data,error,2*1024*1024)){std::cerr<<name<<": "<<error<<"\n";return 1;}
  report(name,data);
 }
 return 0;
}
