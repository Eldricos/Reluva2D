// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_candidate_palette.h"
#include "engines/stafu/rm_palette_windows.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-rm-palette-windows <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 std::vector<uint8_t> standard;
 if(!resources.read("standard.pal",standard,error,4096)||standard.size()!=384){
  std::cerr<<"standard.pal missing or invalid size: "<<error<<"\n";return 1;
 }
 for(const char* name:{"BRIDGE.IMG","DBRIDGE.IMG","color.img"}){
  std::vector<uint8_t> bytes;
  if(!resources.read(name,bytes,error,1024*1024)){
   std::cout<<"PALETTE "<<name<<" unavailable="<<error<<"\n";continue;
  }
  const auto palette=reluva::stafu::extractRmGifPalette(bytes.data(),bytes.size());
  if(!palette.valid){std::cout<<"PALETTE "<<name<<" invalid="<<palette.error<<"\n";continue;}
  auto windows=reluva::stafu::rmPaletteWindows(standard.data(),standard.size(),
                                               palette.rgb.data(),palette.entries*3);
  std::sort(windows.begin(),windows.end(),[](const auto& a,const auto& b){
   return a.exact!=b.exact?a.exact>b.exact:a.offset<b.offset;
  });
  std::cout<<"PALETTE "<<name<<" entries="<<palette.entries<<" windows="<<windows.size()<<"\n";
  for(size_t i=0;i<std::min<size_t>(10,windows.size());++i)
   std::cout<<"WINDOW offset="<<windows[i].offset<<" exact="<<windows[i].exact<<"/128\n";
 }
 std::cout<<"NOTE: A matching window is only numerical evidence, not proof of RM palette usage.\n";
 return 0;
}
