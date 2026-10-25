// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_standard_palette.h"
#include <iostream>
#include <string>
#include <vector>
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-rm-palette-check <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 std::vector<uint8_t> palette;
 if(!resources.read("STANDARD.PAL",palette,error,4096)){
  error.clear();
  if(!resources.read("standard.pal",palette,error,4096)){
   std::cerr<<"standard.pal: "<<error<<"\n";return 1;
  }
 }
 std::cout<<"standard.pal bytes="<<palette.size()<<" expected=384\n";
 if(palette.size()!=384)return 1;
 for(const char* name:{"BRIDGE.RM","VIEWSCR.RM"}){
  std::vector<uint8_t> bytes;
  if(!resources.read(name,bytes,error)){std::cerr<<name<<": "<<error<<"\n";return 1;}
  const auto stats=reluva::stafu::inspectRmIndices(bytes.data(),bytes.size());
  std::cout<<name<<" pixels="<<stats.pixels<<" distinct="<<stats.distinct
   <<" min="<<stats.minimum<<" max="<<stats.maximum
   <<" indices128to255="<<stats.unsupported<<" supported="<<(stats.pixels-stats.unsupported)<<"\n";
  if(!stats.pixels)return 1;
 }
 std::cout<<"NOTE: Index compatibility alone does not establish palette ownership.\n";
 return 0;
}
