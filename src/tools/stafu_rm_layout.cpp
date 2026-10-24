// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 58: read-only candidate raw raster layout investigation.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_layout_probe.h"
#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace {
void score(const char* name,const std::vector<uint8_t>& bytes){
 std::cout<<"RESOURCE "<<name<<" bytes="<<bytes.size()<<"\n";
 for(const size_t stride:{size_t(160),size_t(320),size_t(512),size_t(640),size_t(768),size_t(800),size_t(1024)}){
  for(const size_t offset:{size_t(0),size_t(384)}){
   const auto r=reluva::stafu::scoreRmRows(bytes.data(),bytes.size(),stride,offset);
   if(r.rows<2)continue;
   std::cout<<"ROW_CANDIDATE stride="<<stride<<" offset="<<offset
    <<" completeRows="<<r.rows<<" adjacentEqual="<<r.adjacentEqual
    <<"/"<<r.adjacentCompared<<" verticalEqual="<<r.verticalEqual
    <<"/"<<r.verticalCompared<<"\n";
  }
 }
 const auto tail=reluva::stafu::surveyRmTail(bytes.data(),bytes.size(),384);
 const auto head=reluva::stafu::compareRmRegions(bytes.data(),bytes.size(),
                                               bytes.data(),bytes.size(),0,384);
 (void)head;
 std::cout<<"TAIL lastBytes="<<tail.bytes<<" zeroBytes="<<tail.zero
  <<" distinctByteValues="<<tail.distinct<<"\n";
 std::cout<<"CANDIDATE_640x480 sizeDelta="
  <<(static_cast<long long>(bytes.size())-307200LL)
  <<" (size arithmetic only)\n";
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-rm-layout <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 std::vector<uint8_t> a,b;
 if(!resources.read("BRIDGE.RM",a,error)){std::cerr<<"BRIDGE.RM: "<<error<<"\n";return 1;}
 if(!resources.read("VIEWSCR.RM",b,error)){std::cerr<<"VIEWSCR.RM: "<<error<<"\n";return 1;}
 score("BRIDGE.RM",a);score("VIEWSCR.RM",b);
 const size_t common=std::min(a.size(),b.size());
 for(const size_t offset:{size_t(0),size_t(384),size_t(76800),size_t(153600),size_t(230400),size_t(307200)}){
  const auto r=reluva::stafu::compareRmRegions(a.data(),a.size(),b.data(),b.size(),offset,384);
  std::cout<<"CROSS_REGION offset="<<offset<<" compared="<<r.compared
   <<" equal="<<r.equal<<" bothZero="<<r.bothZero
   <<" bridgeZero="<<r.firstZero<<" viewscrZero="<<r.secondZero<<"\n";
 }
 const auto full=reluva::stafu::compareRmRegions(a.data(),a.size(),b.data(),b.size(),0,common);
 std::cout<<"CROSS_TOTAL compared="<<full.compared<<" equal="<<full.equal
  <<" bothZero="<<full.bothZero<<" bridgeZero="<<full.firstZero
  <<" viewscrZero="<<full.secondZero<<"\n";
 std::cout<<"NOTE Row candidates are byte-level correlations; no width, palette, header, or image format is confirmed.\n";
 return 0;
}
