// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_seam_probe.h"
#include <iostream>
#include <string>
#include <vector>
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-rm-seam <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 for(const char* name:{"BRIDGE.RM","VIEWSCR.RM"}){
  std::vector<uint8_t> data;
  if(!resources.read(name,data,error,2*1024*1024)){std::cerr<<name<<": "<<error<<"\n";return 1;}
  std::cout<<"RESOURCE "<<name<<" bytes="<<data.size()<<"\n";
  for(size_t seam:{size_t(256),size_t(383),size_t(384),size_t(385)}){
   const auto e=reluva::stafu::rmEdgeEvidence(data.data(),data.size(),640,480,seam);
   std::cout<<"SEAM "<<seam<<" pairs="<<e.pairs<<" sameRow="<<e.sameRow
    <<" previousRowLeft="<<e.priorRow<<" nextRowLeft="<<e.nextRow<<"\n";
  }
  const auto a=reluva::stafu::rmSeamEvidence(data.data(),data.size());
  std::cout<<"ROTATE_RIGHT_256 rawSplit=384 outputSplit=0 rows="<<a.rows
   <<" sameRow="<<a.rawEqual<<" priorRowLeft="<<a.alignedPreviousRowEqual
   <<" nextRowLeft="<<a.alignedNextRowEqual<<"\n";
 }
 std::cout<<"NOTE: Counts measure identical indices, not visual edge quality or color correctness.\n";
 return 0;
}
